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
        using passinfo_handle = enum {
            passreg_pre,
            passreg_post,
        };
        using NormalCtor_t = std::unique_ptr<Pass> (*)();

        PassInfo(std::string_view name, std::string_view desc, passinfo_handle handle, const void *pi, NormalCtor_t normal)
            : PassName(name), PassDesc(desc), RegHandle(handle), PassID(pi), NormalCtor(normal) {}

        std::string_view getPassName() const { return PassName; }
        std::string_view getPassDesc() const { return PassDesc; }
        AnalysisID getPassID() const { return PassID; }
        std::unique_ptr<Pass> getPass() const { return std::move(NormalCtor()); }
        bool valid() const {
            if (!PassName.length() || !PassID || !NormalCtor)
                return false;
            if (RegHandle != passreg_pre && RegHandle != passreg_post)
                return false;
            return true;
        }
        passinfo_handle getRegHandle() const { return RegHandle; }
    private:
        std::string_view PassName;
        std::string_view PassDesc;
        passinfo_handle RegHandle;
        AnalysisID PassID;
        NormalCtor_t NormalCtor = nullptr;
    };

    class PassRegistry
    {
    public:
        static PassRegistry& getPassRegistry() {
            static PassRegistry m_instance;
            return m_instance;
        }
        void Initialize();
        const PassInfo* getPassInfo(AnalysisID ID) const;
        const PassInfo* getPassInfo(const std::string& pass_name) const;
        std::unique_ptr<Pass> getPass(AnalysisID ID) const;
        std::unique_ptr<Pass> getPass(const std::string& pass_name) const;
        void registerPass(PassInfo &PI);
        std::shared_ptr<PassManagerBase> getPassManager(Pass::passmanager_type type) const;
        bool sequence(std::vector<std::string>& sequence_list);
        bool run(std::shared_ptr<Module> PM);
        bool run(std::shared_ptr<Function> PF);
        bool run(std::shared_ptr<BasicBlock> PB);
        bool run(std::shared_ptr<CapInsn> PC);
        AnalysisID GetPassID(const std::string& pass_name);

        static std::vector<std::string> sequence_passes_define;
    private:
        PassRegistry() {
            Initialize();
        }
        ~PassRegistry() = default;
        std::vector<PassInfo> passinfos;
        std::vector<std::shared_ptr<PassManagerBase>> passmanagerbases;
    };

    template <typename PassName> std::unique_ptr<Pass> callDefaultCtor() { 
        return std::make_unique<PassName>(); 
    }

    template <typename passName> struct RegisterPass : PassInfo {
        // Register Pass using default constructor...
        RegisterPass(std::string_view Name, std::string_view PassDesc, PassInfo::passinfo_handle handle)
            : PassInfo(Name, PassDesc, handle, &passName::ID,
                PassInfo::NormalCtor_t(callDefaultCtor<passName>)) {
            PassRegistry::getPassRegistry().registerPass(*this);
        }
    };

#define GetPassRegistry() PassRegistry::getPassRegistry()
} // namespace cchips
