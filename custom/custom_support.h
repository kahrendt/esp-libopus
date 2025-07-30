/* Custom support for ESP-IDF Opus component with PSRAM allocation */
#ifndef CUSTOM_SUPPORT_H
#define CUSTOM_SUPPORT_H

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include "esp_heap_caps.h"

#include "thread_local_stack.h"

/* Override opus_alloc to use PSRAM for ALL Opus allocations */
#define OVERRIDE_OPUS_ALLOC

static inline void *opus_alloc(size_t size)
{
    void *ptr = NULL;
    
    /* Try to allocate from PSRAM first */
    ptr = heap_caps_malloc(size, MALLOC_CAP_SPIRAM);
    
    /* Fall back to internal memory if PSRAM allocation fails */
    if (ptr == NULL) {
        ptr = heap_caps_malloc(size, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
    }
    
    return ptr;
}

/* Override opus_alloc_scratch to use PSRAM for the pseudostack */
#define OVERRIDE_OPUS_ALLOC_SCRATCH

static inline void *opus_alloc_scratch(size_t size)
{
    void *ptr = NULL;
    
/* Initialize thread-local storage system if needed */
    static bool tls_initialized = false;
    if (!tls_initialized) {
        init_thread_local_pseudostack_system();
        tls_initialized = true;
    }
    
    return opus_alloc(size);
}

/* Override opus_free to use heap_caps_free */
#define OVERRIDE_OPUS_FREE

static inline void opus_free(void *ptr)
{
    heap_caps_free(ptr);
}

/* Function called on pseudostack overflow - required for NONTHREADSAFE_PSEUDOSTACK mode */
static inline void celt_fatal(const char *str, const char *file, int line)
{
    printf("FATAL ERROR: %s at %s:%d\n", str, file, line);
    abort();
}

/* Don't define the pseudostack variables here - let stack_alloc.h handle them
   as it knows when to define vs declare them based on CELT_C */

#endif /* CUSTOM_SUPPORT_H */