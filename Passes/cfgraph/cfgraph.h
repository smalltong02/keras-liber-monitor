#pragma once
#include <Windows.h>
#include <map>
#include <memory>
#include <set>
#include <vector>
#include "PassManager.h"

namespace cchips {

    class CflowGraph : public ModulePass
    {
    public:
        using cfgerror_code = enum {
            error_success = 0,
        };

#define MODSTATUS_SUCCESS(st) (st >= 0)

        static char ID;
        CflowGraph() : ModulePass(ID) {}
        ~CflowGraph() {}

        virtual bool runOnModule(std::shared_ptr<Module> M) override;

    private:
        cfgerror_code run(std::shared_ptr<Module> M);
        static const std::map<cfgerror_code, std::string> _error_code_descs;
    };
} // namespace cchips
