# 第四节 API定义

包含API定义的Json文件目前有3个：system.json、services.json和ole32.json。该类型Json文件中包含了所有的Win32 API钩子定义，HipsHook将根据这些定义来挂接API钩子并进行钩子的处理。下面以system.json为例，对此类Json文件的定义格式进行详细说明：

## System.json文件

一个典型的API定义Json文件首先包含2个数据域的定义：
* "Category": "System", Json文件中定义的Win32 API都属于System类型。
* "Calling convention": "WINAPI", Json文件中定义的所有Win32 API函数的调用约定都为"WINAPI"，即stdcall调用约定。目前支持三种调用约定：stdcall、cdecl、fastcall

### 1. Win32 API函数

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

* "LoadLibraryA"字段是想要挂钩的API函数名称。
* "Signature"字段定义了API原始函数的一些属性值：
  * "Library": "kernel32.dll", "LoadLibraryA"由kernel32.dll导出。
  * "Return": "HMODULE", "LoadLibraryA"返回值是"HMODULE"类型。
  * "Special": false, Special值为false，表示处理此钩子时如果是重入状态，那么这个钩子什么也不做直接返回，Special值默认为False。
  * "Feature": "L1"，"L1"将被作为"Action"字段写入到R3Log中，此值与Fortitracer log中定义的action值一致。
 * "Parameters"字段定义了此API的参数信息，"LoadLibraryA"只包含一个有效的参数。
   * "lpLibFileName": "LPCSTR", 参数信息由参数名"lpLibFileName"和参数类型"LPCSTR"组成。

### 2. Ensure字段

​    Ensure字段定义了某个参数值，或者某些表达式必须是有效或者为True，如果不能满足条件即表达式为False，那么Prelog、PostLog、Precheck、Postcheck等后续处理都将不会再做。比如：

```c++
"Ensure": [
	"lpLibFileName"
],
"PreLog": [
	"lpLibFileName"
]
```

参数"lpLibFileName"的类型被定义成"LPCSTR"，因此如果"lpLibFileName"是空字符串，那么后面的Prelog字段将会被忽略。

### 3. Prelog字段

​    Prelog字段定义了某个参数值或者某表达式将会在调用原始函数之前被记录到R3Log中。比如：

```c++
"PreLog": [
	"lpLibFileName",
	"dwFlags"
  ]
```

​    参数"lpLibFileName"的类型被定义成"LPCSTR"，"dwFlags"的类型被定义成"LOADFLAGS"标记值，所以这两个参数会在调用原始函数"LoadLibraryExA"之前被记录到R3Log中。参数"lpLibFileName"将被记录为一个字符串，比如"Kernel32.dll"，"dwFlags"将被转换为一个标记字符串值并被记录下来，比如"LOAD_LIBRARY_SEARCH_SYSTEM32 | LOAD_LIBRARY_AS_DATAFILE_EXCLUSIVE"。

### 4. Postlog字段

Postlog字段定义了某个参数值或者某表达式将会在调用原始函数成功之后被记录到R3Log中，不成功将不会被记录。比如：

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

返回值"Return"的类型被定义成"DWORD"，所以返回值会在调用原始函数"GetTickCount"成功之后被记录到R3Log中。

### 5. Precheck、Postcheck字段

​    "Precheck"字段和"Postcheck"字段的定义规则比较复杂，因为在"Xxxcheck"字段中给了开发者更多的灵活性。比如你可以根据需求重定义前面的参数或返回值的类型；你也可以根据参数或返回值组成的表达式进行一些逻辑或算数运算，并根据运算的结果来决定是否修改和如何修改这个API的参数和返回值；你甚至可以决定不调用原始函数并通过参数和返回值去返回一些虚假的信息给调用者。下面用一些真实的例子来带你了解"Xxxcheck"字段：

#### 用例1：

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

​    在这个定义中HipsHook挂钩了Win32 API "ExitWindowsEx"，当sample调用这个API打算关闭计算机的时候，HipsHook将会拦截到这个行为，并在"Prelog"中记录下这个API的两个参数"uFlags"和"dwReason"。

​    在之后的Precheck中，首先使用了"Define"字段重新定义了"uFlags"，让这个参数变为了"DWORD"值。

​    在"Check"字段中，对这个重新定义的参数"uFlags"进行了检查，表达式"(%var0 & 1) || (%var0 & 2) || (%var0 & 8)"表示，如果"uFlags"的值被设置了1,2或8整个表达式将会返回True。检查Types.json得知，1等价于"EWX_SHUTDOWN"，2等价于"EWX_REBOOT"，8等价于"EWX_POWEROFF"，因此整个表达式表示的意思是，如果sample样本调用"ExitWindowsEx"去关机或重启，那么整个表达式返回True。

​    当"Check"字段返回True的时候，"Modify"字段的内容将会起作用。这里将"Return"的值修改为1，表明了当发现sample样本想要重启或者关机的时候，这个钩子不会再调用原始函数"ExitWindowsEx"，而是返回了一个虚假的返回值True用来欺骗调用者。

#### 用例2：

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

​    在这个定义中HipsHook挂钩了Win32 API "GetDiskFreeSpaceExA"，当sample样本调用这个API打算获取磁盘空间大小时，HipsHook将会拦截到这个行为，并在"Postcheck"字段中进行检查和修改。

​    在"Check"字段中，首先对参数"lpDirectoryName"进行了检查，如果这个参数为nullptr或者这个参数的路径以"C:"字符串开头，那么整个表达式将会返回True。

​    在第一个表达式返回True之后，会检查第二个表达式。表达式2对参数"lpTotalNumberOfBytes"所指向的内容进行了检查，因为"lpTotalNumberOfBytes"的类型是"LPULARGE_INTEGER"，所以表达式会首先取到该参数的地址，并根据此地址取出"ULARGE_INTEGER"类型的数值，最后判断此数值是否<= 64424509440（60G）

​    如果表达式1和2都返回True，那么进入"Modify"字段，表达式3会把参数"lpTotalNumberOfBytes"所指向的"ULARGE_INTEGER"的数值修改为1099511627776（1TB）

#### 用例3：

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

此用例比较简单，根据表达式可以得知，无论原始的API "GetTickCount"返回什么值，都会将返回值修改为1234567。此用例展示了如何修改返回值。

#### 用例4：

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

此用例比较简单，在"PostCheck"字段中修改了参数"lpSystemTime"，无论原始函数返回结果如何，都会将年月日修改为2010年12月31日。此用例展示了如何修改结构体参数。

#### 用例5：

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

​    在这个定义中HipsHook挂钩了Win32 API "NtQueryInformationProcess"，当sample样本调用这个API打算获取进程信息的时候，HipsHook将会拦截到这个行为，并在Prelog中记录下这个API的两个参数"ProcessHandle"和"ProcessInformationClass"。

​    "PostCheck"字段比较复杂，它里面定义了2个"check"。第一个"check"通过"Define"字段重定义了参数"ProcessInformation"，把它的类型从"LPVOID"改为"LPULONG"，之后在"check"字段里对参数"ProcessInformationClass"进行了检查，"ProcessInformationClass"是一个标记值，所以直接比对标记值字符串是否为"ProcessDebugPort"。如果表达式返回True，那么就记录下参数"\*ProcessInformation"里面的"ULONG"的数值。

​    第二个"check"也通过"Define"字段再次重定义了参数"ProcessInformation"，把它的类型从"LPVOID"改为"DWORD_PTR\*"，之后在"check"字段里对参数"ProcessInformationClass"进行了检查，"ProcessInformationClass"是一个标记值，所以直接比对标记值字符串是否为"ProcessBreakOnTermination"。如果表达式返回True，那么同样记录下参数"\*ProcessInformation"里面的"ULONG"的数值。

​    经过这两个表达式处理之后，此处要完成的实际功能也已经表达的很清楚了，如果sample通过设置ProcessInformationClass为ProcessDebugPort或者为ProcessBreakOnTermination，那么钩子函数将会把对应的"\*ProcessInformation"的值记录到R3Log当中并发给服务器端。



------

