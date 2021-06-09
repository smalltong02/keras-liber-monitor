#include "PassManager.h"
#include "PassSupport.h"
#include "PassStructure.h"
#include "CapstoneImpl.h"

namespace cchips {

    class PassManagerImpl : public Pass, public PMDataManager
    {
    public:
        static char ID;
        explicit PassManagerImpl() :
            Pass(Pass::pass_passmanager, ID), PMDataManager() {}

        bool Add(AnalysisID ID, std::unique_ptr<Pass> P);
        bool Run(std::shared_ptr<Module> M);
        void sequence(const std::vector<AnalysisID>& seq_list);
        using Pass::DoInitialization;
        using Pass::DoFinalization;
    private:
        std::vector<AnalysisID> sequencepass;
        std::map<AnalysisID, std::unique_ptr<Pass>> modulepasses;
    };

    char PassManagerImpl::ID = 0;

    class FunctionPassManagerImpl : public Pass, public PMDataManager
    {
    public:
        static char ID;
        explicit FunctionPassManagerImpl() :
            Pass(Pass::pass_passmanager, ID), PMDataManager() {}

        bool Add(AnalysisID ID, std::unique_ptr<Pass> P);
        bool Run(std::shared_ptr<Function> F);
        void sequence(const std::vector<AnalysisID>& seq_list);
        using Pass::DoInitialization;
        using Pass::DoFinalization;
    private:
        std::vector<AnalysisID> sequencepass;
        std::map<AnalysisID, std::unique_ptr<Pass>> functionpasses;
    };

    char FunctionPassManagerImpl::ID = 0;

    class BBPassManagerImpl : public Pass, public PMDataManager
    {
    public:
        static char ID;
        explicit BBPassManagerImpl() :
            Pass(Pass::pass_passmanager, ID), PMDataManager() {}

        bool Add(AnalysisID ID, std::unique_ptr<Pass> P);
        bool Run(std::shared_ptr<BasicBlock> BB);
        void sequence(const std::vector<AnalysisID>& seq_list);
        using Pass::DoInitialization;
        using Pass::DoFinalization;
    private:
        std::vector<AnalysisID> sequencepass;
        std::map<AnalysisID, std::unique_ptr<Pass>> bbpasses;
    };

    char BBPassManagerImpl::ID = 0;

    class InsnPassManagerImpl : public Pass, public PMDataManager
    {
    public:
        static char ID;
        explicit InsnPassManagerImpl() :
            Pass(Pass::pass_passmanager, ID), PMDataManager() {}

        bool Add(AnalysisID ID, std::unique_ptr<Pass> P);
        bool Run(std::shared_ptr<CapInsn> insn);
        void sequence(const std::vector<AnalysisID>& seq_list);
        using Pass::DoInitialization;
        using Pass::DoFinalization;
    private:
        std::vector<AnalysisID> sequencepass;
        std::map<AnalysisID, std::unique_ptr<Pass>> insnpasses;
    };

    char InsnPassManagerImpl::ID = 0;

    std::string_view Pass::getPassName() const {
        static char uname_pass[] = "Unnamed pass: implement Pass::getPassName()";
        AnalysisID AID = getPassID();
        std::unique_ptr<Pass> PI = GetPassRegistry().getPass(AID);
        if (PI)
            return PI->getPassName();
        return uname_pass;
    }

    PassManager::PassManager() {
        MPMI = std::make_unique<PassManagerImpl>();
    }

    bool PassManager::Add(AnalysisID ID, std::unique_ptr<Pass> P)
    {
        if (!ID || !P) return false;
        if (!MPMI) return false;
        return MPMI->Add(ID, std::move(P));
    }

    bool PassManager::Run(std::shared_ptr<Module> M)
    {
        if (!MPMI) return false;
        return MPMI->Run(M);
    }

    void PassManager::sequence(const std::vector<AnalysisID>& seq_list) 
    { 
        MPMI->sequence(seq_list); 
    }

    bool PassManagerImpl::Add(AnalysisID ID, std::unique_ptr<Pass> P)
    {
        if (P->getPassType() != Pass::pass_module)
            return false;
        auto success = modulepasses.try_emplace(ID, std::move(P));
        return success.second;
    }

    bool PassManagerImpl::Run(std::shared_ptr<Module> M) {
        bool Changed = true;

        for (auto& id : sequencepass) {
            auto& pass = modulepasses.find(id);
            if (pass == modulepasses.end()) continue;
            if (!pass->second) continue;
            auto info = GetPassRegistry().getPassInfo(id);
            if (!info || info->getRegHandle() != PassInfo::passreg_pre) continue;
            Changed &= reinterpret_cast<ModulePass*>(pass->second.get())->runOnModule(M);
        }

        if (Changed) {
            for (auto& func : *M) {
                GetPassRegistry().run(func.second);
            }
            for (auto& id : sequencepass) {
                auto& pass = modulepasses.find(id);
                if (pass == modulepasses.end()) continue;
                if (!pass->second) continue;
                auto info = GetPassRegistry().getPassInfo(id);
                if (!info || info->getRegHandle() != PassInfo::passreg_post) continue;
                reinterpret_cast<ModulePass*>(pass->second.get())->runOnModule(M);
            }
        }

        return Changed;
    }

    void PassManagerImpl::sequence(const std::vector<AnalysisID>& seq_list)
    {
        sequencepass = seq_list;
        //for (auto& pass : modulepasses) {
        //    auto find = std::find(sequencepass.begin(), sequencepass.end(), pass.first);
        //    if (find == sequencepass.end())
        //        sequencepass.emplace_back(pass.first);
        //}
        return;
    }

    FunctionPassManager::FunctionPassManager()
    {
        FPMI = std::make_unique<FunctionPassManagerImpl>();
    }

    bool FunctionPassManager::Add(AnalysisID ID, std::unique_ptr<Pass> P)
    {
        if (!ID || !P) return false;
        if (!FPMI) return false;
        return FPMI->Add(ID, std::move(P));
    }

    bool FunctionPassManager::Run(std::shared_ptr<Function> F)
    {
        if (!FPMI) return false;
        return FPMI->Run(F);
    }

    void FunctionPassManager::sequence(const std::vector<AnalysisID>& seq_list) 
    { 
        FPMI->sequence(seq_list); 
    }

    bool FunctionPassManagerImpl::Add(AnalysisID ID, std::unique_ptr<Pass> P)
    {
        if (P->getPassType() != Pass::pass_function)
            return false;
        auto success = functionpasses.try_emplace(ID, std::move(P));
        return success.second;
    }

    bool FunctionPassManagerImpl::Run(std::shared_ptr<Function> F)
    {
        bool Changed = true;

        for (auto& id : sequencepass) {
            auto& pass = functionpasses.find(id);
            if (pass == functionpasses.end()) continue;
            if (!pass->second) continue;
            auto info = GetPassRegistry().getPassInfo(id);
            if (!info || info->getRegHandle() != PassInfo::passreg_pre) continue;
            Changed &= reinterpret_cast<FunctionPass*>(pass->second.get())->runOnFunction(F);
        }

        if (Changed) {
            for (auto& bb : *F) {
                GetPassRegistry().run(bb.second);
            }
            for (auto& id : sequencepass) {
                auto& pass = functionpasses.find(id);
                if (pass == functionpasses.end()) continue;
                if (!pass->second) continue;
                auto info = GetPassRegistry().getPassInfo(id);
                if (!info || info->getRegHandle() != PassInfo::passreg_post) continue;
                reinterpret_cast<FunctionPass*>(pass->second.get())->runOnFunction(F);
            }
        }

        return Changed;
    }

    void FunctionPassManagerImpl::sequence(const std::vector<AnalysisID>& seq_list)
    {
        sequencepass = seq_list;
        //for (auto& pass : functionpasses) {
        //    auto find = std::find(sequencepass.begin(), sequencepass.end(), pass.first);
        //    if (find == sequencepass.end())
        //        sequencepass.emplace_back(pass.first);
        //}
        return;
    }

    BBPassManager::BBPassManager() 
    {
        BPMI = std::make_unique<BBPassManagerImpl>();
    }

    bool BBPassManager::Add(AnalysisID ID, std::unique_ptr<Pass> P)
    {
        if (!ID || !P) return false;
        if (!BPMI) return false;
        return BPMI->Add(ID, std::move(P));
    }
    
    bool BBPassManager::Run(std::shared_ptr<BasicBlock> BB)
    {
        if (!BPMI) return false;
        return BPMI->Run(BB);
    }

    void BBPassManager::sequence(const std::vector<AnalysisID>& seq_list) 
    { 
        BPMI->sequence(seq_list); 
    }

    bool BBPassManagerImpl::Add(AnalysisID ID, std::unique_ptr<Pass> P)
    {
        if (P->getPassType() != Pass::pass_basicblock)
            return false;
        auto success = bbpasses.try_emplace(ID, std::move(P));
        return success.second;
    }

    bool BBPassManagerImpl::Run(std::shared_ptr<BasicBlock> BB)
    {
        bool Changed = true;

        for (auto& id : sequencepass) {
            auto& pass = bbpasses.find(id);
            if (pass == bbpasses.end()) continue;
            if (!pass->second) continue;
            auto info = GetPassRegistry().getPassInfo(id);
            if (!info || info->getRegHandle() != PassInfo::passreg_pre) continue;
            Changed &= reinterpret_cast<BasicBlockPass*>(pass->second.get())->runOnBasicBlock(BB);
        }

        if (Changed) {
            for (auto& insn : *BB) {
                GetPassRegistry().run(insn);
            }
            for (auto& id : sequencepass) {
                auto& pass = bbpasses.find(id);
                if (pass == bbpasses.end()) continue;
                if (!pass->second) continue;
                auto info = GetPassRegistry().getPassInfo(id);
                if (!info || info->getRegHandle() != PassInfo::passreg_post) continue;
                reinterpret_cast<BasicBlockPass*>(pass->second.get())->runOnBasicBlock(BB);
            }
        }

        return Changed;
    }

    void BBPassManagerImpl::sequence(const std::vector<AnalysisID>& seq_list)
    {
        sequencepass = seq_list;
        //for (auto& pass : bbpasses) {
        //    auto find = std::find(sequencepass.begin(), sequencepass.end(), pass.first);
        //    if (find == sequencepass.end())
        //        sequencepass.emplace_back(pass.first);
        //}
        return;
    }

    InsnPassManager::InsnPassManager() 
    {
        IPMI = std::make_unique<InsnPassManagerImpl>();
    }

    bool InsnPassManager::Add(AnalysisID ID, std::unique_ptr<Pass> P)
    {
        if (!ID || !P) return false;
        if (!IPMI) return false;
        return IPMI->Add(ID, std::move(P));
    }

    bool InsnPassManager::Run(std::shared_ptr<CapInsn> insn)
    {
        if (!IPMI) return false;
        return IPMI->Run(insn);
    }

    void InsnPassManager::sequence(const std::vector<AnalysisID>& seq_list) 
    { 
        IPMI->sequence(seq_list); 
    }

    bool InsnPassManagerImpl::Add(AnalysisID ID, std::unique_ptr<Pass> P)
    {
        if (P->getPassType() != Pass::pass_instruction)
            return false;
        auto success = insnpasses.try_emplace(ID, std::move(P));
        return success.second;
    }

    bool InsnPassManagerImpl::Run(std::shared_ptr<CapInsn> insn)
    {
        bool Changed = true;

        for (auto& id : sequencepass) {
            auto& pass = insnpasses.find(id);
            if (pass == insnpasses.end()) continue;
            if (!pass->second) continue;
            auto info = GetPassRegistry().getPassInfo(id);
            if (!info || info->getRegHandle() != PassInfo::passreg_pre) continue;
            Changed &= reinterpret_cast<InstructionPass*>(pass->second.get())->runOnInstruction(insn);
        }

        if (Changed) {
            for (auto& id : sequencepass) {
                auto& pass = insnpasses.find(id);
                if (pass == insnpasses.end()) continue;
                if (!pass->second) continue;
                auto info = GetPassRegistry().getPassInfo(id);
                if (!info || info->getRegHandle() != PassInfo::passreg_post) continue;
                reinterpret_cast<InstructionPass*>(pass->second.get())->runOnInstruction(insn);
            }
        }

        return Changed;
    }

    void InsnPassManagerImpl::sequence(const std::vector<AnalysisID>& seq_list)
    {
        sequencepass = seq_list;
        //for (auto& pass : insnpasses) {
        //    auto find = std::find(sequencepass.begin(), sequencepass.end(), pass.first);
        //    if (find == sequencepass.end())
        //        sequencepass.emplace_back(pass.first);
        //}
        return;
    }
} // namespace cchips
