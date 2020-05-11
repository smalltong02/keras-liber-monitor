#pragma once
#include <stdio.h>
#include <windows.h>
#include <guiddef.h>
#include <WbemCli.h>
#include <WMIUtils.h>
#include <OCIdl.h>
#include <atlbase.h>
#include <winioctl.h>

typedef enum _processing_status {
    processing_continue,
    processing_skip,
}processing_status;
