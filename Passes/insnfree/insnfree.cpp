#include "insnfree.h"
#include "PassSupport.h"

namespace cchips {

    char InsnFree::ID = 0;

    static RegisterPass<InsnFree> X(
        "InsnFree",
        "Instruction free",
        PassInfo::passreg_post
    );

    bool InsnFree::runOnFunction(std::shared_ptr<Function> function)
    {
        if (!function) return false;
        return run(function);
    }

    bool InsnFree::run(std::shared_ptr<Function> function)
    {
        for (auto& bb : *function) {
            if (!bb.second) continue;
            for (auto& insn : *bb.second) {
                if (insn->address() == 0)
                    continue;
                if (insn->size() == 0)
                    continue;
                insn->free_insn();
            }
        }
        return true;
    }
} // namespace cchips
