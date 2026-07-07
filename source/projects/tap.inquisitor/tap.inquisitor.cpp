/// @file
/// @brief      tap.inquisitor — interrogate another object's attributes.
/// @details    Finds an object in the same patcher by its scripting name and either reads back the
///             value of one of its attributes (the `get` message) or dumps the list of attribute
///             names it exposes (the `attributes` message). Faithful port of the original TapTools
///             object — it uses the Max patcher API directly (no Jamoma dependency).
/// @author     Timothy Place
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2009-2026 Timothy Place.

#include "c74_min.h"

using namespace c74::min;

class inquisitor : public object<inquisitor> {
  public:
    MIN_DESCRIPTION{"Interrogate another object's attributes. Names a target object by its "
                    "scripting name and reads attribute values (get) or attribute names "
                    "(attributes) from it."};
    MIN_TAGS{"utilities"};
    MIN_AUTHOR{"Timothy Place"};
    MIN_RELATED{"pattr, pattrforward, universal"};

    inlet<>  m_in{this, "(get/attributes) interrogate the named subject"};
    outlet<> m_out_values{this, "(anything) attribute values from the subject"};
    outlet<> m_out_names{this, "(anything) attribute names from the subject"};
    outlet<> m_dump{this, "(anything) dumpout"};

    attribute<symbol> name{this, "name", k_no_name,
                           setter{MIN_FUNCTION{m_subject = nullptr; // a new name invalidates the cached subject
    return args;
}
}
, description {
    "The scripting name (varname) of the object to interrogate."
}
}
;

message<> get{this, "get", "Get the value of the named attribute from the subject.",
              MIN_FUNCTION{if (args.empty()){return {};
}
const symbol attribute_name = args[0];
resolve_subject();
if (!m_subject) {
    return {};
}

long              ac  = 0;
c74::max::t_atom* av  = nullptr;
auto              err = c74::max::object_attr_getvalueof(m_subject, attribute_name, &ac, &av);

if (!err && ac && av) {
    atoms out;
    out.reserve(ac + 1);
    out.push_back(attribute_name);
    bool usable = true;
    for (long i = 0; i < ac; ++i) {
        switch (c74::max::atom_gettype(av + i)) {
        case c74::max::A_LONG:
            out.push_back(static_cast<int>(c74::max::atom_getlong(av + i)));
            break;
        case c74::max::A_FLOAT:
            out.push_back(c74::max::atom_getfloat(av + i));
            break;
        case c74::max::A_SYM:
            out.push_back(symbol(c74::max::atom_getsym(av + i)));
            break;
        default:
            usable = false;
            break;
        }
    }
    if (usable) {
        m_out_values.send(out);
    }
    else {
        cerr << "the data type for attribute '" << attribute_name << "' is not usable by tap.inquisitor" << endl;
    }
}
else {
    cerr << "problem getting attribute value for " << attribute_name << endl;
}

if (av) {
    c74::max::sysmem_freeptr(av);
}
return {};
}
}
;

message<> dump_attributes{this, "attributes", "Dump the names of the subject's attributes.",
                          MIN_FUNCTION{resolve_subject();
if (!m_subject) {
    return {};
}

long                 count = 0;
c74::max::t_symbol** names = nullptr;
c74::max::object_attr_getnames(m_subject, &count, &names);
if (count && names) {
    atoms out;
    out.reserve(count);
    for (long i = 0; i < count; ++i) {
        out.push_back(symbol(names[i]));
    }
    m_out_names.send(out);
    c74::max::sysmem_freeptr(names);
}
return {};
}
}
;

private:
static const symbol k_no_name;
c74::max::t_object* m_subject{nullptr};

// Locate the subject object in this object's patcher by matching its scripting (var) name.
void resolve_subject() {
    if (m_subject) {
        return;
    }
    const symbol target = name;
    if (target == k_no_name) {
        return;
    }

    c74::min::patcher containing_patcher = patcher(); // inherited accessor for this object's patcher
    if (!containing_patcher) {
        return;
    }

    for (auto& b : containing_patcher.boxes()) {
        if (b.name() == target) {
            m_subject = c74::max::jbox_get_object(b);
            break;
        }
    }
}
}
;

const symbol inquisitor::k_no_name{""};

MIN_EXTERNAL(inquisitor);
