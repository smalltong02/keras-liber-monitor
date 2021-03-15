#pragma once
#include <Windows.h>

namespace cchips {

    class Debugger
    {
    public:
        Debugger() = default;
        ~Debugger() = default;

        bool hideDebugger() {}
        bool unhideDebugger() {}
        bool setSingleStep(unsigned long long address) {}
        bool clrSingleStep(unsigned long long address) {}
        bool enableSingleStep(unsigned long long address) {}
        bool disableSingleStep(unsigned long long address) {}
        bool setLongSingleStep(unsigned long long address) {}
        bool clrLongSingleStep(unsigned long long address) {}
        bool enableLongSingleStep(unsigned long long address) {}
        bool disableLongSingleStep(unsigned long long address) {}
        bool setUd2(unsigned long long address) {}
        bool clrUd2(unsigned long long address) {}
        bool enableUd2(unsigned long long address) {}
        bool disableUd2(unsigned long long address) {}
        bool setAutoBpx(unsigned long long address) {}
        bool clrAutoBpx(unsigned long long address) {}
        bool enableAutoBpx(unsigned long long address) {}
        bool disableAutoBpx(unsigned long long address) {}
        bool setMemBpx(unsigned long long address) {}
        bool clrMemBpx(unsigned long long address) {}
        bool enableMemBpx(unsigned long long address) {}
        bool disableMemBpx(unsigned long long address) {}

    private:

        static BYTE _breakpoint_int3;   // int 3
        static BYTE _breakpoint_long_int3[2]; // int 3
        static BYTE _breakpoint_ud2[2]; // Undefined Instruction, test
    };

} // namespace cchips
