# Main ########################################################################

cmake_minimum_required(VERSION 3.22)

set(PROJECT_ROOT ${CMAKE_CURRENT_LIST_DIR})
set(PREDEF_PROJECT_NAME "Patch")
set(PREDEF_PROJECT_VERSION 0.1)

project("${PREDEF_PROJECT_NAME}_Project" VERSION ${PREDEF_PROJECT_VERSION})

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)


add_subdirectory(submodules/juce)
add_subdirectory(submodules/gtest)
add_subdirectory(source)
add_subdirectory(tests)
