// src/eps_control.c
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "eps_control.h"
#include <stdio.h>


void vEPSMonitoringTask(void *pvParameters) {
    printf("EPS Task initialized and monitoring.\n");
    for(;;) {
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}


void vEPS_SetSafeModePower(int mode_id) {
    printf("STUB CALLED: EPS received power command (Mode ID: %d).\n", mode_id);
}