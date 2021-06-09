#pragma once
#include <Windows.h>
#include <map>
#include <memory>
#include <set>
#include <vector>
#include "PassManager.h"

namespace cchips {

    class LoopInit : public FunctionPass
    {
    public:
        using invariant_context_type = std::pair<SimpleLoop::invariant_type, SimpleLoop::stepsize_type>;
        static char ID;
        LoopInit() : FunctionPass(ID) {}
        ~LoopInit() {}

        virtual bool runOnFunction(std::shared_ptr<Function> Func) override;

    private:
        bool run(std::shared_ptr<Function>& Func);
        bool isInvariantInsn(CapInsn& insn, int eflags);
        bool simulateTraceInvariant(std::shared_ptr<BasicBlock> block, SimpleLoop::invariant_type invariant);
        std::any GetInvariantContext(CapInsn& insn);
        SimpleLoop::value_type GetInvariantValue(SimpleLoop::invariant_type invariant, std::shared_ptr<BasicBlock> block);
        bool isSimpleLoop(std::shared_ptr<BasicBlock> block);

        std::shared_ptr<Abi> _abi;
    };
} // namespace cchips
