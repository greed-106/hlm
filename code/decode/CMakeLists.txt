# CMakeLists.txt for decode library

# Define the library target
add_library(hlmd
    hlmd_cu.c
    hlmd_dpb.c
    hlmd_ecd.c
    hlmd_hls.c
    hlmd_init.c
    hlmd_inter.c
    hlmd_intra.c
    hlmd_lib.c
    hlmd_patch.c
    hlmd_tq.c
)

# Set target properties
set_target_properties(hlmd PROPERTIES
    OUTPUT_NAME hlmd
)

# Include directories
target_include_directories(hlmd PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../../code/common
)

# Link against common library
target_link_libraries(hlmd hlm_common)
if(UNIX)
    target_link_libraries(hlmd m)
endif()

# Compiler-specific definitions
target_compile_definitions(hlmd PUBLIC
    $<$<C_COMPILER_ID:GNU,Clang>:_LINUX64;_M_X64>
    $<$<C_COMPILER_ID:MSVC>:WIN64;_M_X64>
)