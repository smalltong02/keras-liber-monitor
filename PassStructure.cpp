#include <chrono>
#include "ExceptionThrow.h"
#include "utils.h"
#include "PassStructure.h"
#include "rapidjson/prettywriter.h"
#include "PassSupport.h"
#include "utf8.h"

namespace cchips {

    std::atomic_uint64_t Module::m_insn_counts = 0;

    const std::vector<std::string> GlobalIFunc::_main_func_universal_name = {
        {"^(?i).*corexemain$"},
    };

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
            break;
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
            std::shared_ptr<GlobalVariable> reg_ptr = std::make_shared<GlobalVariable>(shared_from_this(), ss.str(), address, GlobalVariable::variable_reg, reg.second / BYTES_SIZE);
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
                func_ptr->to_address = func_ptr->address;
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
                if (export_entry->isLinkage()) {
                    func_ptr->blinkage = true;
                    Range<std::uint8_t*> section_range;
                    section_range.setStartEnd(const_cast<std::uint8_t*>(m_module_context->GetBaseAddress()), const_cast<std::uint8_t*>(m_module_context->GetEndAddress()));
                    if (section_range.contains(func_ptr->address)) {
                        if (isPrintableChar(*(char*)func_ptr->address)) {
                            func_ptr->linkage_name = std::string((char*)func_ptr->address);
                        }
                    }
                    if(!func_ptr->linkage_name.length()) {
                        func_ptr->linkage_name = func_ptr->func_name;
                    }
                }
                AddGlobalIFunc(std::move(func_ptr));
            }
        }
        return true;
    }

    bool Module::Initialize() {
        if (!m_precache_address) {
            if (m_precache_path.empty()) {
                m_precache_address = reinterpret_cast<std::uint8_t*>(GetModuleHandle(nullptr));
                m_module_context = std::make_unique<ModuleContext>(m_precache_address);
            }
            else {
                m_module_context = std::make_unique<ModuleContext>(m_precache_path);
            }
        }
        else {
            m_module_context = std::make_unique<ModuleContext>(m_precache_address);
        }
        
        if (m_module_context && m_module_context->Valid()) {
            InitializeAbi();
            InitializeCommonRegister();
            InitializeGlobalIFunction();
            m_cfgraph = std::make_unique<ControlFlowGraph>(shared_from_this(), ControlFlowGraph::cfg_apiflow);
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
        if (!Valid()) return false;
        if (!func_address) return false;
        std::uint32_t type = Function::func_normal;
        std::string linkage_name;
        if (func_name.length() == 0) {

            if (!GetLinkageFuncName(func_name, func_address)) {
                std::stringstream stream;
                stream << std::hex << reinterpret_cast<DWORD_PTR>(func_address);
                func_name = FUNC_NORMAL_SUB + stream.str();
            }
            else {
                type = Function::func_linkage;
                linkage_name = func_name;
            }
        }
        PVOID address = reinterpret_cast<PVOID>(func_address);
        auto& it = m_function_list.find(address);
        if (it == m_function_list.end()) {
            if (func_address == m_module_context->GetOEP()) {
                type = Function::func_oep;
            }
            if (std::shared_ptr<CBaseStruc> callto_base; callto_base = GetBaseObjectAtAddress(func_address, base_globalifunc)) {
                auto& dynamic_gfunc = std::dynamic_pointer_cast<GlobalIFunc>(callto_base);
                if (dynamic_gfunc && dynamic_gfunc->GetFuncType() == GlobalIFunc::func_struc::func_type::func_export) {
                    type = Function::func_export;
                    if (dynamic_gfunc->Islinkagefunc()) {
                        type |= Function::func_linkage;
                        linkage_name = dynamic_gfunc->GetlinkageName();
                    }
                }
            }
            std::shared_ptr<Function> func_ptr = std::make_shared<Function>(shared_from_this(), func_name, m_function_list.size(), func_address, type);
            if (!func_ptr) return false;
            if (type & Function::func_linkage) {
                func_ptr->SetLinkageName(linkage_name);
            }
            m_function_list[address] = std::move(func_ptr);
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
        report_object obj = { object->GetBaseType(), object->GetBaseAddress(), object };
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
            count++;
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
        pe_format = std::make_unique<PeFormat>(std::move(pe_file));
        return;
    }

    void Module::ModuleContext::InitializeRanges()
    {
        decode_ranges.setArchitectureInstructionAlignment(0);
        if (!pe_format || !pe_format->IsValid())
            return;
        const auto seccounts = pe_format->getDeclaredNumberOfSections();
        for (std::size_t i = 0; i < seccounts; ++i)
        {
            const auto& fsec = pe_format->getPeSection(i);
            if (fsec)
            {
                auto start = fsec->getAddress();
                auto end = fsec->getEndAddress();
                if (end > start) {
                    if (fsec->isSomeCode()) {
                        decode_ranges.addPrimary(start, end);
                    }
                    else if (fsec->isSomeData() || fsec->isUndefined()) {
                        decode_ranges.addAlternative(start, end);
                    }
                }
            }
        }
        if (decode_ranges.primaryEmpty()) {
            decode_ranges.promoteAlternativeToPrimary();
        }
        return;
    }

    std::unique_ptr<PeLib::PeFile> Module::ModuleContext::GetPeFile(const std::string& path)
    {
        PeLib::PeFile32 pefile(path);
        if (pefile.readMzHeader() != PeLib::ERROR_NONE) {
            return nullptr;
        }
        if (!pefile.mzHeader().isValid()) {
            return nullptr;
        }
        if (pefile.readPeHeader() != PeLib::ERROR_NONE) {
            return nullptr;
        }
        if (!pefile.peHeader().isValid()) {
            return nullptr;
        }
        WORD machine = pefile.peHeader().getMachine();
        WORD magic = pefile.peHeader().getMagic();

        if ((machine == PeLib::PELIB_IMAGE_FILE_MACHINE_AMD64
            || machine == PeLib::PELIB_IMAGE_FILE_MACHINE_IA64)
            && magic == PeLib::PELIB_IMAGE_NT_OPTIONAL_HDR64_MAGIC) {
            return std::make_unique<PeLib::PeFile64>(path);
        }
        return std::make_unique<PeLib::PeFile32>(path);
    }

    std::shared_ptr<CBaseStruc> Module::GetBaseObjectAtAddress(std::uint8_t* address, CBaseStruc::base_type basetype) const
    {
        if (basetype == base_invalid || basetype == base_function) {
            auto func = m_function_list.find(address);
            if (func != m_function_list.end())
                return func->second;
        }
        if (basetype == base_invalid || basetype == base_globalifunc) {
            auto ifunc = m_globalifunc_list.find(address);
            if (ifunc != m_globalifunc_list.end())
                return ifunc->second;
        }
        if (basetype == base_invalid || basetype == base_globalvariable) {
            auto variable = m_globalvariable_list.find(address);
            if (variable != m_globalvariable_list.end())
                return variable->second;
        }
        return nullptr;
    }

    bool Module::LinkInsntoGlobal(CapInsn& insn, std::uint8_t* addr) const
    {
        if (!addr) return false;
        if (std::shared_ptr<CBaseStruc> callto_base; callto_base = GetBaseObjectAtAddress(addr)) {
            if (callto_base) {
                switch (callto_base->GetBaseType())
                {
                case CBaseStruc::base_function:
                {
                    std::shared_ptr<Function> post_func = std::static_pointer_cast<Function>(callto_base);
                    insn.setPointerObject(callto_base);
                    insn.updateMnemonic(post_func->GetFuncName(), CapInsn::insn_intfunc);
                    return true;
                }
                break;
                case CBaseStruc::base_globalifunc:
                {
                    std::shared_ptr<GlobalIFunc> post_ifunc = std::static_pointer_cast<GlobalIFunc>(callto_base);
                    insn.setPointerObject(callto_base);
                    insn.updateMnemonic(post_ifunc->GetFullName(), CapInsn::insn_linkfunc);
                    return true;
                }
                break;
                case CBaseStruc::base_globalvariable:
                {
                    std::shared_ptr<GlobalVariable> post_ifunc = std::static_pointer_cast<GlobalVariable>(callto_base);
                    insn.setPointerObject(callto_base);
                    insn.updateMnemonic(post_ifunc->GetName(), CapInsn::insn_linkvar);
                    return true;
                }
                break;
                default:
                    // don't know.
                    break;
                }
            }
        }

        return false;
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
        if (m_cfgraph) {
            m_cfgraph->dump(module_value, allocator);
        }
        document.AddMember("module", module_value, allocator);

        rapidjson::StringBuffer buf;
        rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buf);
        document.Accept(writer);
        jsonFile << buf.GetString();
        return;
    }

    void Module::dump(const std::unique_ptr<cchips::CRapidJsonWrapper>& document, Cfg_view_flags flags) const
    {
        if (!Valid()) return;
        if (!document) return;
        std::unique_ptr<RapidValue> module_value = std::make_unique<RapidValue>();
        if (!module_value) return;
        module_value->SetObject();
        auto& allocator = document->GetAllocator();
        std::string module_name = GetModuleName();
        if (!module_name.length()) module_name = "unknown_module";
        std::stringstream ss;
        ss << "0x" << std::hex << GetBaseAddress();
        module_value->AddMember("module_name", RapidValue(module_name.c_str(), allocator), allocator);
        module_value->AddMember("architecture", RapidValue(GetArchitecture().c_str(), allocator), allocator);
        module_value->AddMember("address", RapidValue(ss.str().c_str(), allocator), allocator);
        module_value->AddMember("functions", size(), allocator);

        for (auto& func : m_function_list) {
            if (func.first && func.second) {
                func.second->dump(*module_value, allocator, flags);
            }
        }
        m_report_object.dump(*module_value, allocator);
        //if (m_cfgraph) {
        //    m_cfgraph->dump(*module_value, allocator);
        //}

        document->CopyRapidValue(std::move(module_value));
        return;
    }

    bool Function::Initialize()
    {
        std::shared_ptr<Module> cur_module = m_parent.lock();
        if (!cur_module) return false;
        if (!m_func_name.length()) return false;
        if (!m_func_address) return false;
        m_basicblocks.clear();
        if (m_func_type & func_linkage) return true;
        JumpTargets jump_targets;
        jump_targets.push(m_func_address, JumpTarget::jmp_type::JMP_ENTRY_POINT, JumpTarget::from_type(0, JumpTarget::jmp_type::JMP_UNKNOWN));
        return Decode(jump_targets);
    }

    void Function::dump(RapidValue& json_object, rapidjson::MemoryPoolAllocator<>& allocator, Cfg_view_flags flags) const
    {
        RapidValue func_value;
        func_value.SetObject();
        func_value.AddMember("func_name", RapidValue(GetFuncName().c_str(), allocator), allocator);
        if (contain_invalid_utf8char(GetLinkageName())) {
            func_value.AddMember("linkage_name", RapidValue(GetFuncName().c_str(), allocator), allocator);
        }
        else {
            func_value.AddMember("linkage_name", RapidValue(GetLinkageName().c_str(), allocator), allocator);
        }
        func_value.AddMember("func_no", RapidValue(getFunctionNo()), allocator);
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

        ss.clear(); ss.str("");
        ss << "function-" << std::dec << getFunctionNo();
        json_object.AddMember(RapidValue(ss.str().c_str(), allocator), func_value, allocator);
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
        auto& ranges = parent->GetContext()->GetRangesToDecode();
        auto addr_range = ranges.getPrimary((std::uint64_t)address);
        if (!addr_range) {
            addr_range = ranges.getAlternative((std::uint64_t)address);
        }
        if (!addr_range) {
            return predict_size;
        }
        predict_size = addr_range->getEnd() - (std::uint64_t)address;
        //std::shared_ptr<PeCoffSection> section = parent->GetContext()->GetPeFormat()->getPeSection(address);
        //if (section) {
        //    predict_size = section->getPredictAddress(address) - reinterpret_cast<unsigned long long>(address);
        //}
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
        std::shared_ptr<Module> cur_module = m_parent.lock();
        if (cur_module) {
            cur_module->RemeberBasicBlock(basic_block);
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
        if (x86_op_type op_type; GetCapstoneImplment().GetJmpAddress(parent, *cap_insn, next_addr, jmp_addr, op_type)) {
            if (!jmp_addr) {
                cur_block->AddCapInsn(std::move(std::shared_ptr<CapInsn>(cap_insn.release())));
                return;
            }
            if (section_range.contains(next_addr)) {
                if (!getBasicBlockAtAddress(next_addr)) {
                    jump_targets.push(next_addr, JumpTarget::jmp_type::JMP_NORMAL, JumpTarget::from_type(insn_code, JumpTarget::jmp_type::JMP_CONTROL_FLOW_BR_FALSE));
                }
            }
            else {
                // Cross section? feature.
            }
            do {
                if (parent->LinkInsntoGlobal(*cap_insn, jmp_addr))
                    break;
                if (!image_range.contains(jmp_addr))
                    break;
                if (op_type == X86_OP_MEM) {
                    tls_check_struct* tls = check_get_tls();
                    if (tls) {
                        tls->active = 1;
                        if (setjmp(tls->jb) == 0) {
                            if (parent->GetContext()->isLoadingFile()) {
                                if (parent->GetContext()->GetBits() == 64) {
                                    jmp_addr = (std::uint8_t*)(*reinterpret_cast<std::uint64_t*>(jmp_addr));
                                }
                                else {
                                    jmp_addr = (std::uint8_t*)(*reinterpret_cast<std::uint32_t*>(jmp_addr));
                                }
                                jmp_addr = (std::uint8_t*)(parent->GetContext()->getValidOffsetFromMemRva((std::uint64_t)jmp_addr));
                            }
                            else {
                                jmp_addr = *reinterpret_cast<std::uint8_t**>(jmp_addr);
                            }
                            tls->active = 0;
                            if (!image_range.contains(jmp_addr))
                                break;
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
                else if (!parent->LinkInsntoGlobal(*cap_insn, jmp_addr)) {
                    jump_targets.push(jmp_addr, JumpTarget::jmp_type::JMP_NORMAL, JumpTarget::from_type(insn_code, JumpTarget::jmp_type::JMP_CONTROL_FLOW_BR_TRUE));
                }
            } while (0);
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
            if (!loop_addr)
                return;
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
                    if (loop_block->getAddress() == cur_block->getAddress()) {
                        std::shared_ptr<BasicBlock> new_block = getBasicBlockAtAddress(loop_addr);
                        if (new_block) {
                            new_block->AddCapInsn(std::move(std::shared_ptr<CapInsn>(cap_insn.release())));
                            return;
                        }
                    }
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
        if (!predict_size) {
            if (!size()) {
                std::shared_ptr<BasicBlock> current_block = CreateBasicBlock(jt);
                if (!current_block)
                    return;
                current_block->setBlockType(BasicBlock::block_end);
            }
            return;
        }
        std::shared_ptr<BasicBlock> current_block = CreateBasicBlock(jt);
        if (!current_block)
            return;
        //linktoFromBasicBlock(current_block, jt);
        std::uint8_t* insn_code = start;
        std::unique_ptr<CapInsn> cap_insn = cchips::GetCapstoneImplment().DisasmOneInsn(insn_code, predict_size);
        while (cap_insn) {
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
            else if (cchips::GetCapstoneImplment().InIntGroup(*cap_insn)) {
                //if (cap_insn->GetInsnId() != X86_INS_INT1 && cap_insn->GetInsnId() != X86_INS_INT3) {
                    current_block->AddCapInsn(std::move(std::shared_ptr<CapInsn>(cap_insn.release())));
                    break;
                //}
                //current_block->AddCapInsn(std::move(std::shared_ptr<CapInsn>(cap_insn.release())));
            }
            else {
                current_block->AddCapInsn(std::move(std::shared_ptr<CapInsn>(cap_insn.release())));
            }
            if (predict_size <= code_size) {
                current_block->setBlockType(BasicBlock::block_end);
                break;
            }
            insn_code += code_size;
            predict_size -= code_size;
            cap_insn = cchips::GetCapstoneImplment().DisasmOneInsn(insn_code, predict_size);
            if (!cap_insn) {
                current_block->setBlockType(BasicBlock::block_end);
            }
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

    bool BasicBlock::AddCapInsn(std::shared_ptr<CapInsn> cap_insn)
    {
        if (!cap_insn) return false;
        if (!cap_insn->valid()) return false;
        GetParent()->GetParent()->IncInsnCounts();
        auto& ranges = GetParent()->GetParent()->GetContext()->GetRangesToDecode();
        ranges.remove(cap_insn->address(), cap_insn->address() + cap_insn->size());
        m_size += cap_insn->size();
        cap_insn->SetParent(shared_from_this());
        m_capinsns.emplace_back(cap_insn);
        return true;
    }

    void BasicBlock::dump(RapidValue& json_object, rapidjson::MemoryPoolAllocator<>& allocator, Cfg_view_flags flags) const
    {
        RapidValue block_value;
        block_value.SetObject();
        block_value.AddMember("block_name", RapidValue(GetName().c_str(), allocator), allocator);
        std::stringstream ss;
        ss << "0x" << std::hex << reinterpret_cast<unsigned long long>(getAddress());
        block_value.AddMember("address", RapidValue(ss.str().c_str(), allocator), allocator);
        block_value.AddMember("uint_type", RapidValue(GetBlockType()), allocator);
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
        ss << "block-" << std::dec << getBlockNo();
        json_object.AddMember(RapidValue(ss.str().c_str(), allocator), block_value, allocator);
        return;
    }

    bool BasicBlock::isAfterBlock(std::shared_ptr<BasicBlock> block, std::shared_ptr<const BasicBlock> first) const
    {
        bool bret = false;
        if (shared_from_this() == block)
            return true;
        if (shared_from_this() == first) {
            return false;
        }
        if (!first) {
            first = shared_from_this();
        }
        if (GetNextBlock() != nullptr) {
            bret = GetNextBlock()->isAfterBlock(block, shared_from_this());
            if (bret) return true;
        }
        if (GetBranchBlock() != nullptr) {
            bret = GetBranchBlock()->isAfterBlock(block, shared_from_this());
            if (bret) return true;
        }
        return false;
    }

    Variable::Variable(std::shared_ptr<CBaseStruc> parent, std::string& name, std::uint8_t* address, variable_type type, std::uint8_t bytes, base_type btype)
        : m_parent(parent),
        CBaseStruc(btype),
        m_address(address),
        m_type(type)
    {
        switch (bytes) {
        case 0:
            break;
        case 1:
        {
            std::shared_ptr<CMetadataTypeObject> byte_ptr = make_metadata_j_ptr<BYTE>(CBaseDef::type_byte, INVALID_VARIABLE_VALUE, CObObject::op_n_equal);
            if (!byte_ptr) return;
            std::shared_ptr<CMetadataTypeObject> varint_ptr = std::make_shared<CMetadataTypeObject>(name, byte_ptr);
            if (!varint_ptr) return;
            m_data = std::move(varint_ptr);
        }
        break;
        case 2:
        {
            std::shared_ptr<CMetadataTypeObject> word_ptr = make_metadata_j_ptr<BYTE>(CBaseDef::type_word, INVALID_VARIABLE_VALUE, CObObject::op_n_equal);
            if (!word_ptr) return;
            std::shared_ptr<CMetadataTypeObject> varint_ptr = std::make_shared<CMetadataTypeObject>(name, word_ptr);
            if (!varint_ptr) return;
            m_data = std::move(varint_ptr);
        }
        break;
        case 4:
        {
            std::shared_ptr<CMetadataTypeObject> dword_ptr = make_metadata_j_ptr<BYTE>(CBaseDef::type_dword, INVALID_VARIABLE_VALUE, CObObject::op_n_equal);
            if (!dword_ptr) return;
            std::shared_ptr<CMetadataTypeObject> varint_ptr = std::make_shared<CMetadataTypeObject>(name, dword_ptr);
            if (!varint_ptr) return;
            m_data = std::move(varint_ptr);
        }
        break;
        case 8:
        {
            std::shared_ptr<CMetadataTypeObject> ulonglong_ptr = make_metadata_j_ptr<BYTE>(CBaseDef::type_ulonglong, INVALID_VARIABLE_VALUE, CObObject::op_n_equal);
            if (!ulonglong_ptr) return;
            std::shared_ptr<CMetadataTypeObject> varint_ptr = std::make_shared<CMetadataTypeObject>(name, ulonglong_ptr);
            if (!varint_ptr) return;
            m_data = std::move(varint_ptr);
        }
        break;
        default:
        {
            std::shared_ptr<CMetadataTypeObject> byte_ptr = make_metadata_j_ptr<BYTE>(CBaseDef::type_byte, INVALID_VARIABLE_VALUE, CObObject::op_n_equal);
            if (!byte_ptr) return;
            std::shared_ptr<CArrayObject> array_ptr = std::make_shared<CArrayObject>(name);
            if (!array_ptr) return;
            array_ptr->AddArray(byte_ptr, bytes);
            m_data = std::move(array_ptr);
        }
        }
        return;
    }

    std::string SimpleLoop::getInvariantName() const
    {
        if (m_invariant.index() == 0) {
            return GetCapstoneImplment().GetRegName(std::get<x86_reg>(m_invariant));
        }
        else if(m_invariant.index() == 1) {
            auto[address, size] = std::get<std::pair<std::uint64_t, size_t>>(m_invariant);
            std::stringstream ss;
            ss << "invar-0x" << std::hex << address << "-" << std::dec << size << std::endl;
            return ss.str();
        }
        return "unknown-invar";
    }

    std::uint64_t SimpleLoop::getInvariant(std::shared_ptr<Debugger::Modifier> ep) const
    {
        if (!ep) return 0;
        if (m_invariant.index() == 0) {
            x86_reg invariant = std::get<x86_reg>(m_invariant);
            switch (invariant) {
            case X86_REG_EAX:
            case X86_REG_RAX:
            {
                return ep->getXax();
            }
            break;
            case X86_REG_EBX:
            case X86_REG_RBX:
            {
                return ep->getXbx();
            }
            break;
            case X86_REG_ECX:
            case X86_REG_RCX:
            {
                return ep->getXcx();
            }
            break;
            case X86_REG_EDX:
            case X86_REG_RDX:
            {
                return ep->getXdx();
            }
            break;
            case X86_REG_EDI:
            case X86_REG_RDI:
            {
                return ep->getXdi();
            }
            break;
            case X86_REG_ESI:
            case X86_REG_RSI:
            {
                return ep->getXsi();
            }
            break;
            default:
                break;
            }
        }
        else if (m_invariant.index() == 1) {
            auto[address, size] = std::get<std::pair<std::uint64_t, size_t>>(m_invariant);
            if (size > sizeof std::uint64_t) {
                return 0;
            }
            std::uint64_t ret_value = 0;
            tls_check_struct *tls = check_get_tls();
            if (tls) {
                tls->active = 1;
                if (setjmp(tls->jb) == 0) {
                    memcpy(&ret_value, reinterpret_cast<const void*>(address), size);
                    tls->active = 0;
                }
                else {
                    //execption occur
                    tls->active = 0;
                }
            }
        }
        return 0;
    }

    bool SimpleLoop::setInvariant(std::shared_ptr<Debugger::Modifier> ep, std::uint64_t value) const
    {
        if (!ep) return false;
        if (m_invariant.index() == 0) {
            x86_reg invariant = std::get<x86_reg>(m_invariant);
            switch (invariant) {
            case X86_REG_AL:
            case X86_REG_AX:
            case X86_REG_EAX:
            case X86_REG_RAX:
            {
                ep->setXax(value);
                return true;
            }
            break;
            case X86_REG_BL:
            case X86_REG_BX:
            case X86_REG_EBX:
            case X86_REG_RBX:
            {
                ep->setXbx(value);
                return true;
            }
            break;
            case X86_REG_CL:
            case X86_REG_CX:
            case X86_REG_ECX:
            case X86_REG_RCX:
            {
                ep->setXcx(value);
                return true;
            }
            break;
            case X86_REG_DL:
            case X86_REG_DX:
            case X86_REG_EDX:
            case X86_REG_RDX:
            {
                ep->setXdx(value);
                return true;
            }
            break;
            case X86_REG_DIL:
            case X86_REG_DI:
            case X86_REG_EDI:
            case X86_REG_RDI:
            {
                ep->setXdi(value);
                return true;
            }
            break;
            case X86_REG_SIL:
            case X86_REG_SI:
            case X86_REG_ESI:
            case X86_REG_RSI:
            {
                ep->setXsi(value);
                return true;
            }
            break;
            default:
                break;
            }
        }
        else if (m_invariant.index() == 1) {
            auto[address, size] = std::get<std::pair<std::uint64_t, size_t>>(m_invariant);
            if (size > sizeof std::uint64_t) {
                return false;
            }
            std::uint64_t ret_value = 0;
            tls_check_struct *tls = check_get_tls();
            if (tls) {
                tls->active = 1;
                if (setjmp(tls->jb) == 0) {
                    memcpy(reinterpret_cast<void*>(address), &ret_value, size);
                    tls->active = 0;
                    return true;
                }
                else {
                    //execption occur
                    tls->active = 0;
                }
            }
        }
        return false;
    }

    void ControlFlowGraph::dump(RapidValue& json_object, rapidjson::MemoryPoolAllocator<>& allocator, Cfg_view_flags flags) const
    {
        RapidValue database_value;
        database_value.SetObject();
        std::uint64_t flowcount = 0;
        for (auto& database : m_cfg_database) {
            RapidValue flow_value;
            flow_value.SetArray();
            for (auto& flowname : database.second) {
                flowcount++;
                flow_value.PushBack(RapidValue(flowname.c_str(), allocator), allocator);
            }
            std::string count_name = database.first + "_count";
            database_value.AddMember(RapidValue(count_name.c_str(), allocator), flowcount, allocator);
            database_value.AddMember(RapidValue(database.first.c_str(), allocator), flow_value, allocator);
        }
        json_object.AddMember("cfgflowgraph", database_value, allocator);
        return;
    }
} // namespace cchips
