#include "PassSupport.h"

namespace cchips {

    std::vector<std::string> PassRegistry::sequence_passes_define =
    {
        "ModInit",
        // add Module pass at here.
        "FuncInit",
        // add Function pass at here.
        "BBInit",
        "BBCheck",
        // add Basicblock pass at here.
        // add Instruction pass at here.
        "InsnFree",
    };
} // namespace cchips
