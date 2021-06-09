#pragma once
#include <Windows.h>
#include <map>
#include <memory>
#include <set>
#include <vector>
#include "PassManager.h"

namespace cchips {

    class InsnFree : public FunctionPass
    {
    public:
        static char ID;
        InsnFree() : FunctionPass(ID) {}
        ~InsnFree() {}

        virtual bool runOnFunction(std::shared_ptr<Function> function) override;

    private:
        bool run(std::shared_ptr<Function> function);
    };
} // namespace cchips
