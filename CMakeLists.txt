cmake_minimum_required(VERSION 3.10)

# Project name and version
project(hlm VERSION 1.0 LANGUAGES C)

# Set C standard
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Set output directories
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Compiler-specific options
if(CMAKE_C_COMPILER_ID STREQUAL "GNU" OR CMAKE_C_COMPILER_ID STREQUAL "Clang")
    # GCC or Clang options
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3 -m64 -D_M_X64")
    if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D_LINUX64")
    endif()

    # Add SSE4.2 support if available (encoder only)
    include(CheckCCompilerFlag)
    check_c_compiler_flag("-msse4.2" COMPILER_SUPPORTS_SSE42)
    if(COMPILER_SUPPORTS_SSE42)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -msse4.2")
    endif()

elseif(MSVC)
    # MSVC options
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /DWIN64 /D_M_X64 /D_CRT_SECURE_NO_WARNINGS")
endif()

# Add -fPIC flag for static libraries
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Add subdirectories
add_subdirectory(code/common)
add_subdirectory(code/encode)
add_subdirectory(code/decode)
add_subdirectory(apps/demo_enc)
add_subdirectory(apps/demo_dec)