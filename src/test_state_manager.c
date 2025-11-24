// src/state_manager.c

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "state_manager.h"
#include "satellite_types.h"
#include <stdio.h>


extern SystemMode_t g_current_mode;
extern SemaphoreHandle_t xModeMutex;

// --- A. SAFELY READ THE CURRENT MODE (Mutex Take/Give) ---
SystemMode_t get_system_mode(void) {
    SystemMode_t mode_copy = MODE_SAFE;
    
    // Attempt to acquire the Mutex (Wait indefinitely: portMAX_DELAY)
    if (xSemaphoreTake(xModeMutex, portMAX_DELAY) == pdTRUE) {
        
        // Mutex acquired! Copy the state variable for safe reading
        mode_copy = g_current_mode;
        
        // Release the Mutex immediately so other tasks can access it
        xSemaphoreGive(xModeMutex);
    }
    return mode_copy;
}

// --- B. SAFELY CHANGE THE MODE (Mutex Take/Give) ---
void set_system_mode(SystemMode_t new_mode) {
    
    // Attempt to acquire the Mutex (Wait for 10 ticks, then give up)
    if (xSemaphoreTake(xModeMutex, pdMS_TO_TICKS(10)) == pdTRUE) {
        
        // Mutex acquired! Change the global state variable
        g_current_mode = new_mode;
        
        printf("Mode Change SUCCESS! New Mode: %s\n", 
               (new_mode == MODE_NOMINAL) ? "NOMINAL" : 
               (new_mode == MODE_SAFE) ? "SAFE" : "CRITICAL");
        
        // Release the Mutex
        xSemaphoreGive(xModeMutex);
    } else {
        printf("WARNING: Could not change mode to %d; Mutex busy.\n", new_mode);
    }
}