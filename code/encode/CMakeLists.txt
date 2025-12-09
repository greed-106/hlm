# CMakeLists.txt for encode library

# Define the library target
add_library(hlmc
    hlmc_common.c
    hlmc_cu.c
    hlmc_dpb.c
    hlmc_ecd.c
    hlmc_hls.c
    hlmc_init.c
    hlmc_inter.c
    hlmc_intra.c
    hlmc_lib.c
    hlmc_patch.c
    hlmc_rc.c
    hlmc_tq.c
)

# Set target properties
set_target_properties(hlmc PROPERTIES
    OUTPUT_NAME hlmc
)

# Include directories
target_include_directories(hlmc PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../../code/common
)

# Link against common library
target_link_libraries(hlmc hlm_common)
if(UNIX)
    target_link_libraries(hlmc m)
endif()

# Compiler-specific definitions
target_compile_definitions(hlmc PUBLIC
    $<$<C_COMPILER_ID:GNU,Clang>:_LINUX64;_M_X64>
    $<$<C_COMPILER_ID:MSVC>:WIN64;_M_X64>
)

# Add SSE4.2 flag only to encode library
if(CMAKE_C_COMPILER_ID STREQUAL "GNU" OR CMAKE_C_COMPILER_ID STREQUAL "Clang")
    target_compile_options(hlmc PRIVATE -msse4.2)
endif()