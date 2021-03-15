#include "Abix64.h"

namespace cchips {

    AbiX64::AbiX64()
    {
        _regs.reserve(X86_REG_ENDING);
        _id2regs.resize(X86_REG_ENDING, nullptr);
        _regStackPointerId = X86_REG_RSP;

        // system calls
        _regSyscallId = X86_REG_EAX;
        _regSyscallReturn = X86_REG_EAX;
        _syscallRegs = {
            X86_REG_RDI,
            X86_REG_RSI,
            X86_REG_RDX,
            X86_REG_R10,
            X86_REG_R8,
            X86_REG_R9 };
    }

    bool AbiX64::isGeneralPurposeRegister(cs_x86_op* operand) const
    {
        if (!operand) return false;
        if (operand->type != X86_OP_REG) return false;
        return operand->reg == X86_REG_RAX
            || operand->reg == X86_REG_RBX
            || operand->reg == X86_REG_RCX
            || operand->reg == X86_REG_RDX
            || operand->reg == X86_REG_RSP
            || operand->reg == X86_REG_RBP
            || operand->reg == X86_REG_RSI
            || operand->reg == X86_REG_RDI
            || operand->reg == X86_REG_R8
            || operand->reg == X86_REG_R9
            || operand->reg == X86_REG_R10
            || operand->reg == X86_REG_R11
            || operand->reg == X86_REG_R12
            || operand->reg == X86_REG_R13
            || operand->reg == X86_REG_R14
            || operand->reg == X86_REG_R15;
    }

    bool AbiX64::isNopInstruction(const CapInsn& insn) const
    {
        if (!insn.valid()) return false;
        if (!insn.self()->detail) return false;
        cs_x86& insn86 = insn.self()->detail->x86;

        // True NOP variants.
        if (insn.self()->id == X86_INS_NOP
            || insn.self()->id == X86_INS_FNOP
            || insn.self()->id == X86_INS_FDISI8087_NOP
            || insn.self()->id == X86_INS_FENI8087_NOP
            || insn.self()->id == X86_INS_INT3)
        {
            return true;
        }
        // e.g. lea esi, [esi]
    //
        else if (insn.self()->id == X86_INS_LEA
            && insn86.disp == 0
            && insn86.op_count == 2
            && insn86.operands[0].type == X86_OP_REG
            && insn86.operands[1].type == X86_OP_MEM
            && insn86.operands[1].mem.segment == X86_REG_INVALID
            && insn86.operands[1].mem.index == X86_REG_INVALID
            && insn86.operands[1].mem.scale == 1
            && insn86.operands[1].mem.disp == 0
            && insn86.operands[1].mem.base == insn86.operands[0].reg)
        {
            return true;
        }
        // e.g. mov esi. esi
        //
        else if (insn.self()->id == X86_INS_MOV
            && insn86.disp == 0
            && insn86.op_count == 2
            && insn86.operands[0].type == X86_OP_REG
            && insn86.operands[1].type == X86_OP_REG
            && insn86.operands[0].reg == insn86.operands[1].reg)
        {
            return true;
        }

        return false;
    }
} // namespace cchips
