#pragma once
#include <Windows.h>
#include <map>
#include <memory>
#include <set>
#include <vector>
#include "PassManager.h"

namespace cchips {

    class FuncCheck : public FunctionPass
    {
    public:
        using funcerror_code = enum {
            error_success = 0,
            error_func_unknown = -1,
            error_func_name = -2,
            error_func_address = -3,
            error_func_empty = -4,
            error_func_wrong_parent = -5,
            error_func_basicblocks = -6,
            error_func_no_end = -7,
        };

#define FUNCSTATUS_SUCCESS(st) (st >= 0)

        static char ID;
        FuncCheck() : FunctionPass(ID) {}
        ~FuncCheck() {}

        virtual bool runOnFunction(std::shared_ptr<Function> Func) override;

    private:
        funcerror_code run(std::shared_ptr<Function> Func);
        static const std::map<funcerror_code, std::string> _error_code_descs;
    };
} // namespace cchips
