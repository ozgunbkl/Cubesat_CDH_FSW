// src/watchdog.c

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "satellite_types.h"
#include <stdio.h>

// Array to store the last time (in ticks) each critical task checked in.
TickType_t g_watchdog_last_pet[WDT_TASK_COUNT] = {0};

// Function called by critical tasks to signal they are alive
void watchdog_pet(WatchdogTaskID_t task_id) {
    if (task_id < WDT_TASK_COUNT) {
        g_watchdog_last_pet[task_id] = xTaskGetTickCount();
    }
}

// WATCHDOG MONITOR TASK
void vSoftwareWatchdogTask(void *pvParameters) {
    // Timeout is 15 seconds (15000 milliseconds)
    const TickType_t xWatchdogTimeout = pdMS_TO_TICKS(15000); 
    
    // Initialize array at boot time so the system doesn't immediately reset
    for (int i = 0; i < WDT_TASK_COUNT; i++) {
        g_watchdog_last_pet[i] = xTaskGetTickCount();
    }
    
    printf("WATCHDOG: Software Watchdog initialized.\n");

    for (;;) {
        vTaskDelay(pdMS_TO_TICKS(5000)); // Check every 5 seconds (1/3 of timeout)

        TickType_t xCurrentTime = xTaskGetTickCount();
        
        // Loop through all critical tasks to check their status
        for (int i = 0; i < WDT_TASK_COUNT; i++) {
            if (xCurrentTime - g_watchdog_last_pet[i] > xWatchdogTimeout) {
                // FAILURE DETECTED: Task has not pet the watchdog in 15 seconds
                printf("WATCHDOG: !!! CRITICAL FAILURE: Task %d timed out !!!\n", i);
                
                // IMPORTANT: This is where a real FSW would call esp_restart() or 
                // transition to MODE_CRITICAL to save the system. For simulation, I just log the event.
            }
        }
    }
}