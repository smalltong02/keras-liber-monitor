#pragma once
#include "capstone\include\capstone.h"
#include "NativeObject.h"
#include "PackageWrapper.h"
#include <string>

std::string hexstring(BYTE byte);

namespace cchips {

    class BasicBlock;

    class CBaseStruc {
    public:
        using base_type = enum {
            base_invalid = 0,
            base_module = 1,
            base_function,
            base_globalifunc,
            base_basicblock,
            base_globalvariable,
            base_localvariable,
            base_instruction,
        };
        CBaseStruc(base_type type) : m_type(type) {}
        ~CBaseStruc() = default;
        base_type GetBaseType() const { return m_type; }
        virtual std::uint64_t GetBaseAddress() const = 0;
    private:
        base_type m_type = base_invalid;
    };

    class CapInsn : public CBaseStruc {
    public:
        CapInsn(csh cs_handle) : CBaseStruc(base_instruction) {
            if (cs_handle == 0) return;
            m_insn = cs_malloc(cs_handle);
            if (m_insn)
            {
                m_count = 1;
            }
        }
        CapInsn(cs_insn* insn, size_t count = 1) : CBaseStruc(base_instruction), m_insn(std::move(insn)), m_count(count) {
            if (m_insn) {
                m_mnemonic_str = std::string(m_insn->mnemonic) + std::string(" ") + std::string(m_insn->op_str);
                m_mnemonic_bytes.resize(m_insn->size);
                memcpy(&m_mnemonic_bytes[0], m_insn->bytes, m_insn->size);
            }
        }
        ~CapInsn() { 
            if(m_insn) cs_free(m_insn, m_count);
        }
        CapInsn(const CapInsn&) = delete;
        CapInsn& operator=(const CapInsn&) = delete;
        
        bool valid() const { 
            if (m_insn && m_count > 0) 
                return true; 
            if (m_address && m_size)
                return true;
            return false; 
        }
        size_t count() const { return m_count; }
        size_t size() const { 
            if (valid()) {
                if (m_insn)
                    return (size_t)m_insn->size;
                else
                    return m_size;
            }
            return 0; 
        }
        std::uint64_t address() const { 
            if (valid()) {
                if (m_insn)
                    return m_insn->address;
                else
                    return reinterpret_cast<std::uint64_t>(m_address);
            }
            return 0;
        }
        std::uint64_t GetBaseAddress() const { return address(); }
        const std::vector<BYTE>& getBytes() const { return m_mnemonic_bytes; }
        const std::string& dump() const { return m_mnemonic_str; }
        std::string dumpbytes() const { 
            if (!m_mnemonic_bytes.size()) return {};
            std::stringstream ss;
            for (int count = 0; count < m_mnemonic_bytes.size() - 1; count++) {
                ss << std::hex << hexstring(m_mnemonic_bytes[count]) << " ";
            }
            ss << std::hex << hexstring(m_mnemonic_bytes[m_mnemonic_bytes.size() - 1]);
            return ss.str();
        }
        cs_insn* self() const { return m_insn; }
        void free_insn() {
            if (m_insn) {
                m_address = reinterpret_cast<std::uint8_t*>(m_insn->address);
                m_size = m_insn->size;
                m_id = m_insn->id;
                cs_free(m_insn, m_count);
            }
            m_count = 0;
            m_insn = nullptr;
        }
        void free_detail() {
            if (m_insn && m_insn->detail) {
                NativeObject::native_free(m_insn->detail);
                m_insn->detail = nullptr;
            }
        }
        unsigned int GetInsnId() const { 
            if (valid()) {
                if (m_insn)
                    return m_insn->id;
                else
                    return m_id;
            }
            return 0;
        }
        void SetParent(std::shared_ptr<BasicBlock> parent) { m_parent = parent; }
        void setPointerObject(std::shared_ptr<CBaseStruc> obj) { m_pobject = obj; }
        void updateMnemonic(const std::string& op_str) { m_mnemonic_str = std::string(m_insn->mnemonic) + std::string(" ") + op_str; }
        void dump(RapidValue& json_object, rapidjson::MemoryPoolAllocator<>& allocator, Cfg_view_flags flags = Cfg_view_flags::cfg_simple) const {
            std::stringstream ss;
            ss << "0x" << std::hex << reinterpret_cast<unsigned long long>(m_address);
            json_object.AddMember(RapidValue(ss.str().c_str(), allocator), RapidValue(m_mnemonic_str.c_str(), allocator), allocator);
            return;
        }
        std::shared_ptr<BasicBlock> GetParent() { return m_parent.lock(); }
    private:
        std::uint8_t* m_address;
        size_t m_size;
        unsigned int m_id;
        cs_insn* m_insn = nullptr;
        size_t m_count = 0;
        std::string m_mnemonic_str;
        std::vector<BYTE> m_mnemonic_bytes;
        std::weak_ptr<BasicBlock> m_parent;
        std::weak_ptr<CBaseStruc> m_pobject;
    };

    class CapstoneImpl
    {
    public:
        static CapstoneImpl& GetInstance()
        {
            static CapstoneImpl m_instance;
            return m_instance;
        }

        bool IsValid() const {
            if (m_cap_error != CS_ERR_OK ||
                m_capstone_handle == 0) return false;
            return true;
        }
        csh GetCapHandle() const {
            return m_capstone_handle;
        }
        void Reset(const cs_opt_mem& cs_mem) {
            if (m_capstone_handle != 0) {
                cs_close(&m_capstone_handle);
            }
            cs_option(0, CS_OPT_MEM, (size_t)(uintptr_t)&cs_mem);

#ifdef _X86_
            m_cap_error = cs_open(CS_ARCH_X86, CS_MODE_32, &m_capstone_handle);
#endif
#ifdef _AMD64_
            m_cap_error = cs_open(CS_ARCH_X86, CS_MODE_64, &m_capstone_handle);
#endif
            if (IsValid()) {
                cs_option(m_capstone_handle, CS_OPT_DETAIL, CS_OPT_ON);
            }
            return;
        }
        std::unique_ptr<CapInsn> DisasmEx(const uint8_t *code, size_t code_size, uint64_t address, size_t code_count) const {
            if (IsValid()) {
                cs_insn *insn = nullptr;
                size_t count =
                    cs_disasm_ex(GetCapHandle(), code, code_size, address, code_count, &insn);
                if (count == 0) 
                    return nullptr;
                if (!insn) 
                    return nullptr;
                std::unique_ptr<CapInsn> cap_insn = std::make_unique<CapInsn>(insn, count);
                return std::move(cap_insn);
            }
            return nullptr;
        }
        std::unique_ptr<CapInsn> DisasmOneInsn(const uint8_t *code, size_t code_size) const {
            return DisasmEx(code, code_size, reinterpret_cast<std::uint64_t>(code), 1);
        }
        bool CsDisasmIter(const uint8_t **code, size_t *code_size, uint64_t *address, CapInsn& insn) const {
            if (IsValid()) {
                if (cs_disasm_iter(m_capstone_handle, code, code_size, address, insn.self())) {
                    return true;
                }
            }
            return false;
        }
        std::string GetRegName(unsigned int reg_id) const {
            if (IsValid()) {
                const char* name_ptr = cs_reg_name(m_capstone_handle, reg_id);
                if (name_ptr) return std::string(name_ptr);
            }
            return {};
        }
        std::string GetInsnName(unsigned int insn_id) const {
            if (IsValid()) {
                const char* name_ptr = cs_insn_name(m_capstone_handle, insn_id);
                if (name_ptr) return std::string(name_ptr);
            }
            return {};
        }
        std::string GetGroupName(unsigned int group_id) const {
            if (IsValid()) {
                const char* name_ptr = cs_group_name(m_capstone_handle, group_id);
                if (name_ptr) return std::string(name_ptr);
            }
            return {};
        }
        bool InJmpGroup(CapInsn& insn) const {
            return InsnInGroup(insn, X86_GRP_JUMP);
        }
        bool InBranchGroup(CapInsn& insn) const {
            return _branchInsnIds.count(insn.GetInsnId());
        }
        bool InCondBranchGroup(CapInsn& insn) const {
            return _condBranchInsnIds.count(insn.GetInsnId());
        }
        bool InCallGroup(CapInsn& insn) const {
            return InsnInGroup(insn, X86_GRP_CALL);
        }
        bool InLoopGroup(CapInsn& insn) const {
            return _loopInsnIds.count(insn.GetInsnId());
        }
        bool InRetGroup(CapInsn& insn) const {
            return InsnInGroup(insn, X86_GRP_RET);
        }
        bool InIntGroup(CapInsn& insn) const {
            return InsnInGroup(insn, X86_GRP_INT);
        }
        bool InsnInIretGroup(CapInsn& insn) const {
            return InsnInGroup(insn, X86_GRP_IRET);
        }
        bool InPriviligeGroup(CapInsn& insn) const {
            return InsnInGroup(insn, X86_GRP_PRIVILEGE);
        }
        bool InRelativeBranchGroup(CapInsn& insn) const {
            return InsnInGroup(insn, X86_GRP_BRANCH_RELATIVE);
        }
        bool InVmGroup(CapInsn& insn) const {
            return InsnInGroup(insn, X86_GRP_VM);
        }
        bool InsnInGroup(CapInsn& insn, unsigned int group_id) const {
            if (IsValid()) {
                return cs_insn_group(m_capstone_handle, insn.self(), group_id);
            }
            return false;
        }
        bool RegInRead(CapInsn& insn, unsigned int reg_id) const {
            if (IsValid()) {
                return cs_reg_read(m_capstone_handle, insn.self(), reg_id);
            }
            return false;
        }
        bool RegInWrite(CapInsn& insn, unsigned int reg_id) const {
            if (IsValid()) {
                return cs_reg_write(m_capstone_handle, insn.self(), reg_id);
            }
            return false;
        }
        int OpInCount(CapInsn& insn, unsigned int op_type) const {
            if (IsValid()) {
                return cs_op_count(m_capstone_handle, insn.self(), op_type);
            }
            return 0;
        }
        int OpInIndex(CapInsn& insn, unsigned int op_type, unsigned int position) const {
            if (IsValid()) {
                return cs_op_index(m_capstone_handle, insn.self(), op_type, position);
            }
            return 0;
        }
        cs_err RegsInAccess(CapInsn& insn, cs_regs regs_read, uint8_t *regs_read_count, cs_regs regs_write, uint8_t *regs_write_count) const {
            if (IsValid()) {
                return cs_regs_access(m_capstone_handle, insn.self(), regs_read, regs_read_count, regs_write, regs_write_count);
            }
            return CS_ERR_MODE;
        }

        bool GetJmpAddress(CapInsn& insn, std::uint8_t*& next_addr, std::uint8_t*& jmp_addr, x86_op_type& op_type) const {
            if (InJmpGroup(insn)) {
                if (OpInCount(insn, X86_OP_IMM)) {
                    int index = OpInIndex(insn, X86_OP_IMM, 1);
                    if (insn.self()->detail) {
                        cs_x86* x86 = &insn.self()->detail->x86;
                        if (insn.GetInsnId() == X86_INS_JMP || insn.GetInsnId() == X86_INS_LJMP)
                            next_addr = nullptr;
                        else
                            next_addr = reinterpret_cast<std::uint8_t*>(insn.address()) + insn.size();
                        jmp_addr = reinterpret_cast<std::uint8_t*>(x86->operands[index].imm);
                        op_type = X86_OP_IMM;
                        return true;
                    }
                }
                else if (OpInCount(insn, X86_OP_REG)) {
                    op_type = X86_OP_REG;
                    if (insn.GetInsnId() == X86_INS_JMP || insn.GetInsnId() == X86_INS_LJMP)
                        next_addr = nullptr;
                    else
                        next_addr = reinterpret_cast<std::uint8_t*>(insn.address()) + insn.size();
                    jmp_addr = nullptr;
                    return true;
                }
                else if (OpInCount(insn, X86_OP_MEM)) {
                    int index = OpInIndex(insn, X86_OP_MEM, 1);
                    if (insn.self()->detail) {
                        cs_x86* x86 = &insn.self()->detail->x86;
                        if (insn.GetInsnId() == X86_INS_JMP || insn.GetInsnId() == X86_INS_LJMP)
                            next_addr = nullptr;
                        else
                            next_addr = reinterpret_cast<std::uint8_t*>(insn.address()) + insn.size();
                        if (x86->operands[index].mem.segment == X86_REG_INVALID &&
                            x86->operands[index].mem.base == X86_REG_INVALID) {
                            jmp_addr = reinterpret_cast<std::uint8_t*>(x86->operands[index].mem.disp);
                        }
                        else {
                            jmp_addr = nullptr;
                        }
                        op_type = X86_OP_MEM;
                        return true;
                    }
                }
            }
            return false;
        }
        bool GetLoopAddress(CapInsn& insn, std::uint8_t*& next_addr, std::uint8_t*& loop_addr, x86_op_type& op_type) const {
            if (InLoopGroup(insn)) {
                if (OpInCount(insn, X86_OP_IMM)) {
                    int index = OpInIndex(insn, X86_OP_IMM, 1);
                    if (insn.self()->detail) {
                        cs_x86* x86 = &insn.self()->detail->x86;
                        next_addr = reinterpret_cast<std::uint8_t*>(insn.address()) + insn.size();
                        loop_addr = reinterpret_cast<std::uint8_t*>(x86->operands[index].imm);
                        op_type = X86_OP_IMM;
                        return true;
                    }
                }
                else if (OpInCount(insn, X86_OP_REG)) {
                    op_type = X86_OP_REG;
                    loop_addr = nullptr;
                    return true;
                }
                else if (OpInCount(insn, X86_OP_MEM)) {
                    int index = OpInIndex(insn, X86_OP_MEM, 1);
                    if (insn.self()->detail) {
                        cs_x86* x86 = &insn.self()->detail->x86;
                        if (insn.GetInsnId() == X86_INS_LOOP)
                            next_addr = nullptr;
                        else
                            next_addr = reinterpret_cast<std::uint8_t*>(insn.address()) + insn.size();
                        if (x86->operands[index].mem.segment == X86_REG_INVALID &&
                            x86->operands[index].mem.base == X86_REG_INVALID) {
                            loop_addr = reinterpret_cast<std::uint8_t*>(x86->operands[index].mem.disp);
                        }
                        else {
                            loop_addr = nullptr;
                        }
                        op_type = X86_OP_MEM;
                        return true;
                    }
                }
            }
            return false;
        }
        bool GetCallAddress(CapInsn& insn, std::uint8_t*& jmp_addr, x86_op_type& op_type) const {
            if (InCallGroup(insn)) {
                if (OpInCount(insn, X86_OP_IMM)) {
                    int index = OpInIndex(insn, X86_OP_IMM, 1);
                    if (insn.self()->detail) {
                        cs_x86* x86 = &insn.self()->detail->x86;
                        jmp_addr = reinterpret_cast<std::uint8_t*>(x86->operands[index].imm);
                        op_type = X86_OP_IMM;
                        return true;
                    }
                }
                else if (OpInCount(insn, X86_OP_MEM)) {
                    int index = OpInIndex(insn, X86_OP_MEM, 1);
                    if (insn.self()->detail) {
                        cs_x86* x86 = &insn.self()->detail->x86;
                        if (x86->operands[index].mem.segment == X86_REG_INVALID &&
                            x86->operands[index].mem.base == X86_REG_INVALID) {
                            jmp_addr = reinterpret_cast<std::uint8_t*>(x86->operands[index].mem.disp);
                        }
                        else {
                            jmp_addr = nullptr;
                            if (x86->operands[index].mem.segment == X86_REG_INVALID) {
                                if (x86->operands[index].mem.base == X86_REG_EIP || x86->operands[index].mem.base == X86_REG_RIP) {
                                    jmp_addr = reinterpret_cast<std::uint8_t*>(insn.address() + insn.size() + x86->operands[index].mem.disp);
                                }
                            }
                        }
                        op_type = X86_OP_MEM;
                        return true;
                    }
                }
                else if (OpInCount(insn, X86_OP_REG)) {
                    op_type = X86_OP_REG;
                    jmp_addr = nullptr;
                    return true;
                }
            }
            return false;
        }
    private:
        CapstoneImpl() { 
#ifdef _X86_
            m_cap_error = cs_open(CS_ARCH_X86, CS_MODE_32, &m_capstone_handle);
#endif
#ifdef _AMD64_
            m_cap_error = cs_open(CS_ARCH_X86, CS_MODE_64, &m_capstone_handle);
#endif
            if (IsValid()) {
                cs_option(m_capstone_handle, CS_OPT_DETAIL, CS_OPT_ON);
            }
        }
        ~CapstoneImpl() { 
            if (m_capstone_handle != 0) {
                cs_close(&m_capstone_handle);
            }
        }
        CapstoneImpl(const CapstoneImpl&) = delete;
        CapstoneImpl& operator=(const CapstoneImpl&) = delete;
        
        csh m_capstone_handle = 0;
        cs_err m_cap_error = CS_ERR_HANDLE;
        const std::set<unsigned int> _branchInsnIds = { X86_INS_JMP, X86_INS_LJMP, };
        const std::set<unsigned int> _loopInsnIds = { X86_INS_LOOP, X86_INS_LOOPE, X86_INS_LOOPNE, };
        const std::set<unsigned int> _condBranchInsnIds = {
            X86_INS_JCXZ,
            X86_INS_JECXZ,
            X86_INS_JRCXZ,
            X86_INS_JAE,
            X86_INS_JA,
            X86_INS_JBE,
            X86_INS_JB,
            X86_INS_JE,
            X86_INS_JGE,
            X86_INS_JG,
            X86_INS_JLE,
            X86_INS_JL,
            X86_INS_JNE,
            X86_INS_JNO,
            X86_INS_JNP,
            X86_INS_JNS,
            X86_INS_JO,
            X86_INS_JP,
            X86_INS_JS,
        };
    };

#define GetCapstoneImplment() CapstoneImpl::GetInstance()
} // namespace cchips
