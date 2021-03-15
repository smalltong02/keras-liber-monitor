#pragma once
#include <Windows.h>
#include <map>
#include <memory>
#include <set>
#include <vector>
#include "PassManager.h"

namespace cchips {

    class ModInit : public ModulePass
    {
    public:
        static char ID;
        ModInit() : ModulePass(ID) {}
        ~ModInit() {}

        virtual bool runOnModule(std::shared_ptr<Module> M) override;
    private:
        bool run();

        std::weak_ptr<Module> _module;
    };
} // namespace cchips
