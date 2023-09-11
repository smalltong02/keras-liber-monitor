#include "PassStructure.h"
#include "CapstoneImpl.h"

namespace cchips {

    const std::map<x86_reg, std::string> CapInsn::_const_reg_name = {
        {X86_REG_INVALID, "invalid"},
        {X86_REG_AH, "ah"}, {X86_REG_AL, "al"}, {X86_REG_AX, "ax"}, {X86_REG_BH, "bh"}, {X86_REG_BL, "bl"},
        {X86_REG_BP, "bp"}, {X86_REG_BPL, "bpl"}, {X86_REG_BX, "bx"}, {X86_REG_CH, "ch"}, {X86_REG_CL, "cl"},
        {X86_REG_CS, "cs"}, {X86_REG_CX, "cx"}, {X86_REG_DH, "dh"}, {X86_REG_DI, "di"}, {X86_REG_DIL, "dil"},
        {X86_REG_DL, "dl"}, {X86_REG_DS, "ds"}, {X86_REG_DX, "dx"}, {X86_REG_EAX, "eax"}, {X86_REG_EBP, "ebp"},
        {X86_REG_EBX, "ebx"}, {X86_REG_ECX, "ecx"}, {X86_REG_EDI, "edi"}, {X86_REG_EDX, "edx"}, {X86_REG_EFLAGS, "eflags"},
        {X86_REG_EIP, "eip"}, {X86_REG_EIZ, "eiz"}, {X86_REG_ES, "es"}, {X86_REG_ESI, "esi"}, {X86_REG_ESP, "esp"},
        {X86_REG_FPSW, "fpsw"}, {X86_REG_FS, "fs"}, {X86_REG_GS, "gs"}, {X86_REG_IP, "ip"}, {X86_REG_RAX, "rax"},
        {X86_REG_RBP, "rbp"}, {X86_REG_RBX, "rbx"}, {X86_REG_RCX, "rcx"}, {X86_REG_RDI, "rdi"}, {X86_REG_RDX, "rdx"},
        {X86_REG_RIP, "rip"}, {X86_REG_RIZ, "riz"}, {X86_REG_RSI, "rsi"}, {X86_REG_RSP, "rsp"}, {X86_REG_SI, "si"},
        {X86_REG_SIL, "sil"}, {X86_REG_SP, "sp"}, {X86_REG_SPL, "spl"}, {X86_REG_SS, "ss"}, {X86_REG_CR0, "cr0"},
        {X86_REG_CR1, "cr1"}, {X86_REG_CR2, "cr2"}, {X86_REG_CR3, "cr3"}, {X86_REG_CR4, "cr4"}, {X86_REG_CR5, "cr5"},
        {X86_REG_CR6, "cr6"}, {X86_REG_CR7, "cr7"}, {X86_REG_CR8, "cr8"}, {X86_REG_CR9, "cr9"}, {X86_REG_CR10, "cr10"},
        {X86_REG_CR11, "cr11"}, {X86_REG_CR12, "cr12"}, {X86_REG_CR13, "cr13"}, {X86_REG_CR14, "cr14"}, {X86_REG_CR15, "cr15"},
        {X86_REG_DR0, "dr0"}, {X86_REG_DR1, "dr1"}, {X86_REG_DR2, "dr2"}, {X86_REG_DR3, "dr3"}, {X86_REG_DR4, "dr4"},
        {X86_REG_DR5, "dr5"}, {X86_REG_DR6, "dr6"}, {X86_REG_DR7, "dr7"}, {X86_REG_DR8, "dr8"}, {X86_REG_DR9, "dr9"},
        {X86_REG_DR10, "dr10"}, {X86_REG_DR11, "dr11"}, {X86_REG_DR12, "dr12"}, {X86_REG_DR13, "dr13"}, {X86_REG_DR14, "dr14"},
        {X86_REG_DR15, "dr15"}, {X86_REG_FP0, "fp0"}, {X86_REG_FP1, "fp1"}, {X86_REG_FP2, "fp2"}, {X86_REG_FP3, "fp3"},
        {X86_REG_FP4, "fp4"}, {X86_REG_FP5, "fp5"}, {X86_REG_FP6, "fp6"}, {X86_REG_FP7, "fp7"},
        {X86_REG_K0, "k0"}, {X86_REG_K1, "k1"}, {X86_REG_K2, "k2"}, {X86_REG_K3, "k3"}, {X86_REG_K4, "k4"},
        {X86_REG_K5, "k5"}, {X86_REG_K6, "k6"}, {X86_REG_K7, "k7"}, {X86_REG_MM0, "mm0"}, {X86_REG_MM1, "mm1"},
        {X86_REG_MM2, "mm2"}, {X86_REG_MM3, "mm3"}, {X86_REG_MM4, "mm4"}, {X86_REG_MM5, "mm5"}, {X86_REG_MM6, "mm6"},
        {X86_REG_MM7, "mm7"}, {X86_REG_R8, "r8"}, {X86_REG_R9, "r9"}, {X86_REG_R10, "r10"}, {X86_REG_R11, "r11"},
        {X86_REG_R12, "r12"}, {X86_REG_R13, "r13"}, {X86_REG_R14, "r14"}, {X86_REG_R15, "r15"},
        {X86_REG_ST0, "st0"}, {X86_REG_ST1, "st1"}, {X86_REG_ST2, "st2"}, {X86_REG_ST3, "st3"},
        {X86_REG_ST4, "st4"}, {X86_REG_ST5, "st5"}, {X86_REG_ST6, "st6"}, {X86_REG_ST7, "st7"},
        {X86_REG_XMM0, "xmm0"}, {X86_REG_XMM1, "xmm1"}, {X86_REG_XMM2, "xmm2"}, {X86_REG_XMM3, "xmm3"}, {X86_REG_XMM4, "xmm4"},
        {X86_REG_XMM5, "xmm5"}, {X86_REG_XMM6, "xmm6"}, {X86_REG_XMM7, "xmm7"}, {X86_REG_XMM8, "xmm8"}, {X86_REG_XMM9, "xmm9"},
        {X86_REG_XMM10, "xmm10"}, {X86_REG_XMM11, "xmm11"}, {X86_REG_XMM12, "xmm12"}, {X86_REG_XMM13, "xmm13"}, {X86_REG_XMM14, "xmm14"},
        {X86_REG_XMM15, "xmm15"}, {X86_REG_XMM16, "xmm16"}, {X86_REG_XMM17, "xmm17"}, {X86_REG_XMM18, "xmm18"}, {X86_REG_XMM19, "xmm19"},
        {X86_REG_XMM20, "xmm20"}, {X86_REG_XMM21, "xmm21"}, {X86_REG_XMM22, "xmm22"}, {X86_REG_XMM23, "xmm23"}, {X86_REG_XMM24, "xmm24"},
        {X86_REG_XMM25, "xmm25"}, {X86_REG_XMM26, "xmm26"}, {X86_REG_XMM27, "xmm27"}, {X86_REG_XMM28, "xmm28"}, {X86_REG_XMM29, "xmm29"},
        {X86_REG_XMM30, "xmm30"}, {X86_REG_XMM31, "xmm31"}, {X86_REG_YMM0, "ymm0"}, {X86_REG_YMM1, "ymm1"}, {X86_REG_YMM2, "ymm2"},
        {X86_REG_YMM3, "ymm3"}, {X86_REG_YMM4, "ymm4"}, {X86_REG_YMM5, "ymm5"}, {X86_REG_YMM6, "ymm6"}, {X86_REG_YMM7, "ymm7"},
        {X86_REG_YMM8, "ymm8"}, {X86_REG_YMM9, "ymm9"}, {X86_REG_YMM10, "ymm10"}, {X86_REG_YMM11, "ymm11"}, {X86_REG_YMM12, "ymm12"},
        {X86_REG_YMM13, "ymm13"}, {X86_REG_YMM14, "ymm14"}, {X86_REG_YMM15, "ymm15"}, {X86_REG_YMM16, "ymm16"}, {X86_REG_YMM17, "ymm17"},
        {X86_REG_YMM18, "ymm18"}, {X86_REG_YMM19, "ymm19"}, {X86_REG_YMM20, "ymm20"}, {X86_REG_YMM21, "ymm21"}, {X86_REG_YMM22, "ymm22"},
        {X86_REG_YMM23, "ymm23"}, {X86_REG_YMM24, "ymm24"}, {X86_REG_YMM25, "ymm25"}, {X86_REG_YMM26, "ymm26"}, {X86_REG_YMM27, "ymm27"},
        {X86_REG_YMM28, "ymm28"}, {X86_REG_YMM29, "ymm29"}, {X86_REG_YMM30, "ymm30"}, {X86_REG_YMM31, "ymm31"}, {X86_REG_ZMM0, "zmm0"},
        {X86_REG_ZMM1, "zmm1"}, {X86_REG_ZMM2, "zmm2"}, {X86_REG_ZMM3, "zmm3"}, {X86_REG_ZMM4, "zmm4"}, {X86_REG_ZMM5, "zmm5"},
        {X86_REG_ZMM6, "zmm6"}, {X86_REG_ZMM7, "zmm7"}, {X86_REG_ZMM8, "zmm8"}, {X86_REG_ZMM9, "zmm9"}, {X86_REG_ZMM10, "zmm10"},
        {X86_REG_ZMM11, "zmm11"}, {X86_REG_ZMM12, "zmm12"}, {X86_REG_ZMM13, "zmm13"}, {X86_REG_ZMM14, "zmm14"}, {X86_REG_ZMM15, "zmm15"},
        {X86_REG_ZMM16, "zmm16"}, {X86_REG_ZMM17, "zmm17"}, {X86_REG_ZMM18, "zmm18"}, {X86_REG_ZMM19, "zmm19"}, {X86_REG_ZMM20, "zmm20"},
        {X86_REG_ZMM21, "zmm21"}, {X86_REG_ZMM22, "zmm22"}, {X86_REG_ZMM23, "zmm23"}, {X86_REG_ZMM24, "zmm24"}, {X86_REG_ZMM25, "zmm25"},
        {X86_REG_ZMM26, "zmm26"}, {X86_REG_ZMM27, "zmm27"}, {X86_REG_ZMM28, "zmm28"}, {X86_REG_ZMM29, "zmm29"}, {X86_REG_ZMM30, "zmm30"},
        {X86_REG_ZMM31, "zmm31"}, {X86_REG_R8B, "r8b"}, {X86_REG_R9B, "r9b"}, {X86_REG_R10B, "r10b"}, {X86_REG_R11B, "r11b"},
        {X86_REG_R12B, "r12b"}, {X86_REG_R13B, "r13b"}, {X86_REG_R14B, "r14b"}, {X86_REG_R15B, "r15b"}, {X86_REG_R8D, "r8d"},
        {X86_REG_R9D, "r9d"}, {X86_REG_R10D, "r10d"}, {X86_REG_R11D, "r11d"}, {X86_REG_R12D, "r12d"}, {X86_REG_R13D, "r13d"},
        {X86_REG_R14D, "r14d"}, {X86_REG_R15D, "r15d"}, {X86_REG_R8W, "r8w"}, {X86_REG_R9W, "r9w"}, {X86_REG_R10W, "r10w"},
        {X86_REG_R11W, "r11w"}, {X86_REG_R12W, "r12w"}, {X86_REG_R13W, "r13w"}, {X86_REG_R14W, "r14w"}, {X86_REG_R15W, "r15w"},
    };

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

    bool CapstoneImpl::GetEffectiveAddress(CapInsn& insn, std::uint8_t*& effect_addr, x86_op_type& op_type) const
    {
        std::shared_ptr<BasicBlock> block = insn.GetParent();
        if (!block) return false;
        std::shared_ptr<Function> function = block->GetParent();
        if (!function) return false;
        std::shared_ptr<Module> cur_module = function->GetParent();
        if (!cur_module) return false;
        if (InLeaGroup(insn)) {
            if (OpInCount(insn, X86_OP_IMM)) {
                int index = OpInIndex(insn, X86_OP_IMM, 1);
                if (insn.self()->detail) {
                    cs_x86* x86 = &insn.self()->detail->x86;
                    if (cur_module->GetContext()->isLoadingFile()) {
                        auto rva = cur_module->GetContext()->getValidRvaAddressFromFileAddress(insn.address());
                        effect_addr = reinterpret_cast<std::uint8_t*>(cur_module->GetContext()->getValidOffsetFromMemRva(rva + (x86->operands[index].imm - insn.address())));
                    }
                    else {
                        effect_addr = reinterpret_cast<std::uint8_t*>(x86->operands[index].imm);
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
                        effect_addr = reinterpret_cast<std::uint8_t*>(x86->operands[index].mem.disp);
                    }
                    else {
                        if (x86->operands[index].mem.segment == X86_REG_INVALID) {
                            if (x86->operands[index].mem.base == X86_REG_EIP || x86->operands[index].mem.base == X86_REG_RIP) {
                                if (cur_module->GetContext()->GetBits() == 32) {
                                    if (cur_module->GetContext()->isLoadingFile()) {
                                        effect_addr = reinterpret_cast<std::uint8_t*>(cur_module->GetContext()->getValidOffsetFromMemRva(x86->operands[index].mem.disp));
                                    }
                                    else {
                                    }
                                }
                                else if (cur_module->GetContext()->GetBits() == 64) {
                                    if (cur_module->GetContext()->isLoadingFile()) {
                                        auto rva = cur_module->GetContext()->getValidRvaAddressFromFileAddress(insn.address() + insn.size());
                                        effect_addr = reinterpret_cast<std::uint8_t*>(cur_module->GetContext()->getValidOffsetFromMemRva(rva + x86->operands[index].mem.disp));
                                    }
                                    else {
                                        effect_addr = reinterpret_cast<std::uint8_t*>(insn.address() + insn.size() + x86->operands[index].mem.disp);
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
                effect_addr = nullptr;
                return true;
            }
        }
        return false;
    }
} // namespace cchips
