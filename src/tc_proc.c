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


void process_telecommand(TelecommandPacket_t *tc_packet);

extern QueueHandle_t xTelemetryQueue;
extern QueueHandle_t xCommandQueue;

void vCommandProcessorTask(void *pvParameters){
    HK_Telemetry_t rx_packet;
    TelecommandPacket_t rx_command;
    printf("TC Processor Task initialized and waiting for commands.\n");
    for(;;) {
        if(xQueueReceive(xCommandQueue, &rx_command, pdMS_TO_TICKS(100)) == pdPASS) {
            process_telecommand(&rx_command);
        }
        
        
        if(xQueueReceive(xTelemetryQueue, &rx_packet, 0) == pdPASS) {
            printf("Received Telemetry Packet - Timestamp: %lu, Bus Voltage: %.2f V\n",
                   rx_packet.timestamp, (double)rx_packet.bus_voltage);
        }
        if (xTaskGetTickCount() > pdMS_TO_TICKS(10000) && get_system_mode() == MODE_SAFE) {
            set_system_mode(MODE_NOMINAL); // This calls the protected function!
        }
        watchdog_pet(WDT_TASK_CMD_PROC);
    }
}


void process_telecommand(TelecommandPacket_t *tc_packet) {
    size_t crc_data_length = sizeof(TelecommandPacket_t) - sizeof(uint16_t);

    uint16_t calculated_crc = crc16_ccitt((const uint8_t *)tc_packet, crc_data_length);

    if(calculated_crc != tc_packet->crc) {
        printf("TC PROC: ERROR! CRC FAILURE! Packet discarded.\n");
        printf("           Expected CRC: 0x%X, Calculated CRC: 0x%X\n", tc_packet->crc, calculated_crc);
        return;
    }
    printf("TC PROC: CRC OK. Executing Command ID: %d\n", tc_packet->command_id);

    switch (tc_packet->command_id) {
        case TC_SET_MODE:
            // The new mode is expected to be in the first byte of the payload
            SystemMode_t new_mode = (SystemMode_t)tc_packet->payload[0];
            set_system_mode(new_mode);
            break;

        case TC_REQUEST_HK:
            printf("TC PROC: Requesting immediate Telemetry burst.\n");
            break;
            
        case TC_NO_OP:
            printf("TC PROC: NO-OP command received. Link OK.\n");
            break;

        default:
            printf("TC PROC: ERROR! Unknown command ID: %d\n", tc_packet->command_id);
            break;
    }
}

void vTC_SetSystemMode(int new_mode){
    printf("STUB CALLED: Setting system mode to %d.\n", new_mode);
}