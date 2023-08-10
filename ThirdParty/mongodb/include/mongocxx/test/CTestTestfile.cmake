# CMake generated Testfile for 
# Source directory: E:/github-project/ThirdParty/mongo-cxx-driver-r3.8.0/src/mongocxx/test
# Build directory: E:/github-project/ThirdParty/mongo-cxx-driver-r3.8.0/build/src/mongocxx/test
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(driver "test_driver")
set_tests_properties(driver PROPERTIES  ENVIRONMENT "CLIENT_SIDE_ENCRYPTION_TESTS_PATH=E:/github-project/ThirdParty/mongo-cxx-driver-r3.8.0/src/mongocxx/../../data/client_side_encryption;URI_OPTIONS_TESTS_PATH=E:/github-project/ThirdParty/mongo-cxx-driver-r3.8.0/src/mongocxx/../../data/uri-options" _BACKTRACE_TRIPLES "E:/github-project/ThirdParty/mongo-cxx-driver-r3.8.0/src/mongocxx/test/CMakeLists.txt;223;add_test;E:/github-project/ThirdParty/mongo-cxx-driver-r3.8.0/src/mongocxx/test/CMakeLists.txt;0;")
add_test(logging "test_logging")
set_tests_properties(logging PROPERTIES  _BACKTRACE_TRIPLES "E:/github-project/ThirdParty/mongo-cxx-driver-r3.8.0/src/mongocxx/test/CMakeLists.txt;224;add_test;E:/github-project/ThirdParty/mongo-cxx-driver-r3.8.0/src/mongocxx/test/CMakeLists.txt;0;")
add_test(instance "test_instance")
set_tests_properties(instance PROPERTIES  _BACKTRACE_TRIPLES "E:/github-project/ThirdParty/mongo-cxx-driver-r3.8.0/src/mongocxx/test/CMakeLists.txt;225;add_test;E:/github-project/ThirdParty/mongo-cxx-driver-r3.8.0/src/mongocxx/test/CMakeLists.txt;0;")
add_test(crud_specs "test_crud_specs")
set_tests_properties(crud_specs PROPERTIES  ENVIRONMENT "CRUD_LEGACY_TESTS_PATH=E:/github-project/ThirdParty/mongo-cxx-driver-r3.8.0/src/mongocxx/../../data/crud/legacy" _BACKTRACE_TRIPLES "E:/github-project/ThirdParty/mongo-cxx-driver-r3.8.0/src/mongocxx/test/CMakeLists.txt;226;add_test;E:/github-project/ThirdParty/mongo-cxx-driver-r3.8.0/src/mongocxx/test/CMakeLists.txt;0;")
add_test(gridfs_specs "test_gridfs_specs")
set_tests_properties(gridfs_specs PROPERTIES  ENVIRONMENT "GRIDFS_TESTS_PATH=E:/github-project/ThirdParty/mongo-cxx-driver-r3.8.0/src/mongocxx/../../data/gridfs" _BACKTRACE_TRIPLES "E:/github-project/ThirdParty/mongo-cxx-driver-r3.8.0/src/mongocxx/test/CMakeLists.txt;227;add_test;E:/github-project/ThirdParty/mongo-cxx-driver-r3.8.0/src/mongocxx/test/CMakeLists.txt;0;")
add_test(client_side_encryption_specs "test_client_side_encryption_specs")
set_tests_properties(client_side_encryption_specs PROPERTIES  ENVIRONMENT "CLIENT_SIDE_ENCRYPTION_LEGACY_TESTS_PATH=E:/github-project/ThirdParty/mongo-cxx-driver-r3.8.0/src/mongocxx/../../data/client_side_encryption/legacy" _BACKTRACE_TRIPLES "E:/github-project/ThirdParty/mongo-cxx-driver-r3.8.0/src/mongocxx/test/CMakeLists.txt;228;add_test;E:/github-project/ThirdParty/mongo-cxx-driver-r3.8.0/src/mongocxx/test/CMakeLists.txt;0;")
add_test(command_monitoring_specs "test_command_monitoring_specs")
set_tests_properties(command_monitoring_specs PROPERTIES  ENVIRONMENT "COMMAND_MONITORING_TESTS_PATH=E:/github-project/ThirdParty/mongo-cxx-driver-r3.8.0/src/mongocxx/../../data/command-monitoring" _BACKTRACE_TRIPLES "E:/github-project/ThirdParty/mongo-cxx-driver-r3.8.0/src/mongocxx/test/CMakeLists.txt;229;add_test;E:/github-project/ThirdParty/mongo-cxx-driver-r3.8.0/src/mongocxx/test/CMakeLists.txt;0;")
add_test(transactions_specs "test_transactions_specs")
set_tests_properties(transactions_specs PROPERTIES  ENVIRONMENT "TRANSACTIONS_TESTS_PATH=E:/github-project/ThirdParty/mongo-cxx-driver-r3.8.0/src/mongocxx/../../data/transactions;WITH_TRANSACTION_TESTS_PATH=E:/github-project/ThirdParty/mongo-cxx-driver-r3.8.0/src/mongocxx/../../data/with_transaction" _BACKTRACE_TRIPLES "E:/github-project/ThirdParty/mongo-cxx-driver-r3.8.0/src/mongocxx/test/CMakeLists.txt;230;add_test;E:/github-project/ThirdParty/mongo-cxx-driver-r3.8.0/src/mongocxx/test/CMakeLists.txt;0;")
add_test(retryable_reads_spec "test_retryable_reads_specs")
set_tests_properties(retryable_reads_spec PROPERTIES  ENVIRONMENT "RETRYABLE_READS_LEGACY_TESTS_PATH=E:/github-project/ThirdParty/mongo-cxx-driver-r3.8.0/src/mongocxx/../../data/retryable-reads/legacy" _BACKTRACE_TRIPLES "E:/github-project/ThirdParty/mongo-cxx-driver-r3.8.0/src/mongocxx/test/CMakeLists.txt;231;add_test;E:/github-project/ThirdParty/mongo-cxx-driver-r3.8.0/src/mongocxx/test/CMakeLists.txt;0;")
add_test(read_write_concern_specs "test_read_write_concern_specs")
set_tests_properties(read_write_concern_specs PROPERTIES  ENVIRONMENT "READ_WRITE_CONCERN_OPERATION_TESTS_PATH=E:/github-project/ThirdParty/mongo-cxx-driver-r3.8.0/src/mongocxx/../../data/read-write-concern/operation" _BACKTRACE_TRIPLES "E:/github-project/ThirdParty/mongo-cxx-driver-r3.8.0/src/mongocxx/test/CMakeLists.txt;232;add_test;E:/github-project/ThirdParty/mongo-cxx-driver-r3.8.0/src/mongocxx/test/CMakeLists.txt;0;")
add_test(unified_format_spec "test_unified_format_spec")
set_tests_properties(unified_format_spec PROPERTIES  ENVIRONMENT "CHANGE_STREAMS_UNIFIED_TESTS_PATH=E:/github-project/ThirdParty/mongo-cxx-driver-r3.8.0/src/mongocxx/../../data/change-streams/unified;CLIENT_SIDE_ENCRYPTION_UNIFIED_TESTS_PATH=E:/github-project/ThirdParty/mongo-cxx-driver-r3.8.0/src/mongocxx/../../data/client_side_encryption/unified;COLLECTION_MANAGEMENT_TESTS_PATH=E:/github-project/ThirdParty/mongo-cxx-driver-r3.8.0/src/mongocxx/../../data/collection-management;CRUD_UNIFIED_TESTS_PATH=E:/github-project/ThirdParty/mongo-cxx-driver-r3.8.0/src/mongocxx/../../data/crud/unified;SESSION_UNIFIED_TESTS_PATH=E:/github-project/ThirdParty/mongo-cxx-driver-r3.8.0/src/mongocxx/../../data/sessions/unified/;RETRYABLE_READS_UNIFIED_TESTS_PATH=E:/github-project/ThirdParty/mongo-cxx-driver-r3.8.0/src/mongocxx/../../data/retryable-reads/unified/;RETRYABLE_WRITES_UNIFIED_TESTS_PATH=E:/github-project/ThirdParty/mongo-cxx-driver-r3.8.0/src/mongocxx/../../data/retryable-writes/unified/;UNIFIED_FORMAT_TESTS_PATH=E:/github-project/ThirdParty/mongo-cxx-driver-r3.8.0/src/mongocxx/../../data/unified-format;VERSIONED_API_TESTS_PATH=E:/github-project/ThirdParty/mongo-cxx-driver-r3.8.0/src/mongocxx/../../data/versioned-api" _BACKTRACE_TRIPLES "E:/github-project/ThirdParty/mongo-cxx-driver-r3.8.0/src/mongocxx/test/CMakeLists.txt;233;add_test;E:/github-project/ThirdParty/mongo-cxx-driver-r3.8.0/src/mongocxx/test/CMakeLists.txt;0;")
add_test(versioned_api "test_versioned_api")
set_tests_properties(versioned_api PROPERTIES  _BACKTRACE_TRIPLES "E:/github-project/ThirdParty/mongo-cxx-driver-r3.8.0/src/mongocxx/test/CMakeLists.txt;234;add_test;E:/github-project/ThirdParty/mongo-cxx-driver-r3.8.0/src/mongocxx/test/CMakeLists.txt;0;")