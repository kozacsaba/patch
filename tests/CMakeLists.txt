# Tests ########################################################################

message(STATUS "### Configuring Tests ###")

project(TestBench)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

enable_testing()

add_executable(${PROJECT_NAME})

target_include_directories(${PROJECT_NAME} PUBLIC
    ${CMAKE_SOURCE_DIR}/tests
)   

target_sources(${PROJECT_NAME} PUBLIC
    TestRunner.cc
)

target_link_libraries(${PROJECT_NAME} PUBLIC 
    Patch
    GTest::gtest
)

include(GoogleTest)
gtest_discover_tests(${PROJECT_NAME})
