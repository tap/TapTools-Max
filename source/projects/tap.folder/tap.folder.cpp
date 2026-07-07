/// @file
/// @brief      tap.folder — create, delete, copy, and move folders in the filesystem.
/// @details    Performs filesystem operations on folders and files, banging the outlet when an
///             operation completes. The original implementation relied on AppleScript (macOS) and
///             the Win32 shell; this revival reimplements the same operations on portable C++
///             std::filesystem, so the behavior is identical and cross-platform without those
///             platform-specific dependencies.
/// @note       `unzip` is not provided — the standard library has no portable archive support; use a
///             dedicated unzip object/tool instead.
/// @author     Timothy Place
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2003-2026 Timothy Place.

#include <filesystem>
#include <string>
#include <system_error>

#include "c74_min.h"

using namespace c74::min;
namespace fs = std::filesystem;

class folder : public object<folder> {
  public:
    MIN_DESCRIPTION{"Create, delete, copy, and move folders (and files) in the filesystem. Bangs "
                    "the outlet when an operation completes."};
    MIN_TAGS{"system"};
    MIN_AUTHOR{"Timothy Place"};
    MIN_RELATED{"folder, dropfile, filepath, conformpath"};

    inlet<>  m_in{this, "(make/delete/copy/move <path...>) filesystem commands"};
    outlet<> m_done{this, "(bang) sent when an operation completes"};

    message<> make{this, "make", "Create a folder (and any missing parents) at the given path.",
                   MIN_FUNCTION{if (require(args, 1, "make")){std::error_code ec;
    fs::create_directories(path_of(args[0]), ec);
    report(ec, "creating folder");
} return {};
}
}
;

message<> remove{this, "delete", "Delete the folder or file at the given path (recursively).",
                 MIN_FUNCTION{if (require(args, 1, "delete")){std::error_code ec;
fs::remove_all(path_of(args[0]), ec);
report(ec, "deleting");
}
return {};
}
}
;

message<>      copy{this, "copy", "Copy a folder (recursively) or file from a source to a destination path.",
               MIN_FUNCTION{if (require(args, 2, "copy")){std::error_code ec;
const fs::path src = path_of(args[0]);
fs::copy(src, path_of(args[1]), fs::copy_options::recursive | fs::copy_options::overwrite_existing, ec);
report(ec, "copying");
}
return {};
}
}
;

message<> move{this, "move", "Move/rename a folder or file from a source to a destination path.",
               MIN_FUNCTION{if (require(args, 2, "move")){std::error_code ec;
fs::rename(path_of(args[0]), path_of(args[1]), ec);
report(ec, "moving");
}
return {};
}
}
;

private:
static fs::path path_of(const atom& a) {
    // Go through c_str(): symbol has both operator std::string() and operator const char*(),
    // which makes a direct std::string conversion ambiguous under MSVC.
    return fs::path(static_cast<symbol>(a).c_str());
}

bool require(const atoms& args, size_t n, const char* name) {
    if (args.size() < n) {
        cerr << name << ": expected " << n << " path argument(s)" << endl;
        return false;
    }
    return true;
}

void report(const std::error_code& ec, const char* action) {
    if (ec) {
        cerr << "error " << action << ": " << ec.message() << endl;
    }
    else {
        m_done.send("bang");
    }
}
}
;

MIN_EXTERNAL(folder);
