#include "stdafx.h"
#include <stdio.h>
#include <windows.h>
#include <string>
#include <vector>
#include "commutils.h"
#include "HipsCfgObject.h"
#include "HookImplementObject.h"
#include "HipsHookTest.h"
#include "benchmark\benchmark.h"
#include "gtest\gtest.h"
#include "gmock\gmock.h"
#include "PeInfo.h"
#include "PassStructure.h"
#include "PassSupport.h"

#ifdef _FUNCTION_TEST
class HookAnalysisInsnTest : public testing::Test
{
protected:
    HookAnalysisInsnTest() {}
    ~HookAnalysisInsnTest() override {}

    virtual void SetUp() override {
    }

    void TearDown() override {
    }

    BYTE fake_pe_head[0x30] = {
        0x4D, 0x5A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    };
};

// test FileDetector::DetectFileFormat
TEST_F(HookAnalysisInsnTest, FileDetector_DetectFileFormat)
{
    HMODULE hmodule = GetModuleHandle(nullptr);
    ASSERT_TRUE(hmodule);
    FileDetector::file_format self_format = FileDetector::DetectFileFormat(reinterpret_cast<const uint8_t*>(hmodule));
    ASSERT_EQ(self_format, FileDetector::format_pe);
    FileDetector::file_format fake_format = FileDetector::DetectFileFormat(reinterpret_cast<const uint8_t*>(fake_pe_head));
    ASSERT_EQ(fake_format, FileDetector::format_dos);
    FileDetector::file_format except_format = FileDetector::DetectFileFormat(reinterpret_cast<const uint8_t*>(0x1000));
    ASSERT_EQ(except_format, FileDetector::format_undetectable);
}

//  test FileDetector::GetEntryPoint
TEST_F(HookAnalysisInsnTest, FileDetector_GetEntryPoint)
{
    HMODULE hmodule = GetModuleHandle(nullptr);
    ASSERT_TRUE(hmodule);
    const std::uint8_t* ep = FileDetector::GetEntryPoint(reinterpret_cast<const uint8_t*>(hmodule));
    ASSERT_NE(ep, nullptr);
    HMODULE hkernel32 = GetModuleHandle("kernel32.dll");
    ASSERT_TRUE(hkernel32);
    const std::uint8_t* ep_dll = FileDetector::GetEntryPoint(reinterpret_cast<const uint8_t*>(hkernel32));
    ASSERT_NE(ep_dll, nullptr);
    const std::uint8_t* ep_failed = FileDetector::GetEntryPoint(reinterpret_cast<const uint8_t*>(0x1000));
    ASSERT_EQ(ep_failed, nullptr);
}

// test FileDetector::GenerateAsmInstruction
TEST_F(HookAnalysisInsnTest, FileDetector_GetAsmInstruction)
{
    HMODULE hmodule = GetModuleHandle(nullptr);
    ASSERT_TRUE(hmodule);
    const std::uint8_t* ep = FileDetector::GetEntryPoint(reinterpret_cast<const uint8_t*>(hmodule));
    ASSERT_NE(ep, nullptr);
    std::uint8_t* next_insn_addr = const_cast<std::uint8_t*>(ep);
    size_t insn_sum = 1;
    while (std::unique_ptr<CapInsn> cap_insn = FileDetector::GetAsmInstruction(next_insn_addr)) {
        ASSERT_NE(cap_insn->size(), 0);
        //std::cout << "asm_insn("<< insn_sum << "): " << cap_insn->dump() << std::endl;
        insn_sum++;
        next_insn_addr += cap_insn->size();
        if (insn_sum > 100) break;
    }

    next_insn_addr = reinterpret_cast<uint8_t*>(0x1000);
    std::unique_ptr<CapInsn> cap_insn = FileDetector::GetAsmInstruction(next_insn_addr);
    ASSERT_EQ(cap_insn, nullptr);
}

// test class Module
TEST_F(HookAnalysisInsnTest, Module_Class)
{
    bool bret = false;
    GetPassRegistry().sequence(GetPassRegistry().sequence_passes_define);
    std::shared_ptr<Module> krnl_module = std::make_shared<Module>();
    ASSERT_NE(krnl_module, nullptr);
    krnl_module->SetPrecacheAddress(reinterpret_cast<uint8_t*>(0x10000));
    ASSERT_FALSE(krnl_module->Valid());
    HMODULE hmodule = GetModuleHandle("kernel32");
    krnl_module->SetModuleName(std::string("kernel32.dll"));
    krnl_module->SetPrecacheAddress(reinterpret_cast<uint8_t*>(hmodule));
    bret = GetPassRegistry().run(krnl_module);
    ASSERT_TRUE(bret);
    ASSERT_TRUE(krnl_module->Valid());
    //std::cout << "kernel32.dll: insns(" << std::dec << krnl_module->GetInsnCounts() << ")" << std::endl;
#ifdef _X86_
    krnl_module->dump("C:\\work\\kernel32_x86.json");
#endif
#ifdef _AMD64_
    krnl_module->dump("C:\\work\\kernel32_x64.json");
#endif

    std::shared_ptr<Module> user_module = std::make_shared<Module>();
    ASSERT_NE(user_module, nullptr);
    user_module->SetPrecacheAddress(reinterpret_cast<uint8_t*>(0x10000));
    ASSERT_FALSE(user_module->Valid());
    HMODULE user_hmod = GetModuleHandle("user32");
    user_module->SetModuleName(std::string("user32.dll"));
    user_module->SetPrecacheAddress(reinterpret_cast<uint8_t*>(user_hmod));
    bret = GetPassRegistry().run(user_module);
    ASSERT_TRUE(bret);
    ASSERT_TRUE(user_module->Valid());
    //std::cout << "user32.dll: insns(" << std::dec << user_module->GetInsnCounts() << ")" << std::endl;
#ifdef _X86_
    user_module->dump("C:\\work\\user32_x86.json");
#endif
#ifdef _AMD64_
    user_module->dump("C:\\work\\user32_x64.json");
#endif

    std::shared_ptr<Module> advi_module = std::make_shared<Module>();
    ASSERT_NE(advi_module, nullptr);
    advi_module->SetPrecacheAddress(reinterpret_cast<uint8_t*>(0x10000));
    ASSERT_FALSE(advi_module->Valid());
    HMODULE advi_hmod = GetModuleHandle("advapi32");
    advi_module->SetModuleName(std::string("advapi32.dll"));
    advi_module->SetPrecacheAddress(reinterpret_cast<uint8_t*>(advi_hmod));
    bret = GetPassRegistry().run(advi_module);
    ASSERT_TRUE(bret);
    ASSERT_TRUE(advi_module->Valid());
    //std::cout << "advapi32.dll: insns(" << std::dec << std::dec << advi_module->GetInsnCounts() << ")" << std::endl;
#ifdef _X86_
    advi_module->dump("C:\\work\\advapi32_x86.json");
#endif
#ifdef _AMD64_
    advi_module->dump("C:\\work\\advapi32_x64.json");
#endif

    HMODULE self_hmod = GetModuleHandle(nullptr);
    std::shared_ptr<Module> self_module = std::make_shared<Module>();
    self_module->SetModuleName(std::string("HipsHookTest.exe"));
    self_module->SetPrecacheAddress(reinterpret_cast<uint8_t*>(self_hmod));
    bret = GetPassRegistry().run(self_module);
    ASSERT_TRUE(bret);
    ASSERT_TRUE(self_module->Valid());
    //std::cout << "HipsHookTest.exe: insns(" << self_module->GetInsnCounts() << ")" << std::endl;
#ifdef _X86_
    self_module->dump("C:\\work\\HipsHookTest_x86.json");
#endif
#ifdef _AMD64_
    self_module->dump("C:\\work\\HipsHookTest_x64.json");
#endif
}
#endif

#ifdef _BENCHMARK_TEST

#endif
