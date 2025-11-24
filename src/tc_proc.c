// src/tc_proc.c

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "satellite_types.h"
#include "state_manager.h"
#include <stdio.h>
#include <stdint.h>
#include "tc_proc.h"
#include "esp_log.h"


extern QueueHandle_t xTelemetryQueue;

void vCommandProcessorTask(void *pvParameters){
    HK_Telemetry_t rx_packet;
    printf("TC Processor Task initialized and waiting for commands.\n");
    for(;;) {
        if(xQueueReceive(xTelemetryQueue, &rx_packet, pdMS_TO_TICKS(500)) == pdPASS) {
            printf("Received Telemetry Packet - Timestamp: %lu, Bus Voltage: %.2f V\n",
                   rx_packet.timestamp, (double)rx_packet.bus_voltage);
        }
        if (xTaskGetTickCount() > pdMS_TO_TICKS(10000) && get_system_mode() == MODE_SAFE) {
            set_system_mode(MODE_NOMINAL); // This calls the protected function!
        }
    }
}


void vTC_SetSystemMode(int new_mode){
    printf("STUB CALLED: Setting system mode to %d.\n", new_mode);
}