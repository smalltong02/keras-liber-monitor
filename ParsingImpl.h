//===- ParsingImpl.h - metadata type implement define -------------------*- C++ -*-===//
// 
// This file contains Lexer class and parser class, they service for metadata define.
//
//===------------------------------------------------------------------------------===//
#pragma once
#include <Windows.h>
#include <cstdio>
#include <string>
#include <vector>
#include <queue>
#include <map>
#include <memory>
#include <functional>
#include <variant>
#include <regex>
#include "hookimpl.h"
#include "MetadataTypeImpl.h"
#include "LogObject.h"

namespace cchips {

#define HD_DUPLICATE "Duplicate"
#define HD_ADDTARGET "AddTarget"

    extern std::unique_ptr<CTypeSymbolTableObject> g_typesymbolobject;

    using VAR_SYMBOLTABLE = TYPE_SYMBOLTABLE;
    using SYMBOLTABLE = TYPE_SYMBOLTABLE;
    using IDENTIFIERTABLE = std::map<std::string, std::pair<int, std::shared_ptr<CObObject>>>;

    class CLexerObject
    {
    public:
        enum _symbol {
            symbol_type = 0,
            symbol_variable = 1,
        };

        using function_type = enum {
            function_getvalue = 0,
            function_setvalue,
            function_success,
        };

        using iterator = SYMBOLTABLE::iterator;
        using const_iterator = SYMBOLTABLE::const_iterator;

        CLexerObject() { CreateVariableSymbol(); }
        CLexerObject(std::unique_ptr<SYMBOLTABLE>& table) : m_variablesymbols(std::move(table)) { }
        ~CLexerObject() = default;

        std::string GetWordRoot(const std::string& identifier_str) const;
        std::shared_ptr<CObObject> GetVarIdentifier(const std::string& identifier_str) const;
        std::shared_ptr<CObObject> GetTyIdentifier(const std::string& identifier_str) const;
        bool AddVarIdentifier(const IDENPAIR& iden_pair);
        bool AddTyIdentifier(const IDENPAIR& iden_pair);
        int GetSymbolTableSize() const {
            if (m_variablesymbols)
                return (int)(*m_variablesymbols).size();
            return 0;
        }
        std::shared_ptr<CObObject> GetFinalObObject(std::shared_ptr<CObObject> object) const {
            while (object)
            {
                switch (object->GetObType())
                {
                case CObObject::ob_basetype:
                case CObObject::ob_flag:
                    return object;
                case CObObject::ob_reference:
                case CObObject::ob_stringref:
                {
                    if (std::static_pointer_cast<CReferenceObject>(object)->GetData() == nullptr)
                        return object;
                    object = std::static_pointer_cast<CReferenceObject>(object)->GetData();
                }
                break;
                case CObObject::ob_array:
                {
                    ASSERT(0);
                    if (std::static_pointer_cast<CArrayObject>(object)->GetData() == nullptr)
                        return object;
                    object = std::static_pointer_cast<CArrayObject>(object)->GetData();
                }
                break;
                case CObObject::ob_tuple:
                {
                    if (std::static_pointer_cast<CTupleObject>(object)->GetData().size() != 1)
                        return object;
                    object = *std::static_pointer_cast<CTupleObject>(object)->GetData().begin();
                }
                break;
                case CObObject::ob_struct:
                {
                    if (std::static_pointer_cast<CStructObject>(object)->GetData().size() != 1)
                        return object;
                    object = (*std::static_pointer_cast<CStructObject>(object)->GetData().begin()).second.second.second;
                }
                break;
                default:
                    return nullptr;
                }
            }
            return object;
        }

        std::variant<bool, std::stringstream> ImplFuncFinalObObject(std::shared_ptr<CObObject> object, function_type f_type, char* pdata, const std::stringstream* ss = nullptr) const
        {
            ASSERT(object != nullptr);
            ASSERT(pdata != nullptr);
            if (!object) return false;
            if (!pdata) return false;
            char* p = pdata;
            auto default_rr = [=]() ->std::variant<bool, std::stringstream> { if (f_type == function_getvalue) return std::stringstream(""); else return false; };
            auto call_func = [](std::shared_ptr<CObObject> object, function_type f_type, char* pdata, const std::stringstream* ss = nullptr) ->std::variant<bool, std::stringstream> {
                switch (f_type)
                {
                case function_success:
                {
                    const std::function<bool(char*)> func = std::bind(&CObObject::Success, object, std::placeholders::_1);
                    return func(pdata);
                }
                break;
                case function_getvalue:
                {
                    const std::function<std::stringstream(char*)> func = std::bind(&CObObject::GetValue, object, std::placeholders::_1);
                    return func(pdata);
                }
                break;
                case function_setvalue:
                {
                    const std::function<bool(char*, const std::stringstream&)> func = std::bind(&CObObject::SetValue, object, std::placeholders::_1, std::placeholders::_2);
                    ASSERT(ss != nullptr); if (!ss->str().length()) return false; return func(pdata, *ss);
                }
                break;
                default:
                {; }
                }
                return false;
            };

            while (object)
            {
                switch (object->GetObType())
                {
                case CObObject::ob_basetype:
                case CObObject::ob_flag:
                case CObObject::ob_stringref:
                    return call_func(object, f_type, p, ss);
                    break;
                case CObObject::ob_reference:
                {
                    if (std::static_pointer_cast<CReferenceObject>(object)->GetData() == nullptr)
                        return call_func(object, f_type, p, ss);
                    char* nodig = nullptr;
                    std::stringstream p_str = object->GetValue(p);
                    p = (char*)std::strtoll(p_str.str().c_str(), &nodig, 16);
                    if (!p) return default_rr();
                    object = std::static_pointer_cast<CReferenceObject>(object)->GetData();
                }
                break;
                case CObObject::ob_array:
                {
                    ASSERT(0);
                }
                break;
                case CObObject::ob_tuple:
                {
                    ASSERT(0);
                }
                break;
                case CObObject::ob_struct:
                {
                    if (std::static_pointer_cast<CStructObject>(object)->GetData().size() != 1)
                        return call_func(object, f_type, p, ss);
                    std::shared_ptr<CStructObject> ob_ptr = std::static_pointer_cast<CStructObject>(GetTyIdentifier(std::static_pointer_cast<CStructObject>(object)->GetName()));
                    if (!ob_ptr) return default_rr();
                    int offset = ob_ptr->GetElementOffset((*std::static_pointer_cast<CStructObject>(object)->GetData().begin()).first);
                    ASSERT(offset >= 0);
                    p += offset;
                    object = (*std::static_pointer_cast<CStructObject>(object)->GetData().begin()).second.second.second;
                }
                break;
                default:
                { return default_rr(); }
                }
            }
            return default_rr();
        }

        bool IsValidValue(std::shared_ptr<CObObject> object, char* pdata) const {
            ASSERT(object != nullptr);
            ASSERT(pdata != nullptr);
            if (!object) return false;
            if (!pdata) return false;
            char* p = pdata;
            while (object)
            {
                switch (object->GetObType())
                {
                case CObObject::ob_basetype:
                case CObObject::ob_stringref:
                    return object->IsValidValue(p);
                    break;
                case CObObject::ob_flag:
                    return true;
                    break;
                case CObObject::ob_reference:
                {
                    if (std::static_pointer_cast<CReferenceObject>(object)->GetData() == nullptr)
                        return object->IsValidValue(p);
                    char* nodig = nullptr;
                    std::stringstream ss = object->GetValue(p);
                    p = (char*)std::strtoll(ss.str().c_str(), &nodig, 16);
                    if (!p) return false;
                    object = std::static_pointer_cast<CReferenceObject>(object)->GetData();
                }
                break;
                case CObObject::ob_array:
                {
                    ASSERT(0);
                    if (std::static_pointer_cast<CArrayObject>(object)->GetData() == nullptr)
                        return object->IsValidValue(p);
                    int dim = std::static_pointer_cast<CArrayObject>(object)->GetDim();
                    size_t size = std::static_pointer_cast<CArrayObject>(object)->GetData()->GetObSize();
                    ASSERT(dim >= 0);
                    ASSERT(size != 0);
                    if (dim < 0 || size == 0) return false;
                    p += (dim*size);
                    object = std::static_pointer_cast<CArrayObject>(object)->GetData();
                }
                break;
                case CObObject::ob_tuple:
                {
                    if (std::static_pointer_cast<CTupleObject>(object)->GetData().size() != 1)
                        return object->IsValidValue(p);
                    char* nodig = nullptr;
                    std::stringstream ss = object->GetValue(p);
                    p = (char*)std::strtoll(ss.str().c_str(), &nodig, 16);
                    ASSERT(p != nullptr);
                    if (!p) return false;
                    object = *std::static_pointer_cast<CTupleObject>(object)->GetData().begin();
                }
                break;
                case CObObject::ob_struct:
                {
                    if (std::static_pointer_cast<CStructObject>(object)->GetData().size() != 1)
                        return object->IsValidValue(p);
                    std::shared_ptr<CStructObject> ob_ptr = std::static_pointer_cast<CStructObject>(GetTyIdentifier(std::static_pointer_cast<CStructObject>(object)->GetName()));
                    if (!ob_ptr) return false;
                    int offset = ob_ptr->GetElementOffset((*std::static_pointer_cast<CStructObject>(object)->GetData().begin()).first);
                    ASSERT(offset >= 0);
                    p += offset;
                    object = (*std::static_pointer_cast<CStructObject>(object)->GetData().begin()).second.second.second;
                }
                break;
                default:
                    return false;
                }
            }
            return false;
        }
        std::stringstream GetValue(std::shared_ptr<CObObject> object, char* pdata) const {
            return std::get<std::stringstream>(ImplFuncFinalObObject(object, function_getvalue, pdata));
        }
        bool SetValue(std::shared_ptr<CObObject> object, char* pdata, const std::stringstream& ss) {
            return std::get<bool>(ImplFuncFinalObObject(object, function_setvalue, pdata, &ss));
        }
        bool Success(std::shared_ptr<CObObject> object, char* pdata) const {
            return std::get<bool>(ImplFuncFinalObObject(object, function_success, pdata));
        }
        bool Failed(std::shared_ptr<CObObject> object, char* pdata) const {
            return !Success(object, pdata);
        }
        iterator		begin() { return (*m_variablesymbols).begin(); }
        const_iterator	begin() const { return (*m_variablesymbols).begin(); }
        iterator		end() { return (*m_variablesymbols).end(); }
        const_iterator	end() const { return (*m_variablesymbols).end(); }
    protected:
        bool CreateVariableSymbol() { if (m_variablesymbols = std::make_unique<SYMBOLTABLE>()) return true; else return false; }
    private:
        std::shared_ptr<CObObject> GetIdentifier(const std::string& identifier_str, _symbol symbol = symbol_variable) const;
        bool AddIdentifier(const IDENPAIR& iden_pair, _symbol symbol = symbol_variable);
        std::shared_ptr<CObObject> GetSymbol(const std::string& sym_str, _symbol symbol = symbol_variable) const;
        std::unique_ptr<SYMBOLTABLE> m_variablesymbols;
    };

    class CParsingObject
    {
    public:
        using iterator = SYMBOLTABLE::iterator;
        using const_iterator = SYMBOLTABLE::const_iterator;

        CParsingObject() = delete;
        CParsingObject(std::shared_ptr<CLexerObject> table) : m_globalsymbols(table) { }
        ~CParsingObject() = default;

        bool ParsingIdentifiers(const std::string& identifiers);
        std::shared_ptr<CLexerObject> GetGlobalSymbols() const { return m_globalsymbols; }
        std::shared_ptr<CLexerObject> GetLocalSymbols() const { return m_localsymbols; }
        std::shared_ptr<CObObject> GetIdentifier(const std::string& identifier_str) const {
            ASSERT(identifier_str.length());
            if (!identifier_str.length()) return nullptr;
            if (m_localsymbols)
            {
                auto& it = (*m_localsymbols).GetVarIdentifier(identifier_str);
                if (it != nullptr)
                    return it;
            }
            if (m_globalsymbols)
            {
                auto& it = (*m_globalsymbols).GetVarIdentifier(identifier_str);
                if (it != nullptr)
                    return it;
            }
            return nullptr;
        }
        bool AddIdentifier(const IDENPAIR& iden_pair) {
            ASSERT(iden_pair.second);
            if (!iden_pair.second) return false;
            if (m_alias.first.length())
            {
                IDENPAIR alias_pair;
                alias_pair.first = m_alias.first + std::to_string(m_alias.second++);
                alias_pair.second = iden_pair.second;
                m_alias_array[alias_pair.first] = iden_pair.first;
                return AddLocalSymbol(alias_pair);
            }
            return AddLocalSymbol(iden_pair);
        }
        const std::string& GetRealName(const std::string& alias_name) {
            auto it = m_alias_array.find(alias_name);
            if (it == m_alias_array.end())
                return alias_name;
            return it->second;
        }
        bool AddLocalSymbol(const IDENPAIR& iden_pair) {
            if (!m_localsymbols) return false;
            return m_localsymbols->AddVarIdentifier(iden_pair);
        }

        iterator		begin() { return (*m_localsymbols).begin(); }
        const_iterator	begin() const { return (*m_localsymbols).begin(); }
        iterator		end() { return (*m_localsymbols).end(); }
        const_iterator	end() const { return (*m_localsymbols).end(); }
    protected:
        void SetAlias(const std::string& alias) {
            m_alias.first = alias;
            m_alias.second = 0;
            return;
        }
    private:
        std::pair<std::string, int> m_alias;
        std::map<std::string, std::string> m_alias_array;
        std::shared_ptr<CLexerObject> m_globalsymbols = nullptr;
        std::shared_ptr<CLexerObject> m_localsymbols = std::make_shared<CLexerObject>();
    };

    class CExpParsing
    {
    public:
        enum _token {
            token_invalid = -1,
            token_def = 0,
            token_extern,
            token_op,
            token_iden,
            token_string,
            token_number,
        };
        using CBaseValue = CBaseDef;
        using ValuePair = std::pair<std::string, std::stringstream>;

        struct _Value {
            _Value() :token(token_invalid), value(nullptr) {}
            _Value(const _Value& val) { *this = val; }
            _Value(std::string string, _token tok) : token(tok) { value = std::make_unique<CBaseType<std::string>>(CBaseDef::type_string, CTraits<std::string>(string)); }
            _Value(std::string string) : token(token_string) { value = std::make_unique<CBaseType<std::string>>(CBaseDef::type_string, CTraits<std::string>(string)); }
            _Value(ULONGLONG num) : token(token_number) { value = std::make_unique<CBaseType<ULONGLONG>>(CBaseDef::type_ulonglong, CTraits<ULONGLONG>(num)); }

            bool isOp() const { return token == token_op; };
            bool isString() const { return token == token_string; }
            bool isNumber() const { return token == token_number; }
            bool isIdentifier() const { return token == token_iden; }
            _token GetToken() const { return token; }

            _Value operator + (_Value& rhs) { if (isNumber()) return (GetNumber() + rhs.GetNumber()); else return (GetString() + rhs.GetString()); }
            _Value operator * (_Value& rhs) { return (GetNumber() + rhs.GetNumber()); }
            _Value operator - (_Value& rhs) { return (GetNumber() - rhs.GetNumber()); }
            _Value operator / (_Value& rhs) { if (rhs.GetNumber() == 0) return 0; else return (GetNumber() / rhs.GetNumber()); }
            _Value operator << (_Value& rhs) { return (GetNumber() << rhs.GetNumber()); }
            _Value operator ^ (_Value& rhs) { return ((ULONGLONG)pow(GetNumber(), rhs.GetNumber())); }
            _Value operator >> (_Value& rhs) { return (GetNumber() >> rhs.GetNumber()); }
            _Value operator > (_Value& rhs) { return (GetNumber() > rhs.GetNumber()); }
            _Value operator >= (_Value& rhs) { return (GetNumber() >= rhs.GetNumber()); }
            _Value operator < (_Value& rhs) { return (GetNumber() < rhs.GetNumber()); }
            _Value operator <= (_Value& rhs) { return (GetNumber() <= rhs.GetNumber()); }
            _Value operator && (_Value& rhs) { return (GetNumber() && rhs.GetNumber()); }
            _Value operator || (_Value& rhs) { return (GetNumber() || rhs.GetNumber()); }
            _Value operator == (_Value& rhs) { if (isNumber()) return (GetNumber() == rhs.GetNumber()); else return (GetString() == rhs.GetString()); }
            _Value operator != (_Value& rhs) { if (isNumber()) return (GetNumber() != rhs.GetNumber()); else return (GetString() != rhs.GetString()); }
            _Value operator ! () { return !GetNumber(); }

            const _Value& operator = (const _Value& val) {
                token = val.GetToken();
                if (val.isNumber())
                    value = std::make_unique<CBaseType<ULONGLONG>>(CBaseDef::type_ulonglong, CTraits<ULONGLONG>(val.GetNumber()));
                else
                    value = std::make_unique<CBaseType<std::string>>(CBaseDef::type_string, CTraits<std::string>(val.GetString()));
                ASSERT(value);
                return *this;
            }
            ULONGLONG GetNumber() const {
                ASSERT(value);
                if (!value) return 0;
                if (isOp()) return 0;
                char* nodig = 0;
                ULONGLONG number = strtoull(value->GetValue().str().c_str(), &nodig, 10);
                return number;
            }
            std::string GetString() const {
                ASSERT(value);
                if (!value) return 0;
                return value->GetValue().str();
            }
        private:
            const CBaseValue& GetValue() const { return *value; }
            _token token;
            std::unique_ptr<CBaseValue> value;
        };

        using _ValueMap = std::map<std::string, _Value>;

        CExpParsing() = default;
        ~CExpParsing() = default;
        static const std::string& GetTokenVarDef() { return _token_var_def; }
        bool Parsing(const std::string& exp_str, std::shared_ptr<CLexerObject> symboltable);
        bool AddTokenIdentifier(IDENPAIR& iden_pair) {
            ASSERT(iden_pair.second);
            ASSERT(iden_pair.first.length());
            if (!iden_pair.second) return false;
            if (!iden_pair.first.length()) return false;
            auto& it = m_identifier_symbol.find(iden_pair.first);
            if (it != m_identifier_symbol.end())
                return false;
            m_identifier_symbol[iden_pair.first] = iden_pair.second;
            return true;
        }
        std::shared_ptr<CObObject> GetTokenIdentifier(const std::string& identifier_str) const {
            ASSERT(identifier_str.length());
            if (!identifier_str.length()) return nullptr;
            const auto& iden = m_identifier_symbol.find(identifier_str);
            if (iden == m_identifier_symbol.end()) return nullptr;
            ASSERT(iden->second != nullptr);
            if (!iden->second) return nullptr;
            return iden->second;
        }
        const SYMBOLTABLE& GetIdenifierSymbol() const {
            return m_identifier_symbol;
        }
        bool SetIdentifierValue(const ValuePair& iden_pair);
        std::variant<bool, std::unique_ptr<ValuePair>> EvalExpression();
    private:
        const std::string& GetTokenName(int token) const {
            auto& it = _token_str_def.find((_token)token);
            ASSERT(it != _token_str_def.end());
            if (it == _token_str_def.end())
                return _token_str_def.begin()->second;
            return it->second;
        }
        const _token GetToken(const std::string& token_str) const {
            for (auto& def : _token_str_def)
            {
                if (_stricmp(def.second.c_str(), token_str.c_str()) == 0)
                    return def.first;
            }
            ASSERT(0);
            return token_invalid;
        }
        std::unique_ptr<std::queue<std::unique_ptr<_Value>>> GetQueueExpressionAst() const {
            std::unique_ptr<std::queue<std::unique_ptr<_Value>>> p = std::make_unique<std::queue<std::unique_ptr<_Value>>>();
            ASSERT(p);
            if (!p) return nullptr;
            for (const auto& exp : m_expression_ast)
            {
                std::unique_ptr<_Value> pvalue = std::make_unique<_Value>(*exp);
                ASSERT(pvalue != nullptr);
                if (!pvalue) return nullptr;
                p->push(std::move(pvalue));
            }
            return p;
        }
        static const std::string _token_var_def;
        static const std::map<_token, std::string> _token_str_def;
        static const std::map<std::string, int> _op_precedence;
        SYMBOLTABLE m_identifier_symbol;
        _ValueMap m_value_map;
        std::deque<std::unique_ptr<_Value>> m_expression_ast;
        std::string m_expression_str;
    };

    class CPrototype
    {
    public:
        using PrototypeArgument = IDENPAIR;
        using PrototypeArguments = IDENTIFIERTABLE;

        enum _call_convention {
            call_unknown = 0,
            call_cdecl,
            call_fastcall,
            call_stdcall,
            call_vectorcall,
        };
        CPrototype() = delete;
        CPrototype(_call_convention conv, const std::string& name) : m_call_conv(conv), m_proto_name(name), m_special(false), m_feature(DEFAULT_FEATURE) {}
        CPrototype(_call_convention conv, const std::string& library, const std::string& name, const PrototypeArguments& args, bool special = false) : m_call_conv(conv), m_library(library), m_special(special), m_feature(DEFAULT_FEATURE), m_proto_name(name), m_args(args) {}
        ~CPrototype() = default;

        const std::string& GetName() const { return m_proto_name; }
        _call_convention GetCallConv() const { return m_call_conv; }
        bool GetSpecial() const { return m_special; }
        const std::string& GetLibrary() const { return m_library; }
        const std::string& GetFeature() const { return m_feature; }
        virtual bool AddArgument(const PrototypeArgument& arg) {
            auto it = m_args.find(arg.first);
            if (it == m_args.end())
            {
                int offset = (int)GetArgumentAlignSize();
                ASSERT(!(offset == 0 && m_args.size() != 0));
                ASSERT(arg.second != nullptr);
                m_args[arg.first] = std::pair<int, std::shared_ptr<CObObject>>(offset, arg.second);
                return true;
            }
            return false;
        }
        virtual bool AddReturn(const PrototypeArgument& Return) {
            m_return = Return;
            return true;
        }
        const PrototypeArguments& GetArguments() const { return m_args; }
        const size_t GetArgumentAlignSize() const {
            size_t nalignsize = 0;
            for (const auto& arg : m_args)
            {
                if (arg.second.second)
                {
                    ASSERT(arg.second.second->GetObSize() != 0);
                    nalignsize += [](size_t size, int align) ->size_t { if (size % align == 0) return (size / align) * align; else return (size / align + 1) * align; }(arg.second.second->GetObSize(), stack_aligned_bytes);
                }
            }
            return nalignsize;
        }
        const size_t GetArgumentSize() const {
            size_t ntotalsize = 0;
            for (const auto& arg : m_args)
            {
                if (arg.second.second)
                {
                    ASSERT(arg.second.second->GetObSize() != 0);
                    ntotalsize += arg.second.second->GetObSize();
                }
            }
            return ntotalsize;
        }
        size_t GetArgumentSize(const std::string& param_name) const {
            ASSERT(param_name.length());
            if (!param_name.length()) return 0;
            const auto& it = m_args.find(param_name);
            if (it == m_args.end()) return 0;
            ASSERT(it->second.second != nullptr);
            if (!it->second.second) return 0;
            return it->second.second->GetObSize();
        }
        size_t GetArgumentSize(int param_ord) const {
            std::map<int, std::shared_ptr<CObObject>> sort_args;
            for (const auto& it : m_args)
                sort_args[it.second.first] = it.second.second;
            int count = 0;
            for (const auto& it : sort_args)
            {
                if (count == param_ord)
                {
                    ASSERT(it.second != nullptr);
                    if (!it.second) return 0;
                    return it.second->GetObSize();
                }
                count++;
            }
            return 0;
        }
        int GetArgumentOffset(const std::string& param_name) const {
            if (!param_name.length()) return invalid_arg_offset;
            const auto& it = m_args.find(param_name);
            if (it == m_args.end()) return invalid_arg_offset;
            return it->second.first;
        }
        int GetArgumentOffset(int param_ord) const {
            std::map<int, std::shared_ptr<CObObject>> sort_args;
            for (const auto& it : m_args)
                sort_args[it.second.first] = it.second.second;
            int count = 0;
            for (const auto& it : sort_args)
            {
                if (count == param_ord)
                {
                    ASSERT(it.second != nullptr);
                    if (!it.second) return 0;
                    return it.first;
                }
                count++;
            }
            return 0;
        }
        int GetArgumentOrdinal(const std::string& param_name) const {
            ASSERT(param_name.length());
            if (!param_name.length()) return -1;
            std::map<int, std::string> sort_args;
            for (const auto& it : m_args)
                sort_args[it.second.first] = it.first;
            int count = 0;
            for (const auto& it : sort_args)
            {
                if (_stricmp(it.second.c_str(), param_name.c_str()) == 0)
                {
                    return count;
                }
                count++;
            }
            ASSERT(0);
            return -1;
        }
        const PrototypeArgument& GetReturn() const { return m_return; }
        void SetLibrary(const std::string& library) { m_library = library; }
        void SetFeature(const std::string& feature) { m_feature = feature; }
        void SetSpecial(bool special) { m_special = special; }
        virtual void* codegen() const = 0;
        static const int invalid_arg_offset = -1;
        static const int stack_aligned_bytes = sizeof(ULONG_PTR);
    private:
        _call_convention m_call_conv;
        std::string m_proto_name;
        std::string m_library;
        std::string m_feature;
        PrototypeArguments m_args;
        PrototypeArgument m_return;
        bool m_special;

    };

    class CHandle
    {
    public:
        CHandle() = default;
        ~CHandle() = default;
    private:

    };

    class CEnsure : public CParsingObject
    {
    public:
        CEnsure() = delete;
        CEnsure(std::shared_ptr<CLexerObject> table) : CParsingObject(table) { }
        ~CEnsure() = default;

        bool AddEnsure(const std::string& name) {
            return ParsingIdentifiers(name);
        }
    private:
    };

    class CLog : public CParsingObject
    {
    public:
        CLog() = delete;
        CLog(std::shared_ptr<CLexerObject> table) : CParsingObject(table) { }
        ~CLog() = default;

        bool AddLogging(const std::string& name) {
            return ParsingIdentifiers(name);
        }
    private:
    };

    class CCheck : public CParsingObject
    {
    public:
        using handle_type = enum {
            handle_duplicate = 0,
            handle_addtarget,
        };

        using HANDLEPAIR = std::pair<std::string, handle_type>;
        CCheck() = delete;
        CCheck(std::shared_ptr<CLexerObject> table) : CParsingObject(table) { SetAlias(CExpParsing::GetTokenVarDef()); }
        ~CCheck() = default;
        void AddLog(const IDENPAIR& iden_pair) {
            m_logs.push_back(iden_pair);
        }
        bool AddCheck(const std::string& exp_str) {
            ASSERT(exp_str.length());
            if (!exp_str.length()) return false;
            std::unique_ptr<CExpParsing> check = std::make_unique<CExpParsing>();
            ASSERT(check != nullptr);
            if (check)
            {
                if (check->Parsing(exp_str, GetLocalSymbols()))
                {
                    m_checks.push_back(std::move(check));
                    return true;
                }
            }
            return false;
        }
        bool AddModify(const std::string& exp_str) {
            ASSERT(exp_str.length());
            if (!exp_str.length()) return false;
            std::unique_ptr<CExpParsing> modify = std::make_unique<CExpParsing>();
            ASSERT(modify != nullptr);
            if (modify)
            {
                if (modify->Parsing(exp_str, GetLocalSymbols()))
                {
                    m_modifys.push_back(std::move(modify));
                    return true;
                }
            }
            return false;
        }
        bool AddHandle(const std::pair<std::string, std::string>& handle_pair) {
            HANDLEPAIR pair;
            ASSERT(handle_pair.first.length());
            ASSERT(handle_pair.second.length());
            if (!handle_pair.first.length()) return false;
            if (!handle_pair.second.length()) return false;
            pair.first = handle_pair.first;
            if (_stricmp(handle_pair.second.c_str(), HD_DUPLICATE) == 0)
            {
                pair.second = handle_duplicate;
                m_handles.push_back(pair);
                return true;
            }
            if (_stricmp(handle_pair.second.c_str(), HD_ADDTARGET) == 0)
            {
                pair.second = handle_addtarget;
                m_handles.push_back(pair);
                return true;
            }
            return false;
        }
        const std::vector<std::unique_ptr<CExpParsing>>& GetChecks() const { return m_checks; }
        const std::vector<std::unique_ptr<CExpParsing>>& GetModifys() const { return m_modifys; }
        const std::vector<HANDLEPAIR>& GetHandles() const { return m_handles; }
        const std::vector<IDENPAIR>& GetLogs() const { return m_logs; }
    private:
        std::vector<IDENPAIR> m_logs;
        std::vector<std::unique_ptr<CExpParsing>> m_checks;
        std::vector<HANDLEPAIR> m_handles;
        std::vector<std::unique_ptr<CExpParsing>> m_modifys;
    };

    class CFunction : public CPrototype, public CLexerObject, public std::enable_shared_from_this<CFunction>
    {
    public:
        using func_proto = void(*)(void);

        CFunction() = delete;
        CFunction(_call_convention conv, const std::string& name) : CPrototype(conv, name) {}
        ~CFunction() = default;

        struct _CheckDefine {
            using DefinePair = std::pair<std::string, std::string>;
            using CheckPair = DefinePair;
            using ModifyPair = DefinePair;
            using HandlePair = DefinePair;
            using CheckLog = std::string;
        };
        struct _CheckPackage : _CheckDefine {
            std::vector<CheckLog> logs;
            std::vector<DefinePair> define_pairs;
            std::vector<CheckPair> check_pair;
            std::vector <ModifyPair> modify_pair;
            std::vector<HandlePair> handle_pair;
        };

        virtual bool AddArgument(const PrototypeArgument& arg) {
            if (CPrototype::AddArgument(arg))
                return AddVarIdentifier(arg);
            return false;
        }
        virtual bool AddReturn(const PrototypeArgument& Return) {
            if (CPrototype::AddReturn(Return))
                return AddVarIdentifier(Return);
            return false;
        }
        virtual void* codegen() const override { return nullptr; }

        bool AddHandle(const std::string& name, bool bpre = true) {}
        bool AddEnsure(const std::string& name) {
            ASSERT(name.length());
            if (!m_pensure)
                m_pensure = std::make_unique<CEnsure>(shared_from_this());
            ASSERT(m_pensure != nullptr);
            if (m_pensure)
                return (*m_pensure).AddEnsure(name);
            return false;
        }
        bool AddLogging(const std::string& name, bool bpre = true) {
            ASSERT(name.length());
            if (bpre)
            {
                if (!m_pprelog)
                    m_pprelog = std::make_unique<CLog>(std::static_pointer_cast<CLexerObject>(shared_from_this()));
                ASSERT(m_pprelog != nullptr);
                if (m_pprelog)
                    return (*m_pprelog).AddLogging(name);
                return false;
            }
            else
            {
                if (!m_ppostlog)
                    m_ppostlog = std::make_unique<CLog>(shared_from_this());
                ASSERT(m_ppostlog != nullptr);
                if (m_ppostlog)
                    return (*m_ppostlog).AddLogging(name);
                return false;
            }
        }
        bool AddChecks(const _CheckPackage& package, bool bpre = true);

        std::shared_ptr<CLexerObject> GetEnsure() const {
            if (m_pensure)
                return (*m_pensure).GetLocalSymbols();
            return nullptr;
        }
        std::shared_ptr<CLexerObject> GetLogging(bool bpre = true) const {
            if (bpre)
            {
                if (m_pprelog)
                    return (*m_pprelog).GetLocalSymbols();
            }
            else
            {
                if (m_ppostlog)
                    return (*m_ppostlog).GetLocalSymbols();
            }
            return nullptr;
        }
        const std::vector<std::unique_ptr<CCheck>>& GetChecks(bool bpre = true) const {
            if (bpre)
                return m_pprechecks;
            else
                return m_ppostchecks;
        }
        bool AddPreProcessing(const func_proto func_proc) {
            ASSERT(func_proc != nullptr);
            if (!func_proc) return false;
            for (const auto& it : m_pre_processing)
            {	// the func_proc is already on this list.
                if (it == func_proc)
                    return false;
            }
            m_pre_processing.push_back(func_proc);
            return true;
        }
        bool AddPostProcessing(const func_proto func_proc) {
            ASSERT(func_proc != nullptr);
            if (!func_proc) return false;
            for (const auto& it : m_post_processing)
            {	// the func_proc is already on this list.
                if (it == func_proc)
                    return false;
            }
            m_post_processing.push_back(func_proc);
            return true;
        }
        bool DelPreProcessing(const func_proto func_proc) {
            ASSERT(func_proc != nullptr);
            if (!func_proc) return false;
            for (std::vector<func_proto>::iterator it = m_pre_processing.begin(); it != m_pre_processing.end(); it++)
            {
                if ((*it) == func_proc)
                {
                    m_pre_processing.erase(it);
                    return true;
                }
            }
            // the func_proc didn't on this list.
            return false;
        }
        bool DelPostProcessing(const func_proto func_proc) {
            ASSERT(func_proc != nullptr);
            if (!func_proc) return false;
            for (std::vector<func_proto>::iterator it = m_post_processing.begin(); it != m_post_processing.end(); it++)
            {
                if ((*it) == func_proc)
                {
                    m_post_processing.erase(it);
                    return true;
                }
            }
            // the func_proc didn't on this list.
            return false;
        }
        const std::vector<func_proto>& GetPreProcessing() const { return m_pre_processing; }
        const std::vector<func_proto>& GetPostProcessing() const { return m_post_processing; }

        processing_status CheckReturn(PVOID pnode) const;
        processing_status ProcessingEnsure(PVOID pnode) const;
        processing_status ProcessingLog(PVOID pnode, bool pre = true) const;
        processing_status ProcessingChecks(PVOID pnode, bool pre = true);
        processing_status PreprocessingLog(PVOID pnode) const { return ProcessingLog(pnode); }
        processing_status PreprocessingChecks(PVOID pnode) { return ProcessingChecks(pnode); }
        processing_status PostprocessingLog(PVOID pnode) const { return ProcessingLog(pnode, false); }
        processing_status PostprocessingChecks(PVOID pnode) { return ProcessingChecks(pnode, false); }

        bool checking(PVOID pnode, const std::unique_ptr<CCheck>& pcheck, const std::shared_ptr<CLogHandle>& log_handle) const;
        processing_status modifying(PVOID pnode, const std::unique_ptr<CCheck>& pcheck, const std::shared_ptr<CLogHandle>& log_handle);
        bool handling(PVOID pnode, const std::unique_ptr<CCheck>& pcheck, const std::shared_ptr<CLogHandle>& log_handle) const;
        bool logging(PVOID pnode, const std::unique_ptr<CCheck>& pcheck, const std::shared_ptr<CLogHandle>& log_handle) const;
    private:
        std::unique_ptr<CHandle> m_phandle;
        std::unique_ptr<CEnsure> m_pensure;
        std::unique_ptr<CLog> m_pprelog;
        std::unique_ptr<CLog> m_ppostlog;
        std::vector<std::unique_ptr<CCheck>> m_pprechecks;
        std::vector<std::unique_ptr<CCheck>> m_ppostchecks;
        std::vector<func_proto> m_pre_processing;
        std::vector<func_proto> m_post_processing;
    };

    class CFunctionProtos
    {
    public:
        using FunctionProto = std::pair<std::string, std::shared_ptr<CFunction>>;
        using FunctionProtos = std::map<std::string, std::shared_ptr<CFunction>>;
        using iterator = FunctionProtos::iterator;
        using const_iterator = FunctionProtos::const_iterator;
        CFunctionProtos() = default;
        ~CFunctionProtos() = default;

        bool AddFunction(const std::shared_ptr<CFunction>& function) {
            ASSERT(function != nullptr);
            if (!function) return false;
            auto it = m_functionprototypes.find(function->GetName());
            if (it == m_functionprototypes.end())
            {
                m_functionprototypes[function->GetName()] = function;
                return true;
            }
            return false;
        }
        const std::shared_ptr<CFunction>& GetFunction(const std::string& name) const {
            ASSERT(name.length());
            if (!name.length()) return nullptr;
            auto it = m_functionprototypes.find(name);
            if (it == m_functionprototypes.end())
                return nullptr;
            return (*it).second;
        }
        const FunctionProtos& GetFunctions() const {
            return m_functionprototypes;
        }
        const int GetFunctionCounts() const { return (int)m_functionprototypes.size(); }
        bool AddPreProcessing(const std::string& func_name, const CFunction::func_proto func_proc) {
            ASSERT(func_name.length());
            ASSERT(func_proc != nullptr);
            if (!func_name.length()) return false;
            if (!func_proc) return false;
            const auto& it = m_functionprototypes.find(func_name);
            if (it == m_functionprototypes.end()) return false;
            if (!it->second) return false;
            return it->second->AddPreProcessing(func_proc);
        }
        bool AddPostProcessing(const std::string& func_name, const CFunction::func_proto func_proc) {
            ASSERT(func_name.length());
            ASSERT(func_proc != nullptr);
            if (!func_name.length()) return false;
            if (!func_proc) return false;
            const auto& it = m_functionprototypes.find(func_name);
            if (it == m_functionprototypes.end()) return false;
            if (!it->second) return false;
            return it->second->AddPostProcessing(func_proc);
        }
        bool DelPreProcessing(const std::string& func_name, const CFunction::func_proto func_proc) {
            ASSERT(func_name.length());
            ASSERT(func_proc != nullptr);
            if (!func_name.length()) return false;
            if (!func_proc) return false;
            const auto& it = m_functionprototypes.find(func_name);
            if (it == m_functionprototypes.end()) return false;
            if (!it->second) return false;
            return it->second->DelPreProcessing(func_proc);
        }
        bool DelPostProcessing(const std::string& func_name, const CFunction::func_proto func_proc) {
            ASSERT(func_name.length());
            ASSERT(func_proc != nullptr);
            if (!func_name.length()) return false;
            if (!func_proc) return false;
            const auto& it = m_functionprototypes.find(func_name);
            if (it == m_functionprototypes.end()) return false;
            if (!it->second) return false;
            return it->second->DelPostProcessing(func_proc);
        }

        iterator		begin() { return m_functionprototypes.begin(); }
        const_iterator	begin() const { return m_functionprototypes.begin(); }
        iterator		end() { return m_functionprototypes.end(); }
        const_iterator	end() const { return m_functionprototypes.end(); }

    private:
        FunctionProtos m_functionprototypes;
    };

    class CWmiObject : public CLexerObject, public std::enable_shared_from_this<CWmiObject>
    {
    public:
        using PrototypeData = IDENPAIR;
        using PrototypeDatas = IDENTIFIERTABLE;
        using WmiProto = std::vector<PrototypeData>;
        using iterator = WmiProto::iterator;
        using const_iterator = WmiProto::const_iterator;
        CWmiObject(const std::string& name) { m_name = name; }
        ~CWmiObject() = default;

        const std::string& GetName() const { return m_name; }
        bool CWmiObject::SetMidlInterface(const std::stringstream& ss);
        bool AddData(const PrototypeData& data_pair) {
            ASSERT(data_pair.first.length());
            ASSERT(data_pair.second != nullptr);
            if (!data_pair.first.length()) return false;
            if (!data_pair.second) return false;
            m_datas.push_back(data_pair);
            return AddVarIdentifier(data_pair);
        }
        std::shared_ptr<CFunctionProtos> GetMethods() { if (!m_methods) m_methods = std::make_shared<CFunctionProtos>(); ASSERT(m_methods != nullptr); return m_methods; }
        bool AddChecks(const CFunction::_CheckPackage& package, bool bpre = true);
        const std::vector<std::unique_ptr<CCheck>>& GetChecks(bool bpre = true) const {
            if (bpre)
                return m_pprechecks;
            else
                return m_ppostchecks;
        }

        iterator		begin() { return m_datas.begin(); }
        const_iterator	begin() const { return m_datas.begin(); }
        iterator		end() { return m_datas.end(); }
        const_iterator	end() const { return m_datas.end(); }
    private:
        std::string m_name;
        std::shared_ptr<CMetadataTypeObject> m_midl_interface;
        std::vector<PrototypeData> m_datas;
        std::vector<std::unique_ptr<CCheck>> m_pprechecks;
        std::vector<std::unique_ptr<CCheck>> m_ppostchecks;
        std::shared_ptr<CFunctionProtos> m_methods;
    };

    class CWmiObjectProtos
    {
    public:
        using WmiObjectProto = std::pair<std::string, std::shared_ptr<CWmiObject>>;
        using WmiObjectProtos = std::map<std::string, std::shared_ptr<CWmiObject>>;
        using iterator = WmiObjectProtos::iterator;
        using const_iterator = WmiObjectProtos::const_iterator;
        CWmiObjectProtos() = default;
        ~CWmiObjectProtos() = default;

        bool AddWmiObject(const std::shared_ptr<CWmiObject>& wmi_object) {
            ASSERT(wmi_object != nullptr);
            if (!wmi_object) return false;
            auto it = m_wmiobjectprototypes.find(wmi_object->GetName());
            if (it == m_wmiobjectprototypes.end())
            {
                m_wmiobjectprototypes[wmi_object->GetName()] = wmi_object;
                return true;
            }
            return false;
        }
        const std::shared_ptr<CWmiObject>& GetWmiObject(const std::string& name) const {
            ASSERT(name.length());
            if (!name.length()) return nullptr;
            auto it = m_wmiobjectprototypes.find(name);
            if (it == m_wmiobjectprototypes.end())
                return nullptr;
            return (*it).second;
        }
        const WmiObjectProtos& GetWmiObjects() const {
            return m_wmiobjectprototypes;
        }
        const int GetWmiObjectCounts() const { return (int)m_wmiobjectprototypes.size(); }

        iterator		begin() { return m_wmiobjectprototypes.begin(); }
        const_iterator	begin() const { return m_wmiobjectprototypes.begin(); }
        iterator		end() { return m_wmiobjectprototypes.end(); }
        const_iterator	end() const { return m_wmiobjectprototypes.end(); }

    private:
        WmiObjectProtos m_wmiobjectprototypes;
    };

} // namespace cchips