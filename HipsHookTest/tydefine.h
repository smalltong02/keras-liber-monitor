#pragma once

typedef enum _PROCESSINFOCLASS {
    ProcessBasicInformation = 0,
    ProcessDebugPort = 7,
    ProcessWow64Information = 26,
    ProcessImageFileName = 27,
    ProcessBreakOnTermination = 29
} PROCESSINFOCLASS;

typedef struct _PROCESS_BASIC_INFORMATION {
    PVOID Reserved1;
    PVOID PebBaseAddress;
    PVOID Reserved2[2];
    ULONG_PTR UniqueProcessId;
    PVOID Reserved3;
} PROCESS_BASIC_INFORMATION;
typedef PROCESS_BASIC_INFORMATION *PPROCESS_BASIC_INFORMATION;

using NtQueryInformationProcess_Define = NTSTATUS(WINAPI*)(HANDLE ProcessHandle, PROCESSINFOCLASS ProcessInformationClass, PVOID ProcessInformation, ULONG ProcessInformationLength, PULONG ReturnLength);
using NtQueryLicenseValue_Define = NTSTATUS(WINAPI*)(PVOID ValueName, PULONG Type, PVOID Data, ULONG DataSize, PULONG ResultDataSize);
using RtlAllocateHeap_Define = PVOID(WINAPI*)(PVOID HeapHandle, ULONG Flags, SIZE_T Size);
using RtlFreeHeap_Define = BOOL(WINAPI*)(PVOID HeapHandle, ULONG Flags, PVOID BaseAddress);
using RtlDestroyHeap_Define = PVOID(WINAPI*)(PVOID HeapHandle);
