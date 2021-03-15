#include "PassSupport.h"

namespace cchips {

    std::unique_ptr<Pass> PassRegistry::getPassInfo(AnalysisID ID) const
    {
        for (auto& pass_info : passinfos) {
            if (pass_info.getPassID() == ID) {
                return pass_info.getPass();
            }
        }
        return nullptr;
    }

    std::unique_ptr<Pass> PassRegistry::getPassInfo(const std::string& pass_name) const
    {
        for (auto& pass_info : passinfos) {
            if (pass_info.getPassName() == pass_name) {
                return pass_info.getPass();
            }
        }
        return nullptr;
    }

    void PassRegistry::Initialize()
    {
        std::shared_ptr<PassManager> passmanager = std::make_shared<PassManager>();
        std::shared_ptr<FunctionPassManager> funcpassmanager = std::make_shared<FunctionPassManager>();
        std::shared_ptr<BBPassManager> bbpassmanager = std::make_shared<BBPassManager>();
        std::shared_ptr<RGPassManager> rgpassmanager = std::make_shared<RGPassManager>();
        std::shared_ptr<LPPassManager> lppassmanager = std::make_shared<LPPassManager>();
        std::shared_ptr<InsnPassManager> insnpassmanager = std::make_shared<InsnPassManager>();
        if (!passmanager) return;
        if (!funcpassmanager) return;
        if (!bbpassmanager) return;
        if (!rgpassmanager) return;
        if (!lppassmanager) return;
        if (!insnpassmanager) return;
        passmanagerbases.push_back(std::move(passmanager));
        passmanagerbases.push_back(std::move(funcpassmanager));
        passmanagerbases.push_back(std::move(bbpassmanager));
        passmanagerbases.push_back(std::move(rgpassmanager));
        passmanagerbases.push_back(std::move(lppassmanager));
        passmanagerbases.push_back(std::move(insnpassmanager));
    }

    void PassRegistry::registerPass(PassInfo &PI)
    {
        if (!PI.valid()) return;
        std::shared_ptr<PassManagerBase>& pass_manager = getPassManager(PI.getPass()->getPotentialPassManagerType());
        if (!pass_manager) return;
        if (pass_manager->Add(PI.getPassID(), std::move(PI.getPass()))) {
            passinfos.emplace_back(PI);
        }
        return;
    }

    AnalysisID PassRegistry::GetPassID(const std::string& pass_name)
    {
        for (auto& pass_info : passinfos) {
            if (pass_info.getPassName() == pass_name) {
                return pass_info.getPassID();
            }
        }
        return nullptr;
    }

    bool PassRegistry::sequence()
    {
        std::map<Pass::passmanager_type, std::vector<AnalysisID>> sequence_types;
        for (auto& sequence : sequence_passes_define) {
            std::unique_ptr<Pass> pass = getPassInfo(sequence);
            if (!pass) continue;
            sequence_types[pass->getPotentialPassManagerType()].emplace_back(pass->getPassID());
        }
        for (auto& manager : sequence_types) {
            std::shared_ptr<PassManagerBase> pass_manager = std::static_pointer_cast<PassManagerBase>(getPassManager(manager.first));
            if (!pass_manager) continue;
            pass_manager->sequence(manager.second);
        }
        for (auto& base : passmanagerbases) {
            auto find = sequence_types.find(base->GetPassManagerType());
            if (find == sequence_types.end()) {
                base->sequence({});
            }
        }
        return true;
    }

    bool PassRegistry::run(std::shared_ptr<Module> PM)
    {
        if (!PM) return false;
        std::shared_ptr<PassManager> pass_manager = std::static_pointer_cast<PassManager>(getPassManager(Pass::passmanager_module));
        if (!pass_manager) return false;
        return pass_manager->Run(PM);
    }

    bool PassRegistry::run(std::shared_ptr<Function> PF)
    {
        if (!PF) return false;
        std::shared_ptr<FunctionPassManager> funcs_manager = std::static_pointer_cast<FunctionPassManager>(getPassManager(Pass::passmanager_function));
        if (!funcs_manager) return false;
        return funcs_manager->Run(PF);
    }

    bool PassRegistry::run(std::shared_ptr<BasicBlock> PB)
    {
        if (!PB) return false;
        std::shared_ptr<BBPassManager> bb_manager = std::static_pointer_cast<BBPassManager>(getPassManager(Pass::passmanager_basicblock));
        if (!bb_manager) return false;
        return bb_manager->Run(PB);
    }

    bool PassRegistry::run(std::shared_ptr<CapInsn> PC)
    {
        if (!PC) return false;
        std::shared_ptr<InsnPassManager> insn_manager = std::static_pointer_cast<InsnPassManager>(getPassManager(Pass::passmanager_instruction));
        if (!insn_manager) return false;
        return insn_manager->Run(PC);
    }

    std::shared_ptr<PassManagerBase> PassRegistry::getPassManager(Pass::passmanager_type type) const
    {
        for (auto& manager : passmanagerbases) {
            if (!manager) continue;
            if (manager->GetPassManagerType() == type)
                return manager;
        }
        return nullptr;
    }
} // namespace cchips
