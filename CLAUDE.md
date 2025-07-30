# CLAUDE.md - esp-libopus Component Development

This file documents the work done to create esp-libopus, an ESP-IDF component wrapper for the Opus audio codec library.

## Overview

Created esp-libopus, a clean ESP-IDF component that:
- Uses the upstream Opus repository as a git submodule (no modifications to upstream code)
- Uses PSRAM-aware memory allocation for temporary buffers (via NONTHREADSAFE_PSEUDOSTACK)
- Supports all ESP32 variants with platform-specific optimizations
- Maintains full compatibility with the Opus library

## Current Structure

```
esp-libopus/
├── opus/                    # Git submodule pointing to https://github.com/xiph/opus.git
├── custom/                  # Custom implementations and optimizations
│   ├── custom_support.h    # PSRAM-aware memory allocation overrides
│   ├── silk/               # SILK codec optimizations
│   │   ├── macros.h       # Override with Xtensa optimizations
│   │   ├── SigProc_FIX.h  # Override with Xtensa optimizations
│   │   └── xtensa/        # LX7-specific optimization headers
│   └── README.md          # Documentation for optimizations
├── examples/               # Example ESP-IDF projects
│   └── simple_decoder/    # Basic decoder example
├── CMakeLists.txt         # ESP-IDF component configuration
├── Kconfig                # menuconfig options
├── idf_component.yml      # Component registry metadata
├── README.md              # Main component documentation
├── CLAUDE.md              # Development history and technical details
└── .gitignore             # Git ignore patterns
```

## Key Features Implemented

### 1. Git Submodule Integration
- Added upstream Opus repository as submodule in `opus/` folder
- No modifications to upstream code - all customizations in separate files
- Clean separation between upstream and ESP-IDF-specific code

### 2. Memory Allocation
- Uses `NONTHREADSAFE_PSEUDOSTACK` mode by default
- Custom `opus_alloc` and `opus_alloc_scratch` implementations that:
  - Allocates pseudostack from PSRAM when available
  - Falls back to internal RAM if PSRAM allocation fails
- Reduces task stack requirements (4-8KB typically sufficient)
- Single global allocation reduces memory fragmentation

### 3. Platform-Specific Builds
- **ESP32**: Fixed-point implementation (Xtensa LX6)
- **ESP32-S3**: Floating-point with FPU optimizations (Xtensa LX7)
- Automatic selection based on target platform

### 4. ESP32-S3 Xtensa LX7 Optimizations
- Cherry-picked assembly optimizations from upstream PR #400
- Provides ~17% performance improvement for SILK codec on ESP32-S3
- Automatically enabled for ESP32-S3 builds

### 5. Configuration System
- Integrated with ESP-IDF's Kconfig system
- menuconfig options for:
  - Floating-point implementation option for ESP32-S3

## Technical Implementation Details

### ESP32-S3 Optimization Override
For ESP32-S3, CMakeLists.txt includes `custom/silk` directory before standard SILK headers to enable Xtensa LX7 optimizations.

### Build Configuration
```cmake
# Opus configuration with PSRAM-aware pseudostack
target_compile_definitions(${COMPONENT_LIB} PRIVATE
    NONTHREADSAFE_PSEUDOSTACK
    CUSTOM_SUPPORT
    HAVE_CONFIG_H
    OPUS_BUILD
    # Platform-specific defines...
)
```

### Memory Allocation Details
The component uses a custom implementation of `opus_alloc_scratch()` defined in `custom/custom_support.h`:
- Attempts to allocate the pseudostack from PSRAM using `heap_caps_malloc(size, MALLOC_CAP_SPIRAM)`
- Falls back to internal RAM if PSRAM is unavailable or allocation fails
- Also overrides `opus_free()` to use `heap_caps_free()` for proper cleanup

## Usage Instructions

### Adding to an ESP-IDF Project
```bash
cd your-project/components
git clone --recursive <repo-url> opus
cd your-project
idf.py menuconfig  # Configure under "Component config → Opus Audio Codec"
```

### Configuration Options
1. Open menuconfig: `idf.py menuconfig`
2. Navigate to: Component config → Opus Audio Codec
3. Configure build options:
   - **Floating-point**: Enable for ESP32-S3 (better quality, uses FPU)

### Example Code
```c
#include "opus.h"

void decode_opus_frame(const uint8_t *data, int len) {
    OpusDecoder *decoder = opus_decoder_create(48000, 2, NULL);
    int16_t pcm[960 * 2];

    int samples = opus_decode(decoder, data, len, pcm, 960, 0);
    // Process PCM samples...

    opus_decoder_destroy(decoder);
}
```

## Benefits of This Approach

1. **Clean Integration**: No modifications to upstream Opus code
2. **Maintainability**: Easy to update Opus version via submodule
3. **PSRAM Support**: Automatically uses PSRAM for temporary buffers when available
4. **ESP32 Optimized**: Platform-specific builds with appropriate optimizations
5. **Memory Efficient**: Single global allocation reduces fragmentation

## Testing

The component has been tested with:
- ESP32 (fixed-point build)
- ESP32-S3 (floating-point build with FPU)
- PSRAM and non-PSRAM configurations

## Recent Updates

### Simplified Configuration
- Uses standard Opus configuration with minimal ESP-IDF-specific changes
- Simple configuration options via menuconfig or manual defines:
  - `CONFIG_OPUS_USE_FLOATING_POINT` - Enables floating-point on ESP32-S3
- PSRAM-aware memory allocation via custom `opus_alloc` implementation

### ESP32-S3 Xtensa LX7 Optimizations
- Added support for Xtensa LX7 assembly optimizations from upstream Opus repository
- Since these optimizations are not included in Opus v1.5.2, we cherry-picked them from PR #400 (https://github.com/xiph/opus/pull/400):
  - `SigProc_FIX_lx7.h` - Optimized SAT16 saturation using LX7's CLAMPS instruction
  - `macros_lx7.h` - Optimized SILK macros using LX7's MULSH instruction
- Integration approach:
  - Optimization headers placed in `custom/silk/xtensa/`
  - Created custom versions of `macros.h` and `SigProc_FIX.h` that include the Xtensa headers
  - CMake configuration ensures custom headers override the original SILK headers
- Automatically enabled for ESP32-S3 builds via `OPUS_XTENSA_LX7` define

### Floating-Point Approximations (FLOAT_APPROX)
- Enabled `FLOAT_APPROX` for ESP32-S3 floating-point builds
- Provides fast approximations for mathematical functions:
  - `celt_log2()` - Fast base-2 logarithm using bit manipulation and polynomial approximation
  - `celt_exp2()` - Fast base-2 exponential using Remez polynomial approximation
  - `celt_isnan()` - Fast NaN detection using IEEE 754 bit patterns
- Safe to use with `-ffast-math` compiler flag (required for Opus with fast-math)
- Performance improvement with minimal accuracy loss suitable for audio processing
- Automatically enabled when using floating-point mode on ESP32-S3

### File Organization
- ESP32-S3 Xtensa optimizations in `custom/silk/` directory
- Clean separation between upstream code and platform optimizations
- Minimal custom code for maximum maintainability

### PSRAM-Aware Memory Allocation (Latest)
- Switched from `USE_ALLOCA` to `NONTHREADSAFE_PSEUDOSTACK` mode
- Added custom memory allocation overrides in `custom/custom_support.h`:
  - Overrides `opus_alloc` to try PSRAM first using `heap_caps_malloc(size, MALLOC_CAP_SPIRAM)`
  - Falls back to internal RAM if PSRAM is unavailable or allocation fails
  - Overrides `opus_alloc_scratch` to handle thread-local storage initialization and delegate to `opus_alloc`
  - Also overrides `opus_free` to use `heap_caps_free` for proper cleanup
  - Provides `celt_fatal` function for pseudostack overflow handling
- Implementation details:
  - Custom headers included before Opus headers via CMake include directories
  - Pseudostack variables (`scratch_ptr`, `global_stack`) managed by `stack_alloc.h`
  - Default stack size is 120KB (defined in `arch.h`)
- Benefits:
  - No modifications to upstream Opus code
  - ALL Opus allocations (encoder/decoder creation, pseudostack, internal structures) prefer PSRAM
  - Automatic PSRAM usage when available with graceful fallback to internal RAM
  - Reduced task stack requirements (4-8KB typically sufficient)
  - Single global allocation reduces memory fragmentation
- Limitations:
  - Global pseudostack shared across all Opus operations
- Kept ESP32-S3 Xtensa LX7 optimizations in `custom/silk/` directory

## Troubleshooting

### Common Issues with PSRAM Implementation

1. **Pseudostack overflow errors**
   - Symptom: "FATAL ERROR: pseudostack overflow" message
   - Solution: Increase `GLOBAL_STACK_SIZE` in your build (default is 120KB)
   - Can be done by adding `-DGLOBAL_STACK_SIZE=200000` to compile flags

2. **PSRAM not being used**
   - Check if PSRAM is properly initialized in your application
   - Verify with `heap_caps_get_free_size(MALLOC_CAP_SPIRAM)`
   - The component will automatically fall back to internal RAM if PSRAM is unavailable

3. **Thread safety issues**
   - Use separate Opus encoder/decoder instances in different tasks

4. **Build errors with custom memory allocation**
   - Ensure `custom` directory is included before Opus headers in CMakeLists.txt
   - Check that `CUSTOM_SUPPORT` and `NONTHREADSAFE_PSEUDOSTACK` are defined

## Thread-Safe Pseudostack Implementation

### Background
The default `NONTHREADSAFE_PSEUDOSTACK` mode uses global variables (`scratch_ptr` and `global_stack`) which are not thread-safe. This prevents concurrent use of Opus from multiple FreeRTOS tasks.

### Implementation Details (Updated to use pthread TLS)
- Uses POSIX pthread thread-local storage APIs for maximum portability
- Each thread gets its own pseudostack allocation (120KB by default)
- Thread-local storage structure tracks:
  - `scratch_ptr` - Current position in the pseudostack
  - `global_stack` - Base pointer of the pseudostack
  - `initialized` - Whether this thread has initialized its pseudostack
  - `owner_task` - TaskHandle for compatibility
- Uses `pthread_key_create()` with destructor for automatic cleanup
- `pthread_once()` ensures thread-safe one-time initialization

### Usage
1. Thread-safe pseudostack is automatically enabled - no configuration needed
2. Each thread automatically gets its own pseudostack on first Opus use
3. Cleanup happens automatically when tasks are deleted (no manual cleanup needed)
4. Memory overhead: 120KB per thread using Opus

### Default Behavior
- Thread-safe mode is always enabled when using NONTHREADSAFE_PSEUDOSTACK mode
- Each thread automatically gets its own pseudostack (120KB) on first use
- Works with ESP-IDF's pthread implementation (enabled by default)
- No configuration option needed - thread safety is built-in

### Advantages of pthread TLS
- Standard POSIX API - maximum portability
- Automatic cleanup on thread exit - no memory leaks
- No configuration dependencies (pthread is always available)
- Thread-safe initialization with pthread_once
- Works with any threading model, not just FreeRTOS tasks

### Configuration Requirements
- `CONFIG_FREERTOS_TLSP_DELETION_CALLBACKS` is automatically enabled by the component
- This is required for proper pthread thread-local storage cleanup with pseudostack mode
- Prevents memory leaks when FreeRTOS tasks using Opus are deleted

### Files Added/Modified
- `custom/thread_local_stack.h` - Thread-local storage API
- `custom/thread_local_stack.c` - Implementation with cleanup tracking
- `custom/stack_alloc.h` - Override of original stack_alloc.h for thread-local access
- `examples/thread_safe_test/` - Example demonstrating concurrent Opus usage

### Stack Allocation Override Method
Due to the way Opus headers include stack_alloc.h from other headers (like cwrs.h), we must overwrite the original file:
- CMakeLists.txt backs up the original stack_alloc.h to stack_alloc.h.original on first build
- Copies our custom stack_alloc.h over the original in the submodule
- Both modified files are added to .gitignore to keep git status clean
- This is the only reliable way to ensure all code uses our thread-local version

## Future Enhancements

Potential improvements identified:
- SIMD optimizations for critical functions (using ESP32-S3 vector instructions)
- Integration with ESP-DSP library for optimized DSP operations
- Performance profiling and optimization for specific use cases
- Additional examples demonstrating encoder and streaming use cases
- Automatic thread cleanup using FreeRTOS task deletion hooks
- Configurable pseudostack size per thread
- Memory pool for pseudostack allocations
