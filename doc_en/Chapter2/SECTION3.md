# 2.3 Type (JSON)

The JSON file that defines all the types is named **Types.json**. This file defines all the variable types used in HipsHook, including the primitive types, the additional types, the struct types and the flag types. ***All the variable and return value types used in HipsHook must be defined in this JSON file***.

## 1. The Primitive Types

These are the primitive types currently included in the file:

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



## 2. Additional Types

The additional types are usually just renamed primitive types. For example:

* "FARPROC": "LPVOID", "FARPROC" is an alias of "LPVOID"
* "BSTR": "OLECHAR\*", "BSTR" are aliases of "OLECHAR\*"
* "REFCLSID": "GUID", "REFCLSID" are aliases of "GUID"

***The additional types are just aliases of other types. Please ensure these other types are defined and valid.***

### 3. Struct Types

Here is an example of defined struct type:

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

The "COAUTHIDENTITY" struct has 7 items, and each item is composed of a **name** and a **type** corresponding to the name.
***Please ensure that the types in a struct are all defined and valid.***

## 3. Flag Types

Here is an example of defined flag type:

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

Below is the explanation of each key in this flag type definition:

*  "Data": defines that each flag in this flag type is actually a _DWORD_ type. Therefore each flag is a 4-byte unsigned variable
*  "OPERATION": defines the relationship among each flag. "OR" means that multiple flags can be passed into APIs, connected with "|" symbol as one argument
*  "Value": defines the numerical value of each flag



------

