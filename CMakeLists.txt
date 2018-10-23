CMAKE_MINIMUM_REQUIRED(VERSION 3.8)
PROJECT(executor)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(AddCXXCompilerFlag)

if (EXISTS "${CMAKE_BINARY_DIR}/conanbuildinfo.cmake")
   set(WITH_CONAN TRUE)
endif()

if (WITH_CONAN)
   include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
   conan_basic_setup(TARGETS)
endif()

add_cxx_compiler_flag(-std=c++14)
add_cxx_compiler_flag(-Wall)
# add_cxx_compiler_flag(-Wextra)
# add_cxx_compiler_flag(-Wshadow)
# add_cxx_compiler_flag(-Werror RELEASE)
# add_cxx_compiler_flag(-Werror RELWITHDEBINFO)
# add_cxx_compiler_flag(-Werror MINSIZEREL)
# add_cxx_compiler_flag(-pedantic)
# add_cxx_compiler_flag(-pedantic-errors)
add_cxx_compiler_flag(-fstrict-aliasing)
add_cxx_compiler_flag(-Wthread-safety)
add_cxx_compiler_flag(-O3 RELEASE)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
   add_cxx_compiler_flag(-stdlib=libc++)
endif ()

ADD_EXECUTABLE(sample src/sample.cpp)

target_include_directories(sample PUBLIC include)

if (NOT WITH_CONAN)
    FIND_PACKAGE(GTest)
    FIND_PACKAGE(GMock)
endif ()

IF (WITH_CONAN OR (GTEST_FOUND AND GMOCK_FOUND))
   ENABLE_TESTING()

   ADD_EXECUTABLE(execution-Tests test/ExecutionTest.cpp)
   target_include_directories(execution-Tests PRIVATE include)

   IF (WITH_CONAN)
      TARGET_LINK_LIBRARIES(execution-Tests PRIVATE CONAN_PKG::gtest)
   ELSE ()
      TARGET_INCLUDE_DIRECTORIES(execution-Tests PRIVATE ${GTEST_INCLUDE_DIR})
      TARGET_LINK_LIBRARIES(execution-Tests PRIVATE ${GTEST_BOTH_LIBRARIES})
   ENDIF ()

   ADD_TEST(execution-Tests ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/execution-Tests)
   IF (UNIX)
      SET_TESTS_PROPERTIES(execution-Tests PROPERTIES ENVIRONMENT "LD_LIBRARY_PATH=${CMAKE_LIBRARY_OUTPUT_DIRECTORY}:$ENV{LD_LIBRARY_PATH}")
   ENDIF()

ENDIF()

