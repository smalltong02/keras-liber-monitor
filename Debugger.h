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

        class Modifier {
        public:
            Modifier() = delete;
            Modifier(PCONTEXT ep) : m_context(ep) {}
            ~Modifier() = default;

            bool isX86() const { 
#ifdef _X86_
                return true; 
#endif
                return false;
            }
            bool isX64() const { 
#ifdef _AMD64_
                return true; 
#endif
                return false;
            }
            bool valid() const {
                if (m_context)
                    return true;
                return false;
            }
            std::uint64_t getXip() const;
            std::uint64_t getXsp() const;
            std::uint64_t getXbp() const;
            std::uint64_t getXdi() const;
            std::uint64_t getXsi() const;
            std::uint64_t getXax() const;
            std::uint64_t getXbx() const;
            std::uint64_t getXcx() const;
            std::uint64_t getXdx() const;
            std::uint32_t getEflags() const;
            std::uint64_t getDr0() const;
            std::uint64_t getDr1() const;
            std::uint64_t getDr2() const;
            std::uint64_t getDr3() const;
            std::uint64_t getDr6() const;
            std::uint64_t getDr7() const;
            std::uint32_t getGs() const;
            std::uint32_t getFs() const;
            std::uint32_t getEs() const;
            std::uint32_t getDs() const;
            std::uint32_t getCs() const;
            std::uint32_t getSs() const;
            // only x64
            std::uint32_t getCFlags() const;
            std::uint32_t getMxCsr() const;
            std::uint64_t getR8() const;
            std::uint64_t getR9() const;
            std::uint64_t getR10() const;
            std::uint64_t getR11() const;
            std::uint64_t getR12() const;
            std::uint64_t getR13() const;
            std::uint64_t getR14() const;
            std::uint64_t getR15() const;
            M128A getXmm0() const;
            M128A getXmm1() const;
            M128A getXmm2() const;
            M128A getXmm3() const;
            M128A getXmm4() const;
            M128A getXmm5() const;
            M128A getXmm6() const;
            M128A getXmm7() const;
            M128A getXmm8() const;
            M128A getXmm9() const;
            M128A getXmm10() const;
            M128A getXmm11() const;
            M128A getXmm12() const;
            M128A getXmm13() const;
            M128A getXmm14() const;
            M128A getXmm15() const;

            bool setXip(std::uint64_t val);
            bool setXsp(std::uint64_t val);
            bool setXbp(std::uint64_t val);
            bool setXdi(std::uint64_t val);
            bool setXsi(std::uint64_t val);
            bool setXax(std::uint64_t val);
            bool setXbx(std::uint64_t val);
            bool setXcx(std::uint64_t val);
            bool setXdx(std::uint64_t val);
            bool setEflags(std::uint32_t val);
            bool setDr0(std::uint64_t val);
            bool setDr1(std::uint64_t val);
            bool setDr2(std::uint64_t val);
            bool setDr3(std::uint64_t val);
            bool setDr6(std::uint64_t val);
            bool setDr7(std::uint64_t val);
            bool setGs(std::uint32_t val);
            bool setFs(std::uint32_t val);
            bool setEs(std::uint32_t val);
            bool setDs(std::uint32_t val);
            bool setCs(std::uint32_t val);
            bool setSs(std::uint32_t val);
            // only x64
            bool setCFlags(std::uint32_t val);
            bool setMxCsr(std::uint32_t val);
            bool setR8(std::uint64_t val);
            bool setR9(std::uint64_t val);
            bool setR10(std::uint64_t val);
            bool setR11(std::uint64_t val);
            bool setR12(std::uint64_t val);
            bool setR13(std::uint64_t val);
            bool setR14(std::uint64_t val);
            bool setR15(std::uint64_t val);
            bool setXmm0(const M128A& val);
            bool setXmm1(const M128A& val);
            bool setXmm2(const M128A& val);
            bool setXmm3(const M128A& val);
            bool setXmm4(const M128A& val);
            bool setXmm5(const M128A& val);
            bool setXmm6(const M128A& val);
            bool setXmm7(const M128A& val);
            bool setXmm8(const M128A& val);
            bool setXmm9(const M128A& val);
            bool setXmm10(const M128A& val);
            bool setXmm11(const M128A& val);
            bool setXmm12(const M128A& val);
            bool setXmm13(const M128A& val);
            bool setXmm14(const M128A& val);
            bool setXmm15(const M128A& val);

        private:
            PCONTEXT m_context;
        };

        using de_callback = std::function<long(std::shared_ptr<Modifier>)>;
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
        long bp_accessviolation_cb(std::shared_ptr<Modifier> ep);
        long bp_singlestep_cb(std::shared_ptr<Modifier> ep);
        long bp_breakpoint_cb(std::shared_ptr<Modifier> ep);
        long bp_ud2_cb(std::shared_ptr<Modifier> ep);

        static BYTE _breakpoint_int3;   // int 3
        static BYTE _breakpoint_long_int3[2]; // int 3
        static BYTE _breakpoint_ud2[2]; // Undefined Instruction, test
        std::map<bp_entry, std::vector<de_callback>> m_bp_list;
        std::map<de_type, std::vector<de_callback>> m_de_list;
    };

#define GetDebugger() Debugger::GetInstance()
} // namespace cchips
