cmake_minimum_required(VERSION 3.0)
project(arvin)

set(CMAKE_VERBOSE_MAKEFILE ON) 
set(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} -rdynamic -O0 -g -std=c++11 -Wall -Wno-deprecated -Werror -Wno-unused-function")
set(CMAKE_BUILD_TYPE Debug)
set(LIB_SRC
    src/log.cc
    src/util.cc
    )

add_library(arvin SHARED ${LIB_SRC})
#add_library(sylar_static STATIC ${LIB_SRC})
#SET_TARGET_PROPERTIES(sylar_static PROPERTIES OUTPUT_NAME "sylar")

add_executable(test tests/test01.cc)
add_dependencies(test arvin)
target_link_libraries(test arvin)

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

