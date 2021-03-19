#include "funccheck.h"
#include "PassSupport.h"
#include "utils.h"
#include "LogObject.h"

namespace cchips {

    char FuncCheck::ID = 0;

    static RegisterPass<FuncCheck> X(
        "FuncCheck",
        "Function check",
        PassInfo::passreg_post
    );

    const std::map<FuncCheck::funcerror_code, std::string> FuncCheck::_error_code_descs = {
        {error_success, "function check success."},
        {error_func_unknown, "unknown function."},
        {error_func_name, "error function name."},
        {error_func_address, "invalid function address."},
        {error_func_empty, "error no basicblock in function."},
        {error_func_wrong_parent, "function indicate module is wrong."},
        {error_func_basicblocks, "include invalid basicblock in function."},
        {error_func_no_end, "no end block in function."},
    };

    bool FuncCheck::runOnFunction(std::shared_ptr<Function> Func)
    {
        if (!Func)
            return false;
        std::shared_ptr<Module> cur_module = Func->GetParent();
        if (!cur_module)
            return false;
        funcerror_code code = run(Func);
        if (!FUNCSTATUS_SUCCESS(code)) {
            std::string error_desc;
            auto& desc = _error_code_descs.find(code);
            if (desc != _error_code_descs.end()) {
                error_desc = desc->second;
            }
            else {
                error_desc = "unknown error.";
            }
            cur_module->GetReportObject().CreateReport(Func, error_desc);
            error_log("funccheck::runOnFunction Function({}), address({}), error desc({})", Func->GetFuncName(), Func->GetFuncAddress(), error_desc);
        }
        return true;
    }

    FuncCheck::funcerror_code FuncCheck::run(std::shared_ptr<Function> Func)
    {
        if (Func->IsUnknown()) return error_func_unknown;
        if (!Func->GetFuncName().length()) return error_func_name;
        if (!Func->GetFuncAddress()) return error_func_address;
        if (Func->empty()) return error_func_empty;
        if (!Func->GetParent()) return error_func_wrong_parent;

        bool bend = false;
        for (auto& block : *Func) {
            if (!block.first) return error_func_basicblocks;
            if (!block.second) return error_func_basicblocks;
            if (block.second->GetBlockType() == BasicBlock::block_end) {
                bend = true;
            }
        }
        if (!bend) {
            if (Func->size() == 1) {
                auto& block = Func->begin();
                if (block->second->GetBlockType() == BasicBlock::block_linkage) {
                    Func->SetFuncType(Function::func_linkage);
                    return error_success;
                }
            }
            return error_func_no_end;
        }
        return error_success;
    }
} // namespace cchips
