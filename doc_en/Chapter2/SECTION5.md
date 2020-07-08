# 2.5 Wmi and Com (JSON)

The configure and processing of WMI and COM hooks are very different from that of the Win32 APIs. The reason is that the targets to be hooked in WMI and COM are data and functions that are included as fields or methods in some pre-defined WMI and COM object classes.


## 1. WMI

**wmiobject.json** is the JSON file that contains hook configurations for WMI. Below will introduce how these configurations work.

A typical WMI JSON file must have two keys:

* "Category": "WMIObject", which means the hook config for the "WMIObject" category is contained in JSON file 
* "Calling Convention": "WINAPI", the calling convention of all the WIN32 APIs defined in the WMI JSON file are "WINAPI" (i.e. stdcall). Three types of calling conventions are supported in HipsHook: stdcall, cdecl, and fastcall

**There are no "Ensure", "Prelog", or "Postlog" keys in the WMI JSON file, and it only has "Precheck", "Postcheck", and the new "Methods" keys.**

### (1) WMI Class Data Hook

```C++
"Win32_DiskDrive": {
    "MIDL_INTERFACE": "{8502C4B2-5FBB-11D2-AAC1-006008C78BC7}",
    "Data": {
      "CreationClassName": "BSTR",
      "Name": "BSTR",
      "Caption": "BSTR",
      "DeviceID": "BSTR",
      "Size": "ULONGLONG"
    },
    "PostCheck": {
      "0": {
        "Check": {
          "Size": "%var0 <= 3298534883328"
        },
        "Modify": {
          "Size": "%var1 = 3298534883328"
        },
        "Log": [
          "CreationClassName",
          "Name",
          "Caption"
        ]
      }
    }
  }
```

Here is some explanation of the keys:

* "Win32_DiskDrive": the name of the WMI object class
* "MIDL_INTERFACE": the UUID of the WMI object class, is unique through the whole system
* "Data": the data fields of the WMI object class, each has a **name** as the key and **type** as the value. These data fields will be used in "Postcheck"
* "PostCheck":
  * "Check": means that when the sample tries to obtain information of the disk drive, if the size is less than or equal to 3298534883328 (3T), then the expression returns **true**
  * "Modify": if the expression in "Check" returns **true**, modify "Size" to equal to 3T
  * "Log": an array that lists the items to be logged when "Win32_DiskDrive" object is queried by the sample through WMI

### (2) WMI Class Method Hook

```c++
"Win32_Process": {
    "MIDL_INTERFACE": "{8502C4DC-5FBB-11D2-AAC1-006008C78BC7}",
    "Data": {
      "CommandLine": "BSTR",
      "ExecutablePath": "BSTR"
    },
    "Methods": {
      "AttachDebugger": {
        "Signature": {
          "Library": "IWbemServices.ExecMethod",
          "Return": "HRESULT"
        },
        "Logging": true
      },

      "Create": {
        "Signature": {
          "Library": "IWbemServices.ExecMethod",
          "Return": "HRESULT"
        },
        "Parameters": {
          "CommandLine": "BSTR",
          "CurrentDirectory": "BSTR",
          "ProcessStartupInformation": "LPVOID",
          "ProcessId": "UINT"
        },
        "PostCheck": {
          "0": {
            "Check": {
              "ProcessId": "%var0 != 0"
            },
            "Handle": {
              "ProcessId": "AddTarget"
            },
            "Log": [
              "CommandLine",
              "CurrentDirectory",
              "ProcessId"
            ]
          }
        }
      },

      "GetOwner": {
        "Signature": {
          "Library": "IWbemServices.ExecMethod",
          "Return": "HRESULT"
        },
        "Parameters": {
          "User": "BSTR",
          "Domain": "BSTR"
        },
        "Logging": true
      },

      "Terminate": {
        "Signature": {
          "Library": "IWbemServices.ExecMethod",
          "Return": "HRESULT"
        },
        "Parameters": {
          "Reason": "UINT"
        },
        "Logging": true
      }
    }
  }
```

* "Win32_Process": the name of the WMI object class

* "MIDL_INTERFACE": the UUID of the WMI object class, is unique through the whole system

* "Data": the data fields of the WMI object class, each has a **name** as the key and **type** as the value. These data fields will be used in "Methods"

* "Methods": the methods you want to hook in the WMI object class. **The configuration and definition of the hook function for each method of a WMI object class is the same as it of a Win32 API**

  * "AttachDebugger": the name of the first method that requires hooking
    
    * "Signature": defines the features of the method
    
      * "Library": "IWbemServices.ExecMethod", is the function which takes in and executes the "AttachDebugger" method. This value will be recorded in the R3Log
      * "Return": the type of the return value of this method is "HRESULT"

    * This method does not have any parameter
    
    * "Logging": is **true**, therefore the return value will be logged in the R3Log when "AttachDebugger" is called
    
      
    
  * "Create": the name of the second method that requires hooking
  
    * "Signature": defines the features of the method
    * "Parameters": parameters of the "Create" method, each is composed of a **name** and a **type**. This method has 4 parameters
    * "PostCheck": checks that if the value of the parameter "ProcessId" is not 0. If this is **true**, the value of "ProcessId" will be added to the _Target_ list and be monitored. Also, 3 of the parameters, "CommandLine", "CurrentDirectory" and "ProcessId" will be recorded in the R3Log
  
  * "GetOwner": the name of the third method that requires hooking
  * "Terminate": the name of the fourth method that requires hooking



------

