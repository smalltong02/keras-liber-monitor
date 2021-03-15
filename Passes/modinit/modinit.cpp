#include "modinit.h"
#include "PassSupport.h"

namespace cchips {

    char ModInit::ID = 0;

    static RegisterPass<ModInit> X(
        "ModInit",
        "Module initializing"
    );

    bool ModInit::runOnModule(std::shared_ptr<Module> m)
    {
        if (!m || m->Valid())
            return false;
        _module = m;
        return run();
    }

    bool ModInit::run()
    {
        std::shared_ptr<Module> cur_module = _module.lock();
        if (!cur_module->Initialize())
            return false;
        if (!cur_module->Valid())
            return false; 
        if (cur_module->AddFunction(std::string("entry_point"), cur_module->GetContext()->GetOEP())) {
            return true;
        }
        return false;
    }
} // namespace cchips
