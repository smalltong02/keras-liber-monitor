#pragma once
#include <string>
#include "PassStructure.h"

namespace cchips {

    class PassManagerImpl;
    class FunctionPassManagerImpl;
    using AnalysisID = const void *;

    class Pass 
    {
    public:
        enum pass_type {
            pass_unknown = 0,
            pass_region = 1,
            pass_loop,
            pass_function,
            pass_callgraphscc,
            pass_module,
            pass_passmanager
        };

        enum passmanager_type {
            passmanager_unknown = 0,
            passmanager_modulepassmanager = 1,
            passmanager_callgraphpassmanager,
            passmanager_functionpassmanager,
            passmanager_looppassmanager,
            passmanager_regionpassmanager,
            passmanager_last
        };

        explicit Pass(pass_type type, char &pass_id) : m_pass_id(&pass_id), m_pass_type(type) {}
        Pass(const Pass &) = delete;
        Pass &operator=(const Pass &) = delete;
        virtual ~Pass() { ; }

        pass_type GetPassType() const { return m_pass_type; }
        virtual std::string_view GetPassName() const;
        AnalysisID GetPassID() const { return m_pass_id; }
        virtual bool DoInitialization(Module &) { return false; }
        virtual bool DoFinalization(Module &) { return false; }
    private:
        const void* m_pass_id;
        pass_type m_pass_type;

    };

    class PMDataManager 
    {

    };

    class PassManagerBase 
    {
    public:
        virtual ~PassManagerBase();

        virtual void Add(Pass *P) = 0;
    };

    class PassManager : public PassManagerBase 
    {
    public:

        PassManager();
        ~PassManager() override;

        void Add(Pass *P) override;

        bool Run(Module &M);

    private:
        PassManagerImpl *PM;
    };

    class FunctionPassManager : public PassManagerBase
    {
    public:
        explicit FunctionPassManager(Module *M);
        ~FunctionPassManager() override;

        void Add(Pass *P) override;
        bool Run(Function &F);
        bool DoInitialization();
        bool DoFinalization();

    private:
        FunctionPassManagerImpl *FPM;
        Module *M;
    };
} // namespace cchips

