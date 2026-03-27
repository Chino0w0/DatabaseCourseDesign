# CMake generated Testfile for 
# Source directory: /home/runner/work/DatabaseCourseDesign/DatabaseCourseDesign/backend
# Build directory: /home/runner/work/DatabaseCourseDesign/DatabaseCourseDesign/backend/build
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(BackendInfraTests "/home/runner/work/DatabaseCourseDesign/DatabaseCourseDesign/backend/build/community_health_tests")
set_tests_properties(BackendInfraTests PROPERTIES  ENVIRONMENT "TEST_DB_HOST=127.0.0.1;TEST_DB_PORT=3306;TEST_DB_USER=ch_admin;TEST_DB_NAME=community_health" _BACKTRACE_TRIPLES "/home/runner/work/DatabaseCourseDesign/DatabaseCourseDesign/backend/CMakeLists.txt;104;add_test;/home/runner/work/DatabaseCourseDesign/DatabaseCourseDesign/backend/CMakeLists.txt;0;")
subdirs("_deps/httplib-build")
subdirs("_deps/nlohmann_json-build")
