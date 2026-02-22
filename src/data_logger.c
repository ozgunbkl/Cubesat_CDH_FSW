#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "satellite_types.h"
#include "state_manager.h"
#include "watchdog.h"
#include "archive_service.h" 
#include "payload_service.h"
#include <stdio.h>

extern QueueHandle_t xTelemetryQueue;
DownlinkMode_t g_downlink_mode = DOWNLINK_INACTIVE;

void vDataLoggerTask(void *pvParameters){
    HK_Telemetry_t rx_log_packet;
    const TickType_t xLogWaitTime = pdMS_TO_TICKS(100);

    Archive_Init(); // Initialize the archive system

    printf("DATA LOGGER: Task initialized, monitoring telemetry queue.\n");

    for(;;) {
        // --- STEP A: REAL ARCHIVING ---
        // Replace your placeholder printf with a real write operation
        if (xQueueReceive(xTelemetryQueue, &rx_log_packet, xLogWaitTime) == pdPASS) {
            
            ArchiveStatus_t write_status = Archive_WriteRecord(REC_ID_HK, (uint8_t*)&rx_log_packet, sizeof(HK_Telemetry_t));
            
            if (write_status == ARCHIVE_OK) {
                // Optional: low-frequency debug log
                // printf("DATA LOGGER: Record saved at T: %lu\n", rx_log_packet.timestamp);
            } else {
                // Archive_WriteRecord already reported the fault to FDIR, 
                // Just print here for your local debug.
                printf("DATA LOGGER: Archiving failed! Error: %d\n", write_status);
            }
        }

        // --- STEP B: REAL DOWNLINK RETRIEVAL ---
        // Instead of a simple loop, pull real data from your simulated flash
        if (g_downlink_mode == DOWNLINK_ACTIVE && get_system_mode() == MODE_NOMINAL) {
            printf("DATA LOGGER: --- DOWNLINK BURST ACTIVE --- \n");
            
            ArchiveRecord_t out_record;
            for (uint16_t i = 0; i < 5; i++) {
                // Use your ReadRecord logic (which includes the CRC check!)
                ArchiveStatus_t read_status = Archive_ReadRecord(i, &out_record);

                if (read_status == ARCHIVE_OK) {
                    printf("DATA LOGGER: Sending Archived Packet %d [ID: 0x%02X, CRC: 0x%04X]\n", 
                            i + 1, out_record.record_id, out_record.crc);
                    // Next step would be pushing 'out_record' to a Radio/Comms Queue
                } else {
                    printf("DATA LOGGER: Read error at index %d (Status: %d)\n", i, read_status);
                }
            }
            
            printf("DATA LOGGER: --- DOWNLINK COMPLETE. ---\n");
            g_downlink_mode = DOWNLINK_INACTIVE;
        }

        // --- STEP C: PERIODIC SCIENCE LOGGING ---
        // Ensure Payload runs so it can call Archive_WriteRecord for science data
        Payload_Update();

        watchdog_pet(WDT_TASK_DATA_LOG);
    }
}