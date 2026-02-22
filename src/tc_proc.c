// src/tc_proc.c

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "satellite_types.h"
#include "state_manager.h"
#include "watchdog.h"
#include <stdio.h>
#include <stdint.h>
#include "tc_proc.h"
#include "utils.h"
#include "esp_log.h"
#include "cdhs_router.h"

extern QueueHandle_t xCommandQueue;

/**
 * @brief The Command Processor Task
 * It pulls raw CCSDS packets from the queue, verifies them, and routes them.
 */
void vCommandProcessorTask(void *pvParameters) {
    TelecommandPacket_t rx_command;
    
    printf("TC PROC: Task initialized. Standard: CCSDS Space Packet Protocol.\n");

    for(;;) {
        // Wait for ground commands from the injection task or radio driver
        if(xQueueReceive(xCommandQueue, &rx_command, pdMS_TO_TICKS(100)) == pdPASS) {
            
            // 1. INTEGRITY CHECK (Layer 2)
            // We calculate CRC over the raw_data buffer excluding the last 2 bytes (the CRC itself)
            uint16_t calculated_crc = crc16_ccitt(rx_command.raw_data, rx_command.length - 2);

            if(calculated_crc == rx_command.packet_crc) {
                printf("TC PROC: [CRC OK] Packet Length: %d bytes. Handing to Router...\n", rx_command.length);
                
                // 2. ROUTING (Layer 3)
                // We pass the raw buffer to the CDHS Router
                CDHS_RoutePacket(rx_command.raw_data, rx_command.length);
                
            } else {
                printf("TC PROC: [CRC ERROR] Dropping packet. Expected: 0x%04X, Computed: 0x%04X\n", 
                        rx_command.packet_crc, calculated_crc);
                
                // Optional: Notify FDIR of a communication link error
            }
        }
        
        // Keep the watchdog happy
        watchdog_pet(WDT_TASK_CMD_PROC);
    }
}