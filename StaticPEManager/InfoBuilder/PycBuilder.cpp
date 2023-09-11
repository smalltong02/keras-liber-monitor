#include "PycBuilder.h"
#include "pyc_numeric.h"
#include "bytecode.h"
#include "ASTree.h"

namespace cchips {
    const std::vector<std::string> CPycBuilder::_pyc_flag_names = {
        {"CO_OPTIMIZED"}, {"CO_NEWLOCALS"}, {"CO_VARARGS"}, {"CO_VARKEYWORDS"},
        {"CO_NESTED"}, {"CO_GENERATOR"}, {"CO_NOFREE"}, {"CO_COROUTINE"},
        {"CO_ITERABLE_COROUTINE"}, {"<0x200>"}, {"<0x400>"}, {"<0x800>"},
        {"CO_GENERATOR_ALLOWED"}, {"CO_FUTURE_DIVISION"},
        {"CO_FUTURE_ABSOLUTE_IMPORT"}, {"CO_FUTURE_WITH_STATEMENT"},
        {"CO_FUTURE_PRINT_FUNCTION"}, {"CO_FUTURE_UNICODE_LITERALS"},
        {"CO_FUTURE_BARRY_AS_BDFL"}, {"CO_FUTURE_GENERATOR_STOP"},
        {"<0x100000>"}, {"<0x200000>"}, {"<0x400000>"}, {"<0x800000>"},
        {"<0x1000000>"}, {"<0x2000000>"}, {"<0x4000000>"}, {"<0x8000000>"},
        {"<0x10000000>"}, {"<0x20000000>"}, {"<0x40000000>"}, {"<0x80000000>"}
    };
    const std::map<std::string, CPycBuilder::py_filetype> CPycBuilder::_py_special_files = {
        {"__init__.py", py_init},
        {"__main__.py", py_main},
        {"__init__.pyc", py_init_c},
        {"__main__.pyc", py_main_c},
        {"__init__.pyi", py_init_i},
        {"requirements.txt", py_requirements},
        {"__about__.py", py_about},
    };

    bool CPycBuilder::Initialize(std::unique_ptr<CJsonOptions>& json_options) {
        m_flag = pyc_package;
        return true;
    }

    bool CPycBuilder::Scan(fs::path& file_path, CFileInfo& file_info) {
        try {
            bool bdir = false;
            if (file_path.empty()) {
                return false;
            }
            if (fs::is_directory(file_path)) {
                if (m_flag != pyc_package) {
                    return false;
                }
                bdir = true;
            }
            else {
                if (m_flag == pyc_package) {
                    return false;
                }
            }

            if (m_flag == pyc_disassemble || m_flag == pyc_decompyle) {
                m_pycmod.loadFromFile(file_path.string().c_str());
                return CommonPycScan(file_info);
            }
            m_input = file_path;
            return CommonSrcScan(file_info);
        }
        catch (const std::exception& e)
        {
        }
        return false;
    }

    bool CPycBuilder::Scan(const std::string& file_buf, CFileInfo& file_info) {
        try {
            if (m_flag == pyc_package) {
                return false;
            }
            if (file_buf.empty()) {
                return false;
            }
            if (m_flag == pyc_disassemble || m_flag == pyc_decompyle) {
                m_pycmod.loadFromBuffer(file_buf.data(), file_buf.length());
                return CommonPycScan(file_info);
            }
            return CommonSrcScan(file_buf, file_info);
        }
        catch (const std::exception& e) {

        }
        return false;
    }

    bool CPycBuilder::CommonPycScan(CFileInfo& file_info) {
        auto major = m_pycmod.majorVer();
        auto minor = m_pycmod.minorVer();
        if (major == -1 || minor == -1) {
            return false;
        }
        std::ostringstream oss;
        try {
            if (m_flag == pyc_decompyle) {
                decompyle(m_pycmod.code(), &m_pycmod, oss);
            }
        }
        catch (const std::exception& e) {

        }
        if (oss.str().empty()) {
            try {
                if (m_flag == pyc_disassemble) {
                    if (!ParsePycObject(m_pycmod.code().try_cast<PycObject>(), 0, oss)) {
                        return false;
                    }
                }
                if (oss.str().empty()) {
                    return false;
                }
            }
            catch (const std::exception& e) {

            }
        }

        return true;
    }

    bool CPycBuilder::CommonSrcScan(CFileInfo& file_info) {
        return false;
    }
    
    bool CPycBuilder::CommonSrcScan(const std::string& file_buf, CFileInfo& file_info) {
        return false;
    }

    bool CPycBuilder::ParsePycObject(PycRef<PycObject> obj, int indent, std::ostringstream& oss) {
        auto iputs = [&](std::ostringstream& oss, int indent, const std::string& text) ->void {
            for (int i = 0; i < indent; i++)
                oss << "    ";
            oss << text;
        };
        auto print_coflags = [&](std::ostringstream& oss, std::uint32_t flags) ->void {
            if (flags == 0) {
                oss << "\n";
                return;
            }

            oss << " (";
            unsigned long f = 1;
            int k = 0;
            while (k < _pyc_flag_names.size()) {
                if ((flags & f) != 0) {
                    flags &= ~f;
                    if (flags == 0)
                        oss << _pyc_flag_names[k];
                    else
                        oss << _pyc_flag_names[k] << " | ";
                }
                ++k;
                f <<= 1;
            }
            oss << ")\n";
        };
        auto ivprintf = [&](std::ostringstream& oss, int indent, const char* fmt, va_list varargs) ->void {
            for (int i = 0; i < indent; i++)
                oss << "    ";
            formatted_printv(oss, fmt, varargs);
        };
        auto iprintf = [&](std::ostringstream& oss, int indent, const char* fmt, ...) ->void {
            va_list varargs;
            va_start(varargs, fmt);
            ivprintf(oss, indent, fmt, varargs);
            va_end(varargs);
        };

        if (obj == NULL) {
            iputs(oss, indent, "<NULL>");
            return true;
        }

        switch (obj->type()) {
        case PycObject::TYPE_CODE:
        case PycObject::TYPE_CODE2:
        {
            PycRef<PycCode> codeObj = obj.cast<PycCode>();
            iputs(oss, indent, "[Code]\n");
            iprintf(oss, indent + 1, "File Name: %s\n", codeObj->fileName()->value());
            iprintf(oss, indent + 1, "Object Name: %s\n", codeObj->name()->value());
            if (m_pycmod.verCompare(3, 11) >= 0)
                iprintf(oss, indent + 1, "Qualified Name: %s\n", codeObj->qualName()->value());
            iprintf(oss, indent + 1, "Arg Count: %d\n", codeObj->argCount());
            if (m_pycmod.verCompare(3, 8) >= 0)
                iprintf(oss, indent + 1, "Pos Only Arg Count: %d\n", codeObj->posOnlyArgCount());
            if (m_pycmod.majorVer() >= 3)
                iprintf(oss, indent + 1, "KW Only Arg Count: %d\n", codeObj->kwOnlyArgCount());
            if (m_pycmod.verCompare(3, 11) < 0)
                iprintf(oss, indent + 1, "Locals: %d\n", codeObj->numLocals());
            if (m_pycmod.verCompare(1, 5) >= 0)
                iprintf(oss, indent + 1, "Stack Size: %d\n", codeObj->stackSize());
            if (m_pycmod.verCompare(1, 3) >= 0) {
                iprintf(oss, indent + 1, "Flags: 0x%08X", codeObj->flags());
                print_coflags(oss, codeObj->flags());
            }

            iputs(oss, indent + 1, "[Names]\n");
            for (int i = 0; i < codeObj->names()->size(); i++)
                ParsePycObject(codeObj->names()->get(i), indent + 2, oss);

            if (m_pycmod.verCompare(1, 3) >= 0) {
                if (m_pycmod.verCompare(3, 11) >= 0)
                    iputs(oss, indent + 1, "[Locals+Names]\n");
                else
                    iputs(oss, indent + 1, "[Var Names]\n");
                for (int i = 0; i < codeObj->localNames()->size(); i++)
                    ParsePycObject(codeObj->localNames()->get(i), indent + 2, oss);
            }

            //if (m_pycmod.verCompare(3, 11) >= 0) {
            //    iputs(oss, indent + 1, "[Locals+Kinds]\n");
            //    ParsePycObject(codeObj->localKinds().cast<PycObject>(), indent + 2, oss);
            //}

            if (m_pycmod.verCompare(2, 1) >= 0 && m_pycmod.verCompare(3, 11) < 0) {
                iputs(oss, indent + 1, "[Free Vars]\n");
                for (int i = 0; i < codeObj->freeVars()->size(); i++)
                    ParsePycObject(codeObj->freeVars()->get(i), indent + 2, oss);

                iputs(oss, indent + 1, "[Cell Vars]\n");
                for (int i = 0; i < codeObj->cellVars()->size(); i++)
                    ParsePycObject(codeObj->cellVars()->get(i), indent + 2, oss);
            }

            iputs(oss, indent + 1, "[Constants]\n");
            for (int i = 0; i < codeObj->consts()->size(); i++)
                ParsePycObject(codeObj->consts()->get(i), indent + 2, oss);

            iputs(oss, indent + 1, "[Disassembly]\n");
            bc_disasm(oss, codeObj, &m_pycmod, indent + 2, 0);

            //if (m_pycmod.verCompare(1, 5) >= 0) {
            //    iputs(oss, indent + 1, "[Line Number Table]\n");
            //    ParsePycObject(codeObj->lnTable().cast<PycObject>(), indent + 2, oss);
            //}

            //if (m_pycmod.verCompare(3, 11) >= 0) {
            //    iputs(oss, indent + 1, "[Exception Table]\n");
            //    ParsePycObject(codeObj->exceptTable().cast<PycObject>(), indent + 2, oss);
            //}
        }
        break;
        case PycObject::TYPE_STRING:
        case PycObject::TYPE_UNICODE:
        case PycObject::TYPE_INTERNED:
        case PycObject::TYPE_ASCII:
        case PycObject::TYPE_ASCII_INTERNED:
        case PycObject::TYPE_SHORT_ASCII:
        case PycObject::TYPE_SHORT_ASCII_INTERNED:
            iputs(oss, indent, "");
            obj.cast<PycString>()->print(oss, &m_pycmod);
            oss << "\n";
            break;
        case PycObject::TYPE_TUPLE:
        case PycObject::TYPE_SMALL_TUPLE:
        {
            iputs(oss, indent, "(\n");
            for (const auto& val : obj.cast<PycTuple>()->values())
                ParsePycObject(val, indent + 1, oss);
            iputs(oss, indent, ")\n");
        }
        break;
        case PycObject::TYPE_LIST:
        {
            iputs(oss, indent, "[\n");
            for (const auto& val : obj.cast<PycList>()->values())
                ParsePycObject(val, indent + 1, oss);
            iputs(oss, indent, "]\n");
        }
        break;
        case PycObject::TYPE_DICT:
        {
            iputs(oss, indent, "{\n");
            PycDict::key_t keys = obj.cast<PycDict>()->keys();
            PycDict::value_t values = obj.cast<PycDict>()->values();
            PycDict::key_t::const_iterator ki = keys.begin();
            PycDict::value_t::const_iterator vi = values.begin();
            while (ki != keys.end()) {
                ParsePycObject(*ki, indent + 1, oss);
                ParsePycObject(*vi, indent + 2, oss);
                ++ki, ++vi;
            }
            iputs(oss, indent, "}\n");
        }
        break;
        case PycObject::TYPE_SET:
        {
            iputs(oss, indent, "{\n");
            for (const auto& val : obj.cast<PycSet>()->values())
                ParsePycObject(val, indent + 1, oss);
            iputs(oss, indent, "}\n");
        }
        break;
        case PycObject::TYPE_FROZENSET:
        {
            iputs(oss, indent, "frozenset({\n");
            for (const auto& val : obj.cast<PycSet>()->values())
                ParsePycObject(val, indent + 1, oss);
            iputs(oss, indent, "})\n");
        }
        break;
        case PycObject::TYPE_NONE:
            iputs(oss, indent, "None\n");
            break;
        case PycObject::TYPE_FALSE:
            iputs(oss, indent, "False\n");
            break;
        case PycObject::TYPE_TRUE:
            iputs(oss, indent, "True\n");
            break;
        case PycObject::TYPE_ELLIPSIS:
            iputs(oss, indent, "...\n");
            break;
        case PycObject::TYPE_INT:
            iprintf(oss, indent, "%d\n", obj.cast<PycInt>()->value());
            break;
        case PycObject::TYPE_LONG:
            iprintf(oss, indent, "%s\n", obj.cast<PycLong>()->repr().c_str());
            break;
        case PycObject::TYPE_FLOAT:
            iprintf(oss, indent, "%s\n", obj.cast<PycFloat>()->value());
            break;
        case PycObject::TYPE_COMPLEX:
            iprintf(oss, indent, "(%s+%sj)\n", obj.cast<PycComplex>()->value(),
                obj.cast<PycComplex>()->imag());
            break;
        case PycObject::TYPE_BINARY_FLOAT:
            iprintf(oss, indent, "%g\n", obj.cast<PycCFloat>()->value());
            break;
        case PycObject::TYPE_BINARY_COMPLEX:
            iprintf(oss, indent, "(%g+%gj)\n", obj.cast<PycCComplex>()->value(),
                obj.cast<PycCComplex>()->imag());
            break;
        default:
            iprintf(oss, indent, "<TYPE: %d>\n", obj->type());
        }
        return true;
    }
} // namespace cchips
