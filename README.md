# Please use [microOpus](https://github.com/esphome-libs/micro-opus) going forward!

# esp-libopus - ESP-IDF Opus Component

This component provides the Opus audio codec library as an ESP-IDF component, making it easy to integrate high-quality audio compression into ESP32 projects.

**This is still in the very early stages and shared to primarily test out the resonate protocol. It needs more cleaning up and optimization work. The documentation is primarily AI generated and hasn't been reviewed.**

## Features

- **Full Opus functionality**: Both encoder and decoder always included
- **Platform-specific optimization**:
  - ESP32: Fixed-point implementation
  - ESP32-S3: Fixed-point by default, floating-point optional
  - ESP32-C3/C6: Fixed-point implementation
- **PSRAM-aware memory allocation**: Uses PSRAM for temporary buffers when available, falls back to internal RAM
- **Thread-safe**: Each thread using Opus gets its own 120KB pseudostack preallocation, enabling concurrent usage with minimal task stack requirements (4-8KB)
- **Minimal configuration**: Works out-of-the-box with sensible defaults
- **Full Opus API**: Access to encoding, decoding, and multistream functionality

## Supported Targets

| Target | Default Implementation | Optional | Notes |
|--------|----------------------|----------|-------|
| ESP32 | Fixed-point |
| ESP32-S3 | Fixed-point | Floating-point | Can use hardware FPU |
| ESP32-C3 | Fixed-point |
| ESP32-C6 | Fixed-point |

## Installation

### Method 1: Add as Git Submodule

```bash
cd your-esp-project
mkdir -p components
cd components
git submodule add https://github.com/your-repo/esp-libopus.git opus
```

### Method 2: Clone Directly

```bash
cd your-esp-project/components
git clone https://github.com/your-repo/esp-libopus.git opus
```

## Configuration

Configure the component using menuconfig:

```bash
idf.py menuconfig
# Navigate to: Component config → Opus Audio Codec
```

### Configuration Options

#### ESP32-S3 Specific:
- **Enable floating-point implementation** - Uses hardware FPU (better quality, more memory)


## Usage Example

### Basic Decoding

```c
#include "opus.h"
#include "esp_log.h"

static const char *TAG = "opus_example";

void opus_decode_example(const uint8_t *opus_data, int opus_bytes) {
    int error;

    // Create decoder (48kHz, stereo)
    OpusDecoder *decoder = opus_decoder_create(48000, 2, &error);
    if (error != OPUS_OK) {
        ESP_LOGE(TAG, "Failed to create decoder: %s", opus_strerror(error));
        return;
    }

    // Decode frame
    int16_t pcm[960 * 2]; // 20ms @ 48kHz, stereo
    int samples = opus_decode(decoder, opus_data, opus_bytes, pcm, 960, 0);

    if (samples < 0) {
        ESP_LOGE(TAG, "Decode error: %s", opus_strerror(samples));
    } else {
        ESP_LOGI(TAG, "Decoded %d samples", samples);
        // Process PCM audio...
    }

    opus_decoder_destroy(decoder);
}
```


### Basic Encoding

```c
#include "opus.h"
#include "esp_log.h"

static const char *TAG = "opus_example";

void opus_encode_example(const int16_t *pcm_data, int frame_size) {
    int error;

    // Create encoder (48kHz, stereo)
    OpusEncoder *encoder = opus_encoder_create(48000, 2, OPUS_APPLICATION_AUDIO, &error);
    if (error != OPUS_OK) {
        ESP_LOGE(TAG, "Failed to create encoder: %s", opus_strerror(error));
        return;
    }

    // Set bitrate (e.g., 128 kbps)
    opus_encoder_ctl(encoder, OPUS_SET_BITRATE(128000));

    // Encode frame
    uint8_t opus_data[4000]; // Maximum packet size
    int bytes = opus_encode(encoder, pcm_data, frame_size, opus_data, sizeof(opus_data));

    if (bytes < 0) {
        ESP_LOGE(TAG, "Encode error: %s", opus_strerror(bytes));
    } else {
        ESP_LOGI(TAG, "Encoded %d bytes", bytes);
        // Transmit or store opus_data...
    }

    opus_encoder_destroy(encoder);
}
```



## Memory Usage

### PSRAM Requirements

⚠️ **PSRAM is essentially required for this component**, especially for multi-threaded usage:
- Each thread using Opus requires a 120KB pseudostack preallocation
- Without PSRAM, even single-threaded usage may exhaust internal RAM
- Multi-threaded decoding without PSRAM is practically impossible

### Thread-Safe Memory Architecture

This component uses a thread-safe pseudostack implementation:
- Each thread gets its own 120KB pseudostack preallocation
- Uses POSIX pthread thread-local storage (TLS) for isolation
- Automatic cleanup when threads exit (pthread destructors)
- Compatible with FreeRTOS tasks via ESP-IDF's pthread layer

The benefit of this approach:
- **Small task stacks**: Only 4-8KB task stack needed (vs 40-60KB without pseudostack)
- **Thread safety**: Each thread has isolated memory space
- **PSRAM efficient**: Large allocations go to PSRAM, preserving internal RAM
- **No fragmentation**: Single large allocation per thread

Memory usage: **120KB × number of concurrent threads using Opus**

## Performance

Typical performance on ESP32-S3 @ 240MHz:
- Decoding 48kHz stereo: ~10-15% CPU usage

Performance varies based on:
- Bitrate and complexity settings
- Sample rate and channel count
- CPU frequency
- Cache configuration

## Advanced Features

### Platform-Specific Optimizations

- **ESP32**: Fixed-point implementation
- **ESP32-S3**:
  - Default: Fixed-point implementation
  - Optional: Floating-point with hardware FPU (enable in menuconfig)
  - Fast-math optimizations when using floating-point
- **ESP32-C3/C6**: Fixed-point implementation
- Future: SIMD optimizations for critical paths

## Troubleshooting

### Build Errors

If you get undefined reference errors, ensure:
1. The git submodule is initialized: `git submodule update --init --recursive`
2. Your CMakeLists.txt includes the component
3. Clean build: `idf.py fullclean && idf.py build`

### Runtime Issues

Enable debug logging:
```c
esp_log_level_set("opus*", ESP_LOG_DEBUG);
```

Check heap usage:
```c
ESP_LOGI(TAG, "Free heap: %d", esp_get_free_heap_size());
ESP_LOGI(TAG, "Free PSRAM: %d", heap_caps_get_free_size(MALLOC_CAP_SPIRAM));
```


## License

Opus is licensed under a BSD-style license. See the [COPYING](opus/COPYING) file in the opus submodule for details.

## Contributing

Contributions are welcome! Please:
1. Test on your target hardware
2. Run basic encode/decode tests
3. Document any platform-specific changes


## Links

- [Opus Official Site](https://opus-codec.org/)
- [Opus Documentation](https://opus-codec.org/docs/)
- [ESP-IDF pthread Thread Local Storage](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/system/pthread.html#thread-local-storage)
