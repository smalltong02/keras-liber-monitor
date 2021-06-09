#include "Abix86.h"

namespace cchips {

    std::map<x86_reg, std::pair<x86_reg, uint32_t>> AbiX86::_reg2parentMap = {
        {X86_REG_AL, {X86_REG_AX, 0}},
        {X86_REG_AH, {X86_REG_AX, 8}},
        {X86_REG_AX, {X86_REG_EAX, 0}},
        {X86_REG_BL, {X86_REG_BX, 0}},
        {X86_REG_BH, {X86_REG_BX, 8}},
        {X86_REG_BX, {X86_REG_EBX, 0}},
        {X86_REG_CL, {X86_REG_CX, 0}},
        {X86_REG_CH, {X86_REG_CX, 8}},
        {X86_REG_CX, {X86_REG_ECX, 0}},
        {X86_REG_DL, {X86_REG_DX, 0}},
        {X86_REG_DH, {X86_REG_DX, 8}},
        {X86_REG_DX, {X86_REG_EDX, 0}},
        {X86_REG_SPL, {X86_REG_SP, 0}},
        {X86_REG_SP, {X86_REG_ESP, 0}},
        {X86_REG_BPL, {X86_REG_BP, 0}},
        {X86_REG_BP, {X86_REG_EBP, 0}},
        {X86_REG_SIL, {X86_REG_SI, 0}},
        {X86_REG_SI, {X86_REG_ESI, 0}},
        {X86_REG_DIL, {X86_REG_DI, 0}},
        {X86_REG_DI, {X86_REG_EDI, 0}},
        {X86_REG_IP, {X86_REG_EIP, 0}},
    };

    std::vector<std::pair<x86_reg, uint32_t>> AbiX86::_reg2SizeMap = {
        // 8 bits
        {X86_REG_AH, 8},
        {X86_REG_AL, 8},
        {X86_REG_CH, 8},
        {X86_REG_CL, 8},
        {X86_REG_DH, 8},
        {X86_REG_DL, 8},
        {X86_REG_BH, 8},
        {X86_REG_BL, 8},
        {X86_REG_SPL, 8},
        {X86_REG_BPL, 8},
        {X86_REG_DIL, 8},
        {X86_REG_SIL, 8},
        // 16 bits
        {X86_REG_AX, 16},
        {X86_REG_CX, 16},
        {X86_REG_DX, 16},
        {X86_REG_BP, 16},
        {X86_REG_BX, 16},
        {X86_REG_DI, 16},
        {X86_REG_SP, 16},
        {X86_REG_SI, 16},
        {X86_REG_SS, 16},
        {X86_REG_CS, 16},
        {X86_REG_DS, 16},
        {X86_REG_ES, 16},
        {X86_REG_FS, 16},
        {X86_REG_GS, 16},
        {X86_REG_IP, 16},
        // 32 bits
        {X86_REG_EAX, 32},
        {X86_REG_EBP, 32},
        {X86_REG_EBX, 32},
        {X86_REG_ECX, 32},
        {X86_REG_EDI, 32},
        {X86_REG_EDX, 32},
        {X86_REG_ESI, 32},
        {X86_REG_ESP, 32},
        {X86_REG_EIP, 32},
        {X86_REG_EIZ, 32},
        {X86_REG_EFLAGS, 32},
        {X86_REG_DR0, 32},
        {X86_REG_DR1, 32},
        {X86_REG_DR2, 32},
        {X86_REG_DR3, 32},
        {X86_REG_DR4, 32},
        {X86_REG_DR5, 32},
        {X86_REG_DR6, 32},
        {X86_REG_DR7, 32},
        {X86_REG_DR8, 32},
        {X86_REG_DR9, 32},
        {X86_REG_DR10, 32},
        {X86_REG_DR11, 32},
        {X86_REG_DR12, 32},
        {X86_REG_DR13, 32},
        {X86_REG_DR14, 32},
        {X86_REG_DR15, 32},
        {X86_REG_CR0, 32},
        {X86_REG_CR1, 32},
        {X86_REG_CR2, 32},
        {X86_REG_CR3, 32},
        {X86_REG_CR4, 32},
        // 64 bits
        {X86_REG_MM0, 64},
        {X86_REG_MM1, 64},
        {X86_REG_MM2, 64},
        {X86_REG_MM3, 64},
        {X86_REG_MM4, 64},
        {X86_REG_MM5, 64},
        {X86_REG_MM6, 64},
        {X86_REG_MM7, 64},
        {X86_REG_FPSW, 80},
        // 128 bits
        { X86_REG_XMM0, 128},
        { X86_REG_XMM1, 128},
        { X86_REG_XMM2, 128},
        { X86_REG_XMM3, 128},
        { X86_REG_XMM4, 128},
        { X86_REG_XMM5, 128},
        { X86_REG_XMM6, 128},
        { X86_REG_XMM7, 128},
    };

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

    std::size_t AbiX86::getRegSize(x86_reg reg_op) const
    {
        for (auto& pair : _reg2SizeMap) {
            if (pair.first == reg_op)
                return (pair.second / BYTES_SIZE);
        }
        return 0;
    }
} // namespace cchips
