//===- FlagsObject.cpp - metadata type implement define -----------------------------*- C++ -*-===//
// 
// This file implements CFlagsCfgObject class code.
//
//===------------------------------------------------------------------------------------------===//
#include "stdafx.h"
#include "FlagsObject.h"
#include "utils.h"
#include "HipsCfgObject.h"
#include "ParsingImpl.h"

namespace cchips {

    void CFlagsCfgObject::InitializeAdditionalType(const CRapidJsonWrapper& document)
    {
        if (auto anyvalue(document.GetMember(FL_ADDITIONAL));
            anyvalue.has_value() && anyvalue.type() == typeid(ConstRapidObject))
        {
            std::for_each(std::any_cast<ConstRapidObject>(anyvalue).begin(), std::any_cast<ConstRapidObject>(anyvalue).end(), [&](auto& additional_object) {

                if (additional_object.value.IsNull())
                {
                    error_log("CFlagsConfigObject::Initialize the additional {} is null!", additional_object.name.GetString());
                }
                else {
                    if (std::string_view alias_name = additional_object.name.GetString(); alias_name.length() && additional_object.value.IsString())
                    {
                        std::string_view iden_name = additional_object.value.GetString();
                        std::shared_ptr<CObObject> ob_object = CLexerTy::GetInstance().GetIdentifier(iden_name);
                        if (ob_object)
                            AddNewType(IDENPAIR{ alias_name, std::move(ob_object) });
                        else
                            AddDelayedType(DELTPAIR(alias_name, iden_name));
                    }
                }
            });
        }
        return;
    }

    void CFlagsCfgObject::InitializeStructType(const CRapidJsonWrapper& document)
    {
        if (auto anyvalue(document.GetMember(FL_STRUCTURE));
            anyvalue.has_value() && anyvalue.type() == typeid(ConstRapidObject))
        {
            std::for_each(std::any_cast<ConstRapidObject>(anyvalue).begin(), std::any_cast<ConstRapidObject>(anyvalue).end(), [&](auto& struct_object) {

                if (struct_object.value.IsNull())
                {
                    error_log("CFlagsConfigObject::Initialize the struct {} is null!", struct_object.name.GetString());
                }
                else {
                    bool bdelayed = false;
                    bool bsuccess = true;
                    std::string_view struct_name = struct_object.name.GetString();
                    std::shared_ptr<CStructObject> ob_object = std::make_shared<CStructObject>(struct_name);
                    for (auto& data_object : struct_object.value.GetObject())
                    {
                        if (std::string_view element_name = data_object.name.GetString(); element_name.length() && !data_object.value.IsNull())
                        {
                            if (data_object.value.IsString())
                            {
                                std::string_view iden_name = data_object.value.GetString();
                                std::shared_ptr<CObObject> iden_object = CLexerTy::GetInstance().GetIdentifier(iden_name);
                                if (!iden_object) bdelayed = true;
                                (*ob_object).AddElement(element_name, CStructObject::ElementTypeDefine(std::string(iden_name), { CStructObject::InvalidOffset, std::move(iden_object) }));
                            }
                            else
                            {
                                error_log("CFlagsConfigObject::Initialize the element {}! not a string!", std::string(element_name));
                                bsuccess = false; break;
                            }
                        }
                        else
                        {
                            error_log("CFlagsConfigObject::Initialize the element {}! is null!", data_object.name.GetString());
                            bsuccess = false; break;
                        }
                    }
                    if (bsuccess) {
                        if (bdelayed)
                            AddDelayedStruct(DELSPAIR(struct_name, std::move(ob_object)));
                        else
                            AddNewType(DELSPAIR(struct_name, std::move(ob_object)));
                    }
                }
            });
        }
        return;
    }

    void CFlagsCfgObject::InitializeFlagType(const CRapidJsonWrapper& document)
    {
        if (auto anyvalue(document.GetMember(FL_FLAGS));
            anyvalue.has_value() && anyvalue.type() == typeid(ConstRapidObject))
        {
            std::for_each(std::any_cast<ConstRapidObject>(anyvalue).begin(), std::any_cast<ConstRapidObject>(anyvalue).end(), [&](auto& flag_object) {

                std::string_view flag_name = flag_object.name.GetString();
                std::shared_ptr<CFlagObject> ob_object = std::make_shared<CFlagObject>(flag_name);
                if (!ob_object) return;
                std::string name_str(flag_name);
                auto anydata(document.GetMember(std::vector<std::string> {FL_FLAGS, name_str, FLS_DATA}));
                auto anyop(document.GetMember(std::vector<std::string> {FL_FLAGS, name_str, FLS_OP}));
                auto anystrflg(document.GetMember(std::vector<std::string> {FL_FLAGS, name_str, FLS_VALUE}));
                if ([&]() ->bool{
                    if (!anydata.has_value() || !anyop.has_value() || !anystrflg.has_value())
                    {
                        error_log("CFlagsConfigObject::Initialize the flags {} is a invalid flags!", std::string(flag_name));
                        return false;
                    }
                    if( anydata.type() != typeid(std::string_view) ||
                        anyop.type() != typeid(std::string_view) ||
                        anystrflg.type() != typeid(ConstRapidObject) )
                    {
                        error_log("CFlagsConfigObject::Initialize the flags {} data format incorrect!", std::string(flag_name));
                        return false;
                    }
                    return true;
                    }())
                {
                    if (CFlagObject::_flag_operation flag_op; (flag_op = ob_object->GetFlagOp(std::string(std::any_cast<std::string_view>(anyop)))) == CFlagObject::flag_invalid)
                    {
                        error_log("CFlagsConfigObject::Initialize the flags {} flag_op incorrect!", std::string(flag_name));
                    }
                    else {
                        if (std::shared_ptr<CObObject> data_object; (data_object = CLexerTy::GetInstance().GetIdentifier(std::string(std::any_cast<std::string_view>(anydata)))) == nullptr)
                        {
                            error_log("CFlagsConfigObject::Initialize the flags {} data_object incorrect!", std::string(flag_name));
                        }
                        else {
                            if (std::unique_ptr<std::map<std::string, unsigned int>> flag_values = std::make_unique<std::map<std::string, unsigned int>>(); flag_values)
                            {
                                for (auto& value_object : std::any_cast<ConstRapidObject>(anystrflg))
                                    (*flag_values)[value_object.name.GetString()] = value_object.value.GetUint();
                                if (flag_values->size() == 0)
                                {
                                    error_log("CFlagsConfigObject::Initialize the flags {} flag_values null!", std::string(flag_name));
                                }
                                else {
                                    if (!ob_object->AddFlags(std::move(data_object), flag_op, std::move(flag_values)))
                                    {
                                        error_log("CFlagsConfigObject::Initialize the flags {} AddFlags failed!", std::string(flag_name));
                                    }
                                    else {
                                        AddNewType(IDENPAIR{ flag_name, std::move(ob_object) });
                                    }
                                }
                            }
                        }
                    }
                }
            });
        }
        return;
    }

    bool CFlagsCfgObject::Initialize(const std::string_view& json_str)
    {
        if (json_str.length() == 0) return false;

        CRapidJsonWrapper document(json_str);
        if (!document.IsValid()) {
            //config data is incorrect.
            error_log("CFlagsConfigObject::Initialize failed!");
            return false;
        }

        InitializeAdditionalType(document);
        InitializeStructType(document);
        InitializeFlagType(document);
        // processing delayed types
        ProcessingDelayTypes();
        m_bValid = true;
        return true;
    }

    bool CFlagsCfgObject::AddNewType(const IDENPAIR& delay_pair) const
    {
        return CLexerTy::GetInstance().AddIdentifier(delay_pair);
    }

    bool CFlagsCfgObject::AddDelayedType(const DELTPAIR& delay_pair)
    {
        auto it = m_delayed_type_list.find(delay_pair.first);
        if (it != m_delayed_type_list.end())
        {
            error_log("AddDelayedType: the type {} has exist!", delay_pair.first);
            return false;
        }
        m_delayed_type_list[delay_pair.first] = delay_pair.second;
        return true;
    }

    bool CFlagsCfgObject::AddDelayedStruct(const DELSPAIR& delay_pair)
    {
        auto it = m_delayed_struct_list.find(delay_pair.first);
        if (it != m_delayed_struct_list.end())
        {
            error_log("AddDelayedStruct: new struct {} has exist!", it->first);
            return false;
        }
        m_delayed_struct_list[delay_pair.first] = delay_pair.second;
        return true;
    }

    bool CFlagsCfgObject::ProcessingDelayType(const std::string& name)
    {
        const auto it = m_delayed_type_list.find(CLexerTy::GetWordRoot(name));
        if (it == m_delayed_type_list.end()) return false;
        return ProcessingDelayType(*it);
    }

    bool CFlagsCfgObject::ProcessingDelayType(const DELTPAIR& delt_pair)
    {
        std::shared_ptr<CObObject> ob_object = CLexerTy::GetInstance().GetIdentifier(delt_pair.second);
        if (ob_object)
            return AddNewType(IDENPAIR{ delt_pair.first, ob_object });
        else
        {
            if (!ProcessingDelayTypes(delt_pair.second))
            {
                error_log("ProcessingDelayType: delay type {} can not handle!", delt_pair.first);
                return false;
            }
            ob_object = CLexerTy::GetInstance().GetIdentifier(delt_pair.second);
            if (!ob_object)
            {
                error_log("ProcessingDelayType: delay type {} can not handle!", delt_pair.first);
                return false;
            }
            return AddNewType(IDENPAIR{ delt_pair.first, ob_object });
        }
        return false;
    }

    bool CFlagsCfgObject::ProcessingDelayStruct(const std::string& name)
    {
        const auto it = m_delayed_struct_list.find(CLexerTy::GetWordRoot(name));
        if (it == m_delayed_struct_list.end()) return false;
        return ProcessingDelayStruct(*it);
    }

    bool CFlagsCfgObject::ProcessingDelayStruct(const DELSPAIR& dels_pair)
    {
        std::map<int, std::pair<std::string, CStructObject::ElementTypeDefine>> cache_map;
        for (auto delay_elem : (*(dels_pair.second)))
        {
            cache_map[delay_elem.second.second.first] = std::pair<std::string, CStructObject::ElementTypeDefine>(std::move(delay_elem.first), std::move(delay_elem.second));
        }

        std::string_view struct_name = dels_pair.second->GetName();
        std::shared_ptr<CStructObject> ob_object = std::make_shared<CStructObject>(struct_name);
        for (const auto& struc_elem : cache_map)
        {
            std::shared_ptr<CObObject> iden_object = nullptr;
            if (struc_elem.second.second.second.second)
                iden_object = struc_elem.second.second.second.second;
            else
            {
                iden_object = CLexerTy::GetInstance().GetIdentifier(struc_elem.second.second.first);
                if (!iden_object)
                {
                    if (!ProcessingDelayTypes(struc_elem.second.second.first))
                    {
                        error_log("ProcessingDelayStruct: delay struct({}) element({}) can not handle!", dels_pair.first, struc_elem.second.first);
                        return false;
                    }
                    iden_object = CLexerTy::GetInstance().GetIdentifier(struc_elem.second.second.first);
                    if (!iden_object)
                    {
                        error_log("ProcessingDelayStruct: delay struct({}) element({}) can not handle!", dels_pair.first, struc_elem.second.first);
                        return false;
                    }
                }
            }
            (*ob_object).AddElement(struc_elem.second.first, CStructObject::ElementTypeDefine(std::string(struc_elem.second.second.first), { CStructObject::InvalidOffset, std::move(iden_object) }));
        }
        return AddNewType(IDENPAIR{ dels_pair.first, ob_object });
    }

    bool CFlagsCfgObject::ProcessingDelayTypes(const std::string& name)
    {
        if (ProcessingDelayType(name))
            return true;
        return ProcessingDelayStruct(name);
    }

    void CFlagsCfgObject::ProcessingDelayType()
    {
        std::for_each(m_delayed_type_list.begin(), m_delayed_type_list.end(), [&](const auto& pair) {
            
            if (ProcessingDelayType(pair))
                return;
            ProcessingDelayStruct(pair.first);
            });
        m_delayed_type_list.clear();
    }

    void CFlagsCfgObject::ProcessingDelayStruct()
    {
        std::for_each(m_delayed_struct_list.begin(), m_delayed_struct_list.end(), [&](auto& pair) {
            ProcessingDelayStruct(pair);
            });
        m_delayed_struct_list.clear();
    }

    void CFlagsCfgObject::ProcessingDelayTypes()
    {
        ProcessingDelayType();
        ProcessingDelayStruct();
    }

} // namespace cchips





