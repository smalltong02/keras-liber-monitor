#include "Debugger.h"

namespace cchips {

    BYTE Debugger::_breakpoint_int3 = 0xCC;
    BYTE Debugger::_breakpoint_long_int3[2] = { 0xCD, 0x03 };
    BYTE Debugger::_breakpoint_ud2[2] = { 0 };

} // namespace cchips
