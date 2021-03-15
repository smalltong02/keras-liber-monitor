#include "PassSupport.h"

namespace cchips {

    std::vector<std::string> PassRegistry::sequence_passes_define =
    {
        "ModInit",
        "FuncInit",
        "BBInit",
    };
} // namespace cchips
