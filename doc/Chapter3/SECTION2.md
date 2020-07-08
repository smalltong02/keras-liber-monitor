# 第二节  google test单元测试



## 为什么要使用 Google C++ Testing Framework？

HipsHook项目使用了Google test单元测试框架，使用这个框架有许多好处。



某些类型的测试可能有糟糕的内存问题，这些问题只在运行期间随机出现。Google 的测试框架为处理这种情况提供了出色的支持。可以使用 Google 框架重复运行相同的测试一千次。当出现故障的迹象时，它将自动地调用调试器。



与其他许多测试框架不同，可以把 Google 测试框架内置的断言部署在禁用了异常处理（通常由于性能原因）的软件中。因此，也可以在析构函数中安全地使用断言。



运行测试很简单。只需调用预定义的 `RUN_ALL_TESTS` 宏，而不需要通过创建或驱动单独的运行器类来执行测试。这比 CppUnit 等框架方便很多。



## Google Test 断言介绍

在 google test 中实现单元测试，可通过 ASSERT\* 和 EXPECT\* 断言来对程序运行结果进行检查。 ASSERT\* 版本的断言失败时会产生致命失败，并结束当前函数； EXPECT\* 版本的断言失败时产生非致命失败，但不会中止当前函数。因此， ASSERT\* 常常被用于后续测试逻辑强制依赖的处理结果的断言，如创建对象后检查指针是否为空，若为空，则后续对象方法调用会失败；而 EXPECT\* 则用于即使失败也不会影响后续测试逻辑的处理结果的断言，如某个方法返回结果的多个属性的检查。



**基本断言**: 

ASSERT_TRUE(*condition*);

EXPECT_TRUE(*condition*);

ASSERT_FALSE(*condition*);

EXPECT_FALSE(*condition*);



**二进制比较断言**：

ASSERT_EQ(*expected*,*actual*);

EXPECT_EQ(*expected*,*actual*);

ASSERT_NE(*val1*,*val2*);

EXPECT_NE(*val1*,*val2*);

ASSERT_LT(*val1*,*val2*);

EXPECT_LT(*val1*,*val2*);

ASSERT_LE(*val1*,*val2*);

EXPECT_LE(*val1*,*val2*);

ASSERT_GT(*val1*,*val2*);

EXPECT_GT(*val1*,*val2*);

ASSERT_GE(*val1*,*val2*);

EXPECT_GE(*val1*,*val2*);


**字符串比较断言**：

ASSERT_STREQ(*expected_str*,*actual_str*);

EXPECT_STREQ(*expected_str*,*actual_str*);

ASSERT_STRNE(*str1*,*str2*);

EXPECT_STRNE(*str1*,*str2*);

ASSERT_STRCASEEQ(*expected_str*,*actual_str*);

EXPECT_STRCASEEQ(*expected_str*,*actual_str*);

ASSERT_STRCASENE(*str1*,*str2*);

EXPECT_STRCASENE(*str1*,*str2*);


**浮点数比较断言**：

ASSERT_FLOAT_EQ(*val1*, *val2*);

EXPECT_FLOAT_EQ(*val1*, *val2*);

ASSERT_DOUBLE_EQ(*val1*, *val2*);

EXPECT_DOUBLE_EQ(*val1*, *val2*);



**`务必请使用正确的断言语句来编写单元测试`**；关于其它的高级断言语句比如异常断言，死亡断言等请查阅google test官方文档。



## HipsHook项目中包含哪些大的测试分类？

HipsHook的实现由几个大的模块组成，测试应当根据这几个模块进行分类：

1. Json配置格式的分析模块。这个模块主要负责分解Json文件，包括从中读取相关的模块配置信息、数据类型定义信息、Win32 API钩子信息、WMI和COM钩子信息。
2. 钩子模块。这个模块主要负责根据Json文件的配置，对相应的Win32 API、WMI和COM对象挂钩子。
3. 数据类型处理模块。这个模块主要负责根据Json文件的配置，对基础类型，附加类型，结构类型，标记类型等的分析和处理。
4. 语法及语义分析模块。这个模块主要负责根据Json文件的配置，对表达式和算数运算的逻辑分析，并最终得到表达式和算数运算的结果。
5. R3日志系统模块。这个模块主要负责在一个钩子的生存期间，生成相应R3Log并将日志传输给服务器端，其中还包括Error日志和Debug日志的管理。
6. 异常处理模块。这个模块主要负责抓取其他模块产生的异常信息，并对其进行处理和记录。



###  Json配置格式分析模块的测试

待写....

------



### 钩子模块的测试

钩子模块的测试层级结构名称为**`"HookImplementTest"`**，所有和钩子模块相关的测试代码都要保证在此结构名称之下。

* **对参数、返回值根据不同类型的基本测试**

  * 数字类型：比如INT，ULONG，LONGLONG，LARGE_INTEGER等等。

    应该根据类型的宽度，有符号无符号来选取合适的值进行测试。

    比如针对INT有符号类型可以选取：1000（正数），-1000（负数），0（极小值），65535（极大值），70000（越界值）来测试。

    比如针对ULONG无符号类型可以选取：1000(正数)，0（极小值），65535（极大值），70000（越界值）来测试。

    

  * 精度类型：比如FLOAT，DOUBLE等等。

    应该根据类型的宽度，精度来选取合适的值进行测试。

    比如针对DOUBLE类型可以选取：1000（无小数位），-1.79E+308（负数最小值），1.79E+308（正数最大值）来测试。

    

  * 字符类型：比如CHAR、UCHAR、WCHAR等等

    应该根据类型的宽度，有符号无符号来选取合适的值进行测试。

    比如针对CHAR类型可以选取：'\0'（空字符）、 'a'（正常字符）、'#'（特殊字符）、0xE，0xEA（无效字符）、0x202E（越界字符）来测试。

  

  * 字符串类型：比如STRING、WSTRING、CHAR\*、WCHAR\*等等。

    应该根据指针类型的特殊性来测试。

    比如根据WCHAR*类型可以选取：nullptr（无效指针）、"\0"（空字符串）、"Hello World!"（短字符串）、"Hello......"（长度8K的超长字符串）、"0x00,0x00,0xdd,0xdd......"（无效未知长度内存块）来测试。

    

  * 指针类型：比如LPVOID，INT\*，WCHAR\*，GUID*等等。

    应该根据指针的特性和所指向数据的具体类型来选取合适的值进行测试。

    比如针对INT*类型可以选取：nullptr（无效指针）、"\0"（空数据）、"0x45"（正常数据）进行测试。

    比如针对GUID*类型可以选取：nullptr（无效指针）、"\0"（空数据）、"{FAF76B93-798C-11D2-AAD1-006008C78BC7}"（有效数据）、"{FAF76B93-"（残缺数据）来测试。

    

  * 结构类型：比如COAUTHIDENTITY等等。

    应该根据结构类型宽度和其内部数据域来选取合适的值进行测试。

    比如COAUTHIDENTITY.Password数据是一个USHORT\*类型，可以选取："0x0,0x0,0x0,0x0..."（空数据以及空指针），"0x0...0x103420..."（有效指针，但USHORT是空数据），"0x0...0x103420..."（有效指针，有效数据）来测试。

    

  * 标记类型：比如LOADFLAGS，EXITFLAGS等等。

    应该根据结构类型的宽度，值操作（OPERATION）来选取合适的值进行测试。

    比如LOADFLAGS结构，它是一个UINT类型，值操作是或操作（OR），那么可以选取：0，0x800000（无效标记）、2（单标记）、0x2022（组合标记）、0x843022（无效组合标记）来测试。

  

* **创建Win32 API钩子基本测试**

  请符合以下准则：

  * 基本测试名称以**`"函数名称\_字段名称\_变量名称\_测试名称"**`组成，比如**LoadLibraryExA\_Prelog\_lpLibFileName_superlong**。
  * 测试时所选取的参数值，请保证在无R3钩子（DisableAllApis()）的情况下，调用原始函数时不会发出异常。
  * 测试时所选取的参数值，请保证其它参数为正确的值。
  * 针对每一个处理字段中的每一个表达式、算数运算或变量编写合适的能够成功的测试用例，并检查R3Log日志中记录的值是否正确。
  * 针对每一个处理字段中的每一个表达式、算数运算或变量编写合适的能够失败的测试用例，并检查是否返回了正确的error或debug日志。
  * 在DisableAllApis()的环境下，调用以上的测试用例并记录返回值和GetLastError()值；在EnableAllApis()的环境下，调用以上的测试用例并记录返回值和GetLastError()值。对两个值作比较，必须等值。




###### 测试用例1：

```c++
"LoadLibraryExW": {
    "Signature": {
      "Library": "kernel32.dll",
      "Return": "HMODULE",
      "Special": false,
      "Feature": "L1"
    },

    "Parameters": {
      "lpLibFileName": "LPCWSTR",
      "hFile": "HANDLE",
      "dwFlags": "LOADFLAGS"
    },

    "Ensure": [
      "lpLibFileName"
    ],

    "PreLog": [
      "lpLibFileName",
      "dwFlags"
    ]
  }
```

在本测试用例中有两个特殊字段需要处理，它们是"Ensure"和"PreLog"。针对"Ensure"字段，基本测试名称应为"LoadLibraryExW_Ensure_lpLibFileName_xxx"；"Prelog"字段中的测试名称应为"LoadLibraryExW_Prelog_lpLibFileName_xxx"和"LoadLibraryExW_Prelog_dwFlags_xxx"。因为"lpLibFileName"变量在两个字段中都需要测试并且它们是重复的，所以在这里可以去掉重复测试，仅保留两个测试"LoadLibraryExW_Prelog_lpLibFileName_xxx"和"LoadLibraryExW_Prelog_dwFlags_xxx"即可。



在"LoadLibraryExW_Prelog_lpLibFileName_test"中，我们编写测试代码来测试参数变量"lpLibFileName"，该变量是"LPCWSTR"类型，即字符串指针类型。

```c++
TEST_F(HookImplementTest, LoadLibraryExW_Prelog_lpLibFileName_test)
{
	HMODULE pre_hmod1, pre_hmod2, pre_hmod3, pre_hmod4, pre_hmod5;
    DWORD pre_error1, pre_error2, pre_error3, pre_error4, pre_error5;
    HMODULE hooked_hmod1, hooked_hmod2, hooked_hmod3, hooked_hmod4, hooked_hmod5;
    DWORD hooked_error1, hooked_error2, hooked_error3, hooked_error4, hooked_error5;
    DWORD dwFlags = 0;
    
    // first call test API when DisableAllApis().
    ASSERT_TRUE(g_hook_test_object->DisableAllApis());
    // test when lpLibFileName is nullptr
    pre_hmod1 = LoadLibraryExW(nullptr, nullptr, dwFlags);
    pre_error1 = GetLastError();
    // test when lpLibFileName is "\0"
    pre_hmod2 = LoadLibraryExW(L"\0", nullptr, dwFlags);
    pre_error2 = GetLastError();
    // test when lpLibFileName is normal dll name
    pre_hmod3 = LoadLibraryExW(L"kernel32.dll", nullptr, dwFlags);
    pre_error3 = GetLastError();
    // test when lpLibFileName is invalid dll name and super long name.
    pre_hmod4 = LoadLibraryExW(L"test_dllname......", nullptr, dwFlags);
    pre_error4 = GetLastError();
    // test when lpLibFileName is incorrect data block.
    pre_hmod5 = LoadLibraryExW(L"0x00,0x00,0xdd,0xdd......", nullptr, dwFlags);
    pre_error5 = GetLastError();
    
    // second call test API when EnableAllApis().
    ASSERT_TRUE(g_hook_test_object->EnableAllApis());
    // test when lpLibFileName is nullptr
    hooked_hmod1 = LoadLibraryExW(nullptr, nullptr, dwFlags);
    hooked_error1 = GetLastError();
    // test when lpLibFileName is "\0"
    hooked_hmod2 = LoadLibraryExW(L"\0", nullptr, dwFlags);
    hooked_error2 = GetLastError();
    // test when lpLibFileName is normal dll name
    hooked_hmod3 = LoadLibraryExW(L"kernel32.dll", nullptr, dwFlags);
    hooked_error3 = GetLastError();
    // test when lpLibFileName is invalid dll name and super long name.
    hooked_hmod4 = LoadLibraryExW(L"test_dllname......", nullptr, dwFlags);
    hooked_error4 = GetLastError();
    // test when lpLibFileName is incorrect data block.
    hooked_hmod5 = LoadLibraryExW(L"0x00,0x00,0xdd,0xdd......", nullptr, dwFlags);
    hooked_error5 = GetLastError();
    
    // compare return result and error code.
    EXPECT_EQ(pre_hmod1, hooked_hmod1);
    EXPECT_EQ(pre_error1, hooked_error1);
    EXPECT_EQ(pre_hmod2, hooked_hmod2);
    EXPECT_EQ(pre_error2, hooked_error2);
    EXPECT_EQ(pre_hmod3, hooked_hmod3);
    EXPECT_EQ(pre_error3, hooked_error3);
    EXPECT_EQ(pre_hmod4, hooked_hmod4);
    EXPECT_EQ(pre_error4, hooked_error4);
    EXPECT_EQ(pre_hmod5, hooked_hmod5);
    EXPECT_EQ(pre_error5, hooked_error5);
}
```



在"LoadLibraryExW_Prelog_dwFlags_test"中，我们编写测试代码来测试参数变量"dwFlags"，该变量是"LOADFLAGS"类型，即标记类型。在这里我们本应该比较R3Log中返回的标记字符串是否正确，但在例子代码中暂时忽略。

```C++
TEST_F(HookImplementTest, LoadLibraryExW_Prelog_dwFlags_test)
{
    HMODULE pre_hmod1, pre_hmod2, pre_hmod3, pre_hmod4, pre_hmod5;
    DWORD pre_error1, pre_error2, pre_error3, pre_error4, pre_error5;
    HMODULE hooked_hmod1, hooked_hmod2, hooked_hmod3, hooked_hmod4, hooked_hmod5;
    DWORD hooked_error1, hooked_error2, hooked_error3, hooked_error4, hooked_error5;
    
    // first call test API when DisableAllApis().
    ASSERT_TRUE(g_hook_test_object->DisableAllApis());
    // test when dwFlags is 0
    pre_hmod1 = LoadLibraryExW(L"ole32.dll", nullptr, 0);
    pre_error1 = GetLastError();
    // test when dwFlags is invalid flags
    pre_hmod2 = LoadLibraryExW(L"ole32.dll", nullptr, 0x800000);
    pre_error2 = GetLastError();
    // test when dwFlags is single flag.
    pre_hmod3 = LoadLibraryExW(L"ole32.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
    pre_error3 = GetLastError();
    // test when dwFlags is correct combination flags.
    pre_hmod4 = LoadLibraryExW(L"ole32.dll", nullptr, LOAD_LIBRARY_AS_DATAFILE | LOAD_LIBRARY_SEARCH_SYSTEM32 | LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR);
    pre_error4 = GetLastError();
    // test when dwFlags is error combination flags.
    pre_hmod5 = LoadLibraryExW(L"ole32.dll", nullptr, LOAD_LIBRARY_AS_DATAFILE | 0x84000);
    pre_error5 = GetLastError();
    
    // second call test API when EnableAllApis().
    ASSERT_TRUE(g_hook_test_object->EnableAllApis());
    // test when dwFlags is 0
    hooked_hmod1 = LoadLibraryExW(L"ole32.dll", nullptr, 0);
    hooked_error1 = GetLastError();
    // test when dwFlags is invalid flags
    hooked_hmod2 = LoadLibraryExW(L"ole32.dll", nullptr, 0x800000);
    hooked_error2 = GetLastError();
    // test when dwFlags is single flag.
    hooked_hmod3 = LoadLibraryExW(L"ole32.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
    hooked_error3 = GetLastError();
    // test when dwFlags is correct combination flags.
    hooked_hmod4 = LoadLibraryExW(L"ole32.dll", nullptr, LOAD_LIBRARY_AS_DATAFILE | LOAD_LIBRARY_SEARCH_SYSTEM32 | LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR);
    hooked_error4 = GetLastError();
    // test when dwFlags is error combination flags.
    hooked_hmod5 = LoadLibraryExW(L"ole32.dll", nullptr, LOAD_LIBRARY_AS_DATAFILE | 0x84000);
    hooked_error5 = GetLastError();
    
    // compare return result and error code.
    EXPECT_EQ(pre_hmod1, hooked_hmod1);
    EXPECT_EQ(pre_error1, hooked_error1);
    EXPECT_EQ(pre_hmod2, hooked_hmod2);
    EXPECT_EQ(pre_error2, hooked_error2);
    EXPECT_EQ(pre_hmod3, hooked_hmod3);
    EXPECT_EQ(pre_error3, hooked_error3);
    EXPECT_EQ(pre_hmod4, hooked_hmod4);
    EXPECT_EQ(pre_error4, hooked_error4);
    EXPECT_EQ(pre_hmod5, hooked_hmod5);
    EXPECT_EQ(pre_error5, hooked_error5);
    
    // test flags string if correct. coding ignore....
    ......
}
```



测试用例2：

```c++
"GetDiskFreeSpaceExW": {
    "Signature": {
      "Library": "kernel32.dll",
      "Return": "BOOL",
      "Feature": "H11"
    },

    "Parameters": {
      "lpDirectoryName": "LPCWSTR",
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

在本测试用例中有一个特殊字段需要处理，它是"PostCheck"。针对该字段，基本测试名称应为"GetDiskFreeSpaceExW_PostCheck_Check_lpDirectoryName"。因为"\*lpTotalNumberOfBytes"参数变量是一个返回值变量，所以这里不用对它进行过多的测试，可以把它和"\*lpTotalNumberOfBytes"参数变量的测试合并。



在"LoadLibraryExW_Prelog_lpLibFileName_test"中，我们编写测试代码来测试参数变量"lpLibFileName"和"\*lpTotalNumberOfBytes"，前者是"LPCWSTR"类型，即字符串指针类型。后者是"LPULARGE_INTEGER"类型。

```c++
TEST_F(HookImplementTest, GetDiskFreeSpaceExW_PostCheck_Check_lpDirectoryName)
{
    bool pre_bret, pre_bret1, pre_bret2, pre_bret3, pre_bret4, pre_bret5, pre_bret6;
    DWORD pre_error, pre_error1, pre_error2, pre_error3, pre_error4, pre_error5;
    bool hooked_bret, hooked_bret1, hooked_bret2, hooked_bret3, hooked_bret4, hooked_bret5, hooked_bret6;
    DWORD hooked_error, hooked_error1, hooked_error2, hooked_error3, hooked_error4, hooked_error5;
    
    ULARGE_INTEGER pre_free_bytes1, pre_free_bytes2, pre_free_bytes3, pre_free_bytes4, pre_free_bytes5, pre_free_bytes6;
    ULARGE_INTEGER hooked_free_bytes1, hooked_free_bytes2, hooked_free_bytes3, hooked_free_bytes4, hooked_free_bytes5, hooked_free_bytes6;
    ULARGE_INTEGER pre_total_bytes1, pre_total_bytes2, pre_total_bytes3, pre_total_bytes4, pre_total_bytes5, pre_total_bytes6;
    ULARGE_INTEGER hooked_total_bytes1, hooked_total_bytes2, hooked_total_bytes3, hooked_total_bytes4, hooked_total_bytes5, hooked_total_bytes6;
    ULARGE_INTEGER pre_total_free1, pre_total_free2, pre_total_free3, pre_total_free4, pre_total_free5, pre_total_free6;
    ULARGE_INTEGER hooked_total_free1, hooked_total_free2, hooked_total_free3, hooked_total_free4, hooked_total_free5, hooked_total_free6;
    
    // first call test API when DisableAllApis().
    ASSERT_TRUE(g_hook_test_object->DisableAllApis());
    // test when lpTotalNumberOfBytes is nullptr
    pre_bret = GetDiskFreeSpaceExW(L"C:\\Windows", nullptr, nullptr, nullptr);
    pre_error = GetLastError();
    // test when lpLibFileName is nullptr
    pre_bret1 = GetDiskFreeSpaceExW(nullptr, &pre_free_bytes1, &pre_total_bytes1, &pre_total_free1);
    pre_error1 = GetLastError();
    // test when lpLibFileName is "\0"
    pre_bret2 = GetDiskFreeSpaceExW(L"\0", &pre_free_bytes2, &pre_total_bytes2, &pre_total_free2);
    pre_error2 = GetLastError();
    // test when lpLibFileName is normal paths
    pre_bret3 = GetDiskFreeSpaceExW(L"D:\\work", &pre_free_bytes3, &pre_total_bytes3, &pre_total_free3);
    pre_error3 = GetLastError();
    // test when lpLibFileName is normal path that it hit expression "^C:.*".
    pre_bret4 = GetDiskFreeSpaceExW(L"c:\\windows", &pre_free_bytes4, &pre_total_bytes4, &pre_total_free4);
    pre_error4 = GetLastError();
    // test when lpLibFileName is invalid path and super long name but it hit expression "^C:.*".
    pre_bret5 = GetDiskFreeSpaceExW(L"c:\\windows\\system32......", &pre_free_bytes5, &pre_total_bytes5, &pre_total_free5);
    pre_error5 = GetLastError();
    // test when lpLibFileName is incorrect data block.
    pre_bret6 = GetDiskFreeSpaceExW(L"0x00,0x00,0xdd,0xdd......", &pre_free_bytes6, &pre_total_bytes6, &pre_total_free6);
    pre_error6 = GetLastError();
    
    // second call test API when EnableAllApis().
    ASSERT_TRUE(g_hook_test_object->EnableAllApis());
    // test when lpTotalNumberOfBytes is nullptr
    hooked_bret = GetDiskFreeSpaceExW(L"C:\\Windows", nullptr, nullptr, nullptr);
    hooked_error = GetLastError();
    // test when lpLibFileName is nullptr
    hooked_bret1 = GetDiskFreeSpaceExW(nullptr, &hooked_free_bytes1, &hooked_total_bytes1, &hooked_total_free1);
    hooked_error1 = GetLastError();
    // test when lpLibFileName is "\0"
    hooked_bret2 = GetDiskFreeSpaceExW(L"\0", &hooked_free_bytes2, &hooked_total_bytes2, &hooked_total_free2);
    hooked_error2 = GetLastError();
    // test when lpLibFileName is normal paths
    hooked_bret3 = GetDiskFreeSpaceExW(L"D:\\work", &hooked_free_bytes3, &hooked_total_bytes3, &hooked_total_free3);
    hooked_error3 = GetLastError();
    // test when lpLibFileName is normal path that it hit expression "^C:.*".
    hooked_bret4 = GetDiskFreeSpaceExW(L"c:\\windows", &hooked_free_bytes4, &hooked_total_bytes4, &hooked_total_free4);
    hooked_error4 = GetLastError();
    // test when lpLibFileName is invalid path and super long name but it hit expression "^C:.*".
    hooked_bret5 = GetDiskFreeSpaceExW(L"c:\\windows\\system32......", &hooked_free_bytes5, &hooked_total_bytes5, &hooked_total_free5);
    hooked_error5 = GetLastError();
    // test when lpLibFileName is incorrect data block.
    hooked_bret6 = GetDiskFreeSpaceExW(L"0x00,0x00,0xdd,0xdd......", &hooked_free_bytes6, &hooked_total_bytes6, &hooked_total_free6);
    hooked_error6 = GetLastError();
    
    // compare return result, error code and output parameters.
    EXPECT_EQ(pre_bret, hooked_bret);
    EXPECT_EQ(pre_error, hooked_error);
    
    EXPECT_EQ(hooked_bret1, hooked_bret1);
    EXPECT_EQ(pre_error1, hooked_error1);
    if(hooked_bret1 == true) {
        EXPECT_EQ(pre_free_bytes1.QuadPart, hooked_free_bytes1.QuadPart);
        if(pre_total_bytes1.QuadPart <= 64424509440)
            EXPECT_EQ(hooked_total_bytes1.QuadPart, 1099511627776);
        else
        	EXPECT_EQ(pre_total_bytes1.QuadPart, hooked_total_bytes1.QuadPart);
        EXPECT_EQ(pre_total_free1.QuadPart, hooked_total_free1.QuadPart);
    }
    
    EXPECT_EQ(pre_bret2, hooked_bret2);
    EXPECT_EQ(pre_error2, hooked_error2);
    if(pre_bret2 == true) {
        EXPECT_EQ(pre_free_bytes2.QuadPart, hooked_free_bytes2.QuadPart);
        EXPECT_EQ(pre_total_bytes2.QuadPart, hooked_total_bytes2.QuadPart);
        EXPECT_EQ(pre_total_free2.QuadPart, hooked_total_free2.QuadPart);
    }
    
    EXPECT_EQ(pre_bret3, hooked_bret3);
    EXPECT_EQ(pre_error3, hooked_error3);
    if(pre_bret3 == true) {
        EXPECT_EQ(pre_free_bytes2.QuadPart, hooked_free_bytes2.QuadPart);
        EXPECT_EQ(pre_total_bytes2.QuadPart, hooked_total_bytes2.QuadPart);
        EXPECT_EQ(pre_total_free2.QuadPart, hooked_total_free2.QuadPart);
    }
    
    EXPECT_EQ(pre_bret4, hooked_bret4);
    EXPECT_EQ(pre_error4, hooked_error4);
    if(pre_bret4 == true) {
        EXPECT_EQ(pre_free_bytes4.QuadPart, hooked_free_bytes4.QuadPart);
        if(pre_total_bytes4.QuadPart <= 64424509440)
            EXPECT_EQ(hooked_total_bytes4.QuadPart, 1099511627776);
        else
        	EXPECT_EQ(pre_total_bytes4.QuadPart, hooked_total_bytes4.QuadPart);
        EXPECT_EQ(pre_total_free4.QuadPart, hooked_total_free4.QuadPart);
    }
    
    EXPECT_EQ(pre_bret5, hooked_bret5);
    EXPECT_EQ(pre_error5, hooked_error5);
    ASSERT_FALSE(pre_bret5);
    
    EXPECT_EQ(pre_bret6, hooked_bret6);
    EXPECT_EQ(pre_error6, hooked_error6);
    ASSERT_FALSE(pre_bret6);
}
```





* **创建WMI钩子基本测试**

  待写....

------



### 数据类型处理模块的测试

待写....

------



### 语法和语义分析模块的测试

待写....

------



### R3日志系统模块的测试

待写....

------



### 异常处理模块的测试

待写....

------



## 关于单元测试的一些特殊说明

------

