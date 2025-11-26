// src/eps_control.c
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "eps_control.h"
#include "state_manager.h"
#include <stdio.h>


void vEPSMonitoringTask(void *pvParameters) {
    printf("EPS Task initialized and monitoring.\n");
    for(;;) {
        SystemMode_t current_mode = get_system_mode();
        
        if (current_mode == MODE_CRITICAL) {
            // Highest priority: Shut down non-essential loads immediately
            printf("EPS MON: CRITICAL MODE DETECTED! Preparing for safe powerdown.\n");
            // Placeholder for FDIR action
            
        } else {
            // Runs in MODE_SAFE and MODE_NOMINAL
            printf("EPS MON: System check (Mode: %d). Power stable.\n", current_mode);
        }

        // Delay longer to reduce CPU load when not in CRITICAL mode
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}


void vEPS_SetSafeModePower(int mode_id) {
    printf("STUB CALLED: EPS received power command (Mode ID: %d).\n", mode_id);
}