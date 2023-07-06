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

    virtual void SetUp() override {}

    void TearDown() override {
    }
    static std::shared_ptr<Module> GetPeModule() { return pe_module; }
    std::unique_ptr<std::function<long(std::shared_ptr<Debugger::Modifier>)>> m_md_cb;

    static long md_callback(std::shared_ptr<Debugger::Modifier> ep);
    static long bp_callback(std::shared_ptr<Debugger::Modifier> ep);
    static long trace_callback(std::shared_ptr<Debugger::Modifier> ep);
    static long end_insn_callback(std::shared_ptr<Debugger::Modifier> ep);
    static long crash_callback(std::shared_ptr<Debugger::Modifier> ep);
    static long m_test_value;
private:
    static std::shared_ptr<Module> pe_module;
};

long TraceDebuggerTest::m_test_value = 0;
std::shared_ptr<Module> TraceDebuggerTest::pe_module = std::make_shared<Module>();

long TraceDebuggerTest::end_insn_callback(std::shared_ptr<Debugger::Modifier> ep)
{
    if (!ep) return 0;
    std::uint64_t address = ep->getXip();
    std::shared_ptr<BasicBlock> block = nullptr;
    for (auto& func : *GetPeModule()) {
        if (!func.second) continue;
        block = func.second->getBasicBlockAtAddress(reinterpret_cast<std::uint8_t*>(address));
        if (block) {
            break;
        }
    }
    if (!block) return 0;
    auto last_insn = block->getEndInsn();
    if (address != last_insn->address()) {
        std::cout << "  error end insn address: " << std::hex << address << " , " << last_insn->address() << std::endl;
    }
    if (GetCapstoneImplment().InJmpGroup(*last_insn)) {
        std::uint8_t* next_addr = nullptr;
        std::uint8_t* jmp_addr = nullptr;
        if (x86_op_type op_type; GetCapstoneImplment().GetJmpAddress(*last_insn, next_addr, jmp_addr, op_type)) {
            if (!last_insn->self()) return false;
            if (!last_insn->self()->detail) return false;
            cs_detail* detail = last_insn->self()->detail;
            cs_x86* x86 = &last_insn->self()->detail->x86;
            //std::cout << "    0x" << std::hex << last_insn->address() << ": " << last_insn->dump() << std::endl;
            printf("    0x%llx: %s    \n", last_insn->address(), last_insn->dump().c_str());
            if (GetCapstoneImplment().InCondBranchGroup(*last_insn)) {
                std::uint64_t eflags = GetCapstoneImplment().GetREflags(*last_insn);
                //std::cout << "       eflags: " << std::dec << eflags << "  : " << ep->getEflags() << std::endl;
                printf("       eflags: %lld : %d\n", eflags, ep->getEflags());
                bool bNE = GetCapstoneImplment().IsCcNE(*last_insn);
                bool bFlag = (std::uint32_t)eflags & ep->getEflags();
                if ((bNE && !bFlag) ||
                    (!bNE && bFlag)) {
                    //std::cout << "       jmpto: " << std::hex << reinterpret_cast<std::uint64_t>(jmp_addr) << std::endl;
                    printf("       jmpto: 0x%llx\n", reinterpret_cast<std::uint64_t>(jmp_addr));
                }
                else {
                    //std::cout << "       jmpto: " << std::hex << reinterpret_cast<std::uint64_t>(next_addr) << std::endl;
                    printf("       jmpto: 0x%llx\n", reinterpret_cast<std::uint64_t>(next_addr));
                }
            }
            else if (GetCapstoneImplment().InBranchGroup(*last_insn)) {
                //std::cout << "       jmpto: " << std::hex << reinterpret_cast<std::uint64_t>(jmp_addr) << std::endl;
                printf("       jmpto: 0x%llx\n", reinterpret_cast<std::uint64_t>(jmp_addr));
            }
        }
    }
    else {
        //std::cout << "    0x" << std::hex << last_insn->address() << ": " << last_insn->dump() << "    " << std::endl;
        printf("    0x%llx: %s    \n", last_insn->address(), last_insn->dump().c_str());
    }
    return 0;
}

long TraceDebuggerTest::crash_callback(std::shared_ptr<Debugger::Modifier> ep)
{
    return 0;
}

long TraceDebuggerTest::trace_callback(std::shared_ptr<Debugger::Modifier> ep)
{
    if (!ep) return 0;
    std::uint64_t address = ep->getXip();
    std::shared_ptr<BasicBlock> block = nullptr;
    for (auto& func : *GetPeModule()) {
        if (!func.second) continue;
        block = func.second->getBasicBlockAtAddress(reinterpret_cast<std::uint8_t*>(address));
        if (block) {
            break;
        } 
    }
    if (!block) return 0;
    //std::cout << "trace_callback:" << std::hex << (UINT64)block->getAddress() << std::endl;
    //std::cout << "  block = " << block->getBlockNo() << std::endl;
    printf("trace_callback: %llx\n", block->getAddress());
    printf("  block = %d\n", block->getBlockNo());
    for (int count = 0; count < block->size() - 1; count++) {
        auto insn = block->getInsn(count);
        if (!insn) return 0;
        //std::cout << "    0x" << std::hex << insn->address() << ": " << insn->dump() << "    " << std::endl;
        printf("    0x%llx: %s    \n", insn->address(), insn->dump().c_str());
    }
    auto last_insn = block->getEndInsn();
    if (!last_insn) return 0;
    if (block->GetBlockType() == BasicBlock::block_end) {
        //std::cout << "    0x" << std::hex << last_insn->address() << ": " << last_insn->dump() << "    " << std::endl;
        printf("    0x%llx: %s    \n", last_insn->address(), last_insn->dump().c_str());
        return 0;
    }
    if (address == last_insn->address()) {
        end_insn_callback(ep);
    }
    bool bsuccess = GetDebugger().setBreakPoint(last_insn->address(), end_insn_callback);

    auto next_block = block->GetNextBlock();
    if (next_block)
        bsuccess = GetDebugger().setBreakPoint(next_block->GetBaseAddress(), trace_callback);
    auto branch_block = block->GetBranchBlock();
    if (branch_block)
        bsuccess = GetDebugger().setBreakPoint(branch_block->GetBaseAddress(), trace_callback);
    return 0;
}

long TraceDebuggerTest::md_callback(std::shared_ptr<Debugger::Modifier> ep)
{
    if (!ep) return 0;
    std::uint64_t address = ep->getXip();
    std::shared_ptr<Function> function = nullptr;
    for (auto& func : *GetPeModule()) {
        if (!func.second) continue;
        auto block = func.second->getBasicBlockAtAddress(reinterpret_cast<std::uint8_t*>(address));
        if (block) {
            function = func.second;
            break;
        }
    }
    if (!function) return 0;
    if (function->getLoops().size() != 1)
        return 0;
    auto loop = function->getLoops()[0];
    if (!loop) return 0;
    if (loop->getLoopType() != Loop::loop_simple)
        return 0;
    std::shared_ptr<SimpleLoop> s_loop = std::static_pointer_cast<SimpleLoop>(loop);
    std::cout << s_loop->getInvariantName() << " setInvariant: " << m_test_value << std::endl;
    s_loop->setInvariant(ep, m_test_value);
    return 0;
};

long TraceDebuggerTest::bp_callback(std::shared_ptr<Debugger::Modifier> ep) {
    if (!ep) return 0;
    std::uint64_t address = ep->getXip();
    if (!address) return 0;
    //std::cout << "breakpoint address = 0x" << std::hex << address << std::endl;
    std::shared_ptr<Function> function = GetPeModule()->GetFunction(reinterpret_cast<std::uint8_t*>(address));
    if (!function) return 0;
    //for (auto& block : *function) {
    //    if (!block.second) break;
    //    std::cout << "  block = " << block.second->getBlockNo() << std::endl;
    //    for (auto& insn : *block.second) {
    //        if (!insn) break;
    //        std::cout << "    0x" << std::hex << insn->address() << ": " << insn->dump() << "    " << insn->dumpbytes() << std::endl;
    //    }
    //}
    if (function->getLoops().size() != 1)
        return 0;
    auto loop = function->getLoops()[0];
    if (!loop) return 0;
    if (loop->getLoopType() != Loop::loop_simple)
        return 0;
    std::uint64_t loop_address = loop->GetBaseAddress();
    if (!loop_address) return 0;
    bool bsuccess = GetDebugger().setBreakPoint(loop_address, md_callback);
    //std::cout << "set md_callback: " << bsuccess << "  addr: " << std::hex << loop_address << std::endl;
    //std::cout << "bp_callback exit!" << std::endl;
    return 0;
};

//TEST_F(TraceDebuggerTest, Debugger_SimpleLoop1_Test)
//{
//    void(*ptr) () = []() {
//        int i = 100;
//        while (i) {
//            std::this_thread::sleep_for(std::chrono::milliseconds(1));
//            i--;
//            m_test_value++;
//        }
//    };
//    m_test_value = 5;
//    GetPassRegistry().sequence(GetPassRegistry().sequence_passes_define);
//    if (!GetPeModule()->Valid()) {
//        GetPeModule()->SetModuleName(std::string("HipsHookTest.exe"));
//        ASSERT_TRUE(GetPassRegistry().run(GetPeModule()));
//    }
//    //std::cout << "HipsHookTest.exe: insns(" << std::dec << GetPeModule()->GetInsnCounts() << ")" << std::endl;
//    bool bret = GetPeModule()->AddFunction(std::string(""), reinterpret_cast<std::uint8_t*>(ptr));
//    ASSERT_TRUE(bret);
//    std::shared_ptr<Function> function = GetPeModule()->GetFunction(reinterpret_cast<std::uint8_t*>(ptr));
//    ASSERT_NE(function, nullptr);
//    std::shared_ptr<FunctionPassManager> funcs_manager = std::static_pointer_cast<FunctionPassManager>(GetPassRegistry().getPassManager(Pass::passmanager_function));
//    ASSERT_NE(funcs_manager, nullptr);
//    bret = funcs_manager->Run(function);
//    ASSERT_TRUE(bret);
//    bool bsuccess = GetDebugger().setBreakPoint(reinterpret_cast<std::uint64_t>(ptr), bp_callback);
//    ASSERT_TRUE(bsuccess);
//    std::thread thread(
//        ptr);
//    thread.join();
//    ASSERT_EQ(m_test_value, 10);
//}
//
//TEST_F(TraceDebuggerTest, Debugger_SimpleLoop2_Test)
//{
//    void(*ptr) () = []() {
//        for (int count = 0; count < 10; count++) {
//            std::this_thread::sleep_for(std::chrono::milliseconds(1));
//            m_test_value++;
//        }
//    };
//    m_test_value = 5;
//    GetPassRegistry().sequence(GetPassRegistry().sequence_passes_define);
//    if (!GetPeModule()->Valid()) {
//        GetPeModule()->SetModuleName(std::string("HipsHookTest.exe"));
//        ASSERT_TRUE(GetPassRegistry().run(GetPeModule()));
//    }
//    ASSERT_TRUE(GetPeModule()->Valid());
//    //std::cout << "HipsHookTest.exe: insns(" << std::dec << GetPeModule()->GetInsnCounts() << ")" << std::endl;
//    bool bret = GetPeModule()->AddFunction(std::string(""), reinterpret_cast<std::uint8_t*>(ptr));
//    ASSERT_TRUE(bret);
//    std::shared_ptr<Function> function = GetPeModule()->GetFunction(reinterpret_cast<std::uint8_t*>(ptr));
//    ASSERT_NE(function, nullptr);
//    std::shared_ptr<FunctionPassManager> funcs_manager = std::static_pointer_cast<FunctionPassManager>(GetPassRegistry().getPassManager(Pass::passmanager_function));
//    ASSERT_NE(funcs_manager, nullptr);
//    bret = funcs_manager->Run(function);
//    ASSERT_TRUE(bret);
//    bool bsuccess = GetDebugger().setBreakPoint(reinterpret_cast<std::uint64_t>(ptr), bp_callback);
//    ASSERT_TRUE(bsuccess);
//    std::thread thread(
//        ptr);
//    thread.join();
//    ASSERT_EQ(m_test_value, 10);
//}
//
//TEST_F(TraceDebuggerTest, Debugger_SimpleLoop3_Test)
//{
//    void(*ptr) () = []() {
//        int count = m_test_value + 5;
//        for (int i = 0; i < count; i++) {
//            std::this_thread::sleep_for(std::chrono::milliseconds(1));
//            m_test_value++;
//        }
//    };
//    m_test_value = 5;
//    GetPassRegistry().sequence(GetPassRegistry().sequence_passes_define);
//    if (!GetPeModule()->Valid()) {
//        GetPeModule()->SetModuleName(std::string("HipsHookTest.exe"));
//        ASSERT_TRUE(GetPassRegistry().run(GetPeModule()));
//    }
//    ASSERT_TRUE(GetPeModule()->Valid());
//    //std::cout << "HipsHookTest.exe: insns(" << std::dec << GetPeModule()->GetInsnCounts() << ")" << std::endl;
//    bool bret = GetPeModule()->AddFunction(std::string(""), reinterpret_cast<std::uint8_t*>(ptr));
//    ASSERT_TRUE(bret);
//    std::shared_ptr<Function> function = GetPeModule()->GetFunction(reinterpret_cast<std::uint8_t*>(ptr));
//    ASSERT_NE(function, nullptr);
//    std::shared_ptr<FunctionPassManager> funcs_manager = std::static_pointer_cast<FunctionPassManager>(GetPassRegistry().getPassManager(Pass::passmanager_function));
//    ASSERT_NE(funcs_manager, nullptr);
//    bret = funcs_manager->Run(function);
//    ASSERT_TRUE(bret);
//    bool bsuccess = GetDebugger().setBreakPoint(reinterpret_cast<std::uint64_t>(ptr), bp_callback);
//    ASSERT_TRUE(bsuccess);
//    std::thread thread(
//        ptr);
//    thread.join();
//    ASSERT_EQ(m_test_value, 10);
//}
//
//TEST_F(TraceDebuggerTest, Debugger_SimpleLoop4_Test)
//{
//    void(*ptr) () = []() {
//        int count = m_test_value + 5;
//        do {
//            std::this_thread::sleep_for(std::chrono::milliseconds(1));
//            count--;
//            m_test_value++;
//        } while (count);
//    };
//    m_test_value = 5;
//    GetPassRegistry().sequence(GetPassRegistry().sequence_passes_define);
//    if (!GetPeModule()->Valid()) {
//        GetPeModule()->SetModuleName(std::string("HipsHookTest.exe"));
//        ASSERT_TRUE(GetPassRegistry().run(GetPeModule()));
//    }
//    ASSERT_TRUE(GetPeModule()->Valid());
//    //std::cout << "HipsHookTest.exe: insns(" << std::dec << GetPeModule()->GetInsnCounts() << ")" << std::endl;
//    bool bret = GetPeModule()->AddFunction(std::string(""), reinterpret_cast<std::uint8_t*>(ptr));
//    ASSERT_TRUE(bret);
//    std::shared_ptr<Function> function = GetPeModule()->GetFunction(reinterpret_cast<std::uint8_t*>(ptr));
//    ASSERT_NE(function, nullptr);
//    std::shared_ptr<FunctionPassManager> funcs_manager = std::static_pointer_cast<FunctionPassManager>(GetPassRegistry().getPassManager(Pass::passmanager_function));
//    ASSERT_NE(funcs_manager, nullptr);
//    bret = funcs_manager->Run(function);
//    ASSERT_TRUE(bret);
//    bool bsuccess = GetDebugger().setBreakPoint(reinterpret_cast<std::uint64_t>(ptr), bp_callback);
//    ASSERT_TRUE(bsuccess);
//    std::thread thread(
//        ptr);
//    thread.join();
//    ASSERT_EQ(m_test_value, 10);
//}
//
//TEST_F(TraceDebuggerTest, Debugger_SimpleLoop5_Test)
//{
//    void(*ptr) () = []() {
//        char count = 100;
//        do {
//            std::this_thread::sleep_for(std::chrono::milliseconds(1));
//            count--;
//            m_test_value++;
//        } while (count);
//    };
//    m_test_value = 5;
//    GetPassRegistry().sequence(GetPassRegistry().sequence_passes_define);
//    if (!GetPeModule()->Valid()) {
//        GetPeModule()->SetModuleName(std::string("HipsHookTest.exe"));
//        ASSERT_TRUE(GetPassRegistry().run(GetPeModule()));
//    }
//    ASSERT_TRUE(GetPeModule()->Valid());
//    //std::cout << "HipsHookTest.exe: insns(" << std::dec << GetPeModule()->GetInsnCounts() << ")" << std::endl;
//    bool bret = GetPeModule()->AddFunction(std::string(""), reinterpret_cast<std::uint8_t*>(ptr));
//    ASSERT_TRUE(bret);
//    std::shared_ptr<Function> function = GetPeModule()->GetFunction(reinterpret_cast<std::uint8_t*>(ptr));
//    ASSERT_NE(function, nullptr);
//    std::shared_ptr<FunctionPassManager> funcs_manager = std::static_pointer_cast<FunctionPassManager>(GetPassRegistry().getPassManager(Pass::passmanager_function));
//    ASSERT_NE(funcs_manager, nullptr);
//    bret = funcs_manager->Run(function);
//    ASSERT_TRUE(bret);
//    bool bsuccess = GetDebugger().setBreakPoint(reinterpret_cast<std::uint64_t>(ptr), bp_callback);
//    ASSERT_TRUE(bsuccess);
//    std::thread thread(
//        ptr);
//    thread.join();
//    //ASSERT_EQ(m_test_value, 10);
//}

TEST_F(TraceDebuggerTest, Debugger_TraceMode1_Test)
{
    void(*ptr) () = []() {
        const auto sta = GetTickCount();
        if (sta % 2) {
            printf("sta %% 2 == 1, sta = %d\n", sta);
        }
        else {
            std::cout << "sta %% 2 == 0, sta = " << std::hex << sta << std::endl;
        }
        return;
    };
    m_test_value = 5;
    std::vector<std::string> sequence_vector = { "ModInit", "FuncInit", "BBInit", "BBCheck", "FuncCheck", "LoopInit","ModCheck" };
    GetPassRegistry().sequence(sequence_vector);
    if (!GetPeModule()->Valid()) {
        GetPeModule()->SetModuleName(std::string("HipsHookTest.exe"));
        ASSERT_TRUE(GetPassRegistry().run(GetPeModule()));
    }
    ASSERT_TRUE(GetPeModule()->Valid());
    //std::cout << "HipsHookTest.exe: insns(" << std::dec << GetPeModule()->GetInsnCounts() << ")" << std::endl;
    bool bret = GetPeModule()->AddFunction(std::string(""), reinterpret_cast<std::uint8_t*>(ptr));
    ASSERT_TRUE(bret);
    std::shared_ptr<Function> function = GetPeModule()->GetFunction(reinterpret_cast<std::uint8_t*>(ptr));
    ASSERT_NE(function, nullptr);
    std::shared_ptr<FunctionPassManager> funcs_manager = std::static_pointer_cast<FunctionPassManager>(GetPassRegistry().getPassManager(Pass::passmanager_function));
    ASSERT_NE(funcs_manager, nullptr);
    bret = funcs_manager->Run(function);
    ASSERT_TRUE(bret);
    for (auto& block : *function) {
        if (!block.second) break;
        std::cout << "  block = " << block.second->getBlockNo() << "  type: " << block.second->getBlockType() << std::endl;
        for (auto& insn : *block.second) {
            if (!insn) break;
            std::cout << "    0x" << std::hex << insn->address() << ": " << insn->dump() << "    " << insn->dumpbytes() << std::endl;
        }
    }
    bool bsuccess = GetDebugger().setBreakPoint(reinterpret_cast<std::uint64_t>(ptr), trace_callback);
    ASSERT_TRUE(bsuccess);
    std::thread thread(
        ptr);
    thread.join();
    ASSERT_EQ(m_test_value, 5);
    std::cout << "Debugger_TraceMode1_Test end!" << std::endl;
}

#endif

#ifdef _BENCHMARK_TEST

#endif
