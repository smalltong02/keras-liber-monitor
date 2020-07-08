# 第一节 介绍

测试是HipsHook项目研发当中最重要的一个环节。目前测试包含两个部分，第一个部分是基于google test的单元测试环节，第二个部分是基于google benchmark test的性能和压力测试环节。



文档不包含有关于google test和benchmark test的安装和使用教程，如果你对相关内容感兴趣，请访问以下链接进行查询:

1. [google test官方文档](https://github.com/google/googletest)

2. [benchmark test官方文档](https://github.com/google/benchmark)

在具体介绍这两个测试环节之前，请先熟记下面的**重要说明**。



## 重要说明

在提交代码时请遵循以下规则：

* 开发过程中每一次提交代码
  * 在提交代码之前，必须运行google单元测试x32和x64 release版本并保证测试***能够顺利通过***。
  * 在提交代码之前，必须运行Benchmark性能测试***Level 0***的x32和x64 release版本，保证测试能够顺利通过并确保性能损失***不超过3%***。

    

* Release阶段每一次提交代码
  * 在提交代码之前，必须运行google单元测试x32和x64 release版本并保证测试***能够顺利通过***。
  * 在提交代码之前，必须运行Benchmark性能测试***Level 1***的x32和x64 release版本，保证测试能够顺利通过并确保性能损失***不超过3%***。
  * 在测试服务器上运行sample目录下精选的sample样本集，保证sample样本集顺利通过测试。



* Release最后发布的版本必须保证
  * 运行google单元测试x32和x64 release版本并保证测试***能够顺利通过***。
  * 运行Benchmark性能测试***Level 2***的x32和x64 release版本，保证测试能够顺利通过并确保性能损失***不超过5%***。
  * 在测试服务器上运行sample目录下所有的sample样本集，保证sample样本集在新的release版本下顺利通过测试。



------

