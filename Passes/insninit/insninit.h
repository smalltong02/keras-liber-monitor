#pragma once
#include <Windows.h>
#include <map>
#include <memory>
#include <set>
#include <vector>
#include "PassManager.h"

namespace cchips {

    class InsnInit : public InstructionPass
    {
    public:
        static char ID;
        InsnInit() : InstructionPass(ID) {}
        ~InsnInit() {}

        virtual bool runOnInstruction(std::shared_ptr<CapInsn> Insn) override;

    private:
        bool run(std::shared_ptr<CapInsn>& Insn);
    };
} // namespace cchips
