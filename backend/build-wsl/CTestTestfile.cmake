# CMake generated Testfile for 
# Source directory: /mnt/e/CppTrainingProject/DatabaseCourseDesign/backend
# Build directory: /mnt/e/CppTrainingProject/DatabaseCourseDesign/backend/build-wsl
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(BackendInfraTests "/mnt/e/CppTrainingProject/DatabaseCourseDesign/backend/build-wsl/community_health_tests")
set_tests_properties(BackendInfraTests PROPERTIES  ENVIRONMENT "TEST_DB_HOST=127.0.0.1;TEST_DB_PORT=3306;TEST_DB_USER=ch_admin;TEST_DB_NAME=community_health" _BACKTRACE_TRIPLES "/mnt/e/CppTrainingProject/DatabaseCourseDesign/backend/CMakeLists.txt;313;add_test;/mnt/e/CppTrainingProject/DatabaseCourseDesign/backend/CMakeLists.txt;0;")
