#include "PassStructure.h"
#include "CapstoneImpl.h"

namespace cchips {

    bool CapstoneImpl::GetJmpAddress(std::shared_ptr<Module> cur_module, CapInsn& insn, std::uint8_t*& next_addr, std::uint8_t*& jmp_addr, x86_op_type& op_type) const
    {
        if (!cur_module) return false;

        if (InJmpGroup(insn)) {
            if (OpInCount(insn, X86_OP_IMM)) {
                int index = OpInIndex(insn, X86_OP_IMM, 1);
                if (insn.self()->detail) {
                    cs_x86* x86 = &insn.self()->detail->x86;
                    if (InBranchGroup(insn))
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
                        x86->operands[index].mem.base == X86_REG_INVALID &&
                        x86->operands[index].mem.index == X86_REG_INVALID) {
                        jmp_addr = reinterpret_cast<std::uint8_t*>(x86->operands[index].mem.disp);
                    }
                    else {
                        jmp_addr = nullptr;
                        if (x86->operands[index].mem.segment == X86_REG_INVALID) {
                            if (x86->operands[index].mem.base == X86_REG_EIP || x86->operands[index].mem.base == X86_REG_RIP) {
                                if (cur_module->GetContext()->GetBits() == 32) {
                                    if (cur_module->GetContext()->isLoadingFile()) {
                                        jmp_addr = reinterpret_cast<std::uint8_t*>(cur_module->GetContext()->getValidOffsetFromMemRva(x86->operands[index].mem.disp));
                                    }
                                    else {
                                    }
                                }
                                else if (cur_module->GetContext()->GetBits() == 64) {
                                    if (cur_module->GetContext()->isLoadingFile()) {
                                        auto rva = cur_module->GetContext()->getValidRvaAddressFromFileAddress(insn.address() + insn.size());
                                        jmp_addr = reinterpret_cast<std::uint8_t*>(cur_module->GetContext()->getValidOffsetFromMemRva(rva + x86->operands[index].mem.disp));
                                    }
                                    else {
                                        jmp_addr = reinterpret_cast<std::uint8_t*>(insn.address() + insn.size() + x86->operands[index].mem.disp);
                                    }
                                }
                            }
                        }
                    }
                    op_type = X86_OP_MEM;
                    return true;
                }
            }
        }
        return false;
    }

    bool CapstoneImpl::GetLoopAddress(CapInsn& insn, std::uint8_t*& next_addr, std::uint8_t*& loop_addr, x86_op_type& op_type) const
    {
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
                        x86->operands[index].mem.base == X86_REG_INVALID &&
                        x86->operands[index].mem.index == X86_REG_INVALID) {
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
    bool CapstoneImpl::GetCallAddress(CapInsn& insn, std::uint8_t*& jmp_addr, x86_op_type& op_type) const
    {
        std::shared_ptr<BasicBlock> block = insn.GetParent();
        if (!block) return false;
        std::shared_ptr<Function> function = block->GetParent();
        if (!function) return false;
        std::shared_ptr<Module> cur_module = function->GetParent();
        if (!cur_module) return false;

        if (InCallGroup(insn)) {
            if (OpInCount(insn, X86_OP_IMM)) {
                int index = OpInIndex(insn, X86_OP_IMM, 1);
                if (insn.self()->detail) {
                    cs_x86* x86 = &insn.self()->detail->x86;
                    if (cur_module->GetContext()->isLoadingFile()) {
                        auto rva = cur_module->GetContext()->getValidRvaAddressFromFileAddress(insn.address());
                        jmp_addr = reinterpret_cast<std::uint8_t*>(cur_module->GetContext()->getValidOffsetFromMemRva(rva + (x86->operands[index].imm - insn.address())));
                    }
                    else {
                        jmp_addr = reinterpret_cast<std::uint8_t*>(x86->operands[index].imm);
                    }
                    op_type = X86_OP_IMM;
                    return true;
                }
            }
            else if (OpInCount(insn, X86_OP_MEM)) {
                int index = OpInIndex(insn, X86_OP_MEM, 1);
                if (insn.self()->detail) {
                    cs_x86* x86 = &insn.self()->detail->x86;
                    if (x86->operands[index].mem.segment == X86_REG_INVALID &&
                        x86->operands[index].mem.base == X86_REG_INVALID &&
                        x86->operands[index].mem.index == X86_REG_INVALID) {
                        jmp_addr = reinterpret_cast<std::uint8_t*>(x86->operands[index].mem.disp);
                    }
                    else {
                        jmp_addr = nullptr;
                        if (x86->operands[index].mem.segment == X86_REG_INVALID) {
                            if (x86->operands[index].mem.base == X86_REG_EIP || x86->operands[index].mem.base == X86_REG_RIP) {
                                if (cur_module->GetContext()->GetBits() == 32) {
                                    if (cur_module->GetContext()->isLoadingFile()) {
                                        jmp_addr = reinterpret_cast<std::uint8_t*>(cur_module->GetContext()->getValidOffsetFromMemRva(x86->operands[index].mem.disp));
                                    }
                                    else {
                                    }
                                }
                                else if (cur_module->GetContext()->GetBits() == 64) {
                                    if (cur_module->GetContext()->isLoadingFile()) {
                                        auto rva = cur_module->GetContext()->getValidRvaAddressFromFileAddress(insn.address() + insn.size());
                                        jmp_addr = reinterpret_cast<std::uint8_t*>(cur_module->GetContext()->getValidOffsetFromMemRva(rva + x86->operands[index].mem.disp));
                                    }
                                    else {
                                        jmp_addr = reinterpret_cast<std::uint8_t*>(insn.address() + insn.size() + x86->operands[index].mem.disp);
                                    }
                                }
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
} // namespace cchips
