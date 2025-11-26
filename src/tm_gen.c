// src/tm_gen.c
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "satellite_types.h"
#include "task_defs.h"
#include "state_manager.h"
#include <stdio.h>

extern void watchdog_pet(WatchdogTaskID_t task_id);
extern QueueHandle_t xTelemetryQueue;


void vTelemetryGeneratorTask(void *pvParameters) {
    
    HK_Telemetry_t tx_packet;

    printf("TM Generator Task initialized and running.\n");
    for(;;) {
        if(get_system_mode() == MODE_NOMINAL){
            printf("TM GEN: -- Running NOMINAL Mission Cycle --\n");
            tx_packet.timestamp = xTaskGetTickCount();
            tx_packet.bus_voltage = 3.3f;
        
            if(xQueueSend(xTelemetryQueue, &tx_packet, 0) != pdPASS) {
                printf("WARNING: Telemetry Queue Full, packet lost.\n");
            }
        }else{
            printf("TM GEN: Waiting for MODE_NOMINAL. Current mode: %d\n", get_system_mode());
        }

        watchdog_pet(WDT_TASK_TM_GEN);
        
        vTaskDelay(pdMS_TO_TICKS(5000)); 
    }
}