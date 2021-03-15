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
    std::shared_ptr<Module> pe_module = std::make_shared<Module>();
    ASSERT_NE(pe_module, nullptr);
    pe_module->SetPrecacheAddress(reinterpret_cast<uint8_t*>(0x10000));
    pe_module->Initialize();
    ASSERT_FALSE(pe_module->Valid());
    HMODULE hmodule = GetModuleHandle("kernel32");
    pe_module->SetModuleName(std::string("kernel32.dll"));
    pe_module->SetPrecacheAddress(reinterpret_cast<uint8_t*>(hmodule));
    bret = GetPassRegistry().run(pe_module);
    ASSERT_TRUE(bret);
    ASSERT_TRUE(pe_module->Valid());
#ifdef _X86_
    pe_module->dump("C:\\work\\test1_x86.json");
#endif
#ifdef _AMD64_
    pe_module->dump("C:\\work\\test1_x64.json");
#endif
    HMODULE self_hmod = GetModuleHandle(nullptr);
    std::shared_ptr<Module> self_module = std::make_shared<Module>();
    self_module->SetModuleName(std::string("HipsHookTest.exe"));
    pe_module->SetPrecacheAddress(reinterpret_cast<uint8_t*>(self_hmod));
    bret = GetPassRegistry().run(self_module);
    ASSERT_TRUE(bret);
    ASSERT_TRUE(self_module->Valid());
#ifdef _X86_
    self_module->dump("C:\\work\\test2_x86.json");
#endif
#ifdef _AMD64_
    self_module->dump("C:\\work\\test2_x64.json");
#endif
}
#endif

#ifdef _BENCHMARK_TEST

#endif
