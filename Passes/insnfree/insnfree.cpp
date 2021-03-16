#include "insnfree.h"
#include "PassSupport.h"

namespace cchips {

    char InsnFree::ID = 0;

    static RegisterPass<InsnFree> X(
        "InsnFree",
        "Instruction free"
    );

    bool InsnFree::runOnInstruction(std::shared_ptr<CapInsn> Insn)
    {
        if (!Insn) return false;
        return run(Insn);
    }

    bool InsnFree::run(std::shared_ptr<CapInsn>& Insn)
    {
        if (Insn->address() == 0)
            return false;
        if (Insn->size() == 0)
            return false;
        Insn->free_insn();
        return true;
    }
} // namespace cchips
