#include "Abi.h"

namespace cchips {

    bool Abi::isRegister(cs_x86_op* operand) const
    {
        if (!operand) return false;
        return operand->type == X86_OP_REG;
    }

    bool Abi::isFlagRegister(cs_x86_op* operand) const
    {
        if (!operand) return false;
        if (operand->type != X86_OP_REG) return false;
        return operand->reg == X86_REG_EFLAGS;
    }

    bool Abi::isStackPointerRegister(cs_x86_op* operand) const
    {
        if (!operand) return false;
        if (operand->type != X86_OP_REG) return false;
        return operand->reg == _regStackPointerId;
    }

    x86_reg Abi::getRegisterId(cs_x86_op* operand) const
    {
        if (!operand) return X86_REG_INVALID;
        if (operand->type != X86_OP_REG) return X86_REG_INVALID;
        return operand->reg;
    }

    std::size_t Abi::getRegisterByteSize(cs_x86_op* operand) const
    {
        if (!operand) return 0;
        if (operand->type != X86_OP_REG) return 0;
        return operand->size;
    }

} // namespace cchips
