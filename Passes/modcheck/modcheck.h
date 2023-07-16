#pragma once
#include <Windows.h>
#include <map>
#include <memory>
#include <set>
#include <vector>
#include "PassManager.h"

namespace cchips {

    class ModCheck : public ModulePass
    {
    public:
        using moderror_code = enum {
            error_success = 0,
            error_mod_non_prefin,
            error_func_non_postfin,
            error_block_non_postfin,
            error_insn_non_postfin,
        };

#define MODSTATUS_SUCCESS(st) (st >= 0)

        static char ID;
        ModCheck() : ModulePass(ID) {}
        ~ModCheck() {}

        virtual bool runOnModule(std::shared_ptr<Module> M) override;

    private:
        moderror_code run(std::shared_ptr<Module> M);
        static const std::map<moderror_code, std::string> _error_code_descs;
    };
} // namespace cchips
