// src/cmd_inject.c

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "satellite_types.h"
#include "utils.h"
#include <stdio.h>

extern QueueHandle_t xCommandQueue;
extern DownlinkMode_t g_downlink_mode;

void vCommandInjectionTask(void *pvParameters) {
    TelecommandPacket_t tx_command;

    size_t crc_data_length = sizeof(TelecommandPacket_t) - sizeof(uint16_t);

    // 1. Wait 5 seconds after boot to ensure all system tasks are initialized
    vTaskDelay(pdMS_TO_TICKS(5000));

    // --- TEST 1: Send Command to Switch Mode to NOMINAL (After initial 5s delay) ---
    tx_command.timestamp = xTaskGetTickCount();
    tx_command.command_id = TC_SET_MODE;
    
    // Payload: Send the new mode (MODE_NOMINAL = 1) in the first byte
    tx_command.payload[0] = MODE_NOMINAL; 

    tx_command.crc = crc16_ccitt((const uint8_t *)&tx_command, crc_data_length);

    printf("INJECTOR: Sending TC_SET_MODE to NOMINAL (Payload: %d, CRC: 0x%X)\n", tx_command.payload[0], tx_command.crc);

    // Send the packet to the Command Queue (Wait 100ms max)
    if (xQueueSend(xCommandQueue, &tx_command, pdMS_TO_TICKS(100)) != pdPASS) {
        printf("INJECTOR: ERROR! Command Queue full or unavailable.\n");
    }

    // 2. Wait another 15 seconds to simulate ground station delay
    vTaskDelay(pdMS_TO_TICKS(15000)); 

    // --- TEST 2: Send NO-OP Command later to test connectivity ---
    tx_command.timestamp = xTaskGetTickCount();
    tx_command.command_id = TC_NO_OP;

    tx_command.crc = crc16_ccitt((const uint8_t *)&tx_command, crc_data_length);
    
    printf("INJECTOR: Sending TC_NO-OP command (CRC: 0x%X).\n", tx_command.crc);
    xQueueSend(xCommandQueue, &tx_command, pdMS_TO_TICKS(100));

    // --- TEST 3: Trigger the Downlink Window (After 20s) ---
    vTaskDelay(pdMS_TO_TICKS(5000)); // Wait another 5 seconds

    g_downlink_mode = DOWNLINK_ACTIVE; // <<< Simulate entering Ground Station visibility
    printf("INJECTOR: Ground Station Pass Detected! Setting Downlink ACTIVE.\n");

    // The task has completed its simulation job and self-suspends
    vTaskDelete(NULL); 
}