/* Thread-local pseudostack implementation for ESP-IDF Opus component */
#include "thread_local_stack.h"
#include <string.h>
#include <stdio.h>
#include <pthread.h>
#include "esp_log.h"

static const char *TAG = "opus_tls";

/* Global pthread TLS key for pseudostack storage */
static pthread_key_t opus_pseudostack_key;

/* One-time initialization control */
static pthread_once_t init_once = PTHREAD_ONCE_INIT;

/* Flag to track if initialization succeeded */
static bool tls_initialized = false;

/* Destructor function - automatically called when thread exits */
static void pseudostack_destructor(void *data)
{
    thread_local_pseudostack_t *pseudostack = (thread_local_pseudostack_t *)data;
    
    if (pseudostack) {
        if (pseudostack->scratch_ptr) {
            heap_caps_free(pseudostack->scratch_ptr);
            ESP_LOGD(TAG, "Auto-freed pseudostack for exiting thread");
        }
        heap_caps_free(pseudostack);
    }
}

/* One-time initialization function */
static void init_pthread_key(void)
{
    int ret = pthread_key_create(&opus_pseudostack_key, pseudostack_destructor);
    if (ret == 0) {
        tls_initialized = true;
        ESP_LOGD(TAG, "pthread TLS key created successfully");
    } else {
        ESP_LOGE(TAG, "Failed to create pthread TLS key: %d", ret);
    }
}

thread_local_pseudostack_t* get_thread_local_pseudostack(void)
{
    /* Ensure one-time initialization has been done */
    pthread_once(&init_once, init_pthread_key);
    
    if (!tls_initialized) {
        ESP_LOGE(TAG, "pthread TLS initialization failed");
        return NULL;
    }
    
    /* Get the TLS pointer for this thread */
    thread_local_pseudostack_t *pseudostack = 
        (thread_local_pseudostack_t *)pthread_getspecific(opus_pseudostack_key);
    
    /* If not initialized, create a new pseudostack for this thread */
    if (pseudostack == NULL) {
        /* Allocate the pseudostack structure */
        pseudostack = heap_caps_malloc(sizeof(thread_local_pseudostack_t), MALLOC_CAP_INTERNAL);
        if (pseudostack == NULL) {
            ESP_LOGE(TAG, "Failed to allocate TLS pseudostack structure");
            return NULL;
        }
        
        /* Initialize the structure */
        memset(pseudostack, 0, sizeof(thread_local_pseudostack_t));
        pseudostack->owner_task = xTaskGetCurrentTaskHandle();
        pseudostack->initialized = true;
        
        /* Set the TLS pointer - destructor will be called automatically on thread exit */
        int ret = pthread_setspecific(opus_pseudostack_key, pseudostack);
        if (ret != 0) {
            ESP_LOGE(TAG, "Failed to set pthread TLS: %d", ret);
            heap_caps_free(pseudostack);
            return NULL;
        }
        
        ESP_LOGD(TAG, "Initialized thread-local pseudostack for thread %p", (void*)pthread_self());
    }
    
    return pseudostack;
}

void cleanup_thread_pseudostack(TaskHandle_t task)
{
    /* With pthread TLS and destructor callbacks, manual cleanup is optional.
     * This function is kept for backward compatibility. The pthread destructor
     * will handle cleanup automatically when the thread exits. */
    
    /* For pthread, we can only clean up the current thread's data */
    if (task != NULL && task != xTaskGetCurrentTaskHandle()) {
        ESP_LOGW(TAG, "pthread TLS can only clean up current thread's data");
        return;
    }
    
    if (!tls_initialized) {
        return;
    }
    
    /* Get the TLS pointer for the current thread */
    thread_local_pseudostack_t *pseudostack = 
        (thread_local_pseudostack_t *)pthread_getspecific(opus_pseudostack_key);
    
    if (pseudostack) {
        /* Clear the TLS pointer first to prevent double-free */
        pthread_setspecific(opus_pseudostack_key, NULL);
        
        /* Free the pseudostack buffer */
        if (pseudostack->scratch_ptr) {
            heap_caps_free(pseudostack->scratch_ptr);
            ESP_LOGD(TAG, "Manually freed pseudostack for current thread");
        }
        
        /* Free the structure */
        heap_caps_free(pseudostack);
    }
}

void init_thread_local_pseudostack_system(void)
{
    /* With pthread TLS and destructor callbacks, the system is automatically
     * initialized on first use and cleanup happens automatically when threads exit */
    static bool logged = false;
    if (!logged) {
        logged = true;
        ESP_LOGV(TAG, "Thread-local pseudostack system initialized (using pthread TLS)");
        ESP_LOGV(TAG, "Automatic cleanup enabled - no need to manually call cleanup_thread_pseudostack()");
    }
}