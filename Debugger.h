#pragma once
#include <Windows.h>
#include <string>
#include <map>
#include <vector>
#include <functional>

namespace cchips {

    class Module;

    class Debugger
    {
    public:
#define DEBUGGER_NOP_INSTRUCTION 0x90
        using bp_type = enum {
            bp_unknown,
            bp_int3,
            bp_long_int3,
            bp_ud2,
            bp_hardware,
            bp_mem_read,
            bp_mem_write,
            bp_mem_execute,
            bp_max,
        };
        using de_type = enum {
            de_unknown,
            de_single_step,
            de_breakpoint,
            de_access_violation,
            de_guard_page_violation,
            de_page_error,
            de_illegal_instruction,
            de_noncontinuable_exception,
            de_array_bounds_exceeded,
            de_float_denormal_operand,
            de_float_divide_byzero,
            de_integer_divide_byzero,
            de_integer_overflow,
            de_privileged_instruction,
            de_heap_corruption,
            de_stack_overrun,
            de_max,
        };

        struct bp_entry {
            mutable bool bactive;
            std::uint64_t address;
            bp_type type;
            size_t res_size;
            unsigned long old_protection;
            std::vector<BYTE> restore_bytes;

            bool operator<(const bp_entry& be) const {
                return address < be.address;
            }
            bool operator>(const bp_entry& be) const {
                return address > be.address;
            }
            bool operator==(const bp_entry& be) const {
                return address == be.address;
            }
        };
        using de_callback = std::function<long(PVOID)>;
        static Debugger& GetInstance() {
            static Debugger m_instance;
            return m_instance;
        }

        bool hideDebugger();
        bool unhideDebugger();
        long Dispatch(struct _EXCEPTION_POINTERS *ep);
        bool setBreakPoint(std::uint64_t address, de_callback callback);
        bool clrBreakPoint(std::uint64_t address);
        bool enableBreakPoint(std::uint64_t address);
        bool disableBreakPoint(std::uint64_t address);
        bool setLongBreakPoint(std::uint64_t address, de_callback callback);
        bool clrLongBreakPoint(std::uint64_t address);
        bool enableLongBreakPoint(std::uint64_t address);
        bool disableLongBreakPoint(std::uint64_t address);
        bool setUd2(std::uint64_t address, de_callback callback);
        bool clrUd2(std::uint64_t address);
        bool enableUd2(std::uint64_t address);
        bool disableUd2(std::uint64_t address);
        bool setSingleStep(std::uint64_t address, de_callback callback);
        bool clrSingleStep(std::uint64_t address);
        bool enableSingleStep(std::uint64_t address);
        bool disableSingleStep(std::uint64_t address);
        bool setMemBpx(std::uint64_t address, de_callback callback);
        bool clrMemBpx(std::uint64_t address);
        bool enableMemBpx(std::uint64_t address);
        bool disableMemBpx(std::uint64_t address);
        bool setCustomHandler(de_type type, de_callback callback);

    private:
        Debugger() {
            Initialize();
        }
        ~Debugger() = default;

        bool Initialize();
        long bp_breakpoint_cb(PVOID data);
        long bp_ud2_cb(PVOID data);

        static BYTE _breakpoint_int3;   // int 3
        static BYTE _breakpoint_long_int3[2]; // int 3
        static BYTE _breakpoint_ud2[2]; // Undefined Instruction, test
        std::map<bp_entry, std::vector<de_callback>> m_bp_list;
        std::map<de_type, std::vector<de_callback>> m_de_list;
    };

#define GetDebugger() Debugger::GetInstance()
} // namespace cchips
