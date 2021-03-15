#pragma once
#include <Windows.h>
#include <map>
#include <memory>
#include <set>
#include <vector>
#include "PassManager.h"

namespace cchips {

    class BBInit : public BasicBlockPass
    {
    public:
        static char ID;
        BBInit() : BasicBlockPass(ID) {}
        ~BBInit() {}

        virtual bool runOnBasicBlock(std::shared_ptr<BasicBlock> Block) override;

    private:
        bool run(std::shared_ptr<BasicBlock>& Block);
        bool processEndBlock(CapInsn& insn);
        bool processJmpInstruction(CapInsn& insn);
        bool processLoopInstruction(CapInsn& insn);
        void processCallInstruction(CapInsn& insn);
    };
} // namespace cchips
