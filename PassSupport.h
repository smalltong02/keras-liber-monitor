#pragma once
#include <Windows.h>
#include <map>
#include <vector>
#include <string>
#include "PassManager.h"
#include "Passes.h"

namespace cchips {

    class Pass;
    class PassManager;

    class PassInfo {
    public:
        using NormalCtor_t = std::unique_ptr<Pass> (*)();

        PassInfo(std::string_view name, std::string_view desc, const void *pi, NormalCtor_t normal)
            : PassName(name), PassDesc(desc), PassID(pi), NormalCtor(normal) {}

        std::string_view getPassName() const { return PassName; }
        std::string_view getPassDesc() const { return PassDesc; }
        AnalysisID getPassID() const { return PassID; }
        std::unique_ptr<Pass> getPass() const { return std::move(NormalCtor()); }
        bool valid() const {
            if (!PassName.length() || !PassID || !NormalCtor)
                return false;
            return true;
        }
        
    private:
        std::string_view PassName;
        std::string_view PassDesc;
        AnalysisID PassID;
        NormalCtor_t NormalCtor = nullptr;
    };

    class PassRegistry
    {
    public:
        PassRegistry() {
            Initialize();
        }
        ~PassRegistry() = default;

        static PassRegistry& getPassRegistry() {
            static PassRegistry m_instance;
            return m_instance;
        }
        void Initialize();
        std::unique_ptr<Pass> getPassInfo(AnalysisID ID) const;
        std::unique_ptr<Pass> getPassInfo(const std::string& pass_name) const;
        void registerPass(PassInfo &PI);
        std::shared_ptr<PassManagerBase> getPassManager(Pass::passmanager_type type) const;
        bool sequence();
        bool run(std::shared_ptr<Module> PM);
        bool run(std::shared_ptr<Function> PF);
        bool run(std::shared_ptr<BasicBlock> PB);
        bool run(std::shared_ptr<CapInsn> PC);
        AnalysisID GetPassID(const std::string& pass_name);
    private:
        std::vector<PassInfo> passinfos;
        std::vector<std::shared_ptr<PassManagerBase>> passmanagerbases;
        static std::vector<std::string> sequence_passes_define;
    };

    template <typename PassName> std::unique_ptr<Pass> callDefaultCtor() { 
        return std::make_unique<PassName>(); 
    }

    template <typename passName> struct RegisterPass : PassInfo {
        // Register Pass using default constructor...
        RegisterPass(std::string_view Name, std::string_view PassDesc)
            : PassInfo(Name, PassDesc, &passName::ID,
                PassInfo::NormalCtor_t(callDefaultCtor<passName>)) {
            PassRegistry::getPassRegistry().registerPass(*this);
        }
    };

#define GetPassRegistry() PassRegistry::getPassRegistry()
} // namespace cchips
