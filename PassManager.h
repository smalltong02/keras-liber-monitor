#pragma once
#include <string>
#include "PassStructure.h"

namespace cchips {

    class PassManagerImpl;
    class FunctionPassManagerImpl;
    class BBPassManagerImpl;
    class InsnPassManagerImpl;
    using AnalysisID = const void *;

    class Pass 
    {
    public:
        enum pass_type {
            pass_unknown = 0,
            pass_region = 1,
            pass_basicblock,
            pass_loop,
            pass_function,
            pass_instruction,
            pass_callgraphscc,
            pass_module,
            pass_passmanager
        };

        enum passmanager_type {
            passmanager_unknown = 0,
            passmanager_module = 1,
            passmanager_callgraph,
            passmanager_function,
            passmanager_loop,
            passmanager_region,
            passmanager_basicblock,
            passmanager_instruction,
            passmanager_last
        };

        Pass() {}
        explicit Pass(pass_type type, char &pass_id) : m_pass_id(&pass_id), m_pass_type(type) {}
        Pass(const Pass &) = delete;
        Pass &operator=(const Pass &) = delete;
        virtual ~Pass() {}

        pass_type getPassType() const { return m_pass_type; }
        virtual std::string_view getPassName() const;
        AnalysisID getPassID() const { return m_pass_id; }
        virtual passmanager_type getPotentialPassManagerType() const { return passmanager_unknown; }
        virtual bool DoInitialization(Module &) { return false; }
        virtual bool DoFinalization(Module &) { return false; }
    private:
        const void* m_pass_id;
        pass_type m_pass_type;

    };

    class ModulePass : public Pass
    {
    public:
        explicit ModulePass(char &pid) : Pass(Pass::pass_module, pid) {}
        ~ModulePass() override {}

        virtual bool runOnModule(std::shared_ptr<Module> M) = 0;

        Pass::passmanager_type getPotentialPassManagerType() const override { return Pass::passmanager_module; }
    private:
    };

    class FunctionPass : public Pass
    {
    public:
        explicit FunctionPass(char &pid) : Pass(Pass::pass_function, pid) {}
        ~FunctionPass() override {}

        virtual bool runOnFunction(std::shared_ptr<Function> Func) = 0;

        Pass::passmanager_type getPotentialPassManagerType() const override { return Pass::passmanager_function; }
    private:
    };

    class BasicBlockPass : public Pass
    {
    public:
        explicit BasicBlockPass(char &pid) : Pass(Pass::pass_basicblock, pid) {}
        ~BasicBlockPass() override {}

        virtual bool runOnBasicBlock(std::shared_ptr<BasicBlock> BB) = 0;

        Pass::passmanager_type getPotentialPassManagerType() const override { return Pass::passmanager_basicblock; }
    private:
    };

    class RegionPass : public Pass
    {
    public:
        explicit RegionPass(char &pid) : Pass(Pass::pass_region, pid) {}
        ~RegionPass() override {}

        virtual bool runOnRegion(/*Region *region*/) = 0;

        Pass::passmanager_type getPotentialPassManagerType() const override { return Pass::passmanager_region; }
    private:
    };

    class LoopPass : public Pass
    {
    public:
        explicit LoopPass(char &pid) : Pass(Pass::pass_loop, pid) {}
        ~LoopPass() override {}

        virtual bool runOnLoop(/*Loop *region*/) = 0;

        Pass::passmanager_type getPotentialPassManagerType() const override { return Pass::passmanager_loop; }
    private:
    };

    class InstructionPass : public Pass
    {
    public:
        explicit InstructionPass(char &pid) : Pass(Pass::pass_instruction, pid) {}
        ~InstructionPass() override {}

        virtual bool runOnInstruction(std::shared_ptr<CapInsn> insn) = 0;

        Pass::passmanager_type getPotentialPassManagerType() const override { return Pass::passmanager_instruction; }
    private:
    };

    class PMDataManager 
    {

    };

    class PassManagerBase 
    {
    public:
        virtual ~PassManagerBase() = default;

        virtual bool Add(AnalysisID ID, std::unique_ptr<Pass> P) = 0;
        virtual Pass::passmanager_type GetPassManagerType() const = 0;
        virtual void sequence(const std::vector<AnalysisID>& seq_list) = 0;
    };

    class PassManager : public PassManagerBase 
    {
    public:

        PassManager();
        ~PassManager() = default;

        bool Add(AnalysisID ID, std::unique_ptr<Pass> P) override;
        Pass::passmanager_type GetPassManagerType() const {
            return Pass::passmanager_module;
        }
        bool Run(std::shared_ptr<Module> M);
        void sequence(const std::vector<AnalysisID>& seq_list);

    private:
        std::unique_ptr<PassManagerImpl> MPMI;
    };

    class FunctionPassManager : public PassManagerBase
    {
    public:
        FunctionPassManager();
        ~FunctionPassManager() = default;

        bool Add(AnalysisID ID, std::unique_ptr<Pass> P) override;
        Pass::passmanager_type GetPassManagerType() const {
            return Pass::passmanager_function;
        }
        bool Run(std::shared_ptr<Function> F);
        void sequence(const std::vector<AnalysisID>& seq_list);

    private:
        std::unique_ptr<FunctionPassManagerImpl> FPMI;
    };

    class BBPassManager : public PassManagerBase
    {
    public:
        BBPassManager();
        ~BBPassManager() = default;

        bool Add(AnalysisID ID, std::unique_ptr<Pass> P) override;
        Pass::passmanager_type GetPassManagerType() const {
            return Pass::passmanager_basicblock;
        }
        bool Run(std::shared_ptr<BasicBlock> BB);
        void sequence(const std::vector<AnalysisID>& seq_list);

    private:
        std::unique_ptr<BBPassManagerImpl> BPMI;
    };

    class RGPassManager : public PassManagerBase
    {
    public:
        RGPassManager() {
            //RPMI = std::make_unique<RGPassManagerImpl>();
        }
        ~RGPassManager() = default;

        bool Add(AnalysisID ID, std::unique_ptr<Pass> P) override { return false; }
        Pass::passmanager_type GetPassManagerType() const {
            return Pass::passmanager_region;
        }
        bool Run(std::shared_ptr<BasicBlock> BB) {}
        void sequence(const std::vector<AnalysisID>& seq_list) { return; }

    private:
        //std::unique_ptr<RGPassManagerImpl> RPMI;
    };

    class LPPassManager : public PassManagerBase
    {
    public:
        LPPassManager() {
            //LPMI = std::make_unique<LPPassManagerImpl>();
        }
        ~LPPassManager() = default;

        bool Add(AnalysisID ID, std::unique_ptr<Pass> P) override { return false; }
        Pass::passmanager_type GetPassManagerType() const {
            return Pass::passmanager_loop;
        }
        bool Run(std::shared_ptr<BasicBlock> BB) {}
        void sequence(const std::vector<AnalysisID>& seq_list) { return; }

    private:
        //std::unique_ptr<LPPassManagerImpl> LPMI;
    };

    class InsnPassManager : public PassManagerBase
    {
    public:
        InsnPassManager();
        ~InsnPassManager() = default;

        bool Add(AnalysisID ID, std::unique_ptr<Pass> P) override;
        Pass::passmanager_type GetPassManagerType() const {
            return Pass::passmanager_instruction;
        }
        bool Run(std::shared_ptr<CapInsn> insn);
        void sequence(const std::vector<AnalysisID>& seq_list);

    private:
        std::unique_ptr<InsnPassManagerImpl> IPMI;
    };
} // namespace cchips

