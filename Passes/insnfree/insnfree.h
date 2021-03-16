#pragma once
#include <Windows.h>
#include <map>
#include <memory>
#include <set>
#include <vector>
#include "PassManager.h"

namespace cchips {

    class InsnFree : public InstructionPass
    {
    public:
        static char ID;
        InsnFree() : InstructionPass(ID) {}
        ~InsnFree() {}

        virtual bool runOnInstruction(std::shared_ptr<CapInsn> Insn) override;

    private:
        bool run(std::shared_ptr<CapInsn>& Insn);
    };
} // namespace cchips
