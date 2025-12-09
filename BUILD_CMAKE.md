# HLM Video Codec - CMake Build System

This project has been converted from Makefile to CMake for better cross-platform support.

## Project Structure

The project consists of:
- **code/common**: Common code shared between encoder and decoder
- **code/encode**: HLM encoder library (hlmc)
- **code/decode**: HLM decoder library (hlmd) 
- **apps/demo_enc**: Encoder demo application
- **apps/demo_dec**: Decoder demo application

## Building with CMake

### Prerequisites
- CMake 3.10 or higher
- A C compiler (GCC, Clang, or MSVC)

### Build Steps

1. Create a build directory:
```bash
mkdir build
cd build
```

2. Configure the project:
```bash
cmake ..
# Or specify a generator on Windows:
# cmake .. -G "Visual Studio 16 2019"
```

3. Build the project:
```bash
cmake --build .
# Or on Unix-like systems:
# make
```

### Generated Targets

- `hlm_common`: Static library with common functions
- `hlmc`: Encoder static library
- `hlmd`: Decoder static library  
- `hlmc_demo`: Encoder demo application
- `hlmd_demo`: Decoder demo application

## Cross-Platform Support

This CMake build system supports:
- Linux (GCC/Clang)
- Windows (MSVC, MinGW)
- macOS (Clang)

The build flags are automatically set based on the platform:
- Optimization level: -O3
- Architecture: 64-bit
- The encoder library (hlmc) includes SSE4.2 support when available
- Platform-specific macros are defined automatically

## Compatibility with Original Build

This CMake system maintains compatibility with the original Makefile system:
- Same compilation flags
- Same library names (with platform-specific extensions)
- Same output locations
- Same dependencies