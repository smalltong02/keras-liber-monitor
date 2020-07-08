# 第三节 Type定义

类型的Json文件是Types.json，它定义了HipsHook中用到的所有类型信息，包括基础类型，附加类型，结构类型和标记类型，***在HipsHook项目中所有的变量、返回值类型都必须在类型Json文件中定义过***。

## 1. 基础类型的定义

HipsHook项目中实现了以下的基础类型：

```c++
"VOID",
"BYTE",
"BOOL",
"CHAR",
"UCHAR",
"INT16",
"UINT16",
"SHORT",
"USHORT",
"WORD",
"WCHAR",
"INT",
"UINT",
"LONG",
"LONG_PTR",
"ULONG",
"ULONG_PTR",
"DWORD",
"DWORD_PTR",
"FLOAT",
"INT64",
"UINT64",
"LONGLONG",
"ULONGLONG",
"QWORD",
"DOUBLE",
"LARGE_INTEGER",
"ULARGE_INTEGER",
"HANDLE",
"HMODULE",
"HRESULT",
"SC_HANDLE",
"NTSTATUS",
"STRING",
"WSTRING"
```



## 2. 附加类型的定义

在"Additional"字段中对附加类型进行了定义，通常附加类型仅仅是其他类型的一个别名称而已，比如：

* "FARPROC": "LPVOID", "FARPROC"是"LPVOID"基础类型的一个别名。
* "BSTR": "OLECHAR\*", "BSTR"是"OLECHAR\*"指针类型的一个别名。
* "REFCLSID": "GUID", "REFCLSID"是"GUID"结构体类型的一个别名。

***附加类型仅仅是其他类型的一个别名，请确保其他类型是一个有效的类型定义。***

### 3. 结构体类型的定义

在"Structure"字段中对结构体类型进行了定义，比如：

```c++
"COAUTHIDENTITY": {
    "User": "USHORT*",
    "UserLength": "ULONG",
    "Domain": "USHORT*",
    "DomainLength": "ULONG",
    "Password": "USHORT*",
    "PasswordLength": "ULONG",
    "Flags": "ULONG"
    }
```



"COAUTHIDENTITY"结构体类型包含了7个数据域，每一个数据域由**名称**和**类型**组成。***请确保结构体类型中的每一个数据域的定义，都必须是一个有效类型的定义。***

## 3. 标记类型的定义

在"Flags"字段中对标记类型进行了定义，比如：

```c++
"LOADFLAGS": {
    "Data": "DWORD",
    "OPERATION": "OR",
    "Value": {
        "DONT_RESOLVE_DLL_REFERENCES": 1,
        "LOAD_IGNORE_CODE_AUTHZ_LEVEL": 16,
        "LOAD_LIBRARY_AS_DATAFILE": 2,
        "LOAD_LIBRARY_AS_DATAFILE_EXCLUSIVE": 64,
        "LOAD_LIBRARY_AS_IMAGE_RESOURCE": 32,
        "LOAD_LIBRARY_SEARCH_APPLICATION_DIR": 512,
        "LOAD_LIBRARY_SEARCH_DEFAULT_DIRS": 4096,
        "LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR": 256,
        "LOAD_LIBRARY_SEARCH_SYSTEM32": 2048,
        "LOAD_LIBRARY_SEARCH_USER_DIRS": 1024,
        "LOAD_WITH_ALTERED_SEARCH_PATH": 8
        }
    }
```



"LOADFLAGS"标记类型包含以下重要的数据域：

*  "Data"数据域定义了"LOADFLAGS"标记类型在内存中存储的实际类型是一个"DWORD"，所以此标记类型在内存中占位4个字节并且是无符号值。
*  "OPERATION"数据域定义了每一个标记值之间的运算关系，"OR"说明标记值之间是或的关系。
*  "Value"数据域定义了每一个标记值的字符串和数字表示。



------

