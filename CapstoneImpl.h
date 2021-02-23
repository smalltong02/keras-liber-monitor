#pragma once
#include "capstone\include\capstone.h"

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
            base_instruction,
        };
        CBaseStruc(base_type type) : m_type(type) {}
        ~CBaseStruc() = default;
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
            if (valid()) {
                m_mnemonic_str = std::string(m_insn->mnemonic) + std::string(" ") + std::string(m_insn->op_str);
            }
        }
        ~CapInsn() { 
            if(valid()) cs_free(m_insn, m_count);
        }
        CapInsn(const CapInsn&) = delete;
        CapInsn& operator=(const CapInsn&) = delete;

        bool valid() const { if (m_insn && m_count > 0) return true; return false; }
        size_t count() const { return m_count; }
        size_t size() const { if (valid()) return (size_t)m_insn->size; return 0; }
        const std::string& dump() const { return m_mnemonic_str; }
        cs_insn* self() const { return m_insn; }
        unsigned int GetInsnId() const { if (valid()) return m_insn->id; else return 0; }
        void SetParent(std::shared_ptr<BasicBlock> parent) { m_parent = parent; }
    private:
        cs_insn* m_insn = nullptr;
        size_t m_count = 0;
        std::string m_mnemonic_str;
        std::weak_ptr<BasicBlock> m_parent;
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
        std::unique_ptr<CapInsn> DisasmEx(const uint8_t *code, size_t code_size, uint64_t address, size_t code_count) {
            if (IsValid()) {
                cs_insn *insn = nullptr;
                size_t count =
                    cs_disasm_ex(GetCapHandle(), code, code_size, address, code_count, &insn);
                if (count == 0) return 0;
                if (!insn) return 0;
                std::unique_ptr<CapInsn> cap_insn = std::make_unique<CapInsn>(insn, count);
                return std::move(cap_insn);
            }
            return nullptr;
        }
        bool CsDisasmIter(const uint8_t **code, size_t *code_size, uint64_t *address, std::unique_ptr<CapInsn>& insn) {
            if (IsValid()) {
                if (cs_disasm_iter(m_capstone_handle, code, code_size, address, insn->self())) {
                    return true;
                }
            }
            return false;
        }
        std::string GetRegName(unsigned int reg_id) {
            if (IsValid()) {
                const char* name_ptr = cs_reg_name(m_capstone_handle, reg_id);
                if (name_ptr) return std::string(name_ptr);
            }
            return {};
        }
        std::string GetInsnName(unsigned int insn_id) {
            if (IsValid()) {
                const char* name_ptr = cs_insn_name(m_capstone_handle, insn_id);
                if (name_ptr) return std::string(name_ptr);
            }
            return {};
        }
        std::string GetGroupName(unsigned int group_id) {
            if (IsValid()) {
                const char* name_ptr = cs_group_name(m_capstone_handle, group_id);
                if (name_ptr) return std::string(name_ptr);
            }
            return {};
        }
        bool InsnInGroup(std::unique_ptr<CapInsn>& insn, unsigned int group_id) {
            if (IsValid()) {
                return cs_insn_group(m_capstone_handle, insn->self(), group_id);
            }
            return false;
        }
        bool RegInRead(std::unique_ptr<CapInsn>& insn, unsigned int reg_id) {
            if (IsValid()) {
                return cs_reg_read(m_capstone_handle, insn->self(), reg_id);
            }
            return false;
        }
        bool RegInWrite(std::unique_ptr<CapInsn>& insn, unsigned int reg_id) {
            if (IsValid()) {
                return cs_reg_write(m_capstone_handle, insn->self(), reg_id);
            }
            return false;
        }
        int OpInCount(std::unique_ptr<CapInsn>& insn, unsigned int op_type) {
            if (IsValid()) {
                return cs_op_count(m_capstone_handle, insn->self(), op_type);
            }
            return 0;
        }
        int OpInIndex(std::unique_ptr<CapInsn>& insn, unsigned int op_type, unsigned int position) {
            if (IsValid()) {
                return cs_op_index(m_capstone_handle, insn->self(), op_type, position);
            }
            return 0;
        }
        cs_err RegsInAccess(std::unique_ptr<CapInsn>& insn, cs_regs regs_read, uint8_t *regs_read_count, cs_regs regs_write, uint8_t *regs_write_count) {
            if (IsValid()) {
                return cs_regs_access(m_capstone_handle, insn->self(), regs_read, regs_read_count, regs_write, regs_write_count);
            }
            return CS_ERR_MODE;
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
    };

#define GetCapstoneImplment() CapstoneImpl::GetInstance()
} // namespace cchips
