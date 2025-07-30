/* Thread-safe Opus test example for ESP-IDF */
#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_heap_caps.h"
#include "opus.h"
#include "thread_local_stack.h"

static const char *TAG = "opus_thread_test";

/* Test parameters */
#define SAMPLE_RATE 48000
#define CHANNELS 2
#define FRAME_SIZE 960
#define NUM_THREADS 3
#define NUM_ITERATIONS 100

/* Test task that creates and uses an Opus decoder */
static void opus_test_task(void *pvParameters)
{
    int task_id = (int)pvParameters;
    int err;
    
    ESP_LOGI(TAG, "Task %d starting", task_id);
    
    /* Create decoder */
    OpusDecoder *decoder = opus_decoder_create(SAMPLE_RATE, CHANNELS, &err);
    if (err != OPUS_OK || !decoder) {
        ESP_LOGE(TAG, "Task %d: Failed to create decoder: %s", task_id, opus_strerror(err));
        vTaskDelete(NULL);
        return;
    }
    
    /* Allocate buffers */
    opus_int16 *pcm = heap_caps_malloc(sizeof(opus_int16) * FRAME_SIZE * CHANNELS, MALLOC_CAP_8BIT);
    unsigned char *packet = heap_caps_malloc(1000, MALLOC_CAP_8BIT);
    
    if (!pcm || !packet) {
        ESP_LOGE(TAG, "Task %d: Failed to allocate buffers", task_id);
        opus_decoder_destroy(decoder);
        vTaskDelete(NULL);
        return;
    }
    
    /* Create a simple test packet (silence) */
    memset(packet, 0, 2);
    
    /* Perform decoding operations */
    for (int i = 0; i < NUM_ITERATIONS; i++) {
        int samples = opus_decode(decoder, packet, 2, pcm, FRAME_SIZE, 0);
        if (samples < 0) {
            ESP_LOGE(TAG, "Task %d: Decode error at iteration %d: %s", 
                     task_id, i, opus_strerror(samples));
            break;
        }
        
        if (i % 10 == 0) {
            ESP_LOGI(TAG, "Task %d: Iteration %d complete (decoded %d samples)", 
                     task_id, i, samples);
        }
        
        vTaskDelay(pdMS_TO_TICKS(10));
    }
    
    /* Clean up */
    heap_caps_free(pcm);
    heap_caps_free(packet);
    opus_decoder_destroy(decoder);
    
    ESP_LOGI(TAG, "Task %d: Complete", task_id);
    
    /* With pthread TLS, cleanup happens automatically when thread exits */
    
    vTaskDelete(NULL);
}

void app_main(void)
{
    ESP_LOGI(TAG, "Starting Opus thread safety test");
    
    /* Print memory info */
    ESP_LOGI(TAG, "Free heap: %d bytes", esp_get_free_heap_size());
    ESP_LOGI(TAG, "Free PSRAM: %d bytes", heap_caps_get_free_size(MALLOC_CAP_SPIRAM));
    
ESP_LOGI(TAG, "Thread-safe pseudostack is always enabled");
    
    /* Create multiple tasks that use Opus concurrently */
    for (int i = 0; i < NUM_THREADS; i++) {
        char task_name[32];
        snprintf(task_name, sizeof(task_name), "opus_task_%d", i);
        
        BaseType_t ret = xTaskCreate(opus_test_task, 
                                     task_name,
                                     8192,  /* Stack size */
                                     (void*)i,  /* Task ID as parameter */
                                     5,     /* Priority */
                                     NULL);
        
        if (ret != pdPASS) {
            ESP_LOGE(TAG, "Failed to create task %d", i);
        } else {
            ESP_LOGI(TAG, "Created task %d", i);
        }
        
        /* Stagger task creation slightly */
        vTaskDelay(pdMS_TO_TICKS(100));
    }
    
    /* Monitor memory usage */
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(5000));
        ESP_LOGI(TAG, "Free heap: %d bytes", esp_get_free_heap_size());
        ESP_LOGI(TAG, "Free PSRAM: %d bytes", heap_caps_get_free_size(MALLOC_CAP_SPIRAM));
    }
}