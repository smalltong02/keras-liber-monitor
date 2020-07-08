# 2.4 APIs (JSON)

There are currently 3 JSON files defining APIs: **system.json**, **services.json**, and **ole32.json**. This type of JSON files contains all the Win32 API hooks. Later in this documentation, **system.json** will be taken as an example to explain how the JSON files work.

## System.json

A typical API JSON file must have two keys:

* "Category": "System", since all the APIs defined in the **system.json** file belong to System category
* "Calling convention": "WINAPI", the calling convention of all the WIN32 APIs defined in the System JSON file are "WINAPI" (i.e. stdcall). Three types of calling conventions are supported in HipsHook: stdcall, cdecl, and fastcall

### 1. Win32 API

```c++
"LoadLibraryA": {
    "Signature": {
        "Library": "kernel32.dll",
        "Return": "HMODULE",
        "Special": false,
        "Feature": "L1"
    },
    "Parameters": {
        "lpLibFileName": "LPCSTR"
        }
    }
```

Here is some explanation of the keys:
* "LoadLibraryA": name of the API
* "Signature": defines the features of the API
  * "Library": "kernel32.dll", meaning that the API is imported from kernal32.dll
  * "Return": "HMODULE", meaning that the return value type of the API is HMODULE
  * "Special": false, meaning that if the API calls other APIs, those APIs will not be hooked nor recorded
  * "Feature": "L1"，which is the value of "Action" key for TracerLogs
* "Parameters": defines the parameters of the API
  * "lpLibFileName": "LPCSTR", the only parameter of this API is called lpLibFileName, and has type "LPCSTR"

### 2. Ensure

​The value of the _Ensure_ key is an array of parameter names of the API, or some expressions containing the parameter names. If the parameters defined here is NULL (or equivalent values depending on the variable type), or the expression is False, _processing_skip_ will be returned and HipsHook will skip this particular call of the API. Here is an example:

```c++
"Ensure": [
	"lpLibFileName"
],
"PreLog": [
	"lpLibFileName"
]
```

The type of parameter "lpLibFileName" is "LPCSTR", and if it is empty, the "Prelog" key will be ignored as well as other processing steps.

### 3. Prelog Key

The _Prelog_ key defines that before the execution of the original API, which parameters or expessions will be recorded in R3Log. For example:

```c++
"PreLog": [
	"lpLibFileName",
	"dwFlags"
  ]
```

Parameters "lpLibFileName" and "dwFlags" will be recorded into R3Log before calling the "LoadLibraryExA" API. For example, "lpLibFileName" will be recorded as a string, such as "Kernel32.dll", and "dwFlags" will be recorded as a string as well, such as "LOAD_LIBRARY_SEARCH_SYSTEM32 | LOAD_LIBRARY_AS_DATAFILE_EXCLUSIVE".

### 4. Postlog

Postlog defines the parameters or expressions to be recorded after calling the original API, unless the call failed. For example:

```c++
"GetTickCount": {
	"Signature": {
		"Library": "kernel32.dll",
		"Return": "DWORD",
		"Feature": "H12"
	},
	"PostLog": [
		"Return"
	]
}
```

The return value will be recorded in R3Log after calling the original API "GetTickCount".

### 5. PreCheck and PostCheck

The "PreCheck" and "PostCheck" keys are a bit more complicated but more flexible compared with the other keys. For example, you can re-define the type of return value; you can form expressions with parameters and the return value, and modify the parameters or the return value based on the value of the expressions; you can even skip calling the original API and returns a fake (or predicted) value depending on the values of the parameters. Here are some examples of "PreCheck" and "PostCheck":

#### Example 1

```c++
"ExitWindowsEx": {
    "Signature": {
        "Library": "user32.dll",
        "Return": "BOOL",
        "Feature": "P41"
    },
    "Parameters": {
        "uFlags": "EXITFLAGS",
        "dwReason": "EXITREASON"
    },
    "PreLog": [
        "uFlags",
        "dwReason"
    ],
    "PreCheck": {
        "0": {
            "Define": {
                "uFlags": "DWORD"
        },
        "Check": {
            "uFlags": "(%var0 & 1) || (%var0 & 2) || (%var0 & 8)"
        },
        "Modify": {
            "Return": "%var1 = 1"
        }
     }
  }
}
```

The API used in this example is "ExitWindowsEx", and when the sample calls this API to shut-down or restart the computer (VMs for FortiTracer), HipsHook will catch and then block this behaviour, and record the values of parameters "uFlags" and "dwReason".
Inside "PreCheck", the "Define" key is first used to re-define the type of "uFlags" to "DWORD".
In the "Check" key, it checks for the value of expression "(%var0 & 1) || (%var0 & 2) || (%var0 & 8)", in which "%var0" represents the parameter "uFlags". The expression checks that if the value of "uFlags" is 1, 2 or 8, and if this expression is **True**, it then looks at the content of the "Modify" key.
The "Modify" key will only take effect if the expression above in the "Check" key is **True**. It will modify the value of the return value to 1. The "%var1" here represents the return value. Since a fake value is returned, HipsHook will skip the execution of the original "ExitWindows" API.

***Note that you can check for all the possible "uFlags" values in types.json or online. 1 corresponds to "EWX_SHUTDOWN", 2 corresponds to "EWX_REBOOT", and 8 corresponds to "EWX_POWEROFF".***

#### Example 2

```c++
"GetDiskFreeSpaceExA": {
    "Signature": {
        "Library": "kernel32.dll",
        "Return": "BOOL",
        "Feature": "H10"
    },
    "Parameters": {
        "lpDirectoryName": "LPCSTR",
        "lpFreeBytesAvailableToCaller": "LPULARGE_INTEGER",
        "lpTotalNumberOfBytes": "LPULARGE_INTEGER",
        "lpTotalNumberOfFreeBytes": "LPULARGE_INTEGER"
    },
    "PostCheck": {
        "0": {
            "Check": {
                "lpDirectoryName": "(%var0 == 'nullptr' || %var0 =~ '^C:.*')",
                "*lpTotalNumberOfBytes": "(%var1 <= 64424509440)"
            },
            "Modify": {
                "*lpTotalNumberOfBytes": "%var2 = 1099511627776"
            }
        }
    }
}
```

The API used in this example is "GetDiskFreeSpaceExA". When the sample tries to obtain the amount of free space on the disk, HipsHook will catch and modifies return values in "PostCheck" if necessary.
​In "Check", it checks if the value of "lpDirectoryName" is nullptr or starts with "C:", and if this is **true**, it will proceed to the second check. The second check checks if the value of "\*lpTotalNumberOfBytes" (the value which lpTotalNumberOfBytes is pointing to) is less than or equal to 64424509440 (60G), and if it is also **true**, then the "Modify" key will change the value of "\*lpTotalNumberOfBytes" to 1099511627776 (1TB).

***Note that in the HipsHook program, to retrieve the value of "\*lpTotalNumberOfBytes", it will first get the value of "lpTotalNumberOfBytes", which is an address. Since the type of "lpTotalNumberOfBytes" is "LPULARGE_INTEGER", HipsHook will go to the address and extract the data of the correct type (ULARGE_INTEGER).***

#### Example 3

```C++
"GetTickCount": {
    "Signature": {
        "Library": "kernel32.dll",
        "Return": "DWORD",
        "Feature": "H12"
    },
    "PostLog": [
        "Return"
    ],
    "PostCheck": {
        "0": {
            "Modify": {
                "Return": "%var0 = 1234567"
            }
        }
    }
}
```

This is a relatively simple example, and you can see that it modifies the return value to 1234567 no matter what value is returned from the original API, "GetTickCount".

#### Example 4

```c++
"GetSystemTime": {
    "Signature": {
        "Library": "kernel32.dll",
        "Return": "VOID",
        "Feature": "H13"
    },
    "Parameters": {
        "lpSystemTime": "LPSYSTEMTIME"
    },
    "PostCheck": {
        "0": {
            "Modify": {
                "lpSystemTime->wYear": "%var0 = 2010",
                "lpSystemTime->wMonth": "%var1 = 12",
                "lpSystemTime->wDay": "%var2 = 31"
            }
        }
    }
}
```

This is also a relatively simple example. You can see that no matter what value is returned from the original API, "GetSystemTime", it will always modify the date, which "lpSystemTime" is pointing to, to 2010.12.31. This example is just a demonstration of how to modify the parameters.

#### Example 5

```C++
"NtQueryInformationProcess": {
    "Signature": {
        "Library": "ntdll.dll",
        "Return": "NTSTATUS",
        "Feature": "P62"
    },
    "Parameters": {
        "ProcessHandle": "HANDLE",
        "ProcessInformationClass": "PROCESSINFOCLASS",
        "ProcessInformation": "LPVOID",
        "ProcessInformationLength": "ULONG",
        "ReturnLength": "LPULONG"
    }
    "PreLog": [
        "ProcessHandle",
        "ProcessInformationClass"
    ],
    "PostCheck": {
        "0": {
            "Define": {
                "ProcessInformation": "LPULONG"
            },
            "Check": {
                "ProcessInformationClass": "(%var0 == 'ProcessDebugPort')"
            },
            "Log": [
                "*ProcessInformation"
            ]
        },
        "1": {
            "Define": {
                "ProcessInformation": "DWORD_PTR*"
            },
            "Check": {
                "ProcessInformationClass": "(%var0 == 'ProcessBreakOnTermination')"
            },
            "Log": [
                "*ProcessInformation"
            ]
        }
    }
}
```

The API used in this example is "NtQueryInformationProcess". When the sample tries to use this API to retrieve information about a particular process, HipsHook will catch and block this behaviour, and record the values of the parameters, "ProcessHandle" and "ProcessInformationClass".

The "PostCheck" is a bit complex in this example, since it contains two "check"'s. The first check re-defines the type of the parameter "ProcessInformation" from "LPVOID" to "LPULONG". It then checks if the value of "ProcessInformationClass" is "ProcessDebugPort", and if this is **true**, it will record the value of "\*ProcessInformation".

The second check fist re-defines the type of the parameter "ProcessInformation" from "LPVOID" to "DWORD_PTR\*". It then checks if the value of "ProcessInformationClass" is "ProcessBreakOnTermination", and is this is **true**, it will record the value of "\*ProcessInformation".




------

