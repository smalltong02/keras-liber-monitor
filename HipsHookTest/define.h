#pragma once
#include "stdafx.h"
#include <stdio.h>
#include <windows.h>
#include <powerbase.h>
#include <Lm.h>
#include <lmjoin.h>
#include <lmaccess.h>
#include <wininet.h>
#include <windns.h>
#include <iphlpapi.h>
#include <sysinfoapi.h>

typedef struct addrinfo
{
    int                 ai_flags;       // AI_PASSIVE, AI_CANONNAME, AI_NUMERICHOST
    int                 ai_family;      // PF_xxx
    int                 ai_socktype;    // SOCK_xxx
    int                 ai_protocol;    // 0 or IPPROTO_xxx for IPv4 and IPv6
    size_t              ai_addrlen;     // Length of ai_addr
    char *              ai_canonname;   // Canonical name for nodename
    _Field_size_bytes_(ai_addrlen) struct sockaddr *   ai_addr;        // Binary address
    struct addrinfo *   ai_next;        // Next structure in linked list
}
ADDRINFOA, *PADDRINFOA;

typedef struct addrinfoW
{
    int                 ai_flags;       // AI_PASSIVE, AI_CANONNAME, AI_NUMERICHOST
    int                 ai_family;      // PF_xxx
    int                 ai_socktype;    // SOCK_xxx
    int                 ai_protocol;    // 0 or IPPROTO_xxx for IPv4 and IPv6
    size_t              ai_addrlen;     // Length of ai_addr
    PWSTR               ai_canonname;   // Canonical name for nodename
    _Field_size_bytes_(ai_addrlen) struct sockaddr *   ai_addr;        // Binary address
    struct addrinfoW *  ai_next;        // Next structure in linked list
}
ADDRINFOW, *PADDRINFOW;

#ifdef __cplusplus
extern "C" {
#endif

    __declspec(dllimport)
        INT
        __stdcall
        getaddrinfo(
            _In_opt_        PCSTR               pNodeName,
            _In_opt_        PCSTR               pServiceName,
            _In_opt_        const ADDRINFOA *   pHints,
            _Outptr_        PADDRINFOA *        ppResult
        );

    __declspec(dllimport)
        VOID
        __stdcall
        freeaddrinfo(
            _In_opt_        PADDRINFOA      pAddrInfo
        );

    __declspec(dllimport)
        INT
        __stdcall
        GetAddrInfoW(
            _In_opt_        PCWSTR              pNodeName,
            _In_opt_        PCWSTR              pServiceName,
            _In_opt_        const ADDRINFOW *   pHints,
            _Outptr_        PADDRINFOW *        ppResult
        );

    __declspec(dllimport)
        VOID 
        __stdcall
        FreeAddrInfoW(
            _In_opt_        PADDRINFOW       pAddrInfo
    );
#ifdef __cplusplus
}
#endif

