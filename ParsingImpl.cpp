//===- ParsingImpl.cpp - metadata type implement define -------------------*- C++ -*-===//
// 
// This file implements Lexer class and parser class code.
//
//===--------------------------------------------------------------------------------===//
#include "stdafx.h"
#include "ParsingImpl.h"
#include "HipsCfgObject.h"
#include "HookImplementObject.h"

namespace cchips {

    const std::map<CExpParsing::_token, std::string> CExpParsing::_token_str_def = {
        {token_invalid, "invalid"}, {token_def, "def"}, {token_extern, "extern"}, {token_string, "string"}, {token_number, "number"},
    };
    const std::map<std::string, int> CExpParsing::_op_precedence = {
        {"(", -10}, {"&&", -2}, {"||", -3}, {">", -1}, {">=", -1}, {"<", -1}, {"<=", -1}, {"==", -1}, {"!=", -1}, {"=~", -1}, {"!~", -1}, {"<<", 1}, {">>", 1}, {"+", 2}, {"-", 2},
        {"*", 3}, {"/", 3}, {"^", 4}, {"!", 5}, {"=", -5},
    };
    const std::string CExpParsing::_token_var_def = "%var";

    std::shared_ptr<CObObject> CLexerObject::GetVarIdentifier(const std::string& identifier_str) const
    {
        return GetIdentifier(identifier_str);
    }

    std::shared_ptr<CObObject> CLexerObject::GetTyIdentifier(const std::string& identifier_str) const
    {
        return GetIdentifier(identifier_str, symbol_type);
    }

    bool CLexerObject::AddVarIdentifier(const IDENPAIR& iden_pair)
    {
        return AddIdentifier(iden_pair);
    }

    bool CLexerObject::AddTyIdentifier(const IDENPAIR& iden_pair)
    {
        return AddIdentifier(iden_pair, symbol_type);
    }

    std::string CLexerObject::GetWordRoot(const std::string& identifier_str) const
    {
        ASSERT(identifier_str.length());
        if (!identifier_str.length()) return "";
        std::string root_str = identifier_str;
        size_t length = identifier_str.length() - 1;
        if ((identifier_str[0] == '(') && (identifier_str[length] == ')'))
            return GetWordRoot(identifier_str.substr(1, length - 1));
        else if (identifier_str[0] == '[')
        {
            size_t pos = identifier_str.find(']');
            if (pos == -1 || pos <= 1)
            {
                ASSERT(0);
                error_log("GetWordRoot Fatal error: array {} define error!", identifier_str);
                return "";
            }
            return GetWordRoot(identifier_str.substr(pos + 1, length));
        }
        else if (identifier_str[length] == ']')
        {
            size_t pos = identifier_str.rfind('[');
            if (pos == -1 || pos > (length - 1))
            {
                ASSERT(0);
                error_log("GetWordRoot Fatal error: array {} define error!", identifier_str);
                return "";
            }
            return GetWordRoot(identifier_str.substr(0, pos));
        }
        else if (identifier_str[0] == 'L' && identifier_str[1] == 'P')
            return GetWordRoot(identifier_str.substr(2));
        else if (identifier_str[0] == '*')
            return GetWordRoot(identifier_str.substr(1));
        else if (identifier_str[length] == '*')
            return GetWordRoot(identifier_str.substr(0, length));
        return root_str;
    }

    std::shared_ptr<CObObject> CLexerObject::GetIdentifier(const std::string& identifier_str, _symbol symbol) const
    {
        ASSERT(identifier_str.length());
        if (!identifier_str.length()) return nullptr;
        std::shared_ptr<CObObject> id_ptr;
        std::string root_str;
        if ((id_ptr = GetSymbol(identifier_str, symbol)) != nullptr)
            return id_ptr;

        size_t length = identifier_str.length() - 1;
        if ((identifier_str[0] == '(') && (identifier_str[length] == ')'))
        {
            root_str = identifier_str.substr(1, length - 1);
            id_ptr = GetIdentifier(root_str, symbol);
            return id_ptr;
        }
        else if (identifier_str[0] == '[')
        {
            size_t pos = identifier_str.find(']');
            if (pos == -1 || pos <= 1)
            {
                ASSERT(0);
                error_log("GetIdentifier Fatal error: array {} define error!", identifier_str);
                return false;
            }
            int dim = atoi(identifier_str.substr(1, pos - 1).c_str());
            root_str = identifier_str.substr(pos + 1, length);
            if ((id_ptr = GetSymbol(root_str, symbol)) != nullptr || (id_ptr = GetIdentifier(root_str, symbol)) != nullptr)
            {
                std::shared_ptr<CArrayObject> parent_ptr = std::make_shared<CArrayObject>(identifier_str);
                if (!parent_ptr) return nullptr;
                parent_ptr->AddArray(id_ptr, dim);
                return parent_ptr;
            }
        }
        else if (identifier_str[length] == ']')
        {
            size_t pos = identifier_str.rfind('[');
            if (pos == -1 || pos > (length - 1))
            {
                ASSERT(0);
                error_log("GetIdentifier Fatal error: array {} define error!", identifier_str);
                return false;
            }
            int dim = atoi(identifier_str.substr(pos + 1, length - 1).c_str());
            root_str = identifier_str.substr(0, pos);
            if ((id_ptr = GetSymbol(root_str, symbol)) != nullptr || (id_ptr = GetIdentifier(root_str, symbol)) != nullptr)
            {
                std::shared_ptr<CArrayObject> parent_ptr = std::make_shared<CArrayObject>(identifier_str);
                ASSERT(parent_ptr != nullptr);
                if (!parent_ptr) return nullptr;
                parent_ptr->AddArray(id_ptr, dim);
                return parent_ptr;
            }
        }
        else if (identifier_str[0] == 'L' && identifier_str[1] == 'P')
        {
            root_str = identifier_str.substr(2);
            if ((id_ptr = GetSymbol(root_str, symbol)) != nullptr || (id_ptr = GetIdentifier(root_str, symbol)) != nullptr)
            {
                std::shared_ptr<CReferenceObject> parent_ptr = std::make_shared<CReferenceObject>(identifier_str);
                ASSERT(parent_ptr != nullptr);
                if (!parent_ptr) return nullptr;
                parent_ptr->AddReference(id_ptr);
                return parent_ptr;
            }
        }
        else if (identifier_str[0] == '*')
        {
            root_str = identifier_str.substr(1);
            if ((id_ptr = GetSymbol(root_str, symbol)) != nullptr || (id_ptr = GetIdentifier(root_str, symbol)) != nullptr)
            {
                std::shared_ptr<CReferenceObject> parent_ptr = std::make_shared<CReferenceObject>(identifier_str);
                ASSERT(parent_ptr != nullptr);
                if (!parent_ptr) return nullptr;
                parent_ptr->AddReference(id_ptr);
                return parent_ptr;
            }
        }
        else if (identifier_str[length] == '*')
        {
            root_str = identifier_str.substr(0, length);
            if ((id_ptr = GetSymbol(root_str, symbol)) != nullptr || (id_ptr = GetIdentifier(root_str, symbol)) != nullptr)
            {
                std::shared_ptr<CReferenceObject> parent_ptr = std::make_shared<CReferenceObject>(identifier_str);
                ASSERT(parent_ptr != nullptr);
                if (!parent_ptr) return nullptr;
                parent_ptr->AddReference(id_ptr);
                return parent_ptr;
            }
        }
        return nullptr;
    }

    std::shared_ptr<CObObject> CLexerObject::GetSymbol(const std::string& sym_str, _symbol symbol) const
    {
        ASSERT(sym_str.length());
        if (!sym_str.length()) return nullptr;
        if (symbol == symbol_variable && m_variablesymbols)
        {
            auto& it = (*m_variablesymbols).find(sym_str);
            if (it != (*m_variablesymbols).end())
                return (*it).second;
        }
        if (symbol == symbol_type && g_typesymbolobject)
        {
            auto& it = (*g_typesymbolobject).GetTypeSymbolRefrence(sym_str);
            if (it != nullptr)
                return it;
        }
        return nullptr;
    }

    bool CLexerObject::AddIdentifier(const IDENPAIR& iden_pair, _symbol symbol)
    {
        ASSERT(iden_pair.first.length());
        ASSERT(iden_pair.second != nullptr);
        if (symbol == symbol_variable && m_variablesymbols)
        {
            auto& it = (*m_variablesymbols).find(iden_pair.first);
            if (it != (*m_variablesymbols).end())
                return false;
            (*m_variablesymbols)[iden_pair.first] = iden_pair.second;
            return true;
        }
        else if (symbol == symbol_type && g_typesymbolobject)
        {
            return (*g_typesymbolobject).AddTypeSymbol(iden_pair);
        }
        return false;
    }

    bool CParsingObject::ParsingIdentifiers(const std::string& identifiers)
    {
        ASSERT(identifiers.length());
        if (!identifiers.length()) return false;
        std::string iden_str = identifiers;

        if (!m_globalsymbols) return false;

        size_t pos = identifiers.find('|');
        if (pos != std::string::npos)
        {
            if (ParsingIdentifiers(identifiers.substr(0, pos)))
                return ParsingIdentifiers(identifiers.substr(pos + 1));
            else return false;
        }

        auto func = std::make_shared<std::unique_ptr<std::function<std::shared_ptr<CObObject>(std::shared_ptr<CLexerObject>, std::shared_ptr<CObObject>, std::string)>>>();
        *func = std::make_unique<std::function<std::shared_ptr<CObObject>(std::shared_ptr<CLexerObject>, std::shared_ptr<CObObject>, std::string)>>
            ([=](std::shared_ptr<CLexerObject> symbols, std::shared_ptr<CObObject> iden_ptr, std::string& identifiers) -> std::shared_ptr<CObObject>
                {
                    size_t pos = identifiers.find_first_of("*.-");
                    if (pos == std::string::npos)
                    {
                        std::shared_ptr<CObObject> ob_ptr = nullptr;
                        if (!iden_ptr)
                        {
                            ob_ptr = std::static_pointer_cast<CObObject>(symbols->GetVarIdentifier(identifiers));
                        }
                        else if (iden_ptr->IsStruct())
                        {
                            std::shared_ptr<CStructObject> struct_ptr = std::static_pointer_cast<CStructObject>(iden_ptr);
                            ob_ptr = struct_ptr->GetElement(identifiers);
                        }
                        else ob_ptr = iden_ptr;
                        if (!ob_ptr || (!ob_ptr->IsBaseTy() && !ob_ptr->IsReference() && !ob_ptr->IsFlag()))
                            return nullptr;
                        if (ob_ptr->GetObType() == CObObject::ob_reference)
                        {
                            std::shared_ptr<CReferenceObject> ref_ptr = std::make_shared<CReferenceObject>(identifiers);
                            ASSERT(ref_ptr != nullptr);
                            return ref_ptr;
                        }
                        return ob_ptr;
                    }
                    if (!iden_ptr)
                    {
                        if (identifiers[pos] == '.')
                        {
                            std::shared_ptr<CStructObject> ob_ptr = std::static_pointer_cast<CStructObject>(symbols->GetVarIdentifier(identifiers.substr(0, pos)));
                            if (!ob_ptr || !ob_ptr->IsStruct())
                                return nullptr;
                            std::shared_ptr<CStructObject> new_ptr = std::make_shared<CStructObject>(identifiers.substr(0, pos));
                            ASSERT(new_ptr != nullptr);
                            if (!new_ptr) return nullptr;
                            std::shared_ptr<CObObject> pelement = (**func)(symbols, ob_ptr, identifiers.substr(pos + 1));
                            if (!pelement) return nullptr;
                            if ((*new_ptr).AddElement(identifiers.substr(pos + 1), CStructObject::ElementTypeDefine(pelement->GetName(), { CStructObject::InvalidOffset, pelement })))
                                return new_ptr;
                            return nullptr;
                        }
                        else if (identifiers[pos] == '-' && identifiers[pos + 1] == '>')
                        {
                            std::shared_ptr<CReferenceObject> ob_ptr = std::static_pointer_cast<CReferenceObject>(symbols->GetVarIdentifier(identifiers.substr(0, pos)));
                            if (!ob_ptr || !ob_ptr->IsReference() || !ob_ptr->GetData())
                                return nullptr;
                            std::shared_ptr<CReferenceObject> reference_ptr = std::make_shared<CReferenceObject>(identifiers.substr(0, pos));
                            ASSERT(reference_ptr != nullptr);
                            if (!reference_ptr) return nullptr;
                            if (!(*ob_ptr->GetData()).IsStruct())
                                return nullptr;
                            std::shared_ptr<CStructObject> new_ptr = std::make_shared<CStructObject>(std::static_pointer_cast<CStructObject>(ob_ptr->GetData())->GetName());
                            ASSERT(new_ptr != nullptr);
                            if (!new_ptr) return nullptr;
                            if (!reference_ptr->AddReference(std::static_pointer_cast<CObObject>(new_ptr)))
                                return nullptr;
                            std::shared_ptr<CObObject> pelement = (**func)(symbols, ob_ptr->GetData(), identifiers.substr(pos + 2));
                            if (!pelement) return nullptr;
                            if ((*new_ptr).AddElement(identifiers.substr(pos + 2), CStructObject::ElementTypeDefine(pelement->GetName(), { CStructObject::InvalidOffset,pelement })))
                                return reference_ptr;
                            return nullptr;
                        }
                        else if (identifiers[pos] == '*')
                        {
                            // only support like to "*lpName", "*lpTotalNumberOfBytes"
                            // not support "(*SystemInformation).dwNumberOfProcessors", please used to "SystemInformation->dwNumberOfProcessors"
                            if (pos != 0) return nullptr;
                            std::shared_ptr<CObObject> ob_ptr = symbols->GetVarIdentifier(identifiers.substr(1));
                            if (!ob_ptr || !ob_ptr->IsReference())
                                return nullptr;
                            std::shared_ptr<CReferenceObject> reference_ptr = std::make_shared<CReferenceObject>(identifiers);
                            if (!reference_ptr) return nullptr;
                            std::shared_ptr<CObObject> pelement = std::static_pointer_cast<CReferenceObject>(ob_ptr)->GetData();
                            if (!pelement) return nullptr;
                            if (!reference_ptr->AddReference(pelement))
                                return nullptr;
                            return reference_ptr;
                        }
                        else return nullptr;
                    }
                    else
                    {
                        if (identifiers[pos] == '.')
                        {
                            std::shared_ptr<CStructObject> struct_ptr = std::static_pointer_cast<CStructObject>(iden_ptr);
                            std::shared_ptr<CObObject> ob_ptr = struct_ptr->GetElement(identifiers.substr(0, pos));
                            if (!ob_ptr || !ob_ptr->IsStruct()) return nullptr;
                            std::shared_ptr<CStructObject> new_ptr = std::make_shared<CStructObject>(std::static_pointer_cast<CStructObject>(ob_ptr)->GetName());
                            ASSERT(new_ptr != nullptr);
                            if (!new_ptr) return nullptr;
                            std::shared_ptr<CObObject> pelement = (**func)(symbols, ob_ptr, identifiers.substr(pos + 1));
                            if (!pelement) return nullptr;
                            if ((*new_ptr).AddElement(identifiers.substr(pos + 1), CStructObject::ElementTypeDefine(pelement->GetName(), { CStructObject::InvalidOffset,pelement })))
                                return new_ptr;
                            return nullptr;
                        }
                        else if (identifiers[pos] == '-' && identifiers[pos + 1] == '>')
                        {
                            std::shared_ptr<CStructObject> struct_ptr = std::static_pointer_cast<CStructObject>(iden_ptr);
                            std::shared_ptr<CReferenceObject> ob_ptr = std::static_pointer_cast<CReferenceObject>(struct_ptr->GetElement(identifiers.substr(0, pos)));
                            if (!ob_ptr || !ob_ptr->IsReference() || !ob_ptr->GetData())
                                return nullptr;
                            std::shared_ptr<CReferenceObject> reference_ptr = std::make_shared<CReferenceObject>(identifiers.substr(0, pos));
                            ASSERT(reference_ptr != nullptr);
                            if (!reference_ptr) return nullptr;
                            if (!(*ob_ptr->GetData()).IsStruct())
                                return nullptr;
                            std::shared_ptr<CStructObject> new_ptr = std::make_shared<CStructObject>(std::static_pointer_cast<CStructObject>(ob_ptr->GetData())->GetName());
                            ASSERT(new_ptr != nullptr);
                            if (!new_ptr) return nullptr;
                            if (!reference_ptr->AddReference(std::static_pointer_cast<CObObject>(new_ptr)))
                                return nullptr;
                            std::shared_ptr<CObObject> pelement = (**func)(symbols, ob_ptr->GetData(), identifiers.substr(pos + 2));
                            if (!pelement) return nullptr;
                            if ((*new_ptr).AddElement(identifiers.substr(pos + 2), CStructObject::ElementTypeDefine(pelement->GetName(), { CStructObject::InvalidOffset,pelement })))
                                return reference_ptr;
                            return nullptr;
                        }
                        else if (identifiers[pos] == '*')
                            return nullptr;
                        else return nullptr;

                    }
                    return nullptr;
                }
        );
        std::shared_ptr<CObObject> ob_ptr = nullptr;
        if (GetLocalSymbols()->GetSymbolTableSize())
            ob_ptr = (**func)(GetLocalSymbols(), nullptr, identifiers);
        if (!ob_ptr && GetGlobalSymbols()->GetSymbolTableSize())
            ob_ptr = (**func)(GetGlobalSymbols(), nullptr, identifiers);
        ASSERT(ob_ptr != nullptr);
        return AddIdentifier(IDENPAIR(identifiers, ob_ptr));
    }

    bool CExpParsing::Parsing(const std::string& exp_str, std::shared_ptr<CLexerObject> symboltable)
    {
        if (symboltable == nullptr) return false;
        if (!exp_str.length()) return false;
        int step = 0;
        if (m_expression_str.length()) return false;
        auto isvarchar = [](int c) {if (isalpha(c) || c == '_' || c == '%') return true; else return false; };
        std::stack<std::string> operator_stack;
        bool last_tok_was_op = true;

        while (exp_str[step] && isspace(exp_str[step])) ++step;
        while (exp_str[step])
        {
            if (isdigit(exp_str[step]))
            {
                char* nodig = 0;
                double digit = strtod(&exp_str[step], &nodig);

                std::unique_ptr<_Value> value = std::make_unique<_Value>(digit);
                m_expression_ast.push_back(std::move(value));
                step = nodig - &exp_str[0];
                last_tok_was_op = false;
            }
            else if (isvarchar(exp_str[step]))
            {
                std::stringstream ss;
                do {
                    ss << exp_str[step];
                    ++step;
                } while (isvarchar(exp_str[step]));

                std::string key = ss.str();
                if (key == "true")
                    m_expression_ast.push_back(std::make_unique<_Value>("true"));
                else if (key == "false")
                    m_expression_ast.push_back(std::make_unique<_Value>("false"));
                else if (key == _token_var_def) {
                    while (isdigit(exp_str[step]))
                    {
                        ss << exp_str[step];
                        ++step;
                    }
                    key = ss.str();
                    std::shared_ptr<CObObject> ptr = symboltable->GetVarIdentifier(key);
                    ASSERT(ptr != nullptr);
                    if (ptr == nullptr)
                        return false;
                    AddTokenIdentifier(IDENPAIR(key, ptr));
                    m_expression_ast.push_back(std::make_unique<_Value>(key, token_iden));
                }
                last_tok_was_op = false;
            }
            else if (exp_str[step] == '\'' || exp_str[step] == '"')
            {
                char startChr = exp_str[step];
                ++step;
                std::stringstream ss;
                do {
                    ss << exp_str[step];
                    ++step;
                } while (exp_str[step] && exp_str[step] != startChr);
                if (exp_str[step]) step++;

                m_expression_ast.push_back(std::make_unique<_Value>(ss.str()));
                last_tok_was_op = false;
            }
            else
            {
                switch (exp_str[step]) {
                case '(':
                    operator_stack.push("(");
                    ++step;
                    break;
                case ')':
                    while (operator_stack.top().compare("(")) {
                        m_expression_ast.push_back(std::make_unique<_Value>(operator_stack.top(), token_op));
                        operator_stack.pop();
                    }
                    operator_stack.pop();
                    ++step;
                    break;
                default:
                {
                    std::stringstream ss;
                    ss << exp_str[step];
                    ++step;
                    while (exp_str[step] && !isspace(exp_str[step]) && !isdigit(exp_str[step])
                        && !isvarchar(exp_str[step]) && exp_str[step] != '(' && exp_str[step] != ')') {
                        ss << exp_str[step];
                        ++step;
                    }
                    ss.clear();
                    std::string str;
                    ss >> str;

                    if (last_tok_was_op) {
                        if (!str.compare("-") || !str.compare("+") || !str.compare("!"))
                            m_expression_ast.push_back(std::make_unique<_Value>(0));
                        else
                        {
                            ASSERT(0);
                            error_log("Unrecognized unary operator: '{}'", str);
                        }

                    }

                    while ([&]() ->bool {
                        if (operator_stack.empty()) return false;
                        auto it1 = _op_precedence.find(str);
                        auto it2 = _op_precedence.find(operator_stack.top());
                        if (it1 == _op_precedence.end()) return false;
                        if (it2 == _op_precedence.end()) return false;
                        if (it1->second > it2->second) return false;
                        return true;
                        }())
                    {
                        m_expression_ast.push_back(std::make_unique<_Value>(operator_stack.top(), token_op));
                        operator_stack.pop();
                    }
                        operator_stack.push(str);
                        last_tok_was_op = true;
                }
                }
            }
            while (exp_str[step] && isspace(exp_str[step])) ++step;
        }
        while (!operator_stack.empty()) {
            m_expression_ast.push_back(std::make_unique<_Value>(operator_stack.top(), token_op));
            operator_stack.pop();
        }

        m_expression_str = exp_str;
        return true;
    }

    bool CExpParsing::SetIdentifierValue(const ValuePair& iden_pair) {
        ASSERT(iden_pair.first.length());
        ASSERT(iden_pair.second.str().length());
        if (!iden_pair.first.length()) return false;
        if (!iden_pair.second.str().length()) return false;
        auto value = m_value_map.find(iden_pair.first);
        if (value != m_value_map.end())
            return true;

        auto ptr = m_identifier_symbol.find(iden_pair.first);
        if (ptr == m_identifier_symbol.end())
        {
            ASSERT(0); return false;
        }
        ASSERT(ptr->second != nullptr);

        CLexerObject lexer_object;
        auto final_ptr = lexer_object.GetFinalObObject(ptr->second);
        ASSERT(final_ptr != nullptr);
        if (!final_ptr) return nullptr;
        _token tok = token_invalid;
        switch (final_ptr->GetObType())
        {
        case CObObject::ob_basetype:
        {
            std::shared_ptr<CMetadataTypeObject> obj_ptr = std::reinterpret_pointer_cast<CMetadataTypeObject>(final_ptr);
            ASSERT(obj_ptr->GetMetadataDef() != nullptr);
            int std_type = obj_ptr->GetMetadataDef()->GetBaseDef();
            switch (std_type)
            {
            case CBaseDef::type_byte: case CBaseDef::type_bool: case CBaseDef::type_int16: case CBaseDef::type_uint16: case CBaseDef::type_short: case CBaseDef::type_ushort:
            case CBaseDef::type_word: case CBaseDef::type_int: case CBaseDef::type_uint: case CBaseDef::type_long: case CBaseDef::type_ulong: case CBaseDef::type_dword:
            case CBaseDef::type_float: case CBaseDef::type_int64: case CBaseDef::type_uint64: case CBaseDef::type_longlong: case CBaseDef::type_ulonglong: case CBaseDef::type_qword:
            case CBaseDef::type_double: case CBaseDef::type_large_integer: case CBaseDef::type_ularge_integer: case CBaseDef::type_hresult: case CBaseDef::type_ntstatus:
            {
                tok = token_number;
            }
            break;
            case CBaseDef::type_char: case CBaseDef::type_uchar: case CBaseDef::type_wchar: case CBaseDef::type_long_ptr: case CBaseDef::type_ulong_ptr: case CBaseDef::type_dword_ptr:
            case CBaseDef::type_lpvoid: case CBaseDef::type_hmodule: case CBaseDef::type_schandle: case CBaseDef::type_handle: case CBaseDef::type_string: case CBaseDef::type_wstring:
            {
                tok = token_string;
            }
            break;
            case CBaseDef::type_void:
            case CBaseDef::type_guid:
            default:
                break;
            }
        }
        break;
        case CObObject::ob_reference:
        case CObObject::ob_stringref:
        case CObObject::ob_flag:
        {
            tok = token_string;
        }
        break;
        default:
            break;
        }
        if (tok == token_string)
            m_value_map[iden_pair.first] = _Value(iden_pair.second.str());
        else if (tok == token_number)
        {
            char* nodig = 0;
            double digit = strtod(iden_pair.second.str().c_str(), &nodig);
            m_value_map[iden_pair.first] = _Value(digit);
        }
        else
        {
            ASSERT(0);
            error_log("SetIdentifierValue failed because tok incorrect!");
            return false;
        }
        return true;
    }

    std::variant<bool, std::unique_ptr<CExpParsing::ValuePair>> CExpParsing::EvalExpression()
    {
        ASSERT(m_expression_ast.size());
        ASSERT(m_value_map.size());
        if (!m_expression_ast.size())
            return false;
        if (!m_value_map.size())
            return false;

        std::string last_identifier;
        std::stack<_Value> evaluation;
        std::unique_ptr<std::queue<std::unique_ptr<_Value>>> expression_ast = std::move(GetQueueExpressionAst());
        ASSERT(expression_ast != nullptr && (*expression_ast).size());
        if (!expression_ast || !expression_ast->size())
            return false;

        while (!expression_ast->empty()) {
            std::unique_ptr<_Value> token = std::move(expression_ast->front());
            expression_ast->pop();

            if (token->isOp())
            {
                std::string str = token->GetString();
                if (evaluation.size() < 2) {
                    ASSERT(!evaluation.size());
                    if (evaluation.size() == 0)
                        return false;
                    _Value value = evaluation.top(); evaluation.pop();
                    if (!str.compare("!"))
                        evaluation.push(!value.GetNumber());
                }
                _Value rhs = evaluation.top(); evaluation.pop();
                _Value lhs = evaluation.top(); evaluation.pop();
                if (!str.compare("+") && lhs.isNumber())
                    evaluation.push(lhs + rhs);
                else if (!str.compare("*"))
                    evaluation.push(lhs * rhs);
                else if (!str.compare("-"))
                    evaluation.push(lhs - rhs);
                else if (!str.compare("/"))
                    evaluation.push(lhs / rhs);
                else if (!str.compare("<<"))
                    evaluation.push(lhs << rhs);
                else if (!str.compare("^"))
                    evaluation.push(lhs ^ rhs);
                else if (!str.compare(">>"))
                    evaluation.push(lhs >> rhs);
                else if (!str.compare(">"))
                    evaluation.push(lhs > rhs);
                else if (!str.compare(">="))
                    evaluation.push(lhs >= rhs);
                else if (!str.compare("<"))
                    evaluation.push(lhs < rhs);
                else if (!str.compare("<="))
                    evaluation.push(lhs <= rhs);
                else if (!str.compare("&&"))
                    evaluation.push(lhs && rhs);
                else if (!str.compare("||"))
                    evaluation.push(lhs || rhs);
                else if (!str.compare("=="))
                    evaluation.push(lhs == rhs);
                else if (!str.compare("!="))
                    evaluation.push(lhs != rhs);
                else if (!str.compare("="))
                {
                    evaluation.push(lhs = rhs);
                    if (last_identifier.length() && evaluation.size() == 1 && expression_ast->empty())
                    {
                        // Back propagation
                        auto result_value = std::make_unique<ValuePair>(last_identifier, lhs.GetString());
                        return result_value;
                    }
                }
                else if (!str.compare("=~"))
                {
                    std::regex reg_ex(rhs.GetString(), std::regex::icase);
                    evaluation.push(std::regex_match(lhs.GetString(), reg_ex));
                }
                else if (!str.compare("!~"))
                {
                    std::regex reg_ex(rhs.GetString(), std::regex::icase);
                    evaluation.push(!std::regex_match(lhs.GetString(), reg_ex));
                }
                else
                {
                    error_log("Unknown operator: {} {} {}.", lhs.GetString(), str, rhs.GetString());
                    return false;
                }
            }
            else if (token->isNumber() || token->isString())
            {
                evaluation.push(*token);
            }
            else if (token->isIdentifier())
            {
                last_identifier = token->GetString();
                auto r_token = m_value_map.find(token->GetString());
                if (r_token == m_value_map.end())
                {
                    ASSERT(0);
                    error_log("Unknown identifier: {}", token->GetString());
                }
                evaluation.push(r_token->second);
            }
            else
            {
                error_log("Invalid token '{}'.", token->GetString());
                return false;
            }
        }
        if (evaluation.top().GetNumber() == 0)
            return false;
        return true;
    }

    bool CFunction::AddChecks(const _CheckPackage& package, bool bpre)
    {
        std::unique_ptr<CCheck> pcheck = std::make_unique<CCheck>(shared_from_this());
        if (!pcheck) return false;

        for (const auto& define : package.define_pairs)
        {
            std::shared_ptr<CObObject> define_ptr = GetTyIdentifier(define.second);
            if (define_ptr)
                pcheck->AddLocalSymbol(PrototypeArgument(define.first, define_ptr));
        }
        for (const auto& log : package.logs)
        {
            std::shared_ptr<CObObject> log_ptr = pcheck->GetIdentifier(log);
            if (log_ptr)
                pcheck->AddLog(IDENPAIR(log, log_ptr));
        }
        for (const auto& handle : package.handle_pair)
        {
            pcheck->AddHandle(handle);
        }
        for (const auto& check : package.check_pair)
        {
            if (check.first.length() &&
                pcheck->ParsingIdentifiers(check.first))
            {
                pcheck->AddCheck(check.second);
            }
        }
        for (const auto& modify : package.modify_pair)
        {
            if (modify.first.length() &&
                pcheck->ParsingIdentifiers(modify.first))
            {
                pcheck->AddModify(modify.second);
            }
        }
        if (bpre)
            m_pprechecks.push_back(std::move(pcheck));
        else
            m_ppostchecks.push_back(std::move(pcheck));

        return true;
    }

    processing_status CFunction::CheckReturn(PVOID pnode) const
    {
        ASSERT(pnode != nullptr);
        CHookImplementObject::detour_node* de_node = reinterpret_cast<CHookImplementObject::detour_node*>(pnode);
        if (!de_node || !de_node->log_entry) return processing_skip;

        ASSERT(de_node->return_va != nullptr);
        std::shared_ptr<CObObject> ob_ptr = GetVarIdentifier(SI_RETURN);
        ASSERT(ob_ptr != nullptr);
        if (!ob_ptr) return processing_continue;
        if (Success(ob_ptr, reinterpret_cast<char*>(de_node->return_va)))
        {
            return processing_continue;
        }
        return processing_skip;
    }

    processing_status CFunction::ProcessingEnsure(PVOID pnode) const
    {
        ASSERT(pnode != nullptr);
        if (!m_pensure) return processing_continue;
        CHookImplementObject::detour_node* de_node = reinterpret_cast<CHookImplementObject::detour_node*>(pnode);
        if (!de_node || !de_node->log_entry) return processing_skip;
        BEGIN_LOG("ensure");

        for (const auto& ensure : *m_pensure)
        {
            ASSERT(ensure.second != nullptr);
            if (!ensure.second)
                continue;
            if (_stricmp(GetWordRoot(ensure.first).c_str(), SI_RETURN) == 0)
                continue;
            int arg_offset = GetArgumentOffset(GetWordRoot(ensure.first));
            if (arg_offset == invalid_arg_offset)
            {
                arg_offset = GetArgumentOffset(ensure.second->GetName());
                if (arg_offset == invalid_arg_offset)
                    continue;
            }

            ASSERT(de_node->pparams != nullptr);
            if (!IsValidValue(ensure.second, de_node->pparams + arg_offset))
            {
                ASSERT(0);
                error_log("Function({}): argument({}) invalid!", GetName(), ensure.first);
                return processing_skip;
            }
            LOGGING(ensure.first, "valid");
        }
        END_LOG(de_node->log_entry);
        return processing_continue;
    }

    processing_status CFunction::ProcessingLog(PVOID pnode, bool pre) const
    {
        CHookImplementObject::detour_node* de_node = reinterpret_cast<CHookImplementObject::detour_node*>(pnode);
        if (!de_node || !de_node->log_entry) return processing_skip;
        std::string log_name;
        if (pre)
        {
            if (!m_pprelog)
                return processing_continue;
            log_name = "pre_log";
        }
        else
        {
            if (!m_ppostlog)
                return processing_continue;
            log_name = "post_log";
        }

        BEGIN_LOG(log_name);
        ASSERT(GetLogging(pre) != nullptr);
        for (const auto& plog : *GetLogging(pre))
        {
            ASSERT(plog.second != nullptr);
            if (!plog.second)
                continue;
            int arg_offset = 0;
            char* param_adress = nullptr;
            if (_stricmp(GetWordRoot(plog.first).c_str(), SI_RETURN) == 0)
            {
                param_adress = reinterpret_cast<char*>(de_node->return_va);
            }
            else
            {
                arg_offset = GetArgumentOffset(GetWordRoot(plog.first));
                if (arg_offset == invalid_arg_offset)
                {
                    arg_offset = GetArgumentOffset(plog.second->GetName());
                    if (arg_offset == invalid_arg_offset)
                        continue;
                }
                param_adress = de_node->pparams;
            }

            ASSERT(param_adress != nullptr);
            std::stringstream ss = GetValue(plog.second, param_adress + arg_offset);
            if (!ss.str().length())
            {
                error_log("Function({}): get argument({}) value failed!", GetName(), plog.first);
                return processing_skip;
            }
            LOGGING(plog.first, ss.str());
        }
        END_LOG(de_node->log_entry);

        return processing_continue;
    }

    bool CFunction::checking(PVOID pnode, const std::unique_ptr<CCheck>& pcheck, const std::shared_ptr<CLogHandle>& log_handle) const
    {
        BEGIN_LOG("checking");
        CHookImplementObject::detour_node* de_node = reinterpret_cast<CHookImplementObject::detour_node*>(pnode);
        if (!de_node || !de_node->log_entry) return false;
        bool bcheck = true;
        for (const auto& check : pcheck->GetChecks())
        {
            ASSERT(pcheck != nullptr);
            if (!check) continue;
            for (const auto& iden : check->GetIdenifierSymbol())
            {
                ASSERT(iden.second != nullptr);
                if (!iden.second)
                    continue;
                int arg_offset = 0;
                char* param_adress = nullptr;
                if (_stricmp(GetWordRoot(pcheck->GetRealName(iden.first)).c_str(), SI_RETURN) == 0)
                {
                    param_adress = reinterpret_cast<char*>(de_node->return_va);
                }
                else
                {
                    arg_offset = GetArgumentOffset(GetWordRoot(pcheck->GetRealName(iden.first)));
                    if (arg_offset == invalid_arg_offset)
                    {
                        arg_offset = GetArgumentOffset(iden.second->GetName());
                        if (arg_offset == invalid_arg_offset)
                            continue;
                    }
                    param_adress = de_node->pparams;
                }

                ASSERT(param_adress != nullptr);
                std::stringstream ss = GetValue(iden.second, param_adress + arg_offset);
                if (!ss.str().length())
                {
                    error_log("Function({}): get argument({}) value failed!", GetName(), iden.first);
                    return false;
                }
                if (!check->SetIdentifierValue(CExpParsing::ValuePair(iden.first, ss.str())))
                {
                    error_log("Function({}): SetCheckIdenValue({}) value failed!", GetName(), iden.first);
                    return false;
                }
            }
            if (!std::get<bool>(check->EvalExpression()))
            {
                bcheck = false;
                break;
            }
        }
        END_LOG(log_handle->GetHandle());
        return bcheck;
    }

    processing_status CFunction::modifying(PVOID pnode, const std::unique_ptr<CCheck>& pcheck, const std::shared_ptr<CLogHandle>& log_handle)
    {
        BEGIN_LOG("modifying");
        CHookImplementObject::detour_node* de_node = reinterpret_cast<CHookImplementObject::detour_node*>(pnode);
        if (!de_node || !de_node->log_entry) return processing_skip;
        bool breturn = false;
        for (const auto& modify : pcheck->GetModifys())
        {
            ASSERT(modify != nullptr);
            if (!modify) continue;
            for (const auto& iden : modify->GetIdenifierSymbol())
            {
                ASSERT(iden.second != nullptr);
                if (!iden.second)
                    continue;
                int arg_offset = 0;
                char* param_adress = nullptr;
                if (_stricmp(GetWordRoot(pcheck->GetRealName(iden.first)).c_str(), SI_RETURN) == 0)
                {
                    param_adress = reinterpret_cast<char*>(de_node->return_va);
                    breturn = true;
                }
                else
                {
                    arg_offset = GetArgumentOffset(GetWordRoot(pcheck->GetRealName(iden.first)));
                    if (arg_offset == invalid_arg_offset)
                    {
                        arg_offset = GetArgumentOffset(iden.second->GetName());
                        if (arg_offset == invalid_arg_offset)
                            continue;
                    }
                    param_adress = de_node->pparams;
                }

                ASSERT(param_adress != nullptr);
                std::stringstream ss = GetValue(iden.second, param_adress + arg_offset);
                if (!ss.str().length())
                {
                    error_log("Function({}): get argument({}) value failed!", GetName(), iden.first);
                    return processing_skip;
                }
                if (!modify->SetIdentifierValue(CExpParsing::ValuePair(iden.first, ss.str())))
                {
                    error_log("Function({}): SetCheckIdenValue({}) value failed!", GetName(), iden.first);
                    return processing_skip;
                }
                // back propagation
                std::unique_ptr<CExpParsing::ValuePair> value = std::get<std::unique_ptr<CExpParsing::ValuePair>>(modify->EvalExpression());
                if (value == nullptr)
                    break;
                if (_stricmp(iden.first.c_str(), value->first.c_str()) != 0)
                {
                    ASSERT(0); continue;
                }
                if (!SetValue(iden.second, param_adress + arg_offset, value->second))
                {
                    error_log("Function({}): ({})SetValue({} : {}) failed!", GetName(), pcheck->GetRealName(iden.first), ss.str(), value->second.str());
                }
                else
                {
                    LOGGING(pcheck->GetRealName(iden.first), value->second.str());
                    debug_log("Function({}): ({})SetValue({} : {}) success!", GetName(), pcheck->GetRealName(iden.first), ss.str(), value->second.str());
                }
            }
        }
        END_LOG(log_handle->GetHandle());
        if (breturn)
            return processing_exit;
        return processing_continue;
    }

    bool CFunction::handling(PVOID pnode, const std::unique_ptr<CCheck>& pcheck, const std::shared_ptr<CLogHandle>& log_handle) const
    {
        BEGIN_LOG("handling");
        CHookImplementObject::detour_node* de_node = reinterpret_cast<CHookImplementObject::detour_node*>(pnode);
        if (!de_node || !de_node->log_entry) return false;
        std::shared_ptr<CHookImplementObject> hook_implement_object = de_node->hook_implement_object;
        bool bhandle = true;
        for (const auto& handle : pcheck->GetHandles())
        {
            if (handle.second == CCheck::handle_addtarget)
            {
                std::shared_ptr<CObObject> ob_ptr = pcheck->GetIdentifier(handle.first);
                ASSERT(ob_ptr != nullptr);
                if (!ob_ptr)
                    continue;
                int arg_offset = 0;
                char* param_adress = nullptr;
                if (_stricmp(GetWordRoot(pcheck->GetRealName(handle.first)).c_str(), SI_RETURN) == 0)
                {
                    param_adress = reinterpret_cast<char*>(de_node->return_va);
                }
                else
                {
                    arg_offset = GetArgumentOffset(GetWordRoot(pcheck->GetRealName(handle.first)));
                    if (arg_offset == invalid_arg_offset)
                    {
                        arg_offset = GetArgumentOffset(ob_ptr->GetName());
                        if (arg_offset == invalid_arg_offset)
                            continue;
                    }
                    param_adress = de_node->pparams;
                }

                ASSERT(param_adress != nullptr);
                std::stringstream ss = GetValue(ob_ptr, param_adress + arg_offset);
                if (ss.str().length())
                {
                    // send process id to driver for add target.
                    LOGGING("AddTarget", ss.str());
                    if (hook_implement_object && hook_implement_object->GetDriverMgr())
                    {
                        char* nodig = nullptr;
                        DWORD BytesReturned = 0;
                        DWORD pid = (DWORD)std::strtoll(ss.str().c_str(), &nodig, 10);
                        if (hook_implement_object->GetDriverMgr()->IoControl(IOCTL_HIPS_SETTARGETPID, &pid, sizeof(DWORD), NULL, 0, &BytesReturned))
                        {
                            debug_log("add target pid({}) success!", pid);
                        }
                        else
                            debug_log("add target pid({}) failed!", pid);
                    }
                }
            }
        }
        END_LOG(log_handle->GetHandle());
        return bhandle;
    }

    bool CFunction::logging(PVOID pnode, const std::unique_ptr<CCheck>& pcheck, const std::shared_ptr<CLogHandle>& log_handle) const
    {
        BEGIN_LOG("logging");
        CHookImplementObject::detour_node* de_node = reinterpret_cast<CHookImplementObject::detour_node*>(pnode);
        if (!de_node || !de_node->log_entry) return false;
        bool blog = true;
        for (const auto& log : pcheck->GetLogs())
        {
            ASSERT(log.second != nullptr);
            if (!log.second)
                continue;
            int arg_offset = 0;
            char* param_adress = nullptr;
            if (_stricmp(GetWordRoot(log.first).c_str(), SI_RETURN) == 0)
            {
                param_adress = reinterpret_cast<char*>(de_node->return_va);
            }
            else
            {
                arg_offset = GetArgumentOffset(GetWordRoot(log.first));
                if (arg_offset == invalid_arg_offset)
                {
                    arg_offset = GetArgumentOffset(log.second->GetName());
                    if (arg_offset == invalid_arg_offset)
                        continue;
                }
                param_adress = de_node->pparams;
            }

            ASSERT(param_adress != nullptr);
            std::stringstream ss = GetValue(log.second, param_adress + arg_offset);
            if (!ss.str().length())
            {
                return false;
            }
            LOGGING(log.first, ss.str());
        }
        END_LOG(log_handle->GetHandle());
        return blog;
    }

    processing_status CFunction::ProcessingChecks(PVOID pnode, bool pre)
    {
        CHookImplementObject::detour_node* de_node = reinterpret_cast<CHookImplementObject::detour_node*>(pnode);
        if (!de_node || !de_node->log_entry) return processing_skip;
        std::string log_name;
        if (pre)
        {
            if (!m_pprechecks.size())
                return processing_continue;
            log_name = "pre_check";
        }
        else
        {
            if (!m_ppostchecks.size())
                return processing_continue;
            log_name = "post_check";
        }

        BEGIN_LOG(log_name);
        processing_status status = processing_continue;
        for (const auto& pcheck : GetChecks(pre))
        {
            ASSERT(pcheck != nullptr);
            if (pcheck == nullptr)
                continue;
            if (checking(pnode, pcheck, LOGGER))
            {
                status = modifying(pnode, pcheck, LOGGER);
                if (status == processing_skip)
                {
                    error_log("modifying error!");
                    return status;
                }
                if (!handling(pnode, pcheck, LOGGER))
                {
                    error_log("handling error!");
                    if (status == processing_exit) return status;
                    return processing_skip;
                }
                if (!logging(pnode, pcheck, LOGGER))
                {
                    error_log("logging error!");
                    if (status == processing_exit) return status;
                    return processing_skip;
                }
            }
            if (status == processing_exit) break;
        }

        END_LOG(de_node->log_entry);
        return status;
    }

    bool CWmiObject::AddChecks(const CFunction::_CheckPackage& package, bool bpre)
    {
        std::unique_ptr<CCheck> pcheck = std::make_unique<CCheck>(shared_from_this());
        if (!pcheck) return false;

        for (const auto& define : package.define_pairs)
        {
            std::shared_ptr<CObObject> define_ptr = GetTyIdentifier(define.second);
            if (define_ptr)
                pcheck->AddLocalSymbol(PrototypeData(define.first, define_ptr));
        }
        for (const auto& log : package.logs)
        {
            std::shared_ptr<CObObject> log_ptr = pcheck->GetIdentifier(log);
            if (log_ptr)
                pcheck->AddLog(IDENPAIR(log, log_ptr));
        }
        for (const auto& handle : package.handle_pair)
        {
            pcheck->AddHandle(handle);
        }
        for (const auto& check : package.check_pair)
        {
            if (check.first.length() &&
                pcheck->ParsingIdentifiers(check.first))
            {
                pcheck->AddCheck(check.second);
            }
        }
        for (const auto& modify : package.modify_pair)
        {
            if (modify.first.length() &&
                pcheck->ParsingIdentifiers(modify.first))
            {
                pcheck->AddModify(modify.second);
            }
        }
        if (bpre)
            m_pprechecks.push_back(std::move(pcheck));
        else
            m_ppostchecks.push_back(std::move(pcheck));

        return true;
    }

} // namespace cchips
