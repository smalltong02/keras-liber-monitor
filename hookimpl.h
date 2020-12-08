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
    processing_continue,    // call orgin API and call all hook handler.
    processing_skip,  // skip all hook handler but call orgin API.   
    processing_exit,  // skip all hook handler and don't call orgin API.
}processing_status;

