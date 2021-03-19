#include <chrono>
#include "ExceptionThrow.h"
#include "utils.h"
#include "PassStructure.h"
#include "rapidjson/prettywriter.h"
#include "PassSupport.h"

namespace cchips {

    bool Module::InitializeAbi()
    {
        if (!Valid()) return false;

        switch (m_module_context->GetBits())
        {
        case 32:
        {
            m_abi = std::make_shared<AbiX86>();
        }
        break;

        case 64:
        {
            m_abi = std::make_shared<AbiX64>();
        }
        break;

        default:
            ;
        }
        return (m_abi != nullptr);
    }

    bool Module::InitializeCommonRegister()
    {
        if (!Valid()) return false;
        if (!m_abi) return false;
        std::uint8_t* address = 0;
        for (auto& reg : m_abi->getReg2Size()) {
            std::string name = GetCapstoneImplment().GetRegName(reg.first);
            if (!name.length()) continue;
            std::stringstream ss;
            ss << GLOBAL_REG_PREFIX << name;
            std::shared_ptr<GlobalVariable> reg_ptr = std::make_shared<GlobalVariable>(shared_from_this(), ss.str(), address, GlobalVariable::variable_reg, reg.second/BYTES_SIZE);
            m_globalvariable_list.emplace(address, std::move(reg_ptr));
            address++;
        }
        return true;
    }

    bool Module::InitializeGlobalIFunction()
    {
        if (!Valid()) return false;
        const std::unique_ptr<ImportTable>& import_table = m_module_context->GetPeFormat()->getImportTable();
        if (import_table) {
            int num_funcs = import_table->getNumberOfImports();
            for (int i = 0; i < num_funcs; i++) {
                auto* import_entry = import_table->getImport(i);
                if (!import_entry) continue;
                std::unique_ptr<GlobalIFunc::func_struc> func_ptr = std::make_unique<GlobalIFunc::func_struc>();
                if (!func_ptr) return false;
                func_ptr->type = GlobalIFunc::func_struc::func_import;
                func_ptr->file_name = import_table->getLibrary(import_entry->getLibraryIndex());
                func_ptr->func_name = import_entry->getName();
                if (!func_ptr->func_name.length()) continue;
                import_entry->getOrdinalNumber(func_ptr->ordinal_number);
                func_ptr->address = reinterpret_cast<std::uint8_t*>(import_entry->getAddress());
                tls_check_struct *tls = check_get_tls();
                if (tls) {
                    tls->active = 1;
                    if (setjmp(tls->jb) == 0) {
                        func_ptr->to_address = *reinterpret_cast<std::uint8_t**>(func_ptr->address);
                        tls->active = 0;
                    }
                    else {
                        //execption occur
                        tls->active = 0;
                        continue;
                    }
                }
                AddGlobalIFunc(std::move(func_ptr));
            }
        }
        const std::unique_ptr<ExportTable>& export_table = m_module_context->GetPeFormat()->getExportTable();
        if (export_table) {
            int num_funcs = export_table->getNumberOfExports();
            for (int i = 0; i < num_funcs; i++) {
                auto* export_entry = export_table->getExport(i);
                if (!export_entry) continue;
                std::unique_ptr<GlobalIFunc::func_struc> func_ptr = std::make_unique<GlobalIFunc::func_struc>();
                if (!func_ptr) return false;
                func_ptr->type = GlobalIFunc::func_struc::func_export;
                func_ptr->file_name = export_table->getName();
                func_ptr->func_name = export_entry->getName();
                if (!func_ptr->func_name.length()) continue;
                export_entry->getOrdinalNumber(func_ptr->ordinal_number);
                func_ptr->address = reinterpret_cast<std::uint8_t*>(export_entry->getAddress());
                func_ptr->to_address = func_ptr->address;
                AddGlobalIFunc(std::move(func_ptr));
            }
        }
        return true;
    }

    bool Module::Initialize() {
        if (!m_precache_address) 
            m_precache_address = reinterpret_cast<std::uint8_t*>(GetModuleHandle(nullptr));
        m_module_context = std::make_unique<ModuleContext>(m_precache_address);
        if (m_module_context && m_module_context->Valid()) {
            InitializeAbi();
            InitializeCommonRegister();
            InitializeGlobalIFunction();
            return true;
        }
        m_module_context = nullptr;
        m_globalvariable_list.clear();
        m_function_list.clear();
        m_globalifunc_list.clear();
        return false;
    }

    bool Module::GetLinkageFuncName(std::string& func_name, std::uint8_t* func_address) const
    {
        if (!Address::validAddress(func_address)) return false;
        auto& it = GetGlobalIFuncs().find(func_address);
        if (it != GetGlobalIFuncs().end() && it->second) {
            func_name = it->second->GetFullName();
            return true;
        }
        return false;
    }

    bool Module::AddFunction(std::string& func_name, std::uint8_t* func_address)
    {
        if (!func_address) return false;
        Function::func_type type = Function::func_normal;
        if (func_name.length() == 0) {

            if (!GetLinkageFuncName(func_name, func_address)) {
                std::stringstream stream;
                stream << std::hex << reinterpret_cast<DWORD_PTR>(func_address);
                func_name = FUNC_NORMAL_SUB + stream.str();
            }
            else {
                type = Function::func_linkage;
            }
        }
        PVOID address = reinterpret_cast<PVOID>(func_address);
        auto& it = m_function_list.find(address);
        if (it == m_function_list.end()) {
            if (func_address == m_module_context->GetOEP()) {
                type = Function::func_oep;
            }
            std::shared_ptr<Function> func_ptr = std::make_shared<Function>(shared_from_this(), func_name, func_address, type);
            if (!func_ptr) return false;
            m_function_list[address] = std::move(func_ptr);
            //m_function_list[address]->Initialize();
            return true;
        }
        return false;
    }

    bool Module::AddGlobalIFunc(std::unique_ptr<GlobalIFunc::func_struc> func_st)
    {
        if (!func_st) return false;
        if (func_st->address == 0 || !func_st->file_name.length() || !func_st->func_name.length())
            return false;
        PVOID address = reinterpret_cast<PVOID>(func_st->to_address);
        auto& it = m_globalifunc_list.find(address);
        if (it == m_globalifunc_list.end()) {
            m_globalifunc_list[address] = std::make_shared<GlobalIFunc>(shared_from_this(), std::move(func_st));
            return true;
        }
        return false;
    }

    void Module::ReportObject::CreateReport(std::shared_ptr<CBaseStruc> object, std::string& desc)
    {
        if (!object) return;
        if (!desc.length()) return;
        report_object obj = { object->GetBaseType(), object->GetBaseAddress(), object};
        auto find = m_report_list.find(obj);
        if (find == m_report_list.end()) {
            m_report_list.emplace(obj, desc);
        }
        return;
    }

    void Module::ReportObject::dump(RapidValue& json_object, rapidjson::MemoryPoolAllocator<>& allocator, Cfg_view_flags flags) const
    {
        RapidValue report_value;
        report_value.SetObject();
        report_value.AddMember("reports", m_report_list.size(), allocator);

        int count = 0;
        for (auto& report : m_report_list) {
            if (!report.first.object.lock() || !report.second.length())
                continue;
            GenerateJsonReport(count, report.first.object.lock(), report.second, report_value, allocator, flags);
        }

        json_object.AddMember("ErrorReport", report_value, allocator);
        return;
    }

    void Module::ReportObject::GenerateJsonReport(int count, std::shared_ptr<CBaseStruc> object, std::string desc, RapidValue& json_object, rapidjson::MemoryPoolAllocator<>& allocator, Cfg_view_flags flags) const
    {
        if (!object) return;
        RapidValue report_value;
        report_value.SetObject();

        switch (object->GetBaseType())
        {
        case base_module:
        {
            std::shared_ptr<Module> cur_module = std::static_pointer_cast<Module>(object);
            std::stringstream ss;
            ss << "0x" << std::hex << reinterpret_cast<unsigned long long>(cur_module->GetContext()->GetBaseAddress());
            report_value.AddMember("object_type", "module", allocator);
            report_value.AddMember("name", RapidValue(cur_module->GetModuleName().c_str(), allocator), allocator);
            report_value.AddMember("address", RapidValue(ss.str().c_str(), allocator), allocator);
            report_value.AddMember("description", RapidValue(desc.c_str(), allocator), allocator);
        }
        break;
        case base_function:
        {
            std::shared_ptr<Function> function = std::static_pointer_cast<Function>(object);
            std::stringstream ss;
            ss << "0x" << std::hex << function->GetFuncAddress();
            report_value.AddMember("object_type", "function", allocator);
            report_value.AddMember("name", RapidValue(function->GetFuncName().c_str(), allocator), allocator);
            report_value.AddMember("address", RapidValue(ss.str().c_str(), allocator), allocator);
            report_value.AddMember("description", RapidValue(desc.c_str(), allocator), allocator);
        }
        break;
        case base_globalifunc:
        {
            std::shared_ptr<GlobalIFunc> ifunc = std::static_pointer_cast<GlobalIFunc>(object);
            report_value.AddMember("object_type", "globalifunc", allocator);
            report_value.AddMember("name", RapidValue(ifunc->GetFullName().c_str(), allocator), allocator);
            report_value.AddMember("description", RapidValue(desc.c_str(), allocator), allocator);
        }
        break;
        case base_basicblock:
        {
            std::shared_ptr<BasicBlock> block = std::static_pointer_cast<BasicBlock>(object);
            std::stringstream ss;
            ss << "0x" << std::hex << reinterpret_cast<unsigned long long>(block->getAddress());
            report_value.AddMember("object_type", "BasicBlock", allocator);
            report_value.AddMember("name", RapidValue(block->GetName().c_str(), allocator), allocator);
            report_value.AddMember("address", RapidValue(ss.str().c_str(), allocator), allocator);
            report_value.AddMember("description", RapidValue(desc.c_str(), allocator), allocator);
        }
        break;
        case base_globalvariable:
        {
            return;
        }
        break;
        case base_instruction:
        {
            std::shared_ptr<CapInsn> insn = std::static_pointer_cast<CapInsn>(object);
            std::stringstream ss;
            ss << "0x" << std::hex << insn->address();
            report_value.AddMember("object_type", "BasicBlock", allocator);
            report_value.AddMember("object_type", "Instruction", allocator);
            report_value.AddMember("name", RapidValue(insn->dump().c_str(), allocator), allocator);
            report_value.AddMember("address", RapidValue(ss.str().c_str(), allocator), allocator);
            report_value.AddMember("description", RapidValue(desc.c_str(), allocator), allocator);
        }
        break;
        default:
            return;
        }
        std::stringstream ss;
        ss << "report-" << count;
        json_object.AddMember(RapidValue(ss.str().c_str(), allocator), report_value, allocator);
    }

    void Module::ModuleContext::InitializePeStructure(std::unique_ptr<PeLib::PeFile> pe_file)
    {
        ASSERT(pe_file);
        tls_check_struct *tls = check_get_tls();
        if (tls) {
            tls->active = 1;
            if (setjmp(tls->jb) == 0) {
                pe_file->readMzHeader();
                pe_file->readPeHeader();
                pe_file->readImportDirectory();
                pe_file->readExportDirectory();
                pe_file->readIatDirectory();
                tls->active = 0;
            }
            else {
                //execption occur
                tls->active = 0;
                pe_file = nullptr;
                module_format = FileDetector::format_unknown;
                return;
            }
        }

        if (tls) {
            tls->active = 1;
            if (setjmp(tls->jb) == 0) {
                pe_file->readBoundImportDirectory();
                tls->active = 0;
            }
            else {
                //execption occur
                tls->active = 0;
            }
        }
        if (tls) {
            tls->active = 1;
            if (setjmp(tls->jb) == 0) {
                pe_file->readDelayImportDirectory();
                tls->active = 0;
            }
            else {
                //execption occur
                tls->active = 0;
            }
        }
        if (tls) {
            tls->active = 1;
            if (setjmp(tls->jb) == 0) {
                pe_file->readCoffSymbolTable();
                tls->active = 0;
            }
            else {
                //execption occur
                tls->active = 0;
            }
        }
        if (tls) {
            tls->active = 1;
            if (setjmp(tls->jb) == 0) {
                pe_file->readDebugDirectory();
                tls->active = 0;
            }
            else {
                //execption occur
                tls->active = 0;
            }
        }
        if (tls) {
            tls->active = 1;
            if (setjmp(tls->jb) == 0) {
                pe_file->readTlsDirectory();
                tls->active = 0;
            }
            else {
                //execption occur
                tls->active = 0;
            }
        }
        if (tls) {
            tls->active = 1;
            if (setjmp(tls->jb) == 0) {
                pe_file->readResourceDirectory();
                tls->active = 0;
            }
            else {
                //execption occur
                tls->active = 0;
            }
        }
        if (tls) {
            tls->active = 1;
            if (setjmp(tls->jb) == 0) {
                pe_file->readSecurityDirectory();
                tls->active = 0;
            }
            else {
                //execption occur
                tls->active = 0;
            }
        }
        if (tls) {
            tls->active = 1;
            if (setjmp(tls->jb) == 0) {
                pe_file->readComHeaderDirectory();
                tls->active = 0;
            }
            else {
                //execption occur
                tls->active = 0;
            }
        }
        pe_format = std::make_unique<PeFormat>(std::move(pe_file));
        return;
    }

    std::shared_ptr<CBaseStruc> Module::GetBaseObjectAtAddress(std::uint8_t* address) const
    {
        auto func = m_function_list.find(address);
        if (func != m_function_list.end())
            return func->second;
        auto ifunc = m_globalifunc_list.find(address);
        if (ifunc != m_globalifunc_list.end())
            return ifunc->second;
        auto variable = m_globalvariable_list.find(address);
        if (variable != m_globalvariable_list.end())
            return variable->second;
        return nullptr;
    }

    void Module::dump(const std::string& output_file, Cfg_view_flags flags) const
    {
        if (!Valid()) return;
        std::ofstream jsonFile(output_file, std::ofstream::trunc);
        if (!jsonFile) return;

        auto tt = std::chrono::system_clock::to_time_t
        (std::chrono::system_clock::now());
        struct tm* ptm = localtime(&tt);
        char date[60] = { 0 };
        sprintf(date, "%d-%02d-%02d-%02d.%02d.%02d",
            (int)ptm->tm_year + 1900, (int)ptm->tm_mon + 1, (int)ptm->tm_mday,
            (int)ptm->tm_hour, (int)ptm->tm_min, (int)ptm->tm_sec);

        RapidDocument document;
        auto& allocator = document.GetAllocator();
        std::string time_str = date;
        document.SetObject();
        document.AddMember("time", RapidValue(time_str.c_str(), allocator), allocator);
        
        RapidValue module_value;
        module_value.SetObject();
        std::string module_name = GetModuleName();
        if (!module_name.length()) module_name = "unknown_module";
        std::stringstream ss;
        ss << "0x" << std::hex << GetBaseAddress();
        module_value.AddMember("module_name", RapidValue(module_name.c_str(), allocator), allocator);
        module_value.AddMember("architecture", RapidValue(GetArchitecture().c_str(), allocator), allocator);
        module_value.AddMember("address", RapidValue(ss.str().c_str(), allocator), allocator);
        module_value.AddMember("functions", size(), allocator);

        for (auto& func : m_function_list) {
            if (func.first && func.second) {
                func.second->dump(module_value, allocator, flags);
            }
        }
        m_report_object.dump(module_value, allocator);
        document.AddMember("module", module_value, allocator);

        rapidjson::StringBuffer buf;
        rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buf);
        document.Accept(writer);
        jsonFile << buf.GetString();
        return;
    }

    bool Function::Initialize()
    {
        if (!m_parent.lock()) return false;
        if (!m_func_name.length()) return false;
        if (!m_func_address) return false;
        if (m_func_type > func_normal) return false;
        m_basicblocks.clear();
        if (m_func_type == func_linkage) return true;
        JumpTargets jump_targets;
        jump_targets.push(m_func_address, JumpTarget::jmp_type::JMP_ENTRY_POINT, JumpTarget::from_type(0, JumpTarget::jmp_type::JMP_UNKNOWN));
        return Decode(jump_targets);
    }

    void Function::dump(RapidValue& json_object, rapidjson::MemoryPoolAllocator<>& allocator, Cfg_view_flags flags) const
    {
        RapidValue func_value;
        func_value.SetObject();
        func_value.AddMember("func_name", RapidValue(GetFuncName().c_str(), allocator), allocator);
        std::stringstream ss;
        ss << "0x" << std::hex << GetFuncAddress();
        func_value.AddMember("address", RapidValue(ss.str().c_str(), allocator), allocator);
        func_value.AddMember("func_type", RapidValue(GetFuncType().c_str(), allocator), allocator);
        func_value.AddMember("basicblocks", getCurrentBlockNo(), allocator);

        for (auto& basicblock : m_basicblocks) {
            if (basicblock.first && basicblock.second) {
                basicblock.second->dump(func_value, allocator, flags);
            }
        }

        json_object.AddMember("Function", func_value, allocator);
        return;
    }

    std::shared_ptr<BasicBlock> Function::getBasicBlockAtAddress(std::uint8_t* address)
    {
        if (!address) return {};
        for (auto& block : m_basicblocks) {
            if (!block.second) continue;
            if (block.second->getAddress() <= address && block.second->getEndAddress() > address)
                return block.second;
        }
        return {};
    }

    size_t Function::GetPredictSize(const JumpTargets& jump_targets, std::uint8_t* address) {
        size_t predict_size = 0;
        std::shared_ptr<Module> parent = m_parent.lock();
        if (!parent) return predict_size;
        std::shared_ptr<PeCoffSection> section = parent->GetContext()->GetPeFormat()->getPeSection(address);
        if (section) {
            predict_size = section->getPredictAddress(address) - reinterpret_cast<unsigned long long>(address);
        }
        std::shared_ptr<BasicBlock> after_block = getBasicBlockAfter(address);
        if (after_block) {
            std::uint8_t* after_addr = after_block->getAddress();
            if (predict_size > after_addr - address)
                predict_size = after_addr - address;
        }
        if (std::uint8_t* after_addr = jump_targets.getAddressAfter(address)) {
            if (predict_size > after_addr - address)
                predict_size = after_addr - address;
        }
        return predict_size;
    }

    std::shared_ptr<BasicBlock> Function::getBasicBlockAfter(std::uint8_t* address)
    {
        std::shared_ptr<BasicBlock> result;
        for (auto& block : m_basicblocks) {
            if (!block.second) continue;
            if (block.second->getAddress() > address) {
                if (!result) result = block.second;
                if (result && block.second->getAddress() < result->getAddress()) {
                    result = block.second;
                }
            }
        }
        return result;
    }

    bool Function::getJumpTarget(JumpTargets& jump_targets, JumpTarget& jt)
    {
        if (!jump_targets.empty())
        {
            jt = jump_targets.top();
            jump_targets.pop();
            return true;
        } 
        return false;
    }

    std::shared_ptr<BasicBlock> Function::CreateBasicBlock(const JumpTarget& jt)
    {
        std::shared_ptr<BasicBlock> basic_block = nullptr;
        switch (jt.getType()) {
        case JumpTarget::jmp_type::JMP_UNKNOWN:
            return {};
        case JumpTarget::jmp_type::JMP_ENTRY_POINT:
        {
            basic_block = std::make_shared<BasicBlock>(shared_from_this(), std::string(FIRST_BLOCK_NAME), jt.getToAddress(), 0, BasicBlock::block_start);
        }
        break;
        default:
        {
            std::stringstream ss;
            ss << BLOCK_NORMAL_LOC << std::hex << reinterpret_cast<DWORD_PTR>(jt.getToAddress());
            basic_block = std::make_shared<BasicBlock>(shared_from_this(), ss.str(), jt.getToAddress(), getCurrentBlockNo(), BasicBlock::block_unknown);
        }
        }
        if (!basic_block) return {};
        if (!basic_block->Valid()) return {};
        auto& exsit_block = m_basicblocks.find(jt.getToAddress());
        if (exsit_block == m_basicblocks.end()) {
            m_basicblocks[jt.getToAddress()] = basic_block;
        }
        return basic_block;
    }

    bool Function::splitBasicBlockAtAddress(const JumpTarget& jt)
    {
        std::uint8_t* start = jt.getToAddress();
        if (!Address::validAddress(start))
            return false;

        std::shared_ptr<BasicBlock> basic_block = getBasicBlockAtAddress(start);
        if (basic_block) {
            if (basic_block->getAddress() == start)
                return true;
            std::shared_ptr<BasicBlock> new_block = basic_block->splitBasicBlockAtAddress(start);
            if (new_block) return true;
        }
        return false;
    }

    void Function::linktoFromBasicBlock(std::shared_ptr<BasicBlock> cur_block, const JumpTarget& jt)
    {
        for (auto& from : jt) {
            if (!Address::validAddress(from.first))
                return;
            std::shared_ptr<BasicBlock> pre_block = getBasicBlockAtAddress(from.first);
            if (!pre_block) return;
            if (!linkBasicBlock(pre_block, cur_block, from.second))
                return;
        }
        return;
    }

    void Function::processJmpInstruction(std::shared_ptr<BasicBlock> cur_block, std::unique_ptr<CapInsn> cap_insn, std::uint8_t* insn_code, JumpTargets& jump_targets)
    {
        std::uint8_t* next_addr = nullptr;
        std::uint8_t* jmp_addr = nullptr;
        std::shared_ptr<Module> parent = m_parent.lock();
        if (!parent) return;
        std::shared_ptr<PeCoffSection> section = parent->GetContext()->GetPeFormat()->getPeSection(cur_block->getAddress());
        if (!section) return;
        Range<std::uint8_t*> section_range;
        section_range.setStartEnd(reinterpret_cast<std::uint8_t*>(section->getAddress()), reinterpret_cast<std::uint8_t*>(section->getEndAddress()));
        Range<std::uint8_t*> image_range;
        image_range.setStartEnd(const_cast<std::uint8_t*>(parent->GetContext()->GetBaseAddress()), const_cast<std::uint8_t*>(parent->GetContext()->GetEndAddress()));
        if (x86_op_type op_type; GetCapstoneImplment().GetJmpAddress(*cap_insn, next_addr, jmp_addr, op_type)) {

            if (section_range.contains(next_addr)) {
                if (!getBasicBlockAtAddress(next_addr)) {
                    jump_targets.push(next_addr, JumpTarget::jmp_type::JMP_NORMAL, JumpTarget::from_type(insn_code, JumpTarget::jmp_type::JMP_CONTROL_FLOW_BR_FALSE));
                }
            }
            else {
                // Cross section? feature.
            }
            if (image_range.contains(jmp_addr)) {
                if (op_type == X86_OP_MEM) {
                    tls_check_struct *tls = check_get_tls();
                    if (tls) {
                        tls->active = 1;
                        if (setjmp(tls->jb) == 0) {
                            jmp_addr = *reinterpret_cast<std::uint8_t**>(jmp_addr);
                            tls->active = 0;
                        }
                        else {
                            //execption occur
                            tls->active = 0;
                        }
                    }
                }
                std::shared_ptr<BasicBlock> jmp_block = getBasicBlockAtAddress(jmp_addr);
                if (jmp_block) {
                    JumpTarget target(jmp_addr, JumpTarget::jmp_type::JMP_NORMAL, JumpTarget::from_type(insn_code, JumpTarget::jmp_type::JMP_CONTROL_FLOW_BR_TRUE));
                    splitBasicBlockAtAddress(target);
                    if (jmp_block->getAddress() == cur_block->getAddress()) {
                        std::shared_ptr<BasicBlock> new_block = getBasicBlockAtAddress(jmp_addr);
                        if (new_block) {
                            new_block->AddCapInsn(std::move(std::shared_ptr<CapInsn>(cap_insn.release())));
                            return;
                        }
                    }
                } 
                else if (std::shared_ptr<CBaseStruc> jmpto_base; jmpto_base = parent->GetBaseObjectAtAddress(jmp_addr)) {
                }
                else {
                    if (image_range.contains(jmp_addr)) {
                        jump_targets.push(jmp_addr, JumpTarget::jmp_type::JMP_NORMAL, JumpTarget::from_type(insn_code, JumpTarget::jmp_type::JMP_CONTROL_FLOW_BR_TRUE));
                    }
                }
            }
            cur_block->AddCapInsn(std::move(std::shared_ptr<CapInsn>(cap_insn.release())));
        }
        return;
    }

    void Function::processLoopInstruction(std::shared_ptr<BasicBlock> cur_block, std::unique_ptr<CapInsn> cap_insn, std::uint8_t* insn_code, JumpTargets& jump_targets)
    {
        std::uint8_t* next_addr = nullptr;
        std::uint8_t* loop_addr = nullptr;
        std::shared_ptr<Module> parent = m_parent.lock();
        if (!parent) return;
        std::shared_ptr<PeCoffSection> section = parent->GetContext()->GetPeFormat()->getPeSection(cur_block->getAddress());
        if (!section) return;
        Range<std::uint8_t*> section_range;
        section_range.setStartEnd(reinterpret_cast<std::uint8_t*>(section->getAddress()), reinterpret_cast<std::uint8_t*>(section->getEndAddress()));
        if (x86_op_type op_type; GetCapstoneImplment().GetLoopAddress(*cap_insn, next_addr, loop_addr, op_type)) {

            if (section_range.contains(next_addr)) {
                if (!getBasicBlockAtAddress(next_addr)) {
                    jump_targets.push(next_addr, JumpTarget::jmp_type::JMP_NORMAL, JumpTarget::from_type(insn_code, JumpTarget::jmp_type::JMP_CONTROL_FLOW_BR_FALSE));
                }
            }
            else {
                // Cross section? feature.
            }
            if (section_range.contains(loop_addr)) {
                if (op_type == X86_OP_MEM) {

                }
                std::shared_ptr<BasicBlock> loop_block = getBasicBlockAtAddress(loop_addr);
                if (loop_block) {
                    JumpTarget target(loop_addr, JumpTarget::jmp_type::JMP_NORMAL, JumpTarget::from_type(insn_code, JumpTarget::jmp_type::JMP_CONTROL_FLOW_BR_TRUE));
                    splitBasicBlockAtAddress(target);
                }
                else {
                    jump_targets.push(loop_addr, JumpTarget::jmp_type::JMP_NORMAL, JumpTarget::from_type(insn_code, JumpTarget::jmp_type::JMP_CONTROL_FLOW_BR_TRUE));
                }
            }
            else {
                // Cross section? feature.
            }
            cur_block->AddCapInsn(std::move(std::shared_ptr<CapInsn>(cap_insn.release())));
        }
        return;
    }

    void Function::decodeJumpTarget(JumpTargets& jump_targets, const JumpTarget& jt)
    {
        std::uint8_t* start = jt.getToAddress();
        if (!Address::validAddress(start))
            return;
        std::shared_ptr<Module> parent = m_parent.lock();
        if (!parent) 
            return;
        size_t predict_size = GetPredictSize(jump_targets, start);
        if (!predict_size) 
            return;
        std::shared_ptr<BasicBlock> current_block = CreateBasicBlock(jt);
        if (!current_block) 
            return;
        //linktoFromBasicBlock(current_block, jt);
        std::uint8_t* insn_code = start;
        while (std::unique_ptr<CapInsn> cap_insn = cchips::GetCapstoneImplment().DisasmOneInsn(insn_code, predict_size)) {
            size_t code_size = cap_insn->size();
            if (GetCapstoneImplment().InJmpGroup(*cap_insn)) {
                processJmpInstruction(current_block, std::move(cap_insn), insn_code, jump_targets);
                break;
            }
            else if (cchips::GetCapstoneImplment().InRetGroup(*cap_insn)) {
                current_block->AddCapInsn(std::move(std::shared_ptr<CapInsn>(cap_insn.release())));
                break;
            }
            else if (cchips::GetCapstoneImplment().InLoopGroup(*cap_insn)) {
                processLoopInstruction(current_block, std::move(cap_insn), insn_code, jump_targets);
                break;
            }
            else {
                current_block->AddCapInsn(std::move(std::shared_ptr<CapInsn>(cap_insn.release())));
            }
            if (predict_size <= code_size) {
                break;
            }
            insn_code += code_size;
            predict_size -= code_size;
        }
    }

    bool Function::Decode(JumpTargets& jump_targets)
    {
        if (!jump_targets.size()) return false;
        JumpTarget jt;
        while (getJumpTarget(jump_targets, jt))
        {
            decodeJumpTarget(jump_targets, jt);
        }
        return true;
    }

    std::shared_ptr<BasicBlock> BasicBlock::splitBasicBlockAtAddress(std::uint8_t* address)
    {
        if (!contains(address)) return {};
        if (address == getAddress()) return {};
        int insn_no = 0;
        std::shared_ptr<Function> parent = m_parent.lock();
        if (!parent) return {};
        std::uint8_t* pre_insn_addr = 0;
        std::uint8_t* cur_insn_addr = getAddress();
        std::shared_ptr<BasicBlock> split_block = nullptr;

        for (auto& insn : m_capinsns) {
            if (!insn) return false;
            if (split_block) {
                split_block->AddCapInsn(insn);
            }
            else {
                if (cur_insn_addr == address) {
                    JumpTarget jt(address, JumpTarget::jmp_type::JMP_NORMAL, JumpTarget::from_type(pre_insn_addr, JumpTarget::jmp_type::JMP_CONTROL_FLOW_BR_FALSE));
                    split_block = parent->CreateBasicBlock(jt);
                    if (!split_block) return {};
                    if (!split_block->Valid()) return {};
                    split_block->AddCapInsn(insn);
                    continue;
                }
                insn_no++;
                pre_insn_addr = cur_insn_addr;
                cur_insn_addr += insn->size();
            }
        }
        if (!split_block) return {};
        m_capinsns.erase(begin() + insn_no, end());
        m_size -= split_block->getBlockSize();
        return split_block;
    }

    void BasicBlock::dump(RapidValue& json_object, rapidjson::MemoryPoolAllocator<>& allocator, Cfg_view_flags flags) const
    {
        RapidValue block_value;
        block_value.SetObject();
        block_value.AddMember("block_name", RapidValue(GetName().c_str(), allocator), allocator);
        std::stringstream ss;
        ss << "0x" << std::hex << reinterpret_cast<unsigned long long>(getAddress());
        block_value.AddMember("address", RapidValue(ss.str().c_str(), allocator), allocator);
        block_value.AddMember("block_type", RapidValue(getBlockType().c_str(), allocator), allocator);
        block_value.AddMember("block_no", getBlockNo(), allocator);

        RapidValue insns_value;
        insns_value.SetObject();
        for (auto& insn : m_capinsns) {
            if (insn) {
                insn->dump(insns_value, allocator, flags);
            }
        }
        block_value.AddMember("insns", insns_value, allocator);
        ss.clear(); ss.str("");
        ss << "block-" << getBlockNo();
        json_object.AddMember(RapidValue(ss.str().c_str(), allocator), block_value, allocator);
        return;
    }

    Variable::Variable(std::shared_ptr<Module> parent, std::string& name, std::uint8_t* address, variable_type type, std::uint8_t bytes, base_type btype)
        : m_parent(parent),
        CBaseStruc(btype),
        m_address(address), 
        m_type(type)
    {
        std::shared_ptr<CMetadataTypeObject> byte_ptr = make_metadata_j_ptr<BYTE>(CBaseDef::type_byte, INVALID_VARIABLE_VALUE, CObObject::op_n_equal);
        if (!byte_ptr) return;
        std::unique_ptr<CArrayObject> array_ptr = std::make_unique<CArrayObject>(name);
        if (!array_ptr) return;
        array_ptr->AddArray(byte_ptr, bytes);
        m_object = std::move(array_ptr);
    }
} // namespace cchips
