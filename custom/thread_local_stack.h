/* Thread-local pseudostack implementation for ESP-IDF Opus component */
#ifndef THREAD_LOCAL_STACK_H
#define THREAD_LOCAL_STACK_H

#include <stddef.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_heap_caps.h"

/* Thread-local storage for pseudostack pointers */
typedef struct {
    char *scratch_ptr;
    char *global_stack;
    bool initialized;
    TaskHandle_t owner_task;
} thread_local_pseudostack_t;

/* Get the thread-local pseudostack for the current thread */
thread_local_pseudostack_t* get_thread_local_pseudostack(void);

/* Clean up pseudostack for a specific task (called from deletion hook) */
void cleanup_thread_pseudostack(TaskHandle_t task);

/* Initialize thread-local storage system (call once at startup) */
void init_thread_local_pseudostack_system(void);

#endif /* THREAD_LOCAL_STACK_H */