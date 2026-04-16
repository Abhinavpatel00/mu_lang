cmake_minimum_required(VERSION 3.16)
project(mucc C CXX)

if(NOT LLVM_DIR)
    set(LLVM_DIR "/usr/lib/llvm21/lib/cmake/llvm" CACHE PATH "LLVM 21 CMake config path")
endif()

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(LLVM REQUIRED CONFIG)
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

if(NOT LLVM_INCLUDE_DIRS)
    set(LLVM_INCLUDE_DIRS "/usr/lib/llvm21/include")
    include_directories(${LLVM_INCLUDE_DIRS})
endif()

if(NOT LLVM_LIBRARY_DIRS)
    set(LLVM_LIBRARY_DIRS "/usr/lib/llvm21/lib")
endif()

add_executable(mucc
    main.cpp
)

if(LLVM_LIBRARY_DIRS)
    target_link_directories(mucc PRIVATE ${LLVM_LIBRARY_DIRS})
endif()

if(TARGET LLVM)
    target_link_libraries(mucc LLVM)
else()
    target_link_libraries(mucc LLVM-21)
endif()
