#include "modcheck.h"
#include "PassSupport.h"
#include "utils.h"
#include "LogObject.h"

namespace cchips {

    char ModCheck::ID = 0;

    static RegisterPass<ModCheck> X(
        "ModCheck",
        "Module check",
        PassInfo::passreg_post
    );

    const std::map<ModCheck::moderror_code, std::string> ModCheck::_error_code_descs = {
        {error_success, "module check success."},
        {error_mod_non_prefin, "module state error."},
        {error_func_non_postfin, "function state error."},
        {error_block_non_postfin, "block state error."},
        {error_insn_non_postfin, "insn state error."},
    };

    bool ModCheck::runOnModule(std::shared_ptr<Module> M)
    {
        if (!M)
            return false;
        moderror_code code = run(M);
        if (!MODSTATUS_SUCCESS(code)) {
            std::string error_desc;
            auto& desc = _error_code_descs.find(code);
            if (desc != _error_code_descs.end()) {
                error_desc = desc->second;
            }
            else {
                error_desc = "unknown error.";
            }
            M->GetReportObject().CreateReport(M, error_desc);
            error_log("funccheck::runOnFunction Function({}), address({}), error desc({})", M->GetModuleName(), M->GetBaseAddress(), error_desc);
        }
        return true;
    }

    ModCheck::moderror_code ModCheck::run(std::shared_ptr<Module> M)
    {
        if (!M) return error_mod_non_prefin;
        if (!M->IsPrefin()) {
            return error_mod_non_prefin;
        }
        auto funcs_size = M->size();
        for (std::uint32_t func_no = 0; func_no < funcs_size; func_no++) {
            auto& func = M->GetFunction(func_no);
            if (func) {
                if (!func->IsPostfin()) {
                    return error_func_non_postfin;
                }
                auto blocks_size = func->size();
                for (std::uint32_t block_no = 0; block_no < blocks_size; block_no++) {
                    auto& block = func->GetBasicBlock(block_no);
                    if (block) {
                        if (!block->IsPostfin()) {
                            return error_block_non_postfin;
                        }
                        auto insns_size = block->size();
                        for (std::uint32_t insn_no = 0; insn_no < insns_size; insn_no++) {
                            auto& insn = block->getInsn(insn_no);
                            if (insn) {
                                if (!insn->IsPostfin()) {
                                    return error_insn_non_postfin;
                                }
                            }
                        }
                    }
                }
            }
        }
        return error_success;
    }
} // namespace cchips
