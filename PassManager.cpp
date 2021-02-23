#include "PassManager.h"
#include "PassStructure.h"

namespace cchips {

    class PassManagerImpl : public Pass, public PMDataManager
    {
    public:
        static char ID;
        explicit PassManagerImpl() :
            Pass(Pass::pass_passmanager, ID), PMDataManager() {}

        void Add(Pass *P) { ; }
        bool Run(Module &M);
        using Pass::DoInitialization;
        using Pass::DoFinalization;
    };

    char PassManagerImpl::ID = 0;

    class FunctionPassManagerImpl : public Pass, public PMDataManager
    {
    public:
        static char ID;
        explicit FunctionPassManagerImpl() :
            Pass(Pass::pass_passmanager, ID), PMDataManager() {}

        void Add(Pass *P) { ; }
        bool Run(Function &F);
        bool DoInitialization(Module &M) override;
        bool DoFinalization(Module &M) override;
        Pass::passmanager_type GetPassManagerType() const {
            return Pass::passmanager_functionpassmanager;
        }
    };

    char FunctionPassManagerImpl::ID = 0;

    class MPPassManager : public Pass, public PMDataManager
    {
    public:
        static char ID;
        explicit MPPassManager() :
            Pass(Pass::pass_passmanager, ID), PMDataManager() { }

        ~MPPassManager() override {
            for (auto &manager : m_managers) {
                std::shared_ptr<FunctionPassManagerImpl> p = std::move(manager.second);
            }
        }

    private:
        MapVector<std::shared_ptr<Pass>, std::shared_ptr<FunctionPassManagerImpl>> m_managers;
    };
} // namespace cchips
