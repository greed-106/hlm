# CMakeLists.txt for decoder demo application

# Define the executable target
add_executable(hlmd_demo
    hlmd_demo.c
)

# Include directories
target_include_directories(hlmd_demo PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../../code/common
    ${CMAKE_CURRENT_SOURCE_DIR}/../../code/decode
)

# Link libraries
target_link_libraries(hlmd_demo hlmd hlm_common)
if(UNIX)
    target_link_libraries(hlmd_demo m)
endif()

# Compiler-specific definitions
target_compile_definitions(hlmd_demo PRIVATE
    $<$<C_COMPILER_ID:GNU,Clang>:_LINUX64;_M_X64>
    $<$<C_COMPILER_ID:MSVC>:WIN64;_M_X64>
)