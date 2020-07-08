# 3.1 Introduction

Testing is a extremely important phase in the developing process of HipsHook. Currently, there are two sections for testing, one is the unit test based on google test, and the other one is the benchmard and stress test based on google benchmark test.

This document does not include any tutorials regarding google test or google benchmark test. If you are interested in these topics, please visit the folloing links:

1. [google test](https://github.com/google/googletest)

2. [google benchmark test](https://github.com/google/benchmark)

Please read and take the **important rules** below seriously before you start adding any test or code in HipsHook.



## Important Rules

Please follow the following rules before you commit any code:

* Every commit in the develop stage:
  * You must run the unit tests under x86 and x64 releases platform, and ensure that ***every test passes***
  * You must run the ***Level 0*** benchmark tests under x86 and x64 release platform, and ensure that ***every test passes*** and performance loss does ***not exceed 3%***

    

* Every commit in the release stage:
  * You must run the unit tests under x86 and x64 releases platform, and ensure that ***every test passes***
  * You must run the ***Level 1*** benchmark tests under x86 and x64 release platform, and ensure that ***every test passes*** and performance loss does ***not exceed 3%***
  * You must test with samples under the special sample directory, on the testing server, and ensure that ***every sample passes the test***



* Every released version must:
  * Passes all the unit tests under x86 and x64 releases platform
  * Passes all the ***Level 2*** benchmark tests under x86 and x64 release platform, and the performance loss does ***not exceed 5%***
  * Be tested with samples under the special sample directory, on the testing server, and ensure that ***every sample passes the test***



------

