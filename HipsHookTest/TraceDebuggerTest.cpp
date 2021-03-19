#include "stdafx.h"
#include <stdio.h>
#include <windows.h>
#include <string>
#include <vector>
#include <functional>
#include "HipsHookTest.h"
#include "benchmark\benchmark.h"
#include "gtest\gtest.h"
#include "gmock\gmock.h"
#include "PassStructure.h"
#include "PassSupport.h"
#include "Debugger.h"

#ifdef _FUNCTION_TEST
class TraceDebuggerTest : public testing::Test
{
protected:
    TraceDebuggerTest() {}
    ~TraceDebuggerTest() override {}

    virtual void SetUp() override {
    }

    void TearDown() override {
    }
    std::shared_ptr<Module> GetPeModule() { return pe_module; }

private:
    std::shared_ptr<Module> pe_module = std::make_shared<Module>();
};

TEST_F(TraceDebuggerTest, Debugger_Test)
{
    auto md_callback = [&](PVOID data) ->long {
        struct _EXCEPTION_POINTERS *ep = reinterpret_cast<struct _EXCEPTION_POINTERS*>(data);
#ifdef _X86_
        std::cout << "md_callback modify edi from " << std::hex << ep->ContextRecord->Edi << " to 2." << std::endl;
        ep->ContextRecord->Edi = 2;
#endif
#ifdef _AMD64_
        std::cout << "md_callback modify rbx from " << std::hex << ep->ContextRecord->Rbx << " to 2." << std::endl;
        ep->ContextRecord->Rbx = 2;
#endif
        return 0;
    };

    auto bp_callback = [&](PVOID data) ->long{
        struct _EXCEPTION_POINTERS *ep = reinterpret_cast<struct _EXCEPTION_POINTERS*>(data);
#ifdef _X86_
        static BYTE cmp_bytes[] = { 0x0f, 0x1f, 0x44, 0x00, 0x00 };
        std::uint64_t address = (std::uint64_t)ep->ContextRecord->Eip;
#endif
#ifdef _AMD64_
        static BYTE cmp_bytes[] = { 0x48, 0xbf, 0xb3, 0x94, 0xd6, 0x26, 0xe8, 0x0b, 0x2e, 0x11 };
        std::uint64_t address = (std::uint64_t)ep->ContextRecord->Rip;
#endif
        std::cout << "breakpoint address = 0x" << std::hex << address << std::endl;
        std::shared_ptr<Function> function = GetPeModule()->GetFunction(reinterpret_cast<std::uint8_t*>(address));
        if (!function) return 0;
        for (auto& block : *function) {
            if (!block.second) break;
            std::cout << "  block = " << block.second->getBlockNo() << std::endl;
            for (auto& insn : *block.second) {
                if (!insn) break;
                std::cout << "    0x" << std::hex << insn->address() << ": " << insn->dump() << "    " << insn->dumpbytes() << std::endl;
                if (insn->getBytes().size() == sizeof(cmp_bytes)) {
                    if (memcmp(&insn->getBytes()[0], cmp_bytes, sizeof(cmp_bytes)) == 0) {
                        bool bsuccess = GetDebugger().setBreakPoint(insn->address(), md_callback);
                        std::cout << "set md_callback: " << bsuccess << std::endl;
                    }
                }
            }
        }
        std::cout << "bp_callback exit!" << std::endl;
        return 0;
    };
    void(*ptr) () = []() {
        int i = 5;
        while (i) {
            printf("test number is %d\n", i);
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            i--;
        }
    };
    
    GetPeModule()->SetModuleName(std::string("HipsHookTest.exe"));
    bool bret = GetPassRegistry().run(GetPeModule());
    ASSERT_TRUE(bret);
    bret = GetPeModule()->AddFunction(std::string(""), reinterpret_cast<std::uint8_t*>(ptr));
    ASSERT_TRUE(bret);
    std::shared_ptr<Function> function = GetPeModule()->GetFunction(reinterpret_cast<std::uint8_t*>(ptr));
    ASSERT_NE(function, nullptr);
    std::shared_ptr<FunctionPassManager> funcs_manager = std::static_pointer_cast<FunctionPassManager>(GetPassRegistry().getPassManager(Pass::passmanager_function));
    ASSERT_NE(funcs_manager, nullptr);
    bret = funcs_manager->Run(function);
    ASSERT_TRUE(bret);
    Debugger::de_callback step_callback(bp_callback);
    bool bsuccess = GetDebugger().setBreakPoint(reinterpret_cast<std::uint64_t>(ptr), bp_callback);
    std::thread thread(
        ptr);
    ASSERT_TRUE(bsuccess);
    thread.join();
}

#endif

#ifdef _BENCHMARK_TEST

#endif
