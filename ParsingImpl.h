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
#include "re2/re2.h"
#include "re2/stringpiece.h"
#include "hookimpl.h"
#include "MetadataTypeImpl.h"
#include "LogObject.h"
#include "UniversalObject.h"

namespace cchips {

#define HD_DUPLICATE "Duplicate"
#define HD_ADDTARGET "AddTarget"

    using VAR_SYMBOLTABLE = TYPE_SYMBOLTABLE;
    using SYMBOLTABLE = TYPE_SYMBOLTABLE;
    using IDENTIFIERTABLE = std::map<std::string, std::pair<int, std::shared_ptr<CObObject>>>;

    class CLexer 
    {
    public:
        enum _token {
            token_invalid = -1,
            token_if = 0,
            token_else,
            token_while,
            token_identifier,
            token_type,
            token_define,
            token_extern,
            token_stringconst,
            token_integerconst,
            token_decimalconst,
            token_parentheses_b,
            token_parentheses_e,
            token_curlybraces_b,
            token_curlybraces_e,
            token_brackets_b,
            token_brackets_e,
            token_asterisk,
            token_arrow,
            token_point,
            token_assign,
            token_s_quotes,
            token_d_quotes,
        };
        CLexer() = default;
        ~CLexer() = default;

    private:
        const static std::map<std::string, CLexer::_token> _token_str_def;
    };

    class CLexerTy
    {
    public:
        enum _token_ty {
            token_n_invalid = -1,
            token_reference_b = 0,
            token_reference_e,
            token_brackets_b,
            token_brackets_e,
        };
        enum _reg_re2_ty {
            re2_ty_invalid = -1,
            re2_ty_pattern = 0,
            re2_ty_normal,
            re2_ty_ref_b,
            re2_ty_ref_e,
            re2_ty_arr,
        };
        static CLexerTy& GetInstance()
        {
            static CLexerTy m_instance;
            return m_instance;
        }
        static std::string GetWordRoot(const std::string_view& iden_str);
        std::shared_ptr<CObObject> GetIdentifier(const std::string_view& iden_str) const;
        bool AddIdentifier(const IDENPAIR& iden_pair) {
            return CTypeSymbolTableObject::GetInstance().AddTypeSymbol(iden_pair);
        }
        size_t GetSymbolTableSize() const {
            return CTypeSymbolTableObject::GetInstance().GetSymbolTableSize();
        }
    private:
        CLexerTy() {
            ASSERT(_pattern_ty_ptr);
            if (_pattern_ty_ptr) _pattern_ty_ptr->ok();
        }
        ~CLexerTy() = default;
        const static std::map<std::string, _token_ty> _token_ty_str_def;
        const static std::string _re2_pattern_ty_def;
        static std::unique_ptr<RE2> _pattern_ty_ptr;
    };

    class CLexerVr
    {
    public:
        enum _token_vr {
            token_n_invalid = -1,
            token_parentheses_b = 0,
            token_parentheses_e,
            token_asterisk,
            token_arrow,
            token_point,
        };
        enum _reg_re2_vr {
            re2_vr_invalid = -1,
            re2_vr_pattern = 0,
        };
        using iterator = SYMBOLTABLE::iterator;
        using const_iterator = SYMBOLTABLE::const_iterator;
        CLexerVr() {
            static std::once_flag lexer_vr_flag;
            ASSERT(_pattern_vr_ptr);
            std::call_once(lexer_vr_flag, [](std::unique_ptr<RE2>& pattern_ptr) {
                if (pattern_ptr) pattern_ptr->ok();
                }, _pattern_vr_ptr);
            CreateVariableSymbolTable();
        }
        ~CLexerVr() = default;
        static std::string GetWordRoot(const std::string_view& iden_str);
        std::shared_ptr<CObObject> GetIdentifier(const std::string_view& iden_str) const;
        bool AddIdentifier(const IDENPAIR& iden_pair) {
            if (!GetVariableSymbolTable()) return false;
            if (GetVariableSymbolTable()->find(iden_pair.first) == GetVariableSymbolTable()->end())
            {
                (*GetVariableSymbolTable())[iden_pair.first] = iden_pair.second;
                return true;
            }
            return false;
        }
        size_t GetSymbolTableSize() const {
            if(GetVariableSymbolTable())
                return GetVariableSymbolTable()->size();
            return 0;
        }
        std::shared_ptr<CObObject> GetSymbolReference(const std::string& vr_name) const {
            if (!vr_name.length()) return nullptr;
            if (!GetVariableSymbolTable()) return nullptr;
            auto& it = GetVariableSymbolTable()->find(vr_name);
            if (it == GetVariableSymbolTable()->end())
                return nullptr;
            return it->second;
        }
        iterator        begin() { return (*m_variablesymboltable).begin(); }
        const_iterator  begin() const { return (*m_variablesymboltable).begin(); }
        iterator        end() { return (*m_variablesymboltable).end(); }
        const_iterator  end() const { return (*m_variablesymboltable).end(); }
    protected:
        bool CreateVariableSymbolTable() { if (m_variablesymboltable = std::make_unique<SYMBOLTABLE>()) return true; else return false; }
    private:
        const std::unique_ptr<SYMBOLTABLE>& GetVariableSymbolTable() const { return m_variablesymboltable; }
        const static std::map<std::string, _token_vr> _token_vr_str_def;
        const static std::string _re2_pattern_vr_def;
        static std::unique_ptr<RE2> _pattern_vr_ptr;
        std::unique_ptr<SYMBOLTABLE> m_variablesymboltable;
    };

    class CParsing
    {
    public:
        using iterator = SYMBOLTABLE::iterator;
        using const_iterator = SYMBOLTABLE::const_iterator;

        CParsing() = delete;
        CParsing(std::shared_ptr<CParsing> parent_table) : m_globalsymboltable(parent_table) { }
        ~CParsing() = default;
        std::shared_ptr<CObObject> GetIdentifier(const std::string_view& iden_str) const {
            ASSERT(iden_str.length());
            if (!iden_str.length()) return nullptr;
            if (m_localsymboltable)
            {
                auto& iden_ptr = (*m_localsymboltable).GetIdentifier(iden_str);
                if (iden_ptr != nullptr)
                    return iden_ptr;
            }
            if (m_globalsymboltable)
            {
                auto& iden_ptr = (*m_globalsymboltable).GetIdentifier(iden_str);
                if (iden_ptr != nullptr)
                    return iden_ptr;
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
            if (!m_localsymboltable) return false;
            return m_localsymboltable->AddIdentifier(iden_pair);
        }
        std::shared_ptr<CLexerVr> GetLocalSymbols() const { return m_localsymboltable; }
        iterator        begin() { return (*m_localsymboltable).begin(); }
        const_iterator  begin() const { return (*m_localsymboltable).begin(); }
        iterator        end() { return (*m_localsymboltable).end(); }
        const_iterator  end() const { return (*m_localsymboltable).end(); }
//    protected:
        void SetAlias(const std::string& alias) {
            m_alias.first = alias;
            m_alias.second = 0;
            return;
        }
    private:
        std::pair<std::string, int> m_alias;
        std::map<std::string, std::string> m_alias_array;
        std::shared_ptr<CParsing> m_globalsymboltable = nullptr;
        std::shared_ptr<CLexerVr> m_localsymboltable = std::make_shared<CLexerVr>();
    };

    class CExpParsing
    {
    public:
        enum _token {
            token_invalid = -1,
            token_op,
            token_iden,
            token_string,
            token_integral,
            token_float_pointer,
        };
        enum _op_type {
            unary_type = 0,
            binary_type,
        };
        using CBaseValue = CMetadataTypeObject;
        using ValuePair = std::pair<std::string, std::any>;

        struct _Value {
            _Value() :token(token_invalid), value(nullptr) {}
            _Value(const _Value& val) { *this = val; }
            _Value(std::string string, _token tok) : token(tok) { 
                value = make_metadata_j_ptr<std::string>(CBaseDef::type_string, {}, CObObject::op_n_equal);
                if (value)
                    value->SetCurValue(string);
            }
            _Value(std::string string) : token(token_string) { 
                value = make_metadata_j_ptr<std::string>(CBaseDef::type_string, {}, CObObject::op_n_equal); 
                if(value)
                    value->SetCurValue(string);
            }
            _Value(ULONGLONG number) : token(token_integral) { 
                value = make_metadata_s_ptr<ULONGLONG>(CBaseDef::type_ulonglong); 
                if (value)
                    value->SetCurValue(number);
            }
            _Value(DOUBLE float_pointer) : token(token_float_pointer) { 
                value = make_metadata_s_ptr<DOUBLE>(CBaseDef::type_double); 
                if (value)
                    value->SetCurValue(float_pointer);
            }

            bool isOp() const { return token == token_op; };
            bool isString() const { return token == token_string; }
            bool isIntegral() const { return token == token_integral; }
            bool isFloat() const { return token == token_float_pointer; }
            bool isIdentifier() const { return token == token_iden; }
            bool isConstant() const { if (isString() || isIntegral() || isFloat()) return true; else return false; }
            bool isValid() const { if (GetToken() == token_invalid) return false; return true; }
            _token GetToken() const { return token; }

            _Value operator + (_Value& rhs) { 
                if (isIntegral() && rhs.isIntegral())
                    return (GetIntegral() + rhs.GetIntegral());
                else if (isFloat() && rhs.isFloat())
                    return (GetFloat() + rhs.GetFloat());
                else if (isString() && rhs.isString())
                    return (GetString() + rhs.GetString()); 
                return {};
            }
            _Value operator * (_Value& rhs) { 
                if (isIntegral() && rhs.isIntegral())
                    return (GetIntegral() * rhs.GetIntegral());
                else if (isFloat() && rhs.isFloat())
                    return (GetFloat() * rhs.GetFloat());
                return {};
            }
            _Value operator - (_Value& rhs) { 
                if (isIntegral() && rhs.isIntegral())
                    return (GetIntegral() - rhs.GetIntegral());
                else if (isFloat() && rhs.isFloat())
                    return (GetFloat() - rhs.GetFloat());
                return {};
            }
            _Value operator / (_Value& rhs) { 
                if (isIntegral() && rhs.isIntegral()) {
                    if (rhs.GetIntegral() == 0)
                        return (ULONGLONG)0;
                    else
                        return (GetIntegral() / rhs.GetIntegral());
                }
                else if (isFloat() && rhs.isFloat()) {
                    if (rhs.GetFloat() == 0.0)
                        return 0.0;
                    else
                        return (GetFloat() / rhs.GetFloat());
                }
                return {};
            }
            _Value operator << (_Value& rhs) { 
                if (isIntegral() && rhs.isIntegral())
                    return (GetIntegral() << rhs.GetIntegral());
                return {};
            }
            _Value operator ^ (_Value& rhs) { 
                if (isIntegral() && rhs.isIntegral())
                    return ((ULONGLONG)pow(GetIntegral(), rhs.GetIntegral()));
                return {};
            }
            _Value operator >> (_Value& rhs) { 
                if (isIntegral() && rhs.isIntegral())
                    return (GetIntegral() >> rhs.GetIntegral());
                return {};
            }
            _Value operator > (_Value& rhs) { 
                if (isIntegral() && rhs.isIntegral())
                    return (ULONGLONG)(GetIntegral() > rhs.GetIntegral());
                else if (isFloat() && rhs.isFloat())
                    return (ULONGLONG)(GetFloat() > rhs.GetFloat());
                else if (isString() && rhs.isString())
                    return (ULONGLONG)(GetString() > rhs.GetString());
                else if (isString() && rhs.isIntegral())
                {
                    char* end;
                    ULONGLONG l = strtoull(GetString().c_str(), &end, 10);
                    return (ULONGLONG)(l > rhs.GetIntegral());
                }
                else if (isIntegral() && rhs.isString())
                {
                    char* end;
                    ULONGLONG r = strtoull(rhs.GetString().c_str(), &end, 10);
                    return (ULONGLONG)(GetIntegral() > r);
                }
                return {};
            }
            _Value operator >= (_Value& rhs) { 
                if (isIntegral() && rhs.isIntegral())
                    return (ULONGLONG)(GetIntegral() >= rhs.GetIntegral());
                else if (isFloat() && rhs.isFloat())
                    return (ULONGLONG)(GetFloat() >= rhs.GetFloat());
                else if (isString() && rhs.isString())
                    return (ULONGLONG)(GetString() >= rhs.GetString());
                else if (isString() && rhs.isIntegral())
                {
                    char* end;
                    ULONGLONG l = strtoull(GetString().c_str(), &end, 10);
                    return (ULONGLONG)(l >= rhs.GetIntegral());
                }
                else if (isIntegral() && rhs.isString())
                {
                    char* end;
                    ULONGLONG r = strtoull(rhs.GetString().c_str(), &end, 10);
                    return (ULONGLONG)(GetIntegral() >= r);
                }
                return {};
            }
            _Value operator < (_Value& rhs) {
                if (isIntegral() && rhs.isIntegral())
                    return (ULONGLONG)(GetIntegral() < rhs.GetIntegral());
                else if (isFloat() && rhs.isFloat())
                    return (ULONGLONG)(GetFloat() < rhs.GetFloat());
                else if (isString() && rhs.isString())
                    return (ULONGLONG)(GetString() < rhs.GetString());
                else if (isString() && rhs.isIntegral())
                {
                    char* end;
                    ULONGLONG l = strtoull(GetString().c_str(), &end, 10);
                    return (ULONGLONG)(l < rhs.GetIntegral());
                }
                else if (isIntegral() && rhs.isString())
                {
                    char* end;
                    ULONGLONG r = strtoull(rhs.GetString().c_str(), &end, 10);
                    return (ULONGLONG)(GetIntegral() <= r);
                }
                return {};
            }
            _Value operator <= (_Value& rhs) { 
                if (isIntegral() && rhs.isIntegral())
                    return (ULONGLONG)(GetIntegral() <= rhs.GetIntegral());
                else if (isFloat() && rhs.isFloat())
                    return (ULONGLONG)(GetFloat() <= rhs.GetFloat());
                else if (isString() && rhs.isString())
                    return (ULONGLONG)(GetString() <= rhs.GetString());
                else if (isString() && rhs.isIntegral())
                {
                    char* end;
                    ULONGLONG l = strtoull(GetString().c_str(), &end, 10);
                    return (ULONGLONG)(l <= rhs.GetIntegral());
                }
                else if (isIntegral() && rhs.isString())
                {
                    char* end;
                    ULONGLONG r = strtoull(rhs.GetString().c_str(), &end, 10);
                    return (ULONGLONG)(GetIntegral() <= r);
                }
                return {};
            }
            _Value operator & (_Value& rhs) { 
                if (isIntegral() && rhs.isIntegral())
                    return (ULONGLONG)(GetIntegral() & rhs.GetIntegral());
                return {};
            }
            _Value operator | (_Value& rhs) { 
                if (isIntegral() && rhs.isIntegral())
                    return (ULONGLONG)(GetIntegral() | rhs.GetIntegral());
                return {};
            }
            _Value operator && (_Value& rhs) { 
                if (isIntegral() && rhs.isIntegral())
                    return (ULONGLONG)(GetIntegral() && rhs.GetIntegral());
                else if (isFloat() && rhs.isFloat())
                    return (ULONGLONG)(GetFloat() && rhs.GetFloat());
                return {};
            }
            _Value operator || (_Value& rhs) { 
                if (isIntegral() && rhs.isIntegral())
                    return (ULONGLONG)(GetIntegral() || rhs.GetIntegral());
                else if (isFloat() && rhs.isFloat())
                    return (ULONGLONG)(GetFloat() || rhs.GetFloat());
                return {};
            }
            _Value operator == (_Value& rhs) { 
                if (isIntegral() && rhs.isIntegral())
                    return (ULONGLONG)(GetIntegral() == rhs.GetIntegral());
                else if (isFloat() && rhs.isFloat())
                    return (ULONGLONG)(GetFloat() == rhs.GetFloat());
                else if (isString() && rhs.isString())
                    return (ULONGLONG)(GetString() == rhs.GetString());
                else if (isString() && rhs.isIntegral())
                {
                    char* end;
                    ULONGLONG l = strtoull(GetString().c_str(), &end, 10);
                    return (ULONGLONG)(l == rhs.GetIntegral());
                }
                else if (isIntegral() && rhs.isString())
                {
                    char* end;
                    ULONGLONG r = strtoull(rhs.GetString().c_str(), &end, 10);
                    return (ULONGLONG)(GetIntegral() == r);
                }
                return {};
            }
            _Value operator != (_Value& rhs) { 
                if (isIntegral() && rhs.isIntegral())
                    return (ULONGLONG)(GetIntegral() != rhs.GetIntegral());
                else if (isFloat() && rhs.isFloat())
                    return (ULONGLONG)(GetFloat() != rhs.GetFloat());
                else if (isString() && rhs.isString())
                    return (ULONGLONG)(GetString() != rhs.GetString());
                else if (isString() && rhs.isIntegral())
                {
                    char* end;
                    ULONGLONG l = strtoull(GetString().c_str(), &end, 10);
                    return (ULONGLONG)(l != rhs.GetIntegral());
                }
                else if (isIntegral() && rhs.isString())
                {
                    char* end;
                    ULONGLONG r = strtoull(rhs.GetString().c_str(), &end, 10);
                    return (ULONGLONG)(GetIntegral() != r);
                }
                return {};
            }
            _Value operator ! () {
                if (isIntegral())
                    return (ULONGLONG)(!GetIntegral());
                else if (isFloat())
                    return (DOUBLE)(!GetFloat());
                return {};
            }

            const _Value& operator = (const _Value& val) {
                token = token_invalid;
                if (val.isIntegral())
                    value = make_metadata_s_ptr<ULONGLONG>(CBaseDef::type_ulonglong);
                else if(val.isFloat())
                    value = make_metadata_s_ptr<DOUBLE>(CBaseDef::type_double);
                else
                    value = make_metadata_j_ptr<std::string>(CBaseDef::type_string, {}, CObObject::op_n_equal);
                ASSERT(value);
                value->SetCurValue(val.value->GetCurValue());
                token = val.GetToken();
                return *this;
            }
            ULONGLONG GetIntegral() const {
                ASSERT(value);
                if (!value) return 0;
                if (!isIntegral()) return 0;
                std::any anyvalue = value->GetCurValue();
                if (anyvalue.has_value() && anyvalue.type() == typeid(ULONGLONG))
                    return std::any_cast<ULONGLONG>(anyvalue);
                return 0;
            }
            DOUBLE GetFloat() const {
                ASSERT(value);
                if (!value) return 0.0;
                if (!isFloat()) return 0.0;
                std::any anyvalue = value->GetCurValue();
                if (anyvalue.has_value() && anyvalue.type() == typeid(DOUBLE))
                    return std::any_cast<DOUBLE>(anyvalue);
                return 0.0;
            }
            std::string GetString() const {
                ASSERT(value);
                if (!value) return {};
                std::any anyvalue = value->GetCurValue();
                if (anyvalue.has_value() && anyvalue.type() == typeid(std::string))
                    return std::any_cast<std::string>(anyvalue);
                return {};
            }
            std::any GetAnyValue() const {
                ASSERT(value);
                if (!value) return {};
                return value->GetCurValue();
            }
        private:
            const CBaseValue& GetCurValue() const { return *value; }
            _token token;
            std::shared_ptr<CBaseValue> value;
        };

        using _ValueMap = std::map<std::string, _Value>;

        CExpParsing() {
            static std::once_flag exp_parsing_flag;
            ASSERT(_pattern_exp_ptr);
            std::call_once(exp_parsing_flag, [](std::unique_ptr<RE2>& pattern_ptr) {
                if (pattern_ptr) pattern_ptr->ok();
                }, _pattern_exp_ptr);
        };
        ~CExpParsing() = default;
        static const std::string& GetTokenVarDef() { return _token_var_def; }
        bool Parsing(const std::string& exp_str, std::shared_ptr<CParsing> symboltable);
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
        bool SetIdentifierValue(_ValueMap& value_map, const ValuePair& iden_pair);
        _Value Calculate(std::string& op, _Value& rhs, _Value& lhs);
        std::variant<ULONGLONG, std::unique_ptr<ValuePair>> EvalExpression(const _ValueMap& value_map);
        //test
        void Clear() {
            m_identifier_symbol.clear();
            m_expression_ast.clear();
            m_expression_str.clear();
        }
        const std::deque<std::shared_ptr<_Value>>& GetExpressionAst() const {
            return m_expression_ast;
        }
        const SYMBOLTABLE GetIdentifierSymbol() const {
            return m_identifier_symbol;
        }
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
        static const std::map<std::string, int> _op_type_def;
        const static std::string _re2_pattern_exp_def;
        static std::unique_ptr<RE2> _pattern_exp_ptr;
        SYMBOLTABLE m_identifier_symbol;
        std::deque<std::shared_ptr<_Value>> m_expression_ast;
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
        enum _class_type {
            class_unknown = -1,
            class_normal = 0,
            class_wmi,
            class_vbs,
            class_ps,
            class_kvm,
            class_cshape,
            class_inline,
            class_max,
        };
        struct ClassProto {
            ClassProto() = delete;
            ClassProto(_class_type t, std::string_view name, std::string_view api, unsigned int idx) :type(t), class_name(name), delay_api(api), vtbl_idx(idx) { ; }
            ClassProto(_class_type t, std::string_view name, std::string_view bm) :type(t), class_name(name), bitmap(bm) { ; }
            _class_type GetClassType() const { return type; }
            std::string GetTypeName() const {
                for (const auto& it : _class_predefine)
                {
                    if (type == it.second)
                        return it.first;
                }

                return std::string("");
            }
            const std::string& GetClassNam() const { return class_name; }
            const std::string& GetDelayApi() const { return delay_api; }
            unsigned int GetVtblIdx() const { return vtbl_idx; }
            std::string GetBitmap() const { return bitmap; }
            std::vector<unsigned char> GetVBitmap() const {
                if (!bitmap.length() || bitmap.length() % 2 == 1) return {};
                std::vector<unsigned char> bytes;
                bytes.resize(bitmap.length() / 2);
                hexfromstring(&bytes[0], bytes.size(), &bitmap[0], bitmap.length() / 2);
                if (!bytes.size()) return {};
                return bytes;
            }
            _class_type type;
            std::string class_name;
            std::string delay_api;
            std::string bitmap;
            unsigned int vtbl_idx;
        };
        struct HooksProto {
            HooksProto() = delete;
            HooksProto(std::string_view platform, std::string_view mode, std::string_view category) :os_platform(platform), process_mode(mode), process_category(category) { ; }
            bool MatchHooks(std::unique_ptr<CategoryObject>& category_ob) const {
                if (!category_ob) return false;
                if (os_platform.length()) {
                    if (!CommonFuncsObject::IsMatchCurrentOS(os_platform))
                        return false;
                }
                if (process_mode.length()) {
                    if (!CommonFuncsObject::IsMatchProcType(process_mode))
                        return false;
                }
                if (process_category.length()) {
                    if (!category_ob->IsMatchCategory(process_category))
                        return false;
                }
                return true;
            }
            const std::string& GetOsPlatform() const { return os_platform; }
            const std::string& GetProcessMode() const { return process_mode; }
            const std::string& GetProcessCategory() const { return process_category; }
            std::string os_platform;
            std::string process_mode;
            std::string process_category;
        };
        CPrototype() = delete;
        CPrototype(_call_convention conv, const std::string& name) : m_call_conv(conv), m_proto_name(name), m_special(false), m_feature(DEFAULT_FEATURE) { m_class_proto = nullptr; m_hooks_proto = nullptr; }
        CPrototype(_call_convention conv, const std::string& library, const std::string& name, const PrototypeArguments& args, bool special = false) : m_call_conv(conv), m_library(library), m_special(special), m_feature(DEFAULT_FEATURE), m_proto_name(name), m_args(args) { m_class_proto = nullptr; m_hooks_proto = nullptr; }
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
        const std::unique_ptr<ClassProto>& GetClassProto() const { return m_class_proto; }
        const std::unique_ptr<HooksProto>& GetHooksProto() const { return m_hooks_proto; }
        void ClearHooksProto() { m_hooks_proto = nullptr; }
        const PrototypeArgument& GetReturn() const { return m_return; }
        void SetLibrary(const std::string_view& library) { m_library = library; }
        void SetFeature(const std::string_view& feature) { m_feature = feature; }
        void SetSpecial(bool special) { m_special = special; }
        void SetClassProto(std::unique_ptr<ClassProto> class_proto) { 
            if (class_proto == nullptr) return;
            if (class_proto->type > class_unknown && 
                class_proto->type < class_max &&
                class_proto->class_name.length())
            {
                m_class_proto = std::move(class_proto);
            }
        }
        void SetHooksProto(std::unique_ptr<HooksProto> hooks_proto) {
            if (hooks_proto == nullptr) return;
            m_hooks_proto = std::move(hooks_proto);
        }
        virtual void* codegen() const = 0;
        static _class_type GetClassType(std::string_view class_type) {
            ASSERT(class_type.length());
            if (!class_type.length()) return class_normal;
            auto it = _class_predefine.find(std::string(class_type));
            if (it != _class_predefine.end())
                return it->second;
            return class_normal;
        }
        _class_type GetClassType() const {
            if (!m_class_proto) return class_normal;
            return m_class_proto->type;
        }
        static const int invalid_arg_offset = -1;
        static const int stack_aligned_bytes = sizeof(ULONG_PTR);
    private:
        _call_convention m_call_conv;
        std::string m_proto_name;
        std::string m_library;
        std::string m_feature;
        std::unique_ptr<ClassProto> m_class_proto;
        std::unique_ptr<HooksProto> m_hooks_proto;
        PrototypeArguments m_args;
        PrototypeArgument m_return;
        bool m_special;
        static const std::map<std::string, _class_type> _class_predefine;
    };

    class CHandle
    {
    public:
        CHandle() = default;
        ~CHandle() = default;
    private:

    };

    class CEnsure : public CParsing
    {
    public:
        CEnsure() = delete;
        CEnsure(std::shared_ptr<CParsing> table) : CParsing(table) { }
        ~CEnsure() = default;

        bool AddEnsure(const std::string_view& name) {
            if (!name.length()) return false;
            std::shared_ptr<CObObject> ob_ptr = GetIdentifier(name);
            if (!ob_ptr) return false;
            return AddIdentifier(IDENPAIR(name, ob_ptr));
        }
    private:
    };

    class CLog : public CParsing
    {
    public:
        CLog() = delete;
        CLog(std::shared_ptr<CParsing> table) : CParsing(table) { }
        ~CLog() = default;

        bool AddLogging(const std::string_view& name) {
            if (!name.length()) return false;
            std::shared_ptr<CObObject> ob_ptr = GetIdentifier(name);
            if (!ob_ptr) return false;
            if (ob_ptr->IsFlag()) {
                std::shared_ptr<CTupleObject> tuple_ptr = std::make_shared<CTupleObject>(name);
                if (!tuple_ptr) return false;
                CTupleObject::_tuple_elem elem{};
                elem.ptr = ob_ptr;
                elem.elem_name = ob_ptr->GetName();
                elem.flag.tuple_flg = CTupleObject::_tuple_flg::tuple_flg_str;
                tuple_ptr->AddElement(elem);
                ob_ptr = tuple_ptr;
            }
            return AddIdentifier(IDENPAIR(name, ob_ptr));
        }
    private:
    };

    class CCheck : public CParsing, public std::enable_shared_from_this<CCheck>
    {
    public:
        using handle_type = enum {
            handle_duplicate = 0,
            handle_addtarget,
        };

        using HANDLEPAIR = std::pair<std::string, handle_type>;
        CCheck() = delete;
        CCheck(std::shared_ptr<CParsing> table) : CParsing(table) { SetAlias(CExpParsing::GetTokenVarDef()); }
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
                if (check->Parsing(exp_str, shared_from_this()))
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
                if (modify->Parsing(exp_str, shared_from_this()))
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

    class CFunction : public CPrototype, public CParsing, public std::enable_shared_from_this<CFunction>
    {
    public:
        using func_proto = void(*)(void);

        CFunction() = delete;
        CFunction(_call_convention conv, const std::string& name) : CPrototype(conv, name), CParsing(nullptr) {}
        ~CFunction() = default;

        struct _CheckDefine {
            using DefinePair = std::pair<std::string, std::string>;
            using CheckPair = DefinePair;
            using ModifyPair = DefinePair;
            using HandlePair = DefinePair;
            using CheckLog = std::string;
            using CCode = std::vector<std::string>;
        };
        struct _CheckPackage : _CheckDefine {
            std::vector<CheckLog> logs;
            std::vector<DefinePair> define_pairs;
            std::vector<CCode> ccodes;
            std::vector<CheckPair> check_pair;
            std::vector <ModifyPair> modify_pair;
            std::vector<HandlePair> handle_pair;
        };

        virtual bool AddArgument(const PrototypeArgument& arg) {
            if (CPrototype::AddArgument(arg))
                return AddIdentifier(arg);
            return false;
        }
        virtual bool AddReturn(const PrototypeArgument& Return) {
            if (CPrototype::AddReturn(Return))
                return AddIdentifier(Return);
            return false;
        }
        virtual void* codegen() const override { return nullptr; }

        bool AddHandle(const std::string_view& name, bool bpre = true) {}
        bool AddEnsure(const std::string_view& name) {
            ASSERT(name.length());
            if (!m_pensure)
                m_pensure = std::make_unique<CEnsure>(shared_from_this());
            ASSERT(m_pensure != nullptr);
            if (m_pensure)
                return (*m_pensure).AddEnsure(name);
            return false;
        }
        bool AddLogging(const std::string_view& name, bool bpre = true) {
            ASSERT(name.length());
            if (bpre)
            {
                if (!m_pprelog)
                    m_pprelog = std::make_unique<CLog>(std::static_pointer_cast<CParsing>(shared_from_this()));
                ASSERT(m_pprelog != nullptr);
                if (m_pprelog)
                    return (*m_pprelog).AddLogging(name);
                return false;
            }
            else
            {
                if (!m_ppostlog)
                    m_ppostlog = std::make_unique<CLog>(std::static_pointer_cast<CParsing>(shared_from_this()));
                ASSERT(m_ppostlog != nullptr);
                if (m_ppostlog)
                    return (*m_ppostlog).AddLogging(name);
                return false;
            }
        }
        bool AddChecks(const _CheckPackage& package, bool bpre = true);

        std::shared_ptr<CLexerVr> GetEnsure() const {
            if (m_pensure)
                return (*m_pensure).GetLocalSymbols();
            return nullptr;
        }
        std::shared_ptr<CLexerVr> GetLogging(bool bpre = true) const {
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
        const std::vector<std::shared_ptr<CCheck>>& GetChecks(bool bpre = true) const {
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

        bool checking(PVOID pnode, const std::shared_ptr<CCheck>& pcheck, const std::shared_ptr<CLogHandle>& log_handle) const;
        processing_status modifying(PVOID pnode, const std::shared_ptr<CCheck>& pcheck, const std::shared_ptr<CLogHandle>& log_handle);
        bool handling(PVOID pnode, const std::shared_ptr<CCheck>& pcheck, const std::shared_ptr<CLogHandle>& log_handle) const;
        bool logging(PVOID pnode, const std::shared_ptr<CCheck>& pcheck, const std::shared_ptr<CLogHandle>& log_handle) const;
    private:
        std::unique_ptr<CHandle> m_phandle;
        std::unique_ptr<CEnsure> m_pensure;
        std::unique_ptr<CLog> m_pprelog;
        std::unique_ptr<CLog> m_ppostlog;
        std::vector<std::shared_ptr<CCheck>> m_pprechecks;
        std::vector<std::shared_ptr<CCheck>> m_ppostchecks;
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

        bool AddFunction(std::shared_ptr<CFunction> function) {
            ASSERT(function != nullptr);
            if (!function) return false;
            auto it = m_functionprototypes.find(function->GetName());
            if (it == m_functionprototypes.end())
            {
                m_functionprototypes[function->GetName()] = std::move(function);
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

    class CWmiObject : public CParsing, public std::enable_shared_from_this<CWmiObject>
    {
    public:
        using PrototypeData = IDENPAIR;
        using PrototypeDatas = IDENTIFIERTABLE;
        using WmiProto = std::vector<PrototypeData>;
        using iterator = WmiProto::iterator;
        using const_iterator = WmiProto::const_iterator;
        CWmiObject() = delete;
        CWmiObject(const std::string& name) : CParsing(nullptr) { m_name = name; }
        ~CWmiObject() = default;

        const std::string& GetName() const { return m_name; }
        bool CWmiObject::SetMidlInterface(const std::stringstream& ss);
        bool AddData(const PrototypeData& data_pair) {
            ASSERT(data_pair.first.length());
            ASSERT(data_pair.second != nullptr);
            if (!data_pair.first.length()) return false;
            if (!data_pair.second) return false;
            m_datas.push_back(data_pair);
            return AddIdentifier(data_pair);
        }
        std::shared_ptr<CFunctionProtos> GetMethods() { if (!m_methods) m_methods = std::make_shared<CFunctionProtos>(); ASSERT(m_methods != nullptr); return m_methods; }
        bool AddChecks(const CFunction::_CheckPackage& package, bool bpre = true);
        const std::vector<std::shared_ptr<CCheck>>& GetChecks(bool bpre = true) const {
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
        std::vector<std::shared_ptr<CCheck>> m_pprechecks;
        std::vector<std::shared_ptr<CCheck>> m_ppostchecks;
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