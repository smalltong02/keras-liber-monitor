#include "Abix64.h"

namespace cchips {

    std::map<x86_reg, std::pair<x86_reg, uint32_t>> AbiX64::_reg2parentMap = {
        {X86_REG_AL, {X86_REG_AX, 0}},
        {X86_REG_AH, {X86_REG_AX, 8}},
        {X86_REG_AX, {X86_REG_EAX, 0}},
        {X86_REG_EAX, {X86_REG_RAX, 0}},
        {X86_REG_BL, {X86_REG_BX, 0}},
        {X86_REG_BH, {X86_REG_BX, 8}},
        {X86_REG_BX, {X86_REG_EBX, 0}},
        {X86_REG_EBX, {X86_REG_RBX, 0}},
        {X86_REG_CL, {X86_REG_CX, 0}},
        {X86_REG_CH, {X86_REG_CX, 8}},
        {X86_REG_CX, {X86_REG_ECX, 0}},
        {X86_REG_ECX, {X86_REG_RCX, 0}},
        {X86_REG_DL, {X86_REG_DX, 0}},
        {X86_REG_DH, {X86_REG_DX, 8}},
        {X86_REG_DX, {X86_REG_EDX, 0}},
        {X86_REG_EDX, {X86_REG_RDX, 0}},
        {X86_REG_SPL, {X86_REG_SP, 0}},
        {X86_REG_SP, {X86_REG_ESP, 0}},
        {X86_REG_ESP, {X86_REG_RSP, 0}},
        {X86_REG_BPL, {X86_REG_BP, 0}},
        {X86_REG_BP, {X86_REG_EBP, 0}},
        {X86_REG_EBP, {X86_REG_RBP, 0}},
        {X86_REG_SIL, {X86_REG_SI, 0}},
        {X86_REG_SI, {X86_REG_ESI, 0}},
        {X86_REG_ESI, {X86_REG_RSI, 0}},
        {X86_REG_DIL, {X86_REG_DI, 0}},
        {X86_REG_DI, {X86_REG_EDI, 0}},
        {X86_REG_EDI, {X86_REG_RDI, 0}},
        {X86_REG_IP, {X86_REG_EIP, 0}},
        {X86_REG_EIP, {X86_REG_RIP, 0}},
        {X86_REG_EIZ, {X86_REG_RIZ, 0}},
        {X86_REG_R8B, {X86_REG_R8W, 0}},
        {X86_REG_R8W, {X86_REG_R8D, 0}},
        {X86_REG_R8D, {X86_REG_R8, 0}},
        {X86_REG_R9B, {X86_REG_R9W, 0}},
        {X86_REG_R9W, {X86_REG_R9D, 0}},
        {X86_REG_R9D, {X86_REG_R9, 0}},
        {X86_REG_R10B, {X86_REG_R10W, 0}},
        {X86_REG_R10W, {X86_REG_R10D, 0}},
        {X86_REG_R10D, {X86_REG_R10, 0}},
        {X86_REG_R11B, {X86_REG_R11W, 0}},
        {X86_REG_R11W, {X86_REG_R11D, 0}},
        {X86_REG_R11D, {X86_REG_R11, 0}},
        {X86_REG_R12B, {X86_REG_R12W, 0}},
        {X86_REG_R12W, {X86_REG_R12D, 0}},
        {X86_REG_R12D, {X86_REG_R12, 0}},
        {X86_REG_R13B, {X86_REG_R13W, 0}},
        {X86_REG_R13W, {X86_REG_R13D, 0}},
        {X86_REG_R13D, {X86_REG_R13, 0}},
        {X86_REG_R14B, {X86_REG_R14W, 0}},
        {X86_REG_R14W, {X86_REG_R14D, 0}},
        {X86_REG_R14D, {X86_REG_R14, 0}},
        {X86_REG_R15B, {X86_REG_R15W, 0}},
        {X86_REG_R15W, {X86_REG_R15D, 0}},
        {X86_REG_R15D, {X86_REG_R15, 0}},
    };

    std::vector<std::pair<x86_reg, uint32_t>> AbiX64::_reg2SizeMap = {
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
        {X86_REG_R8B, 8},
        {X86_REG_R9B, 8},
        {X86_REG_R10B, 8},
        {X86_REG_R11B, 8},
        {X86_REG_R12B, 8},
        {X86_REG_R13B, 8},
        {X86_REG_R14B, 8},
        {X86_REG_R15B, 8},
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
        {X86_REG_R8W, 16},
        {X86_REG_R9W, 16},
        {X86_REG_R10W, 16},
        {X86_REG_R11W, 16},
        {X86_REG_R12W, 16},
        {X86_REG_R13W, 16},
        {X86_REG_R14W, 16},
        {X86_REG_R15W, 16},
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
        {X86_REG_R8D, 32},
        {X86_REG_R9D, 32},
        {X86_REG_R10D, 32},
        {X86_REG_R11D, 32},
        {X86_REG_R12D, 32},
        {X86_REG_R13D, 32},
        {X86_REG_R14D, 32},
        {X86_REG_R15D, 32},
        {X86_REG_EIP, 32},
        {X86_REG_EIZ, 32},
        // 64 bits
        {X86_REG_RAX, 64},
        {X86_REG_RBP, 64},
        {X86_REG_RBX, 64},
        {X86_REG_RCX, 64},
        {X86_REG_RDI, 64},
        {X86_REG_RDX, 64},
        {X86_REG_RIP, 64},
        {X86_REG_RIZ, 64},
        {X86_REG_RSI, 64},
        {X86_REG_RSP, 64},
        {X86_REG_R8, 64},
        {X86_REG_R9, 64},
        {X86_REG_R10, 64},
        {X86_REG_R11, 64},
        {X86_REG_R12, 64},
        {X86_REG_R13, 64},
        {X86_REG_R14, 64},
        {X86_REG_R15, 64},
        {X86_REG_EFLAGS, 64},
        {X86_REG_DR0, 64},
        {X86_REG_DR1, 64},
        {X86_REG_DR2, 64},
        {X86_REG_DR3, 64},
        {X86_REG_DR4, 64},
        {X86_REG_DR5, 64},
        {X86_REG_DR6, 64},
        {X86_REG_DR7, 64},
        {X86_REG_DR8, 64},
        {X86_REG_DR9, 64},
        {X86_REG_DR10, 64},
        {X86_REG_DR11, 64},
        {X86_REG_DR12, 64},
        {X86_REG_DR13, 64},
        {X86_REG_DR14, 64},
        {X86_REG_DR15, 64},
        {X86_REG_CR0, 64},
        {X86_REG_CR1, 64},
        {X86_REG_CR2, 64},
        {X86_REG_CR3, 64},
        {X86_REG_CR4, 64},
        {X86_REG_CR5, 64},
        {X86_REG_CR6, 64},
        {X86_REG_CR7, 64},
        {X86_REG_CR8, 64},
        {X86_REG_CR9, 64},
        {X86_REG_CR10, 64},
        {X86_REG_CR11, 64},
        {X86_REG_CR12, 64},
        {X86_REG_CR13, 64},
        {X86_REG_CR14, 64},
        {X86_REG_CR15, 64},
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
        {X86_REG_XMM0, 128},
        {X86_REG_XMM1, 128},
        {X86_REG_XMM2, 128},
        {X86_REG_XMM3, 128},
        {X86_REG_XMM4, 128},
        {X86_REG_XMM5, 128},
        {X86_REG_XMM6, 128},
        {X86_REG_XMM7, 128},
        {X86_REG_XMM8, 128},
        {X86_REG_XMM9, 128},
        {X86_REG_XMM10, 128},
        {X86_REG_XMM11, 128},
        {X86_REG_XMM12, 128},
        {X86_REG_XMM13, 128},
        {X86_REG_XMM14, 128},
        {X86_REG_XMM15, 128},
        {X86_REG_XMM16, 128},
        {X86_REG_XMM17, 128},
        {X86_REG_XMM18, 128},
        {X86_REG_XMM19, 128},
        {X86_REG_XMM20, 128},
        {X86_REG_XMM21, 128},
        {X86_REG_XMM22, 128},
        {X86_REG_XMM23, 128},
        {X86_REG_XMM24, 128},
        {X86_REG_XMM25, 128},
        {X86_REG_XMM26, 128},
        {X86_REG_XMM27, 128},
        {X86_REG_XMM28, 128},
        {X86_REG_XMM29, 128},
        {X86_REG_XMM30, 128},
        {X86_REG_XMM31, 128},
    };

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
