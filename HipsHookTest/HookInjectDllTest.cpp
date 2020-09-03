#include "stdafx.h"
#include <stdio.h>
#include <windows.h>
#include <string>
#include <vector>
#include <cstdlib>
#include "commutils.h"
#include "HipsCfgObject.h"
#include "HookImplementObject.h"
#include "HipsHookTest.h"
#include "InjectImpl.h"
#include "benchmark\benchmark.h"
#include "gtest\gtest.h"
#include "gmock\gmock.h"

#ifdef _FUNCTION_TEST
class HookInjectDllTest : public testing::Test
{
protected:
    HookInjectDllTest() {
        ;
    }
    ~HookInjectDllTest() override {}

    virtual void SetUp() override {
        m_injector_object = std::make_unique<CDllInjectorObject>();
        ASSERT_TRUE(m_injector_object != nullptr);
        std::unique_ptr<CDllInjectorObject::_InjectDllInfo> p_dll_info = std::make_unique<CDllInjectorObject::_InjectDllInfo>(CDllInjectorObject::inject_apc, CDllInjectorObject::dll_type_x32, "HipsHook.dll");
        ASSERT_TRUE(p_dll_info != nullptr);
        m_injector_object->AddDllInfo(p_dll_info);
        p_dll_info = std::make_unique<CDllInjectorObject::_InjectDllInfo>(CDllInjectorObject::inject_apc, CDllInjectorObject::dll_type_x64, "HipsHook.dll");
        ASSERT_TRUE(p_dll_info != nullptr);
        m_injector_object->AddDllInfo(p_dll_info);
        p_dll_info = std::make_unique<CDllInjectorObject::_InjectDllInfo>(CDllInjectorObject::inject_apc, CDllInjectorObject::dll_type_wow64, "HipsHook.dll");
        ASSERT_TRUE(p_dll_info != nullptr);
        m_injector_object->AddDllInfo(p_dll_info);
        m_injector_object->InitializeSamplePath("ServicesTest.exe");
        g_server_object->EnableDebugPid();
    }

    void TearDown() override {
        g_server_object->DisableDebugPid();
        m_injector_object.reset(nullptr);
    }
    bool CreateSampleProcess(const std::string& process_path, PROCESS_INFORMATION& proc_info) {
        if (!process_path.length()) return false;

        STARTUPINFO si = {};
        DWORD create_flag = CREATE_DEFAULT_ERROR_MODE | NORMAL_PRIORITY_CLASS | CREATE_SUSPENDED;

        memset(&si, 0, sizeof(si));
        memset(&proc_info, 0, sizeof(proc_info));
        si.cb = sizeof(si);
        si.dwFlags = STARTF_FORCEOFFFEEDBACK | STARTF_USESHOWWINDOW;
        si.wShowWindow = SW_SHOWDEFAULT;

        if (!CreateProcess(
            NULL,
            (LPSTR)process_path.c_str(),
            NULL,
            NULL,
            FALSE,
            create_flag,
            NULL,
            NULL,
            &si,
            &proc_info))
        {
            return false;
        }
        return true;
    }
    std::unique_ptr<CDllInjectorObject> m_injector_object = nullptr;
};

#ifdef _X86_
TEST_F(HookInjectDllTest, InjectDllTestX32toX32)
{
    PROCESS_INFORMATION pi = { };
    ASSERT_TRUE(CreateSampleProcess("..\\test\\ijt\\x32\\al-khaser_x86.exe", pi));
    g_server_object->AddDebugPid(pi.dwProcessId);
    ASSERT_TRUE(m_injector_object->InjectProcess(pi.dwProcessId));
    ResumeThread(pi.hThread);
    WaitForSingleObject(pi.hProcess, INFINITE);
    // please used debug version of hipshook.dll, There will be debugging log.
    int count = g_server_object->GetCountForDebugPid(pi.dwProcessId);
    ASSERT_TRUE(count > 0);
    g_server_object->DelDebugPid(pi.dwProcessId);
}

TEST_F(HookInjectDllTest, InjectDllTestX32toX64)
{
    if (!CInjectProcess::Is64BitOS())
    {
        PROCESS_INFORMATION pi = { };
        ASSERT_TRUE(CreateSampleProcess("..\\test\\ijt\\x64\\ServicesTest.exe --normal", pi));
        g_server_object->AddDebugPid(pi.dwProcessId);
        ASSERT_TRUE(m_injector_object->InjectProcess(pi.dwProcessId) == false);
        ResumeThread(pi.hThread);
        WaitForSingleObject(pi.hProcess, INFINITE);
        g_server_object->DelDebugPid(pi.dwProcessId);
    }
    else
    {
        // test use inject32 for X32toX64
        PROCESS_INFORMATION sample_pi = { };
        ASSERT_TRUE(CreateSampleProcess("..\\test\\ijt\\x64\\ServicesTest.exe --normal", sample_pi));
        g_server_object->AddDebugPid(sample_pi.dwProcessId);
        PROCESS_INFORMATION inject_pi = { };
        std::stringstream ss;
        ss << "..\\test\\ijt\\x64\\Inject64.exe --pid " << sample_pi.dwProcessId << " --tid " << sample_pi.dwThreadId << " --dll " << m_injector_object->GetDllPath(CDllInjectorObject::dll_type_x64);
        ASSERT_TRUE(CreateSampleProcess(ss.str(), inject_pi));
        ResumeThread(inject_pi.hThread);
        WaitForSingleObject(inject_pi.hProcess, INFINITE);
        DWORD returnCode = -1;
        ASSERT(GetExitCodeProcess(inject_pi.hProcess, &returnCode));
        ASSERT(returnCode == 0);
        ResumeThread(sample_pi.hThread);
        WaitForSingleObject(sample_pi.hProcess, INFINITE);
        // please used debug version of hipshook.dll, There will be debugging log.
        int count = g_server_object->GetCountForDebugPid(sample_pi.dwProcessId);
        ASSERT_TRUE(count > 0);
        g_server_object->DelDebugPid(sample_pi.dwProcessId);
    }
}

TEST_F(HookInjectDllTest, InjectNotepadTest)
{
    if (!CInjectProcess::Is64BitOS())
    {
        PROCESS_INFORMATION pi = { };
        ASSERT_TRUE(CreateSampleProcess("C:\\windows\\notepad.exe", pi));
        g_server_object->AddDebugPid(pi.dwProcessId);
        ASSERT_TRUE(m_injector_object->InjectProcess(pi.dwProcessId));
        ResumeThread(pi.hThread);
        WaitForSingleObject(pi.hProcess, 5000);
        TerminateProcess(pi.hProcess, 0);
        int count = g_server_object->GetCountForDebugPid(pi.dwProcessId);
        ASSERT_TRUE(count > 0);
        g_server_object->DelDebugPid(pi.dwProcessId);
    }
    else
    {
        PROCESS_INFORMATION sample_pi = { };
        ASSERT_TRUE(CreateSampleProcess("..\\test\\ijt\\x32\\Hardware_Information_v1.exe", sample_pi));
        g_server_object->AddDebugPid(sample_pi.dwProcessId);
        PROCESS_INFORMATION inject_pi = { };
        std::stringstream ss;
        ss << "..\\test\\ijt\\x32\\Inject32.exe --pid " << sample_pi.dwProcessId << " --tid " << sample_pi.dwThreadId << " --dll " << m_injector_object->GetDllPath(CDllInjectorObject::dll_type_x64);
        ASSERT_TRUE(CreateSampleProcess(ss.str(), inject_pi));
        ResumeThread(inject_pi.hThread);
        WaitForSingleObject(inject_pi.hProcess, INFINITE);
        DWORD returnCode = -1;
        ASSERT(GetExitCodeProcess(inject_pi.hProcess, &returnCode));
        ASSERT(returnCode == 0);
        ResumeThread(sample_pi.hThread);
        WaitForSingleObject(sample_pi.hProcess, INFINITE);
        TerminateProcess(sample_pi.hProcess, 0);
        int count = g_server_object->GetCountForDebugPid(sample_pi.dwProcessId);
        ASSERT_TRUE(count > 0);
        g_server_object->DelDebugPid(sample_pi.dwProcessId);
    }
}

TEST_F(HookInjectDllTest, InjectCmdTest)
{
    if (!CInjectProcess::Is64BitOS())
    {
        PROCESS_INFORMATION pi = { };
        ASSERT_TRUE(CreateSampleProcess("C:\\windows\\system32\\cmd.exe", pi));
        g_server_object->AddDebugPid(pi.dwProcessId);
        ASSERT_TRUE(m_injector_object->InjectProcess(pi.dwProcessId));
        ResumeThread(pi.hThread);
        WaitForSingleObject(pi.hProcess, 5000);
        TerminateProcess(pi.hProcess, 0);
        int count = g_server_object->GetCountForDebugPid(pi.dwProcessId);
        ASSERT_TRUE(count > 0);
        g_server_object->DelDebugPid(pi.dwProcessId);
    }
    else
    {
        PROCESS_INFORMATION sample_pi = { };
        ASSERT_TRUE(CreateSampleProcess("C:\\windows\\system32\\cmd.exe", sample_pi));
        g_server_object->AddDebugPid(sample_pi.dwProcessId);
        PROCESS_INFORMATION inject_pi = { };
        std::stringstream ss;
        ss << "..\\test\\ijt\\x64\\Inject64.exe --pid " << sample_pi.dwProcessId << " --tid " << sample_pi.dwThreadId << " --dll " << m_injector_object->GetDllPath(CDllInjectorObject::dll_type_x64);
        ASSERT_TRUE(CreateSampleProcess(ss.str(), inject_pi));
        ResumeThread(inject_pi.hThread);
        WaitForSingleObject(inject_pi.hProcess, INFINITE);
        DWORD returnCode = -1;
        ASSERT(GetExitCodeProcess(inject_pi.hProcess, &returnCode));
        ASSERT(returnCode == 0);
        ResumeThread(sample_pi.hThread);
        WaitForSingleObject(sample_pi.hProcess, 5000);
        TerminateProcess(sample_pi.hProcess, 0);
        int count = g_server_object->GetCountForDebugPid(sample_pi.dwProcessId);
        ASSERT_TRUE(count > 0);
        g_server_object->DelDebugPid(sample_pi.dwProcessId);
    }
}
#endif

#ifdef _AMD64_
TEST_F(HookInjectDllTest, InjectDllTestX64toX32)
{
    // test use inject32 for X32toX64
    PROCESS_INFORMATION sample_pi = { };
    ASSERT_TRUE(CreateSampleProcess("..\\test\\ijt\\x32\\ServicesTest.exe --normal", sample_pi));
    g_server_object->AddDebugPid(sample_pi.dwProcessId);
    PROCESS_INFORMATION inject_pi = { };
    std::stringstream ss;
    ss << "..\\test\\ijt\\x32\\Inject32.exe --pid " << sample_pi.dwProcessId << " --tid " << sample_pi.dwThreadId << " --dll " << m_injector_object->GetDllPath(CDllInjectorObject::dll_type_wow64);
    ASSERT_TRUE(CreateSampleProcess(ss.str(), inject_pi));
    ResumeThread(inject_pi.hThread);
    WaitForSingleObject(inject_pi.hProcess, INFINITE);
    DWORD returnCode = -1;
    ASSERT(GetExitCodeProcess(inject_pi.hProcess, &returnCode));
    ASSERT(returnCode == 0);
    ResumeThread(sample_pi.hThread);
    WaitForSingleObject(sample_pi.hProcess, INFINITE);
    // please used debug version of hipshook.dll, There will be debugging log.
    int count = g_server_object->GetCountForDebugPid(sample_pi.dwProcessId);
    ASSERT_TRUE(count > 0);
    g_server_object->DelDebugPid(sample_pi.dwProcessId);
}

TEST_F(HookInjectDllTest, InjectDllTestX64toX64)
{
    PROCESS_INFORMATION pi = { };
    ASSERT_TRUE(CreateSampleProcess("..\\test\\ijt\\x64\\ServicesTest.exe --normal", pi));
    g_server_object->AddDebugPid(pi.dwProcessId);
    ASSERT_TRUE(m_injector_object->InjectProcess(pi.dwProcessId));
    ResumeThread(pi.hThread);
    WaitForSingleObject(pi.hProcess, INFINITE);
    // please used debug version of hipshook.dll, There will be debugging log.
    int count = g_server_object->GetCountForDebugPid(pi.dwProcessId);
    ASSERT_TRUE(count > 0);
    g_server_object->DelDebugPid(pi.dwProcessId);
}

TEST_F(HookInjectDllTest, InjectNotepadTest)
{
    PROCESS_INFORMATION pi = { };
    ASSERT_TRUE(CreateSampleProcess("C:\\windows\\notepad.exe", pi));
    g_server_object->AddDebugPid(pi.dwProcessId);
    ASSERT_TRUE(m_injector_object->InjectProcess(pi.dwProcessId));
    ResumeThread(pi.hThread);
    WaitForSingleObject(pi.hProcess, 5000);
    TerminateProcess(pi.hProcess, 0);
    int count = g_server_object->GetCountForDebugPid(pi.dwProcessId);
    ASSERT_TRUE(count > 0);
    g_server_object->DelDebugPid(pi.dwProcessId);
}

TEST_F(HookInjectDllTest, InjectCmdTest)
{
    PROCESS_INFORMATION pi = { };
    ASSERT_TRUE(CreateSampleProcess("C:\\windows\\system32\\cmd.exe", pi));
    g_server_object->AddDebugPid(pi.dwProcessId);
    ASSERT_TRUE(m_injector_object->InjectProcess(pi.dwProcessId));
    ResumeThread(pi.hThread);
    WaitForSingleObject(pi.hProcess, 5000);
    TerminateProcess(pi.hProcess, 0);
    int count = g_server_object->GetCountForDebugPid(pi.dwProcessId);
    ASSERT_TRUE(count > 0);
    g_server_object->DelDebugPid(pi.dwProcessId);
}
#endif
#endif
