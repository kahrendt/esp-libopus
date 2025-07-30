/* Simple Opus Decoder Example for ESP-IDF */

#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_heap_caps.h"
#include "opus.h"

static const char *TAG = "opus_example";

/* Example Opus frame (silent frame) */
static const uint8_t silent_opus_frame[] = {
    0xF8, 0xFF, 0xFE  /* Opus silent frame */
};

void opus_decoder_task(void *pvParameters)
{
    int error;
    
    ESP_LOGI(TAG, "Creating Opus decoder...");
    
    /* Create decoder for 48kHz stereo */
    OpusDecoder *decoder = opus_decoder_create(48000, 2, &error);
    if (error != OPUS_OK) {
        ESP_LOGE(TAG, "Failed to create decoder: %s", opus_strerror(error));
        vTaskDelete(NULL);
        return;
    }
    
    ESP_LOGI(TAG, "Opus decoder created successfully");
    ESP_LOGI(TAG, "Decoder size: %d bytes", opus_decoder_get_size(2));
    
    /* Output buffer for decoded audio */
    int16_t pcm_out[960 * 2]; /* 20ms @ 48kHz, stereo */
    
    /* Decode example frame */
    int decoded_samples = opus_decode(decoder, 
                                     silent_opus_frame, 
                                     sizeof(silent_opus_frame),
                                     pcm_out,
                                     960,
                                     0);
    
    if (decoded_samples < 0) {
        ESP_LOGE(TAG, "Decode error: %s", opus_strerror(decoded_samples));
    } else {
        ESP_LOGI(TAG, "Successfully decoded %d samples", decoded_samples);
        
        /* In a real application, you would send pcm_out to I2S or DAC */
        
        /* Check if output is silent (as expected) */
        bool is_silent = true;
        for (int i = 0; i < decoded_samples * 2; i++) {
            if (pcm_out[i] != 0) {
                is_silent = false;
                break;
            }
        }
        ESP_LOGI(TAG, "Output is %s", is_silent ? "silent (correct)" : "not silent");
    }
    
    /* Print memory usage */
    ESP_LOGI(TAG, "Free heap: %d bytes", esp_get_free_heap_size());
    ESP_LOGI(TAG, "Free internal: %d bytes", 
             heap_caps_get_free_size(MALLOC_CAP_INTERNAL));
#ifdef CONFIG_OPUS_USE_PSRAM
    ESP_LOGI(TAG, "Free PSRAM: %d bytes", 
             heap_caps_get_free_size(MALLOC_CAP_SPIRAM));
#endif
    
    /* Cleanup */
    opus_decoder_destroy(decoder);
    ESP_LOGI(TAG, "Decoder destroyed");
    
    /* Task complete */
    vTaskDelete(NULL);
}

void app_main(void)
{
    ESP_LOGI(TAG, "Opus Decoder Example");
    ESP_LOGI(TAG, "Opus version: %s", opus_get_version_string());
    
    /* Create decoder task */
    xTaskCreate(opus_decoder_task, 
                "opus_decoder", 
                8192,           /* Stack size */
                NULL, 
                5,              /* Priority */
                NULL);
}