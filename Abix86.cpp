#include "Abix86.h"

namespace cchips {

    AbiX86::AbiX86()
    {
        _regs.reserve(X86_REG_ENDING);
        _id2regs.resize(X86_REG_ENDING, nullptr);
        _regStackPointerId = X86_REG_ESP;

        // system calls
        _regSyscallId = X86_REG_EAX;
        _regSyscallReturn = X86_REG_EAX;
        _syscallRegs = {
            X86_REG_EBX,
            X86_REG_ECX,
            X86_REG_EDX,
            X86_REG_ESI,
            X86_REG_EDI,
            X86_REG_EBP };
    }

    bool AbiX86::isGeneralPurposeRegister(cs_x86_op* operand) const
    {
        if (!operand) return false;
        if (operand->type != X86_OP_REG) return false;
        return operand->reg == X86_REG_EAX
            || operand->reg == X86_REG_EBX
            || operand->reg == X86_REG_ECX
            || operand->reg == X86_REG_EDX
            || operand->reg == X86_REG_ESP
            || operand->reg == X86_REG_EBP
            || operand->reg == X86_REG_ESI
            || operand->reg == X86_REG_EDI;
    }

    bool AbiX86::isNopInstruction(const CapInsn& insn) const
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
