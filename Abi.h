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
#define BYTES_SIZE 8
        using x86_reg_eflags = enum {
            X86_REG_CF = X86_REG_ENDING + 1, // 0
            // reserved 1
            X86_REG_PF, // 2
            // reserved 3
            X86_REG_AF, // 4
            // reserved 5
            X86_REG_ZF, // 6
            X86_REG_SF, // 7
            X86_REG_TF, // 8
            X86_REG_IF, // 9
            X86_REG_DF, // 10
            X86_REG_OF, // 11
            X86_REG_IOPL, // 12-13
            X86_REG_NT, // 14
            // reserved 15
            // EFLAGS
            X86_REG_RF, // 16
            X86_REG_VM, // 17
            X86_REG_AC, // 18
            X86_REG_VIF, // 19
            X86_REG_VIP, // 20
            X86_REG_ID // 21
            // reserved 22-31
            // RFLAGS
            // reserved 32-63
        };

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
        virtual std::size_t getRegSize(x86_reg reg_op) const = 0;

        // Instructions.
    public:
        virtual const std::vector<std::pair<x86_reg, uint32_t>>& getReg2Size() const = 0;
        virtual bool isNopInstruction(const CapInsn& insn) const = 0;

        // Private data - registers.
    protected:
#define REG_INVALID 0
        std::vector<std::shared_ptr<GlobalVariable>> _regs;
        std::vector<std::shared_ptr<GlobalVariable>> _id2regs;
        //std::map<const llvm::Value*, uint32_t> _regs2id;
        static std::map<
            std::size_t,
            void (Abi::*)(
                cs_insn&,
                cs_x86*,
                CapInsn&)> _i2fm;

        uint32_t _regStackPointerId = REG_INVALID;
        uint32_t _regFunctionReturnId = REG_INVALID;
        std::vector<uint32_t> _syscallRegs;
        uint32_t _regSyscallReturn = REG_INVALID;
        uint32_t _regSyscallId = REG_INVALID;
        uint32_t _regZeroReg = REG_INVALID;

    protected:
        void translateSub(cs_insn& insn, cs_x86* x86, CapInsn& cap_insn);
    };
} // namespace cchips
