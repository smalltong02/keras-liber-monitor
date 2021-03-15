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
    private:
        //CallingConvention::ID fetchDefaultCC() const;
    };
} // namespace cchips
