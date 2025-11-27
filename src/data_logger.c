#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "satellite_types.h"
#include "state_manager.h"
#include "watchdog.h"
#include <stdio.h>

extern QueueHandle_t xTelemetryQueue;
DownlinkMode_t g_downlink_mode = DOWNLINK_INACTIVE;

void vDataLoggerTask(void *pvParameters){
    HK_Telemetry_t rx_log_packet;
    const TickType_t xLogWaitTime = pdMS_TO_TICKS(100);

    printf("DATA LOGGER: Task initialized, monitoring telemetry queue.\n");

    for(;;){
        // Attempt to retrieve a packet from the Telemetry Queue
        if (xQueueReceive(xTelemetryQueue, &rx_log_packet, xLogWaitTime) == pdPASS) {
            
            // --- DATA RETRIEVED: SIMULATE WRITING TO FLASH/SD CARD ---
            // The FSW is now guaranteed safe access to this data.

            printf("DATA LOGGER: SUCCESS! Archived packet T: %lu | V: %.2f V\n",
                   rx_log_packet.timestamp, 
                   (double)rx_log_packet.bus_voltage);
            
            // Placeholder: In a real system, SPIFFS or FATFS write functions 
            // would be called here to save the data persistently.
        
        } else {
            // Log queue status (optional, for debug)
            // printf("DATA LOGGER: Queue empty, sleeping...\n");
        }

        // --- DOWNLINK STRATEGY: RETRIEVE ARCHIVED DATA ---
        // Downlink only happens if we are over a ground station AND in the correct FSW mode

        if (g_downlink_mode == DOWNLINK_ACTIVE && get_system_mode() == MODE_NOMINAL) {
            // Placeholder: Retrieve data from simulated archive (e.g., SD card)
            // For simplicity, I just print a burst signal.
            printf("DATA LOGGER: --- DOWNLINK BURST ACTIVE --- \n");
        

            for(int i = 0; i<5 ;i++){
                // In a real system, retrieve archived_packet[i] and send via radio queue.
                printf("DATA LOGGER: Sending Archived Packet %d... \n", i + 1);
            }
            printf("DATA LOGGER: --- DOWNLINK COMPLETE. Resuming Archiving. ---\n");

            // After downlink, signal the window is closed
            g_downlink_mode = DOWNLINK_INACTIVE;
        }

        watchdog_pet(WDT_TASK_DATA_LOG);
        // The xLogWaitTime timeout acts as the task's VTaskDelay
    }
}