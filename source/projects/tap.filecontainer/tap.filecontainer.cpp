/// @file
/// @brief      tap.filecontainer — bundle many files into a single SQLite-backed container file.
/// @details    Bundles a number of individual files into a single file for distribution. The
///             implementation creates a SQLite database (via Max's built-in `sqlite` object, driven
///             through the C API) into which the files are inserted as BLOBs. When a previously
///             created container is opened, its files are extracted into a temp folder which is
///             temporarily added to the searchpath, so the contents become accessible in Max.
///
///             Faithful port of the original TapTools object. Jamoma is cut: the DB plumbing is the
///             raw Max C API (`object_new_typed(CLASS_NOBOX, "sqlite", ...)` + `object_method(...,
///             "execstring", ...)`), file I/O is the Max sysfile/path API, and the temp-folder
///             lifecycle is std::filesystem (the original used FSRef/Win32 shell calls).
/// @author     Timothy Place
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2008-2026 Timothy Place.

#include <chrono>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <filesystem>
#include <string>

#include "c74_min.h"

using namespace c74::min;
namespace mx = c74::max;
namespace fs = std::filesystem;

// Table definitions — reproduced verbatim from the original.
static const char* TABLEDEF_FILES = "CREATE TABLE files ( "
                                    "file_id     INTEGER PRIMARY KEY NOT NULL, "
                                    "filename    VARCHAR(256), "
                                    "moddate     DATETIME, "
                                    "content     BLOB, "
                                    "valid       INTEGER "
                                    ")";

static const char* TABLEDEF_ATTRS = "CREATE TABLE attrs ( "
                                    "attr_id     INTEGER PRIMARY KEY NOT NULL, "
                                    "file_id_ext INTEGER, "
                                    "attr_name   VARCHAR(256), "
                                    "attr_value  VARCHAR(256) "
                                    ")";

class filecontainer : public object<filecontainer> {
  public:
    MIN_DESCRIPTION{"Bundle a number of individual files into a single container file for "
                    "distribution. Backed by a SQLite database: files are stored as BLOBs and, "
                    "on open, extracted to a temp folder that is added to the searchpath."};
    MIN_TAGS{"utilities"};
    MIN_AUTHOR{"Timothy Place"};
    MIN_RELATED{"sqlite, tap.folder"};

    inlet<>  m_in{this, "(open/close/addfile/... ) container commands"};
    outlet<> m_dumpout{this, "(anything) dumpout"};

    ~filecontainer() {
        if (m_sqlite) {
            do_close();
        }
    }

    // ---- Container lifecycle ------------------------------------------------

    message<> open{this, "open",
                   "Open a file container by name. If it doesn't exist it is created; "
                   "if it does, its contents are extracted and added to the searchpath.",
                   MIN_FUNCTION{const symbol arg = args.empty() ? symbol{""} : static_cast<symbol>(args[0]);
    if (m_name == arg && m_sqlite) { // already open
        return {};
    }
    if (m_sqlite) { // a different container is open, close it first
        do_close();
    }
    do_open(arg);
    return {};
}
}
;

message<> close{this, "close", "Close the container and clean up the extracted temp files.", MIN_FUNCTION{do_close();
return {};
}
}
;

// ---- File interface -----------------------------------------------------

message<> addfile{this, "addfile", "Add a file to the container. Optional arg specifies the file.",
                  MIN_FUNCTION{if (!m_sqlite){cerr << "no container open for adding file" << endl;
return {};
}
do_addfile(args.empty() ? symbol{""} : static_cast<symbol>(args[0]));
return {};
}
}
;

message<> removefile{this, "removefile", "Remove the named file from the container.",
                     MIN_FUNCTION{if (args.empty()){return {};
}
const std::string name = static_cast<symbol>(args[0]);
char              sql[512];
snprintf(sql, sizeof(sql),
         "DELETE FROM attrs WHERE file_id_ext in (SELECT file_id FROM files WHERE filename = \"%s\" )", name.c_str());
sql_exec(sql, nullptr);
snprintf(sql, sizeof(sql), "DELETE FROM files WHERE filename = \"%s\" ", name.c_str());
sql_exec(sql, nullptr);
return {};
}
}
;

message<> getnumfiles{this, "getnumfiles", "Output the number of files stored in the container.",
                      MIN_FUNCTION{mx::t_object* result = nullptr;
sql_exec("SELECT filename FROM files", &result);
const long n =
    result ? static_cast<long>(reinterpret_cast<intptr_t>(mx::object_method(result, mx::gensym("numrecords")))) : 0;
m_dumpout.send("numfiles", static_cast<int>(n));
if (result) {
    mx::object_free(result);
}
return {};
}
}
;

message<> getfilenames{this, "getfilenames", "Output the names of the files stored in the container.",
                       MIN_FUNCTION{mx::t_object* result = nullptr;
m_dumpout.send("filenames_begin");
sql_exec("SELECT filename FROM files", &result);
char** record = nullptr;
while (result && (record = reinterpret_cast<char**>(mx::object_method(result, mx::gensym("nextrecord"))))) {
    m_dumpout.send("filename", symbol(mx::gensym(*record)));
}
m_dumpout.send("filenames_end");
if (result) {
    mx::object_free(result);
}
return {};
}
}
;

message<> getfilepath{this, "getfilepath", "Output the temp path of a named extracted file.",
                      MIN_FUNCTION{if (args.empty()){return {};
}
const symbol      arg    = static_cast<symbol>(args[0]);
const std::string name   = arg;
mx::t_object*     result = nullptr;
char              sql[512];
snprintf(sql, sizeof(sql), "SELECT file_id FROM files WHERE filename = \"%s\" ", name.c_str());
sql_exec(sql, &result);
const long n =
    result ? static_cast<long>(reinterpret_cast<intptr_t>(mx::object_method(result, mx::gensym("numrecords")))) : 0;
if (n) {
    fs::path full = fs::path(m_temp_fullpath) / name;
    m_dumpout.send("filepath", arg, symbol(mx::gensym(full.string().c_str())));
}
else {
    cerr << "no such file (" << name << ")" << endl;
}
if (result) {
    mx::object_free(result);
}
return {};
}
}
;

// ---- Raw SQL ------------------------------------------------------------

message<> sql{this, "sql", "Run a raw SQL query on the container (do not SELECT the content column).",
              MIN_FUNCTION{if (args.empty()){return {};
}
const std::string query  = static_cast<symbol>(args[0]);
mx::t_object*     result = nullptr;
sql_exec(query.c_str(), &result);
char** record = nullptr;
while (result && (record = reinterpret_cast<char**>(mx::object_method(result, mx::gensym("nextrecord"))))) {
    const long numfields =
        static_cast<long>(reinterpret_cast<intptr_t>(mx::object_method(result, mx::gensym("numfields"))));
    std::string resultstr;
    for (long i = 0; i < numfields; ++i) {
        if (i) {
            resultstr += " ";
        }
        if (record[i]) {
            resultstr += record[i];
        }
    }
    m_dumpout.send("sqlresult", symbol(mx::gensym(resultstr.c_str())));
}
if (result) {
    mx::object_free(result);
}
return {};
}
}
;

// ---- Attribute interface (per-file metadata stored in the attrs table) --

message<> addfileattr{this, "addfileattr", "Add a metadata attribute (name/value) to a named file.",
                      MIN_FUNCTION{if (args.size() != 3){cerr << "wrong num args for addfileattr" << endl;
return {};
}
const std::string filename = static_cast<symbol>(args[0]);
const std::string attrname = static_cast<symbol>(args[1]);

mx::t_object* result = nullptr;
char          sql[512];
snprintf(sql, sizeof(sql), "SELECT file_id FROM files WHERE filename = \"%s\" ", filename.c_str());
sql_exec(sql, &result);
char** record = result ? reinterpret_cast<char**>(mx::object_method(result, mx::gensym("nextrecord"))) : nullptr;
if (record) {
    const atom& v = args[2];
    if (v.a_type == mx::A_LONG) {
        snprintf(sql, sizeof(sql),
                 "INSERT INTO attrs ('file_id_ext', 'attr_name', 'attr_value') VALUES (%s, \"%s\", %ld)", *record,
                 attrname.c_str(), static_cast<long>(static_cast<int>(v)));
    }
    else if (v.a_type == mx::A_FLOAT) {
        snprintf(sql, sizeof(sql),
                 "INSERT INTO attrs ('file_id_ext', 'attr_name', 'attr_value') VALUES (%s, \"%s\", %f)", *record,
                 attrname.c_str(), static_cast<double>(v));
    }
    else {
        const std::string sv = static_cast<symbol>(v);
        snprintf(sql, sizeof(sql),
                 "INSERT INTO attrs ('file_id_ext', 'attr_name', 'attr_value') VALUES (%s, \"%s\", \"%s\")", *record,
                 attrname.c_str(), sv.c_str());
    }
    sql_exec(sql, nullptr);
}
if (result) {
    mx::object_free(result);
}
return {};
}
}
;

message<> removefileattr{this, "removefileattr", "Remove a named attribute from a named file.",
                         MIN_FUNCTION{if (args.size() != 2){cerr << "wrong num args for removefileattr" << endl;
return {};
}
const std::string filename = static_cast<symbol>(args[0]);
const std::string attrname = static_cast<symbol>(args[1]);
char              sql[512];
snprintf(sql, sizeof(sql),
         "DELETE FROM attrs WHERE attr_name = \"%s\" AND file_id_ext in "
         "(SELECT file_id FROM files WHERE filename = \"%s\" )",
         attrname.c_str(), filename.c_str());
sql_exec(sql, nullptr);
return {};
}
}
;

message<> getnumfileattrs{this, "getnumfileattrs", "Output the number of attributes for a named file.",
                          MIN_FUNCTION{if (args.empty()){return {};
}
const std::string filename = static_cast<symbol>(args[0]);
mx::t_object*     result   = nullptr;
char              sql[512];
snprintf(sql, sizeof(sql),
         "SELECT attr_id FROM attrs WHERE file_id_ext in (SELECT file_id FROM files WHERE filename = \"%s\") ",
         filename.c_str());
sql_exec(sql, &result);
const long n =
    result ? static_cast<long>(reinterpret_cast<intptr_t>(mx::object_method(result, mx::gensym("numrecords")))) : 0;
m_dumpout.send("numfileattrs", static_cast<int>(n));
if (result) {
    mx::object_free(result);
}
return {};
}
}
;

message<> getfileattrnames{this, "getfileattrnames", "Output the attribute names for a named file.",
                           MIN_FUNCTION{if (args.empty()){return {};
}
const std::string filename = static_cast<symbol>(args[0]);
mx::t_object*     result   = nullptr;
char              sql[512];
m_dumpout.send("fileattrnames_begin");
snprintf(sql, sizeof(sql),
         "SELECT attr_name FROM attrs WHERE file_id_ext in (SELECT file_id FROM files WHERE filename = \"%s\") ",
         filename.c_str());
sql_exec(sql, &result);
char** record = nullptr;
while (result && (record = reinterpret_cast<char**>(mx::object_method(result, mx::gensym("nextrecord"))))) {
    m_dumpout.send("fileattrname", symbol(mx::gensym(*record)));
}
m_dumpout.send("fileattrnames_end");
if (result) {
    mx::object_free(result);
}
return {};
}
}
;

message<> getfileattrvalue{this, "getfileattrvalue", "Output the value of a named attribute for a named file.",
                           MIN_FUNCTION{if (args.size() != 2){cerr << "wrong num args for getfileattrvalue" << endl;
return {};
}
const std::string filename = static_cast<symbol>(args[0]);
const std::string attrname = static_cast<symbol>(args[1]);
mx::t_object*     result   = nullptr;
char              sql[512];
snprintf(sql, sizeof(sql),
         "SELECT attr_value FROM attrs WHERE attr_name = \"%s\" AND "
         "file_id_ext in (SELECT file_id FROM files WHERE filename = \"%s\")",
         attrname.c_str(), filename.c_str());
sql_exec(sql, &result);
char** record = result ? reinterpret_cast<char**>(mx::object_method(result, mx::gensym("nextrecord"))) : nullptr;
if (record) {
    m_dumpout.send("fileattrvalue", symbol(mx::gensym(*record)));
}
if (result) {
    mx::object_free(result);
}
return {};
}
}
;

private:
mx::t_object* m_sqlite{nullptr}; // sqlite object instance
symbol        m_name{""};        // resolved DB file path
short         m_temp_path{0};    // Max path id of the unpacked-files temp folder
std::string   m_temp_fullpath;   // absolute path to the temp folder

// Issue a SQL statement to the sqlite object; result (if any) is returned in *result.
void sql_exec(const char* statement, mx::t_object** result) {
    if (!m_sqlite) {
        return;
    }
    mx::object_method(m_sqlite, mx::gensym("execstring"), statement, result);
}

// ---- Temp-folder management (was FSRef / Win32; now std::filesystem) -----

void make_temp_folder() {
    // A unique subfolder of the system temp dir, mirroring the original's symbol_unique() scheme.
    const std::string unique = static_cast<symbol>(mx::symbol_unique()).c_str();
    std::error_code   ec;
    fs::path          dir = fs::temp_directory_path(ec) / unique;
    fs::create_directories(dir, ec);

    m_temp_fullpath = dir.string();

    // Register the folder with Max: resolve a Max path id for it and add it to the searchpath.
    char conformed[512];
    mx::path_nameconform(const_cast<char*>(m_temp_fullpath.c_str()), conformed, mx::PATH_STYLE_MAX,
                         mx::PATH_TYPE_ABSOLUTE);

    // path_frompathname splits a trailing filename off to yield the containing folder's path id;
    // pass the folder path with a trailing separator so the "filename" part is empty.
    char        filename[512] = {0};
    std::string probe         = std::string(conformed) + "/";
    mx::path_frompathname(probe.c_str(), &m_temp_path, filename);

    mx::path_addnamed(mx::PATH_SEARCH_PATH, conformed, 1, 0);
}

void remove_temp_folder() {
    if (!m_temp_fullpath.empty()) {
        std::error_code ec;
        fs::remove_all(m_temp_fullpath, ec);
        m_temp_fullpath.clear();
        m_temp_path = 0;
    }
}

// ---- open ---------------------------------------------------------------

void do_open(const symbol arg) {
    char         filename[256];
    short        path    = 0;
    mx::t_fourcc outtype = 0;
    mx::t_fourcc type    = 'cO0p';

    if (!arg || !std::strlen(arg.c_str())) {
        if (mx::open_dialog(filename, &path, &outtype, nullptr, -1)) { // 0 == success
            return;
        }
    }
    else {
        mx::t_fourcc typelist[1] = {'cO0p'};
        std::strncpy(filename, arg.c_str(), sizeof(filename) - 1);
        filename[sizeof(filename) - 1] = 0;
        path                           = 0;
        mx::locatefile_extended(filename, &path, &type, typelist, 0);
    }

    char fullpath[512];
    mx::path_topotentialname(path, filename, fullpath, 0);

    // Conform to a native absolute path for the sqlite @db argument.
    char nativepath[512];
    mx::path_nameconform(fullpath, nativepath, mx::PATH_STYLE_NATIVE, mx::PATH_TYPE_ABSOLUTE);
    m_name = mx::gensym(nativepath);

    // Create the temp folder for extracted files.
    make_temp_folder();

    // Create the SQLite instance: object_new_typed(CLASS_NOBOX, "sqlite", @rambased 0 @db <path>).
    mx::t_atom a[4];
    mx::atom_setsym(&a[0], mx::gensym("@rambased"));
    mx::atom_setlong(&a[1], 0);
    mx::atom_setsym(&a[2], mx::gensym("@db"));
    mx::atom_setsym(&a[3], m_name);
    m_sqlite = static_cast<mx::t_object*>(mx::object_new_typed(mx::CLASS_NOBOX, mx::gensym("sqlite"), 4, a));
    if (!m_sqlite) {
        return;
    }

    mx::object_method(m_sqlite, mx::gensym("starttransaction"));
    sql_exec(TABLEDEF_FILES, nullptr);
    sql_exec(TABLEDEF_ATTRS, nullptr);

    sql_exec("UPDATE files SET valid = 1", nullptr);

    mx::t_object* result = nullptr;
    sql_exec("SELECT file_id, filename, moddate FROM files", &result);
    char** record = nullptr;
    while (result && (record = reinterpret_cast<char**>(mx::object_method(result, mx::gensym("nextrecord"))))) {
        if (!extract_file_wanted(record[0])) {
            continue;
        }
        extract_file(record[0], record[1], record[2], type);
    }
    mx::object_method(m_sqlite, mx::gensym("endtransaction"));
    if (result) {
        mx::object_free(result);
    }
}

// Honor the optional per-file 'platform' attribute: skip a file flagged for the *other* platform.
bool extract_file_wanted(const char* file_id) {
#if defined(MAC_VERSION) || defined(__APPLE__)
    const char* this_platform  = "mac";
    const char* other_platform = "windows";
#else
    const char* this_platform  = "windows";
    const char* other_platform = "mac";
#endif
    mx::t_object* r = nullptr;
    char          sql[512];
    snprintf(sql, sizeof(sql),
             "SELECT file_id_ext FROM attrs WHERE attr_name = 'platform' AND attr_value = '%s' AND file_id_ext = %s ",
             other_platform, file_id);
    sql_exec(sql, &r);
    char** rec    = r ? reinterpret_cast<char**>(mx::object_method(r, mx::gensym("nextrecord"))) : nullptr;
    bool   wanted = true;
    if (rec) {
        // Flagged for the other platform; keep only if also flagged for this platform.
        mx::t_object* r2 = nullptr;
        snprintf(
            sql, sizeof(sql),
            "SELECT file_id_ext FROM attrs WHERE attr_name = 'platform' AND attr_value = '%s' AND file_id_ext = %s ",
            this_platform, file_id);
        sql_exec(sql, &r2);
        char** rec2 = r2 ? reinterpret_cast<char**>(mx::object_method(r2, mx::gensym("nextrecord"))) : nullptr;
        if (!rec2) {
            snprintf(sql, sizeof(sql), "UPDATE files SET valid = 0 WHERE file_id = %s ", file_id);
            sql_exec(sql, nullptr);
            wanted = false;
        }
        if (r2) {
            mx::object_free(r2);
        }
    }
    if (r) {
        mx::object_free(r);
    }
    return wanted;
}

// Write the BLOB content for one file out to the temp folder, and restore its mod date.
void extract_file(const char* file_id, const char* filename, const char* moddate, mx::t_fourcc type) {
    mx::t_filehandle file_handle = nullptr;
    mx::t_max_err    err         = mx::path_createsysfile(filename, m_temp_path, type, &file_handle);
    if (err) {
        cerr << "error " << err << " creating file " << filename << endl;
        return;
    }

    char sql[512];
    snprintf(sql, sizeof(sql), "SELECT content FROM files WHERE file_id = %s", file_id);
    char*          blob = nullptr;
    mx::t_ptr_size len  = 0;
    mx::object_method(m_sqlite, mx::gensym("getblob"), sql, &blob, &len);
    if (blob) {
        err = mx::sysfile_write(file_handle, &len, blob);
        if (err) {
            cerr << "sysfile_write error (" << err << ")" << endl;
        }
    }
    mx::sysfile_seteof(file_handle, len);
    mx::sysfile_close(file_handle);
    if (blob) {
        mx::sysmem_freeptr(blob);
    }

    restore_moddate(filename, moddate);
}

// Best-effort restoration of the file's stored modification date using std::filesystem.
void restore_moddate(const char* filename, const char* moddate) {
    if (!moddate || !moddate[0]) {
        return;
    }
    std::tm tm{};
    if (std::sscanf(moddate, "%4d-%2d-%2d %2d:%2d:%2d", &tm.tm_year, &tm.tm_mon, &tm.tm_mday, &tm.tm_hour, &tm.tm_min,
                    &tm.tm_sec)
        != 6) {
        return;
    }
    tm.tm_year -= 1900;
    tm.tm_mon -= 1;
    std::time_t t = std::mktime(&tm);
    if (t == static_cast<std::time_t>(-1)) {
        return;
    }

    std::error_code ec;
    fs::path        target = fs::path(m_temp_fullpath) / filename;
    const auto      sctp   = std::chrono::time_point_cast<fs::file_time_type::duration>(
        fs::file_time_type::clock::now()
        + (std::chrono::system_clock::from_time_t(t) - std::chrono::system_clock::now()));
    fs::last_write_time(target, sctp, ec); // best-effort; ignore failure
}

// ---- addfile ------------------------------------------------------------

void do_addfile(const symbol arg) {
    char         filename[256];
    short        path    = 0;
    mx::t_fourcc outtype = 0;

    if (!arg || !std::strlen(arg.c_str())) {
        if (mx::open_dialog(filename, &path, &outtype, nullptr, -1)) { // 0 == success
            return;
        }
    }
    else {
        std::strncpy(filename, arg.c_str(), sizeof(filename) - 1);
        filename[sizeof(filename) - 1] = 0;
        path                           = 0;
    }

    mx::t_filehandle file_handle = nullptr;
    mx::t_max_err    err         = mx::path_opensysfile(filename, path, &file_handle, mx::PATH_READ_PERM);
    if (err) {
        cerr << "error opening file" << endl;
        return;
    }

    mx::t_ptr_size size = 0;
    mx::sysfile_geteof(file_handle, &size);

    char* blob = static_cast<char*>(mx::sysmem_newptr(size));
    if (!blob) {
        cerr << "could not allocate memory" << endl;
        mx::sysfile_close(file_handle);
        return;
    }
    err = mx::sysfile_read(file_handle, &size, blob);
    if (err) {
        cerr << "sysfile_read error (" << err << ")" << endl;
    }

    mx::t_ptr_uint moddate = 0;
    mx::path_getfilemoddate(filename, path, &moddate);
    mx::t_datetime moddatetime{};
    mx::systime_secondstodate(moddate, &moddatetime);
    char moddatetimestring[32];
    snprintf(moddatetimestring, sizeof(moddatetimestring), "%4u-%02u-%02u %02u:%02u:%02u",
             static_cast<unsigned>(moddatetime.year), static_cast<unsigned>(moddatetime.month),
             static_cast<unsigned>(moddatetime.day), static_cast<unsigned>(moddatetime.hour),
             static_cast<unsigned>(moddatetime.minute), static_cast<unsigned>(moddatetime.second));

    char sql[512];
    snprintf(sql, sizeof(sql),
             "INSERT INTO files ('filename', 'content', 'moddate','valid') VALUES (\"%s\", ?, '%s', 1)", filename,
             moddatetimestring);
    mx::object_method(m_sqlite, mx::gensym("setblob"), sql, blob, size);

    mx::sysmem_freeptr(blob);
    mx::sysfile_close(file_handle);
}

// ---- close --------------------------------------------------------------

void do_close() {
    if (!m_sqlite) {
        return;
    }
    remove_temp_folder();
    mx::object_free(m_sqlite);
    m_sqlite = nullptr;
    m_name   = symbol{""};
}
}
;

MIN_EXTERNAL(filecontainer);
