#pragma once
#include "Abi.h"

namespace cchips {

    class AbiX64 : public Abi
    {
    public:
        AbiX64();

        virtual bool isGeneralPurposeRegister(cs_x86_op* operand) const override;
        virtual bool isNopInstruction(const CapInsn& insn) const override;
        virtual bool isX86() const override { return false; }
        virtual bool isX64() const override { return true; }
        const std::vector<std::pair<x86_reg, uint32_t>>& getReg2Size() const override { return _reg2SizeMap; }
    private:
        static std::map<x86_reg, std::pair<x86_reg, uint32_t>> _reg2parentMap;
        static std::vector<std::pair<x86_reg, uint32_t>> _reg2SizeMap;
        //CallingConvention::ID fetchDefaultCC() const;
    };
} // namespace cchips
