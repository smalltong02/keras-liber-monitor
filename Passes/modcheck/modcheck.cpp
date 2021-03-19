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
        return error_success;
    }
} // namespace cchips
