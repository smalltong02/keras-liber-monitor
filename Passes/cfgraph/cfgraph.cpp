#include "cfgraph.h"
#include "PassSupport.h"
#include "utils.h"
#include "LogObject.h"

namespace cchips {

    char CflowGraph::ID = 0;

    static RegisterPass<CflowGraph> X(
        "CflowGraph",
        "Control Flow Graph",
        PassInfo::passreg_post
    );

    const std::map<CflowGraph::cfgerror_code, std::string> CflowGraph::_error_code_descs = {
        {error_success, "module check success."},

    };

    bool CflowGraph::runOnModule(std::shared_ptr<Module> M)
    {
        if (!M)
            return false;
        cfgerror_code code = run(M);
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

    CflowGraph::cfgerror_code CflowGraph::run(std::shared_ptr<Module> M)
    {
        if (!M) return error_success;
        std::vector<std::string> cfg_database;
        auto funcs_size =  M->size();
        for (std::uint32_t func_no = 0; func_no < funcs_size; func_no++) {
            auto& func = M->GetFunction(func_no);
            if (func) {
                auto blocks_size = func->size();
                for (std::uint32_t block_no = 0; block_no < blocks_size; block_no++) {
                    auto& block = func->GetBasicBlock(block_no);
                    if (block) {
                        auto insns_size = block->size();
                        for (std::uint32_t insn_no = 0; insn_no < insns_size; insn_no++) {
                            auto& insn = block->getInsn(insn_no);
                            if (insn) {
                                if (insn->GetInsnType() == CapInsn::insn_linkfunc ||
                                    insn->GetInsnType() == CapInsn::insn_linkvar ||
                                    insn->GetInsnType() == CapInsn::insn_constvar) {
                                    std::string mnemonic = insn->GetTargetMnemonic();
                                    if (mnemonic.length()) {
                                        cfg_database.push_back(mnemonic);
                                    }
                                    //std::cout << "CflowGraph: " << insn->GetTargetMnemonic() << std::endl;
                                }
                            }
                        }
                    }
                }
            }
        }
        if (cfg_database.size()) {
            M->AddDatabaseToCfgraph("apiflowcfg", cfg_database);
        }
        return error_success;
    }
} // namespace cchips
