#include "funcinit.h"
#include "PassSupport.h"

namespace cchips {

    char FuncInit::ID = 0;

    static RegisterPass<FuncInit> X(
        "FuncInit",
        "Function initializing"
    );

    bool FuncInit::runOnFunction(std::shared_ptr<Function> Func)
    {
        if (!Func) return false;
        if (Func->getCurrentBlockNo() != 0)
            return false;
        if (!Func->GetParent())
            return false;
        return run(Func);
    }

    bool FuncInit::run(std::shared_ptr<Function>& Func)
    {
        return Func->Initialize();
    }
} // namespace cchips
