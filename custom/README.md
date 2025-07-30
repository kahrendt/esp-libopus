# Custom ESP-IDF Integration Files

This folder contains custom implementations for ESP-IDF integration with the Opus library, including PSRAM-aware memory allocation and ESP32-S3 Xtensa LX7 optimizations.

## Files

### Memory Allocation (PSRAM Support)

#### custom_support.h
Provides PSRAM-aware memory allocation overrides for Opus:
- **Custom `opus_alloc()`**: Attempts to allocate from PSRAM first using `heap_caps_malloc(size, MALLOC_CAP_SPIRAM)`, falls back to internal RAM if PSRAM is unavailable
- **Custom `opus_alloc_scratch()`**: Handles thread-local pseudostack initialization and delegates to `opus_alloc()`
- **Custom `opus_free()`**: Uses `heap_caps_free()` for proper cleanup
- **`celt_fatal()`**: Error handler for pseudostack overflow conditions

#### stack_alloc.h
**Direct replacement** of the original Opus `celt/stack_alloc.h` file. This custom version:
- Implements thread-safe pseudostack using pthread thread-local storage
- Each thread gets its own 120KB pseudostack allocated from PSRAM when available
- Uses `pthread_key_create()` with automatic cleanup on thread exit
- Maintains compatibility with the original stack allocation API
- **Important**: This file is copied over the upstream `opus/celt/stack_alloc.h` during the build process (the original is backed up as `stack_alloc.h.original`)

#### Thread Safety Implementation

##### thread_local_stack.h / thread_local_stack.c
Thread-safe pseudostack implementation using POSIX pthread APIs:
- **Thread-local storage**: Each thread gets its own pseudostack (120KB default)
- **Automatic cleanup**: Memory is freed when threads are destroyed
- **PSRAM-aware**: Uses PSRAM when available, falls back to internal RAM
- **Cross-platform**: Uses standard pthread APIs for maximum portability

### ESP32-S3 Xtensa LX7 Optimizations

#### silk/ folder
Contains custom SILK codec headers with ESP32-S3 optimizations:

##### silk/macros.h and silk/SigProc_FIX.h
Custom versions of the SILK headers that conditionally include Xtensa LX7 optimizations for ESP32-S3.

##### silk/xtensa/ folder
Contains Xtensa LX7 assembly optimizations cherry-picked from upstream Opus (PR https://github.com/xiph/opus/pull/400):
- `macros_lx7.h` - Optimized SILK macros using MULSH instruction
- `SigProc_FIX_lx7.h` - Optimized SAT16 saturation using CLAMPS instruction

These optimizations provide ~17% performance improvement for SILK encoding on ESP32-S3 but are not included in Opus v1.5.2.

## Implementation Strategy

### Header Override Mechanism
The custom files override the standard Opus headers through different mechanisms:

1. **For stack_alloc.h**: The custom version is copied over the original `opus/celt/stack_alloc.h` during the build process (CMake backs up the original as `stack_alloc.h.original`)
2. **For SILK headers**: Custom versions override the original SILK headers for ESP32-S3 builds through CMake include path priority
3. **For memory allocation**: Custom support is injected via preprocessor definitions and include path priority

This approach modifies the upstream Opus behavior at compilation time. The stack_alloc.h file requires direct replacement because it's included by other Opus headers in ways that prevent clean override through include paths.

### Memory Allocation Benefits
- **PSRAM utilization**: All Opus allocations (encoder/decoder creation, pseudostack, internal structures) prefer PSRAM
- **Automatic fallback**: Graceful degradation to internal RAM when PSRAM is unavailable
- **Thread safety**: Each thread gets its own pseudostack, enabling concurrent Opus operations
- **Reduced stack usage**: Task stacks can be smaller (4-8KB typically sufficient)
- **Memory efficiency**: Single global allocation per thread reduces fragmentation

### Thread Safety Features
- **Per-thread pseudostack**: Each FreeRTOS task gets its own 120KB pseudostack
- **Automatic initialization**: Pseudostack is allocated on first Opus API call per thread
- **Automatic cleanup**: Memory is freed when tasks are deleted (requires `CONFIG_FREERTOS_TLSP_DELETION_CALLBACKS`, which is enabled by default)
- **No configuration needed**: Thread safety is built-on on top of the automatically enabled Opus library's `NONTHREADSAFE_PSEUDOSTACK` memory management mode

## Implementation Notes

- ESP32-S3 optimizations are automatically enabled for ESP32-S3 builds via `OPUS_XTENSA_LX7` define
- SILK headers are overridden through CMake include path priority
- The stack_alloc.h file is directly replaced during the build process (original backed up)
- Other upstream Opus source files remain unmodified
- Memory allocation changes affect all ESP32 variants, optimizations only affect ESP32-S3
- Thread safety works with all ESP32 variants and requires no additional configuration