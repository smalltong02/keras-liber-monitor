# 第三节 HipsHook项目

HipsHook项目包含一个静态库lib和一个动态库dll，静态库可以让你将Hipshook的功能模块静态链编到你自己的项目当中，当前工程中的dll项目和测试项目都使用了静态库lib模块，动态库dll则是被注入的一个实体模块。HipsHook项目中包含以下几个主要的类来实现其功能：

## 1. CHipsCfgObject类

该类用来解析Json配置的根文件 hipshook.json（[详细说明](../Chapter2/SECTION2.md)），并从该配置文件中读入其他几个Json配置文件的信息，为进一步解析Json数据做准备。其他的配置文件包括：

(1) [类型定义](../Chapter2/SECTION3.md)，CFlagsCfgObject类负责解析该类型Json文件。

(2) [API钩子定义](../Chapter2/SECTION4.md)，CSigsCfgObject类负责解析该类型Json文件。

(3) [Wmi及Com钩子定义](../Chapter2/SECTION5.md)，CComsCfgObject类和CWmisCfgObject类负责解析该类型Json文件。

## 2. CHookImplementObject类

该类负责挂接和实际管理API、WMI和COM的钩子，并且可以通过该类注册你自己想要处理的钩子函数。以下会对该类中重要的成员函数，成员变量和宏做具体的说明：

### (1) _hookImplementFunction()函数

该函数是所有API、WMI和COM的钩子函数，该函数的执行流程分为以下几个步骤：

  * 从栈中读入原始函数的参数信息和返回值信息，打包到结构hook_node中。

  * 调用Preprocessing函数进行钩子的前处理工作。

    * 检查Special标记，如果Speical被设置为True，表明该钩子可以重入，默认该标记为False。
    * 调用开发者注册的预处理钩子函数，ADD_PRE_PROCESSING宏允许开发者在HipsHook项目中注册C++代码编写的钩子处理函数，这个处理函数可以让开发者在原始函数调用之前处理原始函数的参数或者返回值。
    * 调用ProcessingEnsure函数对Ensure字段进行处理。
    * 调用PreprocessingLog函数对Prelog字段进行处理。
    * 调用PreprocessingChecks函数对Precheck字段进行处理。
    * 对Preprocessing中产生的R3Log信息进行打包，准备传递给Postprocessing处理。

  * 如果Preprocessing函数返回值不是processing_exit，那么调用原始API函数进行处理。
  * 调用Postprocessing函数进行钩子的前后处理工作。

    * 调用开发者注册的后处理钩子函数，ADD_POST_PROCESSING宏允许开发者在HipsHook项目中注册C++代码编写的钩子处理函数，这个处理函数可以让开发者在原始函数调用之后处理原始函数的参数或者返回值。
    * 调用CheckReturn函数检查原始函数返回值，如果返回值表示原始函数调用失败那么直接返回processing_skip，后面的后处理动作都不在被调用。
    * 调用PostprocessingLog函数对Postlog字段进行处理。
    * 调用PostprocessingChecks函数对Postcheck字段进行处理。
    * 对本次钩子的R3Log信息进行打包，并发送给CLogObject去处理。

  * 清理调用栈并返回。

### （2）EnableAllApis()函数和DisableAllApis()函数

可以通过这两个函数在任何运行阶段允许或者禁止所有的钩子（包括API，WMI和COM的所有钩子）。

### （3）AddFilterThread()函数

可以通过该函数将某一个线程加入到filter列表中，加入的线程所产生的API调用将不会被钩子函数处理，当前Log处理线程被加入到了Filter列表当中。

### （4）CExceptionObject类

该类作为成员变量被包含在CHookImplementObject类中，该类用来抓取在钩子函数处理过程中HipsHook.dll自身产生的异常出错信息，并把该信息打包的R3Log中发回给服务器端，以便后续由开发人员进一步查找问题。

## 3. 类型及元数据处理的类

### (1) CMetadataTypeObject类
该类定义了HipsHook中实现的[基本类型](../Chapter2/SECTION3.md)信息，比如bool、Int、DWORD、ULONG、LARGE_INTEGER等等。

### (2) CReferenceObject类
该类定义了HipsHook中实现的指针类型信息，比如char\*、wchar_t\*、Int\* 等等。

### (3) CArrayObject类
该类定义了HipsHook中实现的数组类型信息，比如Int\[10\] 等等。

### (4) CTupleObject类
该类定义了HipsHook中实现的多类型数组信息，关于该类型请参考C++17支持的std::tuple类。

### (5) CStructObject类
该类定义了HipsHook中的结构体类型信息，比如：

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

### (6) CFlagObject类
该类定义了HipsHook中实现的标记类型信息，比如：

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

### (7) CTypeSymbolTableObject类
该类定义了HipsHook中实现的类型符号表。

## 3. 和R3Log相关的类

### (1) CLogObject类
该类负责在钩子处理过程中，对Ensure、Prelog、Postlog、Precheck、Postcheck产生的日志信息进行打包，并最终将打好的包发送给CSocketObject类进行处理。

一个经过CLogObject类进行过打包的R3Log看起来像下面这样：

```c++
{"ensure":{"lpLibFileName":"valid"},"pre_log":{"lpLibFileName":"ntdll"},"API":"LoadLibraryA","Pid":"2420","Tid":"2752","Action":"L1","Time":"2020-06-18 12:09:50","SerialN":1}
```

### (2) CSocketObject类
该类负责将CLogObject类打包过的R3Log进行进一步的处理，它包含有两个子类：CLpcPipeObject类和CLpcLocalObject类。

* CLpcPipeObject类，通过Pipe通讯机制把R3Log发送给服务器端，服务器端会对R3Log进一步处理。
* CLpcLocalObject类，做为Pipe通讯机制的备选方案。当Pipe通讯失败的时候，把R3Log写入到本地的临时文件中，以便供开发人员查找问题。



------

