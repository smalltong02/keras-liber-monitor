# 第二节 Root定义

Json格式的根文件是hipshook.json，它的格式如下：

```C++
{
    "Name": "hipshook",
    "Version": "1.0.0.0",
    "CreateDate": "24//12//2019 2:24PM",
    "Description": "Creating hooks and signature format config information head file",
    "Sigs": [
        104,
        105,
        108
    ],
    "Coms": [
        106
    ],
    "Flags": [
        107
    ],
    "Wmis": [
        109
    ]
}
```



* Name字段定义了根文件的内部名字。
* Version字段定义了HipsHook项目当前的release版本号。
* CreateDate字段定义了release版本的日期。
* Description字段定义了一个简单的描述。
* Sigs字段是一个数组，定义了资源文件中包含API钩子的Json文件资源号码组。
* Flags字段是一个数组，定义了资源文件中包含类型定义信息的Json文件资源号码组。
* Wmis字段是一个数组，定义了资源文件中包含WMI钩子信息的Json文件资源号码组。
* Coms字段是一个数组，定义了资源文件中包含COM钩子信息的Json文件资源号码组。



------

