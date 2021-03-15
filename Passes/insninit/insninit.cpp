#include "insninit.h"
#include "PassSupport.h"

namespace cchips {

    char InsnInit::ID = 0;

    static RegisterPass<InsnInit> X(
        "InsnInit",
        "Instruction initializing"
    );

    bool InsnInit::runOnInstruction(std::shared_ptr<CapInsn> Insn)
    {
        if (!Insn) return false;
        return run(Insn);
    }

    bool InsnInit::run(std::shared_ptr<CapInsn>& Insn)
    {
        if (Insn->address() == 0)
            return false;
        if (Insn->size() == 0)
            return false;
        Insn->free_insn();
        return true;
    }
} // namespace cchips
