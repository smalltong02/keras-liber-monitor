#pragma once
#include <Windows.h>
#include <map>
#include <memory>
#include <set>
#include <vector>
#include "PassManager.h"

namespace cchips {

    class FuncInit : public FunctionPass
    {
    public:
        static char ID;
        FuncInit() : FunctionPass(ID) {}
        ~FuncInit() {}

        virtual bool runOnFunction(std::shared_ptr<Function> Func) override;

    private:
        bool run(std::shared_ptr<Function>& Func);
    };
} // namespace cchips
