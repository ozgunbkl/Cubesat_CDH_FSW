// src/cmd_inject.c

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "satellite_types.h"
#include <stdio.h>

extern QueueHandle_t xCommandQueue;

void vCommandInjectionTask(void *pvParameters) {
    TelecommandPacket_t tx_command;

    // 1. Wait 5 seconds after boot to ensure all system tasks are initialized
    vTaskDelay(pdMS_TO_TICKS(5000));

    // --- TEST 1: Send Command to Switch Mode to NOMINAL (After initial 5s delay) ---
    tx_command.timestamp = xTaskGetTickCount();
    tx_command.command_id = TC_SET_MODE;
    
    // Payload: Send the new mode (MODE_NOMINAL = 1) in the first byte
    tx_command.payload[0] = MODE_NOMINAL; 

    printf("INJECTOR: Sending TC_SET_MODE to NOMINAL (Payload: %d)\n", tx_command.payload[0]);

    // Send the packet to the Command Queue (Wait 100ms max)
    if (xQueueSend(xCommandQueue, &tx_command, pdMS_TO_TICKS(100)) != pdPASS) {
        printf("INJECTOR: ERROR! Command Queue full or unavailable.\n");
    }

    // 2. Wait another 15 seconds to simulate ground station delay
    vTaskDelay(pdMS_TO_TICKS(15000)); 

    // --- TEST 2: Send NO-OP Command later to test connectivity ---
    tx_command.timestamp = xTaskGetTickCount();
    tx_command.command_id = TC_NO_OP;
    
    printf("INJECTOR: Sending TC_NO-OP command.\n");
    xQueueSend(xCommandQueue, &tx_command, pdMS_TO_TICKS(100));

    // The task has completed its simulation job and self-suspends
    vTaskDelete(NULL); 
}