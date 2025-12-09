# CMakeLists.txt for common library

# Define the library target
add_library(hlm_common
    hlm_com_iqt.c
    hlm_com_pred.c
    hlm_common.c
    hlm_mem.c
    hlm_nbi.c
)

# Set target properties
set_target_properties(hlm_common PROPERTIES
    OUTPUT_NAME hlm_common
)

# Include directories
target_include_directories(hlm_common PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Compiler-specific definitions
target_compile_definitions(hlm_common PUBLIC
    $<$<C_COMPILER_ID:GNU,Clang>:_LINUX64;_M_X64>
    $<$<C_COMPILER_ID:MSVC>:WIN64;_M_X64>
)

# Link libraries
if(UNIX)
    target_link_libraries(hlm_common m)
endif()