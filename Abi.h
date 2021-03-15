#pragma once

#include <map>
#include <memory>
#include <set>
#include <vector>
#include "CapstoneImpl.h"

namespace cchips {

    class GlobalVariable;

    class Abi
    {
    public:

        // Ctors, dtors.
        Abi() = default;
        virtual ~Abi() = default;

        // Registers.
    public:
        bool isRegister(cs_x86_op* operand) const;
        bool isFlagRegister(cs_x86_op* operand) const;
        bool isStackPointerRegister(cs_x86_op* operand) const;
        bool isZeroRegister(cs_x86_op* operand) const { return false; }
        virtual bool isGeneralPurposeRegister(cs_x86_op* operand) const = 0;
        virtual bool isX86() const = 0;
        virtual bool isX64() const = 0;

        x86_reg getRegisterId(cs_x86_op* operand) const;
        std::size_t getRegisterByteSize(cs_x86_op* operand) const;

        // Instructions.
    public:
        virtual bool isNopInstruction(const CapInsn& insn) const = 0;

        // Private data - registers.
    protected:
#define REG_INVALID 0
        std::vector<std::shared_ptr<GlobalVariable>> _regs;
        std::vector<std::shared_ptr<GlobalVariable>> _id2regs;
        //std::map<const llvm::Value*, uint32_t> _regs2id;

        uint32_t _regStackPointerId = REG_INVALID;
        uint32_t _regFunctionReturnId = REG_INVALID;
        std::vector<uint32_t> _syscallRegs;
        uint32_t _regSyscallReturn = REG_INVALID;
        uint32_t _regSyscallId = REG_INVALID;
        uint32_t _regZeroReg = REG_INVALID;
    };
} // namespace cchips
