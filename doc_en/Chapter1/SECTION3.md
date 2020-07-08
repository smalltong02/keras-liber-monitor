# 1.3 HipsHook Project

HipsHook project contains a static library (.lib) and a dynamic library (.dll). You can statically link the .lib file into your own solution, which is also linked in the current projects and tests in the HipsHook solution. The dynamic library (.dll), on the other hand, is a module that can be injected or loaded in a process. Below introduces the main classes in the HipsHook project.

## 1. CHipsCfgObject Class

This class takes in and processes the root JSON file, hipshook.json ([more info](../Chapter2/SECTION2.md)). The root JSON file contains the information about other JSON configuration files, which includes:

(1) [Types](../Chapter2/SECTION3.md): **CFlagsCfgObject** class is in charge of this JSON file

(2) [APIs](../Chapter2/SECTION4.md): **CSigsCfgObject** class is in charge of this type of JSON files

(3) [Wmi and Com](../Chapter2/SECTION5.md): **CComsCfgObject** class and **CWmisCfgObject** class are in charge of this type of JSON files

## 2. CHookImplementObject Class

This class manages and actually hooks the hooks for APIs, WHI and COM. You can also write your own pre-processing or post-processing function for each API in this class, for those complex cases when modifying JSON files cannot satisfy your need. Below is the introduction to the functions, fields, and MACROs in this class:

### (1) \_hookImplementFunction() Function

This function is the hook function for all APIs, WHI and COM. The steps are the following:

  * extracts information about API arguments and return value, and pack the information into the hook_node struct

  * Calls the **Preprocessing** function to process any required actions before calling the original API. The current possible actions include the following:

    * Checks for the value of _Special_ key. The default value for this key is **False**. If the value is **true**, it means that if this current API (the "parent" API) calls any other APIs (the "children" APIs) that are also included in the JSON files, the "children" APIs will be hooked and return logs as well. Otherwise, only the "parent" API will be hooked and recorded. 

    * Calls the developer-defined pre-processing function (the ones defined with C++ code in the CHookImplementObject class, not the ones in JSON files). The **ADD_PRE_PRROCESSING** MACRO allows developers to add pre-processing functions in C++ code
    * Calls the **ProcessingEnsure** function to handle the _Ensure_ key
    * Calls the **PreprocessingLog** function to handle the _Prelog_ key
    * Calls the **PreprocessingChecks** function to handle the _Precheck_ key
    * Packs the R3Logs generated in the pre-processing stage, and passes the packed R3Logs on to the Post-processing function

  * Calls the original API if the returned value in **Preprocessing** function is not _processing_exit_
  * Calls the **Postprocessing** function to process any required actions after calling the original API. The current possible actions include the following:

    * Calls the developer-defined post-processing function. The **ADD_POST_PROCESSING** MACRO allows developers to add post-processing functions in C++ code (same idea as pre-processing).
    * Calls the **CheckReturn** function to check the return value of the original API. if the return value indicates the that original API has failed, returns _processing_skip_ to skip the steps below.
    * Calls the **PostprocessingLog** function to handle the _Postlog_ key
    * Calls the **PostprocessingChecks** function to handle the _Postcheck_ key
    * Packs the R3Logs generated in the whole hook process stage, and send the packed R3Logs to the **CLogObject** class

  * Cleans and clears the stack and returns

### (2) EnableAllApis() Function and DisableAllApis() Function

You can enable or disable all the API (including WMI and COM) hooks with these two functions, at any stage of execution.

### (3) AddFilterThread() Function

You can add any thread to the _Filter_ list, and then the APIs called in this thread will not be processed with hooks. Currently, the thread which processes logs is added to the _Filter_ list.

### （4）CExceptionObject Class

This class is included in the **CHookImplementObject** class as a field, and is used to catch the exception error message generated within HipHook.dll. Then it will pack the R3Log of the error message and send it to the server end, for the ease of debugging.

## 3. Classes Processing Types and Metadata

### (1) CMetadataTypeObject Class
This class defines the [primitive data types](../Chapter2/SECTION3.md) realized in the HipsHook solution, for instance, _bool_, _Int_, _DWORD_, _ULONG_, _LARGE_INTEGER_, and etc.

### (2) CReferenceObject Class
This class defines the pointer types realized in the HipsHook solution, for instance, _char\*_, _wchar_t\*_, _Int\*_, and etc.

### (3) CArrayObject Class
This class defines the array types realized in the HipsHook solution, for instance, _Int\[10\]_ and etc.

### (4) CTupleObject Class
This class defines the array/collection types realized in the HipsHook solution which contains heterogeneous values. Please refer to the _std::tuple_ class template.

### (5) CStructObject Class
This class defines the struct types realized in the HipsHook solution. For example,

```c++
"COAUTHIDENTITY" {
    "User": "USHORT*",
    "UserLength": "ULONG",
    "Domain": "USHORT*",
    "DomainLength": "ULONG",
    "Password": "USHORT*",
    "PasswordLength": "ULONG",
    "Flags": "ULONG"
    }
```

### (6) CFlagObject Class
This class defines the flag types realized in the HipsHook solution. For example,

```c++
"LOADFLAGS" {
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
```

### (7) CTypeSymbolTableObject Class
This class defines the symbol table realized in the HipsHook solution.

## 3. Classes related to R3Logs

### (1) CLogObject Class
This class packs the logs generated during the processing of hooks, and send the packed logs to CSocketObject Class for further processing. The processing of hooks includes _Ensure_, _Prelog_, _Postlog_, _Precheck_, and _Postcheck_.

A packed R3Log by **CLogObject** class looks like this:

```c++
{"ensure":{"lpLibFileName":"valid"},"pre_log":{"lpLibFileName":"ntdll"},"API":"LoadLibraryA","Pid":"2420","Tid":"2752","Action":"L1","Time":"2020-06-18 12:09:50","SerialN":1}
```

### (2) CSocketObject Class
This class further processes the packed R3Log received from **CLogObject** class. It has two child classes: **CLpcPipObject** class and **CLpcLocalObject** class.

* **CLpcPipeObject** class: sends R3Logs to the server end through pipe, and the server end will further process the R3Logs
* **CLpcLocalObject** class: an alternative to write R3Logs to files locally when pipe fails. This is for debugging purposes.



------

