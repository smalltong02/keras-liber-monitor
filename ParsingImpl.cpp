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

    const std::map<std::string, CLexer::_token> CLexer::_token_str_def = {
        {"if", token_if}, {"else", token_else}, {"while", token_while}, {"define", token_define}, {"extern", token_extern}, {"(", token_parentheses_b}, {")", token_parentheses_e},
        {"{", token_curlybraces_b}, {"}", token_curlybraces_e}, {"[", token_brackets_b}, {"]", token_brackets_e},{"*", token_asterisk}, {"->", token_arrow},
        {".", token_point}, {"=", token_assign}, {"'", token_s_quotes}, {"\"", token_d_quotes},
    };
    const std::map<std::string, CLexerTy::_token_ty> CLexerTy::_token_ty_str_def = {
        {"LP", token_reference_b}, {"*", token_reference_e}, {"[", token_brackets_b}, {"]", token_brackets_e},
    };
    const std::string CLexerTy::_re2_pattern_ty_def = {
        "^([A-Za-z_]\\w*)\\[([0-9]+)\\]$|^([A-Za-z_]\\w*)[*]$|^LP([A-Za-z_]\\w*)$|^([A-Za-z_]\\w*)$"
    };
    std::unique_ptr<RE2> CLexerTy::_pattern_ty_ptr = std::make_unique<RE2>(_re2_pattern_ty_def, RE2::Quiet);
    const std::map<std::string, CLexerVr::_token_vr> CLexerVr::_token_vr_str_def = {
        {"(", token_parentheses_b}, {")", token_parentheses_e}, {"*", token_asterisk}, {"->", token_arrow}, {".", token_point},
    };
    const std::string CLexerVr::_re2_pattern_vr_def = {
        "^[*]([A-Za-z_(][A-Za-z_0-9)\\[\\].>-]*)$|^[(]([A-Za-z_*][A-Za-z0-9_\\[\\].>-]*)[)]([A-Za-z_0-9\\[\\].>-]*)$|^([A-Za-z_][A-Za-z_0-9\\[\\]]*)->([A-Za-z_][A-Za-z_0-9\\[\\].>-]*)$|^([A-Za-z_][A-Za-z_0-9\\[\\]]*)[.]([A-Za-z_][A-Za-z_0-9\\[\\].>-]*)$|^([A-Za-z_]\\w*)\\[([0-9]+)\\]$|^([A-Za-z_]\\w*)$"
    };
    std::unique_ptr<RE2> CLexerVr::_pattern_vr_ptr = std::make_unique<RE2>(_re2_pattern_vr_def, RE2::Quiet);
    const std::map<CExpParsing::_token, std::string> CExpParsing::_token_str_def = {
        {token_invalid, "invalid"}, {token_string, "string"}, {token_integral, "integral"}, {token_float_pointer, "float_pointer"},
    };
    const std::map<std::string, int> CExpParsing::_op_precedence = {
        {"(", -10}, {")", -10},  {"=", -7}, {"||", -5}, {"&&", -4}, {"|", -3}, {"&", -2}, {">", -1}, {">=", -1}, {"<", -1}, {"<=", -1}, {"==", -1}, {"!=", -1}, {"=~", -1}, {"!~", -1},
        {"<<", 1}, {">>", 1}, {"+", 2}, {"-", 2}, {"*", 3}, {"/", 3}, {"^", 4}, {"!", 5},
    };
    const std::map<std::string, int> CExpParsing::_op_type_def = {
        {"=", binary_type}, {"||", binary_type}, {"&&", binary_type}, {"|", binary_type}, {"&", binary_type}, {">", binary_type}, {">=", binary_type}, {"<", binary_type}, {"<=", binary_type}, {"==", binary_type}, {"!=", binary_type}, 
        {"=~", binary_type}, {"!~", binary_type}, {"<<", binary_type}, {">>", binary_type}, {"+", binary_type}, {"-", binary_type}, {"*", binary_type}, {"/", binary_type}, {"^", binary_type}, {"!", unary_type},
    };
    const std::string CExpParsing::_token_var_def = "%var";
    const std::string CExpParsing::_re2_pattern_exp_def = { "^(%var[0-9]+)$|^'([\\s\\S]+)'$|^\"([\\s\\S]+)\"$|^([1-9]\\d*)$|^([1-9]\\d*\\.\\d*)$|^(0\\.\\d*[1-9]\\d*)$"};
    std::unique_ptr<RE2> CExpParsing::_pattern_exp_ptr = std::make_unique<RE2>(_re2_pattern_exp_def, RE2::Quiet);

    const std::map<std::string, CPrototype::_class_type> CPrototype::_class_predefine = {
        {"WMI", CPrototype::class_wmi}, {"VBS", CPrototype::class_vbs}, {"PS", CPrototype::class_ps}, {"KVM", CPrototype::class_kvm}, {"CShape", CPrototype::class_cshape},
    };

    std::string CLexerTy::GetWordRoot(const std::string_view& iden_str)
    {
        ASSERT(iden_str.length());
        if (!iden_str.length()) return {};
        std::string root_str(iden_str);
        std::string iden_arr_str, sub_str, iden_poi_str, iden_lp_str, res_str;

        auto func = make_y_combinator(
            [](auto&& func, std::string& search_str, std::unique_ptr<RE2>& pattern, std::string& iden_arr_str, std::string& sub_str, std::string& iden_poi_str, std::string& iden_lp_str, std::string& res_str) ->bool{
                if (RE2::FullMatch(search_str, *pattern, &iden_arr_str, &sub_str, &iden_poi_str, &iden_lp_str, &res_str))
                {
                    if (res_str.length())
                    {
                        search_str = res_str;
                        return true;
                    }
                    else if (iden_poi_str.length())
                    {
                        search_str = iden_poi_str;
                        return func(search_str, pattern, iden_arr_str, sub_str, iden_poi_str, iden_lp_str, res_str);
                    }
                    else if (iden_lp_str.length())
                    {
                        search_str = iden_lp_str;
                        return func(search_str, pattern, iden_arr_str, sub_str, iden_poi_str, iden_lp_str, res_str);
                    }
                    else if (iden_arr_str.length() && sub_str.length())
                    {
                        search_str = iden_arr_str;
                        return func(search_str, pattern, iden_arr_str, sub_str, iden_poi_str, iden_lp_str, res_str);
                    }
                }
                return false;
            }
        );
        if(func(root_str, _pattern_ty_ptr, iden_arr_str, sub_str, iden_poi_str, iden_lp_str, res_str))
            return res_str;
        return root_str;
    }

    std::shared_ptr<CObObject> CLexerTy::GetIdentifier(const std::string_view& iden_str) const
    {
        ASSERT(iden_str.length());
        if (!iden_str.length()) return nullptr;
        std::string root_str(iden_str);
        std::string iden_arr_str, sub_str, iden_poi_str, iden_lp_str, res_str;
        std::shared_ptr<CObObject> ob_ptr;

        auto func = make_y_combinator(
            [](auto&& func, std::string& search_str, std::unique_ptr<RE2>& pattern, std::string& iden_arr_str, std::string& sub_str, std::string& iden_poi_str, std::string& iden_lp_str, std::string& res_str) ->std::shared_ptr<CObObject> {
                if (RE2::FullMatch(search_str, *pattern, &iden_arr_str, &sub_str, &iden_poi_str, &iden_lp_str, &res_str))
                {
                    if (res_str.length())
                    {
                        return std::move(CTypeSymbolTableObject::GetInstance().GetTypeSymbolReference(res_str));
                    }
                    else if (iden_poi_str.length())
                    {
                        auto ref_ptr = std::make_shared<CReferenceObject>(search_str);
                        ASSERT(ref_ptr != nullptr);
                        if (!ref_ptr) return nullptr;
                        search_str = iden_poi_str;
                        std::shared_ptr<CObObject> ob_ptr = func(search_str, pattern, iden_arr_str, sub_str, iden_poi_str, iden_lp_str, res_str);
                        if (!ob_ptr) return nullptr;
                        ref_ptr->AddReference(std::move(ob_ptr));
                        return ref_ptr;
                    }
                    else if (iden_lp_str.length())
                    {
                        auto ref_ptr = std::make_shared<CReferenceObject>(search_str);
                        ASSERT(ref_ptr != nullptr);
                        if (!ref_ptr) return nullptr;
                        search_str = iden_lp_str;
                        std::shared_ptr<CObObject> ob_ptr = func(search_str, pattern, iden_arr_str, sub_str, iden_poi_str, iden_lp_str, res_str);
                        if (!ob_ptr) return nullptr;
                        ref_ptr->AddReference(std::move(ob_ptr));
                        return ref_ptr;
                    }
                    else if (iden_arr_str.length() && sub_str.length())
                    {
                        int dim = atoi(sub_str.c_str());
                        auto array_ptr = std::make_shared<CArrayObject>(search_str);
                        ASSERT(array_ptr != nullptr);
                        if (!array_ptr) return nullptr;
                        search_str = iden_arr_str;
                        std::shared_ptr<CObObject> ob_ptr = func(search_str, pattern, iden_arr_str, sub_str, iden_poi_str, iden_lp_str, res_str);
                        if (!ob_ptr) return nullptr;
                        array_ptr->AddArray(std::move(ob_ptr), dim);
                        return array_ptr;
                    }
                }
                return nullptr;
            }
        );
        ob_ptr = func(root_str, _pattern_ty_ptr, iden_arr_str, sub_str, iden_poi_str, iden_lp_str, res_str);
        return ob_ptr;
    }

    std::string CLexerVr::GetWordRoot(const std::string_view& iden_str)
    {
        ASSERT(iden_str.length());
        if (!iden_str.length()) return {};
        std::string root_str(iden_str);
        std::string iden_ast_str, iden_par_b_str, iden_par_e_str, iden_arrow_b_str, iden_arrow_e_str, iden_poi_b_str, iden_poi_e_str, iden_arr_str, sub_str, res_str;

        auto func = make_y_combinator(
            [](auto&& func, std::string& search_str, std::unique_ptr<RE2>& pattern, std::string& iden_ast_str, std::string& iden_par_b_str, std::string& iden_par_e_str, std::string& iden_arrow_b_str, std::string& iden_arrow_e_str, 
                std::string& iden_poi_b_str, std::string& iden_poi_e_str, std::string& iden_arr_str, std::string& sub_str, std::string& res_str) ->bool {
                if (RE2::FullMatch(search_str, *pattern, &iden_ast_str, &iden_par_b_str, &iden_par_e_str, &iden_arrow_b_str, &iden_arrow_e_str, &iden_poi_b_str, &iden_poi_e_str, &iden_arr_str, &sub_str, &res_str))
                {
                    if (res_str.length())
                    {
                        search_str = res_str;
                        return true;
                    }
                    else if (iden_poi_b_str.length())
                    {
                        search_str = iden_poi_b_str;
                        return func(search_str, pattern, iden_ast_str, iden_par_b_str, iden_par_e_str, iden_arrow_b_str, iden_arrow_e_str, iden_poi_b_str, iden_poi_e_str, iden_arr_str, sub_str, res_str);
                    }
                    else if (iden_arrow_b_str.length())
                    {
                        search_str = iden_arrow_b_str;
                        return func(search_str, pattern, iden_ast_str, iden_par_b_str, iden_par_e_str, iden_arrow_b_str, iden_arrow_e_str, iden_poi_b_str, iden_poi_e_str, iden_arr_str, sub_str, res_str);
                    }
                    else if (iden_ast_str.length())
                    {
                        search_str = iden_ast_str;
                        return func(search_str, pattern, iden_ast_str, iden_par_b_str, iden_par_e_str, iden_arrow_b_str, iden_arrow_e_str, iden_poi_b_str, iden_poi_e_str, iden_arr_str, sub_str, res_str);
                    }
                    else if (iden_par_b_str.length())
                    {
                        search_str = iden_par_b_str;
                        return func(search_str, pattern, iden_ast_str, iden_par_b_str, iden_par_e_str, iden_arrow_b_str, iden_arrow_e_str, iden_poi_b_str, iden_poi_e_str, iden_arr_str, sub_str, res_str);
                    }
                    else if (iden_arr_str.length())
                    {
                        search_str = iden_arr_str;
                        return func(search_str, pattern, iden_ast_str, iden_par_b_str, iden_par_e_str, iden_arrow_b_str, iden_arrow_e_str, iden_poi_b_str, iden_poi_e_str, iden_arr_str, sub_str, res_str);
                    }
                }
                return false;
            }
        );
        if (func(root_str, _pattern_vr_ptr, iden_ast_str, iden_par_b_str, iden_par_e_str, iden_arrow_b_str, iden_arrow_e_str, iden_poi_b_str, iden_poi_e_str, iden_arr_str, sub_str, res_str))
            return res_str;
        return {};
    }

    std::shared_ptr<CObObject> CLexerVr::GetIdentifier(const std::string_view& iden_str) const
    {
        if (!GetVariableSymbolTable()) return nullptr;
        ASSERT(iden_str.length());
        if (!iden_str.length()) return nullptr;
        std::string root_str(iden_str);
        std::string iden_ast_str, iden_par_b_str, iden_par_e_str, iden_arrow_b_str, iden_arrow_e_str, iden_poi_b_str, iden_poi_e_str, iden_arr_str, sub_str, res_str;

        auto func = make_y_combinator(
            [this](auto&& func, std::string& search_str, std::unique_ptr<RE2>& pattern, std::string& iden_ast_str, std::string& iden_par_b_str, std::string& iden_par_e_str, std::string& iden_arrow_b_str, std::string& iden_arrow_e_str,
                std::string& iden_poi_b_str, std::string& iden_poi_e_str, std::string& iden_arr_str, std::string& sub_str, std::string& res_str, std::shared_ptr<CTupleObject> final_ptr, bool bast = false) ->bool {
                    auto init_func = [](std::shared_ptr<CObObject>& ptr, std::string_view elem_name, std::shared_ptr<CTupleObject>& final_ptr) ->bool {
                        if (!ptr) return false;
                        if (!elem_name.length()) return false;
                        CTupleObject::_tuple_elem elem{};
                        elem.ptr = ptr;
                        elem.elem_name = elem_name;
                        if (elem.ptr->IsCommReference()) {
                            elem.ref.tuple_ref = CTupleObject::_tuple_ref::tuple_ref_addr;
                        }
                        else if (elem.ptr->IsArray()) {
                            elem.arr.dim = static_cast<unsigned int>(-1);
                        }
                        else if (elem.ptr->IsStruct()) {
                            elem.struc.idx = static_cast<unsigned int>(-1);
                        }
                        else if (elem.ptr->IsFlag()) {
                            elem.flag.tuple_flg = CTupleObject::_tuple_flg::tuple_flg_str;
                        }
                        else {
                            elem.arr.dim = 0;
                        }
                        return final_ptr->AddElement(elem);
                    };
                    if (!final_ptr) return false;
                    if (RE2::FullMatch(search_str, *pattern, &iden_ast_str, &iden_par_b_str, &iden_par_e_str, &iden_arrow_b_str, &iden_arrow_e_str, &iden_poi_b_str, &iden_poi_e_str, &iden_arr_str, &sub_str, &res_str))
                    {
                        if (res_str.length())
                        {
                            if (final_ptr->GetElemCount())
                            {
                                std::any anyvalue = final_ptr->GetElement(static_cast<unsigned int>(-1));
                                if (!anyvalue.has_value() || anyvalue.type() != typeid(CTupleObject::_tuple_elem*)) return false;
                                auto elem = std::any_cast<CTupleObject::_tuple_elem*>(anyvalue);
                                if (!elem->ptr || !elem->ptr->IsStruct()) return false;
                                std::shared_ptr<CStructObject> struct_ptr = std::static_pointer_cast<CStructObject>(elem->ptr);
                                elem->struc.idx = struct_ptr->GetElementIdx(res_str);
                                if (elem->struc.idx == static_cast<unsigned int>(-1)) return false;
                                return init_func(struct_ptr->GetElement(res_str), res_str, final_ptr);
                            }
                            else {
                                std::shared_ptr<CObObject> ob_ptr = GetSymbolReference(res_str);
                                if (!ob_ptr) return false;
                                bool bret = init_func(ob_ptr, res_str, final_ptr);
                                if (!bast)
                                    return bret;
                                else {
                                    if (!bret) return bret;
                                    std::any anyvalue = final_ptr->GetElement(static_cast<unsigned int>(-1));
                                    if (!anyvalue.has_value() || anyvalue.type() != typeid(CTupleObject::_tuple_elem*)) return false;
                                    auto elem = std::any_cast<CTupleObject::_tuple_elem*>(anyvalue);
                                    if (!elem->ptr || !elem->ptr->IsCommReference()) return false;
                                    elem->ref.tuple_ref = CTupleObject::_tuple_ref::tuple_ref_val;
                                    std::shared_ptr<CObObject> ptr = std::static_pointer_cast<CReferenceObject>(elem->ptr)->GetData();
                                    if (!init_func(ptr, "*", final_ptr)) return false;
                                    return true;
                                }
                            }
                        }
                        else if (iden_poi_b_str.length() && iden_poi_e_str.length())
                        {
                            search_str = iden_poi_b_str;
                            std::string bak_search_str = iden_poi_e_str;
                            bool bret = func(search_str, pattern, iden_ast_str, iden_par_b_str, iden_par_e_str, iden_arrow_b_str, iden_arrow_e_str, iden_poi_b_str, iden_poi_e_str, iden_arr_str, sub_str, res_str, final_ptr, bast);
                            if (bret) {
                                if (!final_ptr) return false;
                                std::any anyvalue = final_ptr->GetElement(static_cast<unsigned int>(-1));
                                if (!anyvalue.has_value() || anyvalue.type() != typeid(CTupleObject::_tuple_elem*)) return false;
                                auto elem = std::any_cast<CTupleObject::_tuple_elem*>(anyvalue);
                                if (!elem->ptr || !elem->ptr->IsStruct()) return false;
                                search_str = bak_search_str;
                                return func(search_str, pattern, iden_ast_str, iden_par_b_str, iden_par_e_str, iden_arrow_b_str, iden_arrow_e_str, iden_poi_b_str, iden_poi_e_str, iden_arr_str, sub_str, res_str, final_ptr);
                            }
                            return false;
                        }
                        else if (iden_arrow_b_str.length() && iden_arrow_e_str.length())
                        {
                            search_str = iden_arrow_b_str;
                            std::string bak_search_str = iden_arrow_e_str;
                            bool bret = func(search_str, pattern, iden_ast_str, iden_par_b_str, iden_par_e_str, iden_arrow_b_str, iden_arrow_e_str, iden_poi_b_str, iden_poi_e_str, iden_arr_str, sub_str, res_str, final_ptr, bast);
                            if (bret) {
                                if (!final_ptr) return false;
                                std::any anyvalue = final_ptr->GetElement(static_cast<unsigned int>(-1));
                                if (!anyvalue.has_value() || anyvalue.type() != typeid(CTupleObject::_tuple_elem*)) return false;
                                auto elem = std::any_cast<CTupleObject::_tuple_elem*>(anyvalue);
                                if (!elem->ptr || !elem->ptr->IsCommReference()) return false;
                                elem->ref.tuple_ref = CTupleObject::_tuple_ref::tuple_ref_val;
                                std::shared_ptr<CObObject> ob_ptr = std::static_pointer_cast<CReferenceObject>(elem->ptr)->GetData();
                                if (init_func(ob_ptr, "->", final_ptr)) {
                                    search_str = bak_search_str;
                                    return func(search_str, pattern, iden_ast_str, iden_par_b_str, iden_par_e_str, iden_arrow_b_str, iden_arrow_e_str, iden_poi_b_str, iden_poi_e_str, iden_arr_str, sub_str, res_str, final_ptr);
                                }
                            }
                            return false;
                        }
                        else if (iden_ast_str.length())
                        {
                            search_str = iden_ast_str;
                            return func(search_str, pattern, iden_ast_str, iden_par_b_str, iden_par_e_str, iden_arrow_b_str, iden_arrow_e_str, iden_poi_b_str, iden_poi_e_str, iden_arr_str, sub_str, res_str, final_ptr, true);
                        }
                        else if (iden_par_b_str.length())
                        {
                            search_str = iden_par_b_str;
                            std::string bak_search_str = iden_par_e_str;
                            bool bret = func(search_str, pattern, iden_ast_str, iden_par_b_str, iden_par_e_str, iden_arrow_b_str, iden_arrow_e_str, iden_poi_b_str, iden_poi_e_str, iden_arr_str, sub_str, res_str, final_ptr);
                            if(bret) {
                                if (!final_ptr) return false;
                                if (bast) {
                                    std::any anyvalue = final_ptr->GetElement(static_cast<unsigned int>(-1));
                                    if (!anyvalue.has_value() || anyvalue.type() != typeid(CTupleObject::_tuple_elem*)) return false;
                                    auto elem = std::any_cast<CTupleObject::_tuple_elem*>(anyvalue);
                                    if (!elem->ptr || !elem->ptr->IsCommReference()) return false;
                                    elem->ref.tuple_ref = CTupleObject::_tuple_ref::tuple_ref_val;
                                    std::shared_ptr<CObObject> ob_ptr = std::static_pointer_cast<CReferenceObject>(elem->ptr)->GetData();
                                    if (!init_func(ob_ptr, "*", final_ptr)) return false;
                                }
                                if (!bak_search_str.length())
                                    return true;
                                if (bak_search_str[0] == '.') {
                                    std::any anyvalue = final_ptr->GetElement(static_cast<unsigned int>(-1));
                                    if (!anyvalue.has_value() || anyvalue.type() != typeid(CTupleObject::_tuple_elem*)) return false;
                                    auto elem = std::any_cast<CTupleObject::_tuple_elem*>(anyvalue);
                                    if (!elem->ptr || !elem->ptr->IsStruct()) return false;
                                    search_str = bak_search_str.substr(1);
                                    return func(search_str, pattern, iden_ast_str, iden_par_b_str, iden_par_e_str, iden_arrow_b_str, iden_arrow_e_str, iden_poi_b_str, iden_poi_e_str, iden_arr_str, sub_str, res_str, final_ptr);
                                }
                                else if (bak_search_str[0] == '-' && bak_search_str[1] == '>') {
                                    std::any anyvalue = final_ptr->GetElement(static_cast<unsigned int>(-1));
                                    if (!anyvalue.has_value() || anyvalue.type() != typeid(CTupleObject::_tuple_elem*)) return false;
                                    auto elem = std::any_cast<CTupleObject::_tuple_elem*>(anyvalue);
                                    if (!elem->ptr || !elem->ptr->IsCommReference()) return false;
                                    elem->ref.tuple_ref = CTupleObject::_tuple_ref::tuple_ref_val;
                                    std::shared_ptr<CObObject> ob_ptr = std::static_pointer_cast<CReferenceObject>(elem->ptr)->GetData();
                                    if (init_func(ob_ptr, "->", final_ptr)) {
                                        search_str = bak_search_str.substr(2);
                                        return func(search_str, pattern, iden_ast_str, iden_par_b_str, iden_par_e_str, iden_arrow_b_str, iden_arrow_e_str, iden_poi_b_str, iden_poi_e_str, iden_arr_str, sub_str, res_str, final_ptr);
                                    }
                                    return false;
                                }
                                else if (std::string dim_str; RE2::FullMatch(bak_search_str, "^\\[([0-9]+)\\]$", &dim_str) && dim_str.length()) {
                                    std::any anyvalue = final_ptr->GetElement(static_cast<unsigned int>(-1));
                                    if (!anyvalue.has_value() || anyvalue.type() != typeid(CTupleObject::_tuple_elem*)) return false;
                                    auto elem = std::any_cast<CTupleObject::_tuple_elem*>(anyvalue);
                                    if (!elem->ptr || !elem->ptr->IsArray()) return false;
                                    unsigned int dim = static_cast<unsigned int>(atoi(dim_str.c_str()));
                                    elem->arr.dim = dim;
                                    return true;
                                }
                            }
                            return false;
                        }
                        else if (iden_arr_str.length() && sub_str.length())
                        {
                            search_str = iden_arr_str;
                            std::string bak_sub_str = sub_str;
                            bool bret = func(search_str, pattern, iden_ast_str, iden_par_b_str, iden_par_e_str, iden_arrow_b_str, iden_arrow_e_str, iden_poi_b_str, iden_poi_e_str, iden_arr_str, sub_str, res_str, final_ptr, bast);
                            if (bret) {
                                if (!final_ptr) return false;
                                std::any anyvalue = final_ptr->GetElement(static_cast<unsigned int>(-1));
                                if (!anyvalue.has_value() || anyvalue.type() != typeid(CTupleObject::_tuple_elem*)) return false;
                                auto elem = std::any_cast<CTupleObject::_tuple_elem*>(anyvalue);
                                if (!elem->ptr || !elem->ptr->IsArray()) return false;
                                std::shared_ptr<CArrayObject> ob_array = std::static_pointer_cast<CArrayObject>(elem->ptr);
                                unsigned int dim = static_cast<unsigned int>(atoi(bak_sub_str.c_str()));
                                if (dim >= ob_array->GetDim()) return false;
                                elem->arr.dim = dim;
                                std::shared_ptr<CObObject> ob_ptr = std::static_pointer_cast<CArrayObject>(elem->ptr)->GetData();
                                return init_func(ob_ptr, "[]", final_ptr);
                            }
                        }
                    }
                    return false;
            }
        );
        std::shared_ptr<CObObject> iden_ptr = GetSymbolReference(root_str);
        if (iden_ptr) return iden_ptr;
        else {
            std::shared_ptr<CTupleObject> tuple_ptr = std::make_shared<CTupleObject>(root_str);
            if (func(root_str, _pattern_vr_ptr, iden_ast_str, iden_par_b_str, iden_par_e_str, iden_arrow_b_str, iden_arrow_e_str, iden_poi_b_str, iden_poi_e_str, iden_arr_str, sub_str, res_str, tuple_ptr))
                return tuple_ptr;
        }
        return nullptr;
    }
    
    bool CExpParsing::Parsing(const std::string& exp_str, std::shared_ptr<CParsing> symboltable)
    {
        if (symboltable == nullptr) return false;
        if (!exp_str.length()) return false;
        int step = 0;
        if (m_expression_str.length()) return false;
        std::stack<std::string> operator_stack;
        bool last_tok_was_op = true;
        auto process_final_op = [this](std::deque<std::shared_ptr<_Value>>& expression_ast) {
            if (expression_ast.size() < 2) return;
            auto& it_rbegin = expression_ast.crbegin();
            if (it_rbegin == expression_ast.crend() || !(*it_rbegin)) return;
            if ((*it_rbegin)->GetToken() != token_op) return;
            const auto& type_def = _op_type_def.find((*it_rbegin)->GetString());
            if (type_def == _op_type_def.end()) return;
            if (type_def->second == binary_type) {
                if (expression_ast.size() < 3) return;
                auto& op = (*it_rbegin);
                it_rbegin++;
                if (it_rbegin == expression_ast.crend() || !(*it_rbegin)) return;
                auto& rhs = (*it_rbegin);
                it_rbegin++;
                if (it_rbegin == expression_ast.crend() || !(*it_rbegin)) return;
                auto& lhs = (*it_rbegin);
                if (rhs->isIntegral() && lhs->isIdentifier()) {
                    std::shared_ptr<CObObject> iden_ptr = GetTokenIdentifier(lhs->GetString());
                    if (!iden_ptr) return;
                    if (iden_ptr->IsTuple()) {
                        auto anyvalue = std::static_pointer_cast<CTupleObject>(iden_ptr)->GetElement(static_cast<unsigned int>(-1));
                        if (!anyvalue.has_value() || anyvalue.type() != typeid(CTupleObject::_tuple_elem*)) return;
                        auto elem = std::any_cast<CTupleObject::_tuple_elem*>(anyvalue);
                        if (elem == nullptr || elem->ptr == nullptr) return;
                        if (elem->ptr->IsFlag()) {
                            elem->flag.tuple_flg = CTupleObject::_tuple_flg::tuple_flg_int;
                        }
                    }
                }
                else if (lhs->isIntegral() && rhs->isIdentifier()) {
                    std::shared_ptr<CObObject> iden_ptr = GetTokenIdentifier(rhs->GetString());
                    if (!iden_ptr) return;
                    if (iden_ptr->IsTuple()) {
                        auto anyvalue = std::static_pointer_cast<CTupleObject>(iden_ptr)->GetElement(static_cast<unsigned int>(-1));
                        if (!anyvalue.has_value() || anyvalue.type() != typeid(CTupleObject::_tuple_elem*)) return;
                        auto elem = std::any_cast<CTupleObject::_tuple_elem*>(anyvalue);
                        if (elem == nullptr || elem->ptr == nullptr) return;
                        if (elem->ptr->IsFlag()) {
                            elem->flag.tuple_flg = CTupleObject::_tuple_flg::tuple_flg_int;
                        }
                    }
                }
                else if ((rhs->isIntegral() && lhs->isIntegral()) ||
                    (rhs->isFloat() && lhs->isFloat()) ||
                    (rhs->isString() && lhs->isString())) {
                    std::string op_str = op->GetString();
                    std::shared_ptr<_Value> new_value = std::make_shared<_Value>(Calculate(op_str, *rhs, *lhs));
                    if (!new_value || !new_value->isValid()) return;
                    expression_ast.pop_back();
                    expression_ast.pop_back();
                    expression_ast.pop_back();
                    expression_ast.emplace_back(std::move(new_value));
                }
            }
            return;
        };
        auto get_token_define = [&](const std::stringstream& iden_token, const std::unique_ptr<RE2>& pattern) ->std::shared_ptr<_Value> {
            std::string iden_str, string_str1, string_str2, integral_str, float_str1, float_str2;
            std::shared_ptr<_Value> value = nullptr;
            if (RE2::FullMatch(iden_token.str(), *pattern, &iden_str, &string_str1, &string_str2, &integral_str, &float_str1, &float_str2))
            {
                if (iden_str.length())
                {
                    std::shared_ptr<CObObject> ptr = symboltable->GetIdentifier(iden_str);
                    if (ptr == nullptr) return nullptr;
                    if (!ptr->IsTuple()) {
                        std::shared_ptr<CTupleObject> tuple_ptr = std::make_shared<CTupleObject>(ptr->GetName());
                            CTupleObject::_tuple_elem elem{};
                            elem.ptr = ptr;
                            elem.elem_name = ptr->GetName();
                            if (elem.ptr->IsCommReference()) {
                                elem.ref.tuple_ref = CTupleObject::_tuple_ref::tuple_ref_addr;
                            }
                            else if (elem.ptr->IsArray()) {
                                elem.arr.dim = static_cast<unsigned int>(-1);
                            }
                            else if (elem.ptr->IsStruct()) {
                                elem.struc.idx = static_cast<unsigned int>(-1);
                            }
                            else if (elem.ptr->IsFlag()) {
                                elem.flag.tuple_flg = CTupleObject::_tuple_flg::tuple_flg_str;
                            }
                            else {
                                elem.arr.dim = 0;
                            }
                            if (tuple_ptr->AddElement(elem)) {
                                AddTokenIdentifier(IDENPAIR(iden_str, tuple_ptr));
                            }
                            else {
                                AddTokenIdentifier(IDENPAIR(iden_str, tuple_ptr));
                            }
                    }
                    else {
                        AddTokenIdentifier(IDENPAIR(iden_str, ptr));
                    }
                    value = std::make_shared<_Value>(iden_str, token_iden);
                }
                else if (string_str1.length())
                {
                    value = std::make_shared<_Value>(string_str1);
                }
                else if (string_str2.length())
                {
                    value = std::make_shared<_Value>(string_str2);
                }
                else if (integral_str.length())
                {
                    char* nodig = 0;
                    unsigned long long integral = strtoull(integral_str.c_str(), &nodig, 10);
                    value = std::make_shared<_Value>(integral);
                }
                else if (float_str1.length())
                {
                    char* nodig = 0;
                    double float_pointer = strtod(float_str1.c_str(), &nodig);
                    value = std::make_shared<_Value>(float_pointer);
                }
                else if (float_str2.length())
                {
                    char* nodig = 0;
                    double float_pointer = strtod(float_str1.c_str(), &nodig);
                    value = std::make_shared<_Value>(float_pointer);
                }
            }
            return value;
        };
        

        while (exp_str[step] && isspace(exp_str[step])) ++step;
        std::stringstream ss;
        std::stringstream iden_ss;
        do {
            bool bstr = false;
            while (exp_str[step] && !isspace(exp_str[step]))
            {
                if (exp_str[step] == '\'' || exp_str[step] == '"') {
                    bstr = !bstr;
                    if (!bstr) {
                        iden_ss << exp_str[step++];
                        break;
                    }
                }
                ss << exp_str[step++];
                if (!bstr) {
                    auto it_find = _op_precedence.find(ss.str());
                    if (it_find != _op_precedence.end()) break;
                }
                iden_ss << ss.str();
                ss.str(""); ss.clear();
            }

            if (iden_ss.str().length())
            {
                std::shared_ptr<_Value> value = get_token_define(iden_ss, _pattern_exp_ptr);
                if (!value) {
                    m_expression_ast.clear();
                    return false;
                }
                m_expression_ast.emplace_back(std::move(value));
                last_tok_was_op = false;
                iden_ss.str(""); iden_ss.clear();
            }
          
            if(ss.str().length())
            {
                switch (ss.str()[0]) {
                    case '(':
                        operator_stack.push("(");
                        break;
                    case ')':
                        while (operator_stack.top().compare("(")) {
                            m_expression_ast.push_back(std::make_unique<_Value>(operator_stack.top(), token_op));
                            process_final_op(m_expression_ast);
                            operator_stack.pop();
                        }
                        operator_stack.pop();
                        break;
                    default: {
                        std::stringstream op_ss;
                        op_ss << ss.str();
                        if (exp_str[step] && !isspace(exp_str[step]))
                        {
                            op_ss << exp_str[step];
                            auto it_find = _op_precedence.find(op_ss.str());
                            if (it_find != _op_precedence.end()) {
                                ss.str(""); ss.clear();
                                ss << op_ss.str();
                                step++;
                            }
                        }
                        while ([&]() ->bool {
                            if (operator_stack.empty()) return false;
                            auto it1 = _op_precedence.find(ss.str());
                            auto it2 = _op_precedence.find(operator_stack.top());
                            if (it1 == _op_precedence.end()) return false;
                            if (it2 == _op_precedence.end()) return false;
                            if (it1->second > it2->second) return false;
                            return true;
                            }())
                        {
                            m_expression_ast.emplace_back(std::make_unique<_Value>(operator_stack.top(), token_op));
                            process_final_op(m_expression_ast);
                            operator_stack.pop();
                        }
                        operator_stack.emplace(ss.str());
                        last_tok_was_op = true;
                    }
                }
                ss.str(""); ss.clear();
            }
            while (exp_str[step] && isspace(exp_str[step])) ++step;
        } while (exp_str[step]);

        while (!operator_stack.empty()) {
            m_expression_ast.emplace_back(std::make_unique<_Value>(operator_stack.top(), token_op));
            process_final_op(m_expression_ast);
            operator_stack.pop();
        }
        m_expression_str = exp_str;
        return true;
    }

    bool CExpParsing::SetIdentifierValue(_ValueMap& value_map, const ValuePair& iden_pair) {
        ASSERT(iden_pair.first.length());
        ASSERT(iden_pair.second.has_value());
        if (!iden_pair.first.length()) return false;
        if (!iden_pair.second.has_value()) return false;
        auto value = value_map.find(iden_pair.first);
        if (value != value_map.end())
            return true;
        std::any anyvalue = GetAnyValue(iden_pair.second);
        if (!anyvalue.has_value()) return false;
        if (anyvalue.type() == typeid(ULONGLONG))
            value_map[iden_pair.first] = _Value(std::any_cast<ULONGLONG>(anyvalue));
        else if (anyvalue.type() == typeid(DOUBLE))
            value_map[iden_pair.first] = _Value(std::any_cast<DOUBLE>(anyvalue));
        else if (anyvalue.type() == typeid(std::string))
            value_map[iden_pair.first] = _Value(std::any_cast<std::string>(anyvalue));
        else
        {
            error_log("SetIdentifierValue failed because token incorrect!");
            return false;
        }
        return true;
    }

    CExpParsing::_Value CExpParsing::Calculate(std::string& op, _Value& rhs, _Value& lhs)
    {
        if (!op.compare("!")) {
            return !rhs;
        }
        else if (!op.compare("+")) {
            return (lhs + rhs);
        }
        else if (!op.compare("*")) {
            return (lhs * rhs);
        }
        else if (!op.compare("-")) {
            return (lhs - rhs);
        }
        else if (!op.compare("/")) {
            return (lhs / rhs);
        }
        else if (!op.compare("<<")) {
            return (lhs << rhs);
        }
        else if (!op.compare("^")) {
            return (lhs ^ rhs);
        }
        else if (!op.compare(">>")) {
            return (lhs >> rhs);
        }
        else if (!op.compare(">")) {
            return (lhs > rhs);
        }
        else if (!op.compare(">=")) {
            return (lhs >= rhs);
        }
        else if (!op.compare("<")) {
            return (lhs < rhs);
        }
        else if (!op.compare("<=")) {
            return (lhs <= rhs);
        }
        else if (!op.compare("&")) {
            return (lhs & rhs);
        }
        else if (!op.compare("|")) {
            return (lhs | rhs);
        }
        else if (!op.compare("&&")) {
            return (lhs && rhs);
        }
        else if (!op.compare("||")) {
            return (lhs || rhs);
        }
        else if (!op.compare("==")) {
            return (lhs == rhs);
        }
        else if (!op.compare("!=")) {
            return (lhs != rhs);
        }
        else if (!op.compare("="))
        {
            lhs = rhs;
            return lhs;
        }
        else if (!op.compare("=~"))
        {
            RE2 pattern(rhs.GetString(), RE2::Quiet);
            return (ULONGLONG)RE2::FullMatch(lhs.GetString(), pattern);
        }
        else if (!op.compare("!~"))
        {
            RE2 pattern(rhs.GetString(), RE2::Quiet);
            return (ULONGLONG)(!RE2::FullMatch(lhs.GetString(), pattern));
        }
        return {};
    }

    std::variant<ULONGLONG, std::unique_ptr<CExpParsing::ValuePair>> CExpParsing::EvalExpression(const _ValueMap& value_map)
    {
        if (!value_map.size()) return 0;
        std::unique_ptr<std::queue<std::unique_ptr<_Value>>> expression_ast = std::move(GetQueueExpressionAst());
        ASSERT(expression_ast != nullptr && (*expression_ast).size());
        if (!expression_ast || !expression_ast->size())
            return 0;

        std::string last_identifier;
        std::stack<_Value> evaluation;
        while (!expression_ast->empty()) {
            std::unique_ptr<_Value> token = std::move(expression_ast->front());
            expression_ast->pop();

            if (token->isOp())
            {
                std::string str = token->GetString();
                if (evaluation.size() < 2) {
                    ASSERT(!evaluation.size());
                    if (evaluation.size() == 0)
                        return 0;
                    _Value value = evaluation.top(); evaluation.pop();
                    _Value new_value = Calculate(str, value, _Value());
                    if(new_value.isValid())
                        evaluation.push(!value);
                    else return 0;
                }
                _Value rhs = evaluation.top(); evaluation.pop();
                _Value lhs = evaluation.top(); evaluation.pop();
                if (!str.compare("="))
                {
                    Calculate(str, rhs, lhs);
                    if (!lhs.isValid())
                        return 0;
                    if (last_identifier.length() && evaluation.size() == 0 && expression_ast->empty())
                    {
                        // Back propagation
                        auto result_value = std::make_unique<ValuePair>(last_identifier, lhs.GetAnyValue());
                        return result_value;
                    }
                }
                else {
                    _Value new_value = Calculate(str, rhs, lhs);
                    if (new_value.isValid())
                        evaluation.push(new_value);
                    else {
                        error_log("Unknown operator: {} {} {}.", lhs.GetString(), str, rhs.GetString());
                        return 0;
                    }
                }
            }
            else if (token->isConstant())
            {
                evaluation.push(*token);
            }
            else if (token->isIdentifier())
            {
                last_identifier = token->GetString();
                auto r_token = value_map.find(token->GetString());
                if (r_token == value_map.end())
                {
                    error_log("Unknown identifier: {}", token->GetString());
                    return 0;
                }
                evaluation.push(r_token->second);
            }
            else
            {
                error_log("Invalid token '{}'.", token->GetString());
                return 0;
            }
        }
        if (evaluation.top().GetIntegral() == 0)
            return 0;
        return evaluation.top().GetIntegral();
    }

    bool CFunction::AddChecks(const _CheckPackage& package, bool bpre)
    {
        std::shared_ptr<CCheck> pcheck = std::make_shared<CCheck>(shared_from_this());
        if (!pcheck) return false;
        auto bin_func([](auto it_b, auto it_e) {
            return std::string(it_b, it_e);
            });

        for (const auto& define : package.define_pairs)
        {
            std::shared_ptr<CObObject> define_ptr = CLexerTy::GetInstance().GetIdentifier(define.second);
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
            if (check.first.length())
            {
                bool bsuccess = false;
                std::list<std::string> split_list;
                split_algorithm(std::begin(check.first), std::end(check.first), back_inserter(split_list), '|', bin_func);
                for (const auto& it : split_list) {
                    std::shared_ptr<CObObject> ob_ptr = pcheck->GetIdentifier(it);
                    if (ob_ptr) {
                        bsuccess = pcheck->AddIdentifier(IDENPAIR(it, ob_ptr));
                        if (!bsuccess) break;
                    }
                    else {
                        bsuccess = false;
                        break;
                    }
                }
                if (bsuccess)
                    pcheck->AddCheck(check.second);
            }
        }
        for (const auto& modify : package.modify_pair)
        {
            if (modify.first.length())
            {
                bool bsuccess = false;
                std::list<std::string> split_list;
                split_algorithm(std::begin(modify.first), std::end(modify.first), back_inserter(split_list), '|', bin_func);
                for (const auto& it : split_list) {
                    std::shared_ptr<CObObject> ob_ptr = pcheck->GetIdentifier(it);
                    if (ob_ptr) {
                        bsuccess = pcheck->AddIdentifier(IDENPAIR(it, ob_ptr));
                        if (!bsuccess) break;
                    }
                    else {
                        bsuccess = false;
                        break;
                    }
                }
                if (bsuccess)
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
        std::shared_ptr<CObObject> ob_ptr = GetIdentifier(SI_RETURN);
        ASSERT(ob_ptr != nullptr);
        if (!ob_ptr) return processing_continue;
        if (ob_ptr->Success(reinterpret_cast<char*>(de_node->return_va)))
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
            if (_stricmp(CLexerVr::GetWordRoot(ensure.first).c_str(), SI_RETURN) == 0)
                continue;
            int arg_offset = GetArgumentOffset(CLexerVr::GetWordRoot(ensure.first));
            if (arg_offset == invalid_arg_offset)
            {
                arg_offset = GetArgumentOffset(ensure.second->GetName());
                if (arg_offset == invalid_arg_offset)
                    continue;
            }

            ASSERT(de_node->pparams != nullptr);
            if (!ensure.second->IsValidValue(de_node->pparams + arg_offset))
            {
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
            if (_stricmp(CLexerVr::GetWordRoot(plog.first).c_str(), SI_RETURN) == 0)
            {
                param_adress = reinterpret_cast<char*>(de_node->return_va);
            }
            else
            {
                arg_offset = GetArgumentOffset(CLexerVr::GetWordRoot(plog.first));
                if (arg_offset == invalid_arg_offset)
                {
                    arg_offset = GetArgumentOffset(plog.second->GetName());
                    if (arg_offset == invalid_arg_offset)
                        continue;
                }
                param_adress = de_node->pparams;
            }

            ASSERT(param_adress != nullptr);
            std::any anyvalue = plog.second->GetValue(param_adress + arg_offset);
            std::stringstream ss;
            if (ss = OutputAnyValue(anyvalue); !ss.str().length())
            {
                error_log("Function({}): get argument({}) value failed!", GetName(), plog.first);
                continue;
            }
            LOGGING(plog.first, ss.str());
        }
        END_LOG(de_node->log_entry);

        return processing_continue;
    }

    bool CFunction::checking(PVOID pnode, const std::shared_ptr<CCheck>& pcheck, const std::shared_ptr<CLogHandle>& log_handle) const
    {
        BEGIN_LOG("checking");
        CHookImplementObject::detour_node* de_node = reinterpret_cast<CHookImplementObject::detour_node*>(pnode);
        if (!de_node || !de_node->log_entry) return false;
        bool bcheck = true;
        for (const auto& check : pcheck->GetChecks())
        {
            ASSERT(pcheck != nullptr);
            if (!check) continue;
            CExpParsing::_ValueMap value_map;
            for (const auto& iden : check->GetIdenifierSymbol())
            {
                ASSERT(iden.second != nullptr);
                if (!iden.second)
                    continue;
                int arg_offset = 0;
                char* param_adress = nullptr;
                if (_stricmp(CLexerVr::GetWordRoot(pcheck->GetRealName(iden.first)).c_str(), SI_RETURN) == 0)
                {
                    param_adress = reinterpret_cast<char*>(de_node->return_va);
                }
                else
                {
                    arg_offset = GetArgumentOffset(CLexerVr::GetWordRoot(pcheck->GetRealName(iden.first)));
                    if (arg_offset == invalid_arg_offset)
                    {
                        arg_offset = GetArgumentOffset(iden.second->GetName());
                        if (arg_offset == invalid_arg_offset)
                            continue;
                    }
                    param_adress = de_node->pparams;
                }

                ASSERT(param_adress != nullptr);
                std::any anyvalue = iden.second->GetValue(param_adress + arg_offset);
                if (!anyvalue.has_value())
                {
                    error_log("Function({}): get argument({}) value failed!", GetName(), iden.first);
                    return false;
                }
                if (!check->SetIdentifierValue(value_map, CExpParsing::ValuePair(iden.first, anyvalue)))
                {
                    error_log("Function({}): SetCheckIdenValue({}) value failed!", GetName(), iden.first);
                    return false;
                }
            }
            if (auto variant_value = check->EvalExpression(value_map); variant_value.index() == 0 && !std::get<ULONGLONG>(variant_value))
            {
                bcheck = false;
                break;
            }
        }
        END_LOG(log_handle->GetHandle());
        return bcheck;
    }

    processing_status CFunction::modifying(PVOID pnode, const std::shared_ptr<CCheck>& pcheck, const std::shared_ptr<CLogHandle>& log_handle)
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
                if (_stricmp(CLexerVr::GetWordRoot(pcheck->GetRealName(iden.first)).c_str(), SI_RETURN) == 0)
                {
                    param_adress = reinterpret_cast<char*>(de_node->return_va);
                    breturn = true;
                }
                else
                {
                    arg_offset = GetArgumentOffset(CLexerVr::GetWordRoot(pcheck->GetRealName(iden.first)));
                    if (arg_offset == invalid_arg_offset)
                    {
                        arg_offset = GetArgumentOffset(iden.second->GetName());
                        if (arg_offset == invalid_arg_offset)
                            continue;
                    }
                    param_adress = de_node->pparams;
                }

                ASSERT(param_adress != nullptr);
                std::any anyvalue = iden.second->GetValue(param_adress + arg_offset);
                if (!anyvalue.has_value())
                {
                    error_log("Function({}): get argument({}) value failed!", GetName(), iden.first);
                    return processing_skip;
                }
                CExpParsing::_ValueMap value_map;
                if (!modify->SetIdentifierValue(value_map, CExpParsing::ValuePair(iden.first, anyvalue)))
                {
                    error_log("Function({}): SetCheckIdenValue({}) value failed!", GetName(), iden.first);
                    return processing_skip;
                }
                // back propagation
                auto variant_value = modify->EvalExpression(value_map);
                if (variant_value.index() != 1) break;
                std::unique_ptr<CExpParsing::ValuePair> value = std::move(std::get<std::unique_ptr<CExpParsing::ValuePair>>(variant_value));
                if (value == nullptr)
                    break;
                if (_stricmp(iden.first.c_str(), value->first.c_str()) != 0)
                {
                    ASSERT(0); continue;
                }
                std::any any_new_val = anyvalue;
                if (AssignAnyType(any_new_val, value->second))
                {
                    if (!iden.second->SetValue(param_adress + arg_offset, any_new_val))
                    {
                        error_log("Function({}): ({})SetValue({} : {}) failed!", GetName(), pcheck->GetRealName(iden.first), OutputAnyValue(anyvalue).str(), OutputAnyValue(value->second).str());
                    }
                    else
                    {
                        LOGGING(pcheck->GetRealName(iden.first), OutputAnyValue(value->second).str());
                        debug_log("Function({}): ({})SetValue({} : {}) success!", GetName(), pcheck->GetRealName(iden.first), OutputAnyValue(anyvalue).str(), OutputAnyValue(value->second).str());
                    }
                }
            }
        }
        END_LOG(log_handle->GetHandle());
        if (breturn)
            return processing_exit;
        return processing_continue;
    }

    bool CFunction::handling(PVOID pnode, const std::shared_ptr<CCheck>& pcheck, const std::shared_ptr<CLogHandle>& log_handle) const
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
                if (_stricmp(CLexerVr::GetWordRoot(pcheck->GetRealName(handle.first)).c_str(), SI_RETURN) == 0)
                {
                    param_adress = reinterpret_cast<char*>(de_node->return_va);
                }
                else
                {
                    arg_offset = GetArgumentOffset(CLexerVr::GetWordRoot(pcheck->GetRealName(handle.first)));
                    if (arg_offset == invalid_arg_offset)
                    {
                        arg_offset = GetArgumentOffset(ob_ptr->GetName());
                        if (arg_offset == invalid_arg_offset)
                            continue;
                    }
                    param_adress = de_node->pparams;
                }

                ASSERT(param_adress != nullptr);
                std::any anyvalue = ob_ptr->GetValue(param_adress + arg_offset);
                if (anyvalue.has_value())
                {
                    // send process id to driver for add target.
                    LOGGING("AddTarget", OutputAnyValue(anyvalue).str());
                    if (hook_implement_object && hook_implement_object->GetDriverMgr())
                    {
                        DWORD BytesReturned = 0;
                        DWORD pid = ConvertAnyType<DWORD>(anyvalue);
                        if (pid != static_cast<DWORD>(-1) && hook_implement_object->GetDriverMgr()->IoControl(IOCTL_HIPS_SETTARGETPID, &pid, sizeof(DWORD), NULL, 0, &BytesReturned))
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

    bool CFunction::logging(PVOID pnode, const std::shared_ptr<CCheck>& pcheck, const std::shared_ptr<CLogHandle>& log_handle) const
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
            if (_stricmp(CLexerVr::GetWordRoot(log.first).c_str(), SI_RETURN) == 0)
            {
                param_adress = reinterpret_cast<char*>(de_node->return_va);
            }
            else
            {
                arg_offset = GetArgumentOffset(CLexerVr::GetWordRoot(log.first));
                if (arg_offset == invalid_arg_offset)
                {
                    arg_offset = GetArgumentOffset(log.second->GetName());
                    if (arg_offset == invalid_arg_offset)
                        continue;
                }
                param_adress = de_node->pparams;
            }

            ASSERT(param_adress != nullptr);
            std::any anyvalue = log.second->GetValue(param_adress + arg_offset);
            if (anyvalue.has_value())
            {
                LOGGING(log.first, OutputAnyValue(anyvalue).str());
            }
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
        std::shared_ptr<CCheck> pcheck = std::make_unique<CCheck>(shared_from_this());
        if (!pcheck) return false;

        for (const auto& define : package.define_pairs)
        {
            std::shared_ptr<CObObject> define_ptr = GetIdentifier(define.second);
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
            if (check.first.length())
            {
                std::shared_ptr<CObObject> ob_ptr = pcheck->GetIdentifier(check.first);
                if (ob_ptr) {
                    pcheck->AddIdentifier(IDENPAIR(check.first, ob_ptr));
                    pcheck->AddCheck(check.second);
                }
            }
        }
        for (const auto& modify : package.modify_pair)
        {
            if (modify.first.length())
            {
                std::shared_ptr<CObObject> ob_ptr = pcheck->GetIdentifier(modify.first);
                if (ob_ptr) {
                    pcheck->AddIdentifier(IDENPAIR(modify.first, ob_ptr));
                    pcheck->AddModify(modify.second);
                }
            }
        }
        if (bpre)
            m_pprechecks.push_back(std::move(pcheck));
        else
            m_ppostchecks.push_back(std::move(pcheck));

        return true;
    }

} // namespace cchips
