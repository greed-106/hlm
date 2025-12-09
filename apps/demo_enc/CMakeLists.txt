# CMakeLists.txt for encoder demo application

# Define the executable target
add_executable(hlmc_demo
    hlmc_demo.c
    hlmc_case_cmd.c
    hlmc_case_lib.c
)

# Include directories
target_include_directories(hlmc_demo PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../../code/common
    ${CMAKE_CURRENT_SOURCE_DIR}/../../code/encode
)

# Link libraries
target_link_libraries(hlmc_demo hlmc hlm_common)
if(UNIX)
    target_link_libraries(hlmc_demo m)
endif()

# Compiler-specific definitions
target_compile_definitions(hlmc_demo PRIVATE
    $<$<C_COMPILER_ID:GNU,Clang>:_LINUX64;_M_X64>
    $<$<C_COMPILER_ID:MSVC>:WIN64;_M_X64>
)