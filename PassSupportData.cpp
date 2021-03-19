#include "PassSupport.h"

namespace cchips {

    std::vector<std::string> PassRegistry::sequence_passes_define =
    {
        "ModInit",
        // add Module pre-pass at here.
        "FuncInit",
        // add Function pre-pass at here.
        "BBInit",
        // add Basicblock pre-pass at here.
        // add Instruction pre-pass at here.
        // add Instruction post-pass at here.
        "InsnFree",
        "BBCheck",
        // add Basicblock post-pass at here.
        "FuncCheck",
        // add Function post-pass at here.
        "ModCheck",
        // add Module post-pass at here.
    };
} // namespace cchips
