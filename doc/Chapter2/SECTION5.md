# 第五节 Wmi，Com定义

WMI和COM钩子的处理和Win32 API钩子的处理非常不同，因为WMI和COM是基于对象类去挂载的钩子函数。



## 1. WMI对象类的钩子定义

包含WMI钩子定义的Json文件是wmiobject.json，该类型Json文件中包含了所有的WMI对象类的钩子定义，HipsHook将根据这些定义来挂接WMI对象钩子并进行钩子的处理。下面将对定义格式进行详细说明：

一个典型的WMI对象钩子定义的Json文件首先包含2个数据域的定义：

* "Category": "WMIObject", 表明了Json文件中定义的对象钩子属于WMIObject类型。
* "Calling convention": "WINAPI", Json文件中定义的所有Win32 API函数的调用约定都为"WINAPI"，即stdcall调用约定。目前支持三种调用约定：stdcall、cdecl、fastcall

**WMI对象类的钩子定义中没有"Ensure"、"Prelog"、"Postlog"，仅仅有"Precheck"和"Postcheck"字段以及新增加的"Methods"字段。**

### (1) WMI对象类的数据钩子说明

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

* "Win32_DiskDrive"字段是想要挂钩的WMI对象类的名称。
* "MIDL_INTERFACE"字段定义了这个WMI对象类的UUID值，这是一个整个系统中唯一的值。
* "Data"字段定义了WMI对象的数据域，每一个数据域由**名称**和**类型**组成，这些数据域中定义的变量将在"Postcheck"字段中使用。
* "PostCheck"字段中定义了需要修改的数据域：
  * "Check"字段: 表达式的含义是当sample样本通过WMI接口向WMI服务查询"Win32_DiskDrive"的"Size"数据域时，如果"Size"返回的数值小于等于3T大小，那么表达式返回True。
  * "Modify"字段: 如果"Check"字段中的表达式为True，那么修改"Size"的数值为3T大小。
  * "Log"字段: 是一个数组，定义了某些参数值或者表达式将会在sample样本通过WMI接口向WMI服务查询"Win32_DiskDrive"对象的时候，被记录到R3Log中。

### (2) WMI对象类的方法钩子说明

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

* "Win32_Process"字段是想要挂钩的WMI对象类的名称。

* "MIDL_INTERFACE"字段定义了这个WMI对象类的UUID值，这是一个整个系统中唯一的值。

* "Data"字段定义了WMI对象的数据域，每一个数据域由**名称**和**类型**组成，这些数据域中定义的变量将在"Methods"字段中使用。

* "Methods"字段中定义了需要挂钩的WMI对象类中的方法，**每一个WMI对象类中的方法的定义和Win32 API钩子的定义规则是完全相同的**：
  * ""AttachDebugger""字段: 是需要挂接的第一个方法名称。
    
    * "Signature"字段：同Win32 API钩子定义中的"Signature"字段，定义了方法的一些属性值。
    
      * "Library"字段："IWbemServices.ExecMethod"定义了处理这个方法的成员函数名称，这个字符串仅仅被输出到R3Log中，可以被忽略。
      * "Return"字段：返回值类型被定义为"HRESULT"。
    
    * 这个方法没有参数字段。
    
    * "Logging"字段：当这个方法被调用时，"Return"值将会被记录到R3Log中。
    
      
    
  * "Create"字段: 是需要挂接的第二个方法名称。
  
    * "Signature"字段：同上一个方法。
    * "Parameters"字段："Create"方法的参数列表，每一个参数由**参数名称**和**参数类型**组成，该方法有四个参数。
    * "PostCheck"字段：与Win32 API钩子中的"PostCheck"字段定义相同，"Check"字段将会检查参数"ProcessId"，如果值不等于0，那么将会把"ProcessId"的值添加到监控列表Target里面去。"Log"字段表明当这个方法被调用时，参数"CommandLine"、"CurrentDirectory"和"ProcessId"将会被记录到R3Log中。
  
  
  
  * "GetOwner"字段：是需要挂接的第三个方法名称，其他字段说明同前面的方法。
  * "Terminate"字段：是需要挂接的第四个方法名称，其他字段说明同前面的方法。



------

