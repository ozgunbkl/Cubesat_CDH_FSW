// src/eps_control.c
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "eps_control.h"
#include "state_manager.h"
#include "watchdog.h"
#include "eps_telemetry.h"
#include "eps_commands.h"
#include "fdir_service.h"
#include <stdio.h>

#define CRITICAL_BUS_VOLTAGE 2.5f

void vEPSMonitoringTask(void *pvParameters) {
    printf("CDH EPS Monitor: Now linked to EPS Subsystem Telemetry.\n");

    for(;;) {
        // 1. GET REAL DATA from the EPS Subsystem (not a local variable)
        if(xSemaphoreTake(xEPSDataMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
            float real_voltage = g_eps_telemetry.f_BusVoltage;
            xSemaphoreGive(xEPSDataMutex);

            // 2. GLOBAL FDIR LOGIC
            // If the EPS Subsystem is struggling, CDH triggers the Global Critical Mode
            if(real_voltage < CRITICAL_BUS_VOLTAGE && get_system_mode() != MODE_CRITICAL) {
                
                // --- NEW: REPORT TO THE SYSTEM-WIDE LOG ---
                FaultReport_t global_pwr_fault = {
                    .source = SRC_EPS,
                    .severity = FAULT_CRITICAL,
                    .fault_code = FAULT_EPS_LOW_VOLTAGE,
                    .timestamp = Time_GetSeconds()
                };
                FDIR_ReportFault(global_pwr_fault);

                // 3. GLOBAL RECOVERY
                set_system_mode(MODE_CRITICAL);
                printf("CDH: Emergency! Satellite forced to MODE_CRITICAL.\n");
            }
        }

        watchdog_pet(WDT_TASK_EPS_MON);
        
        // Check once per second (10s was way too slow for safety!)
        vTaskDelay(pdMS_TO_TICKS(1000)); 
    }
}


void vEPS_SetSafeModePower(int mode_id) {
    printf("STUB CALLED: EPS received power command (Mode ID: %d).\n", mode_id);
}