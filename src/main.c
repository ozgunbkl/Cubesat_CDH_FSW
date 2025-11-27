// src/cdh_main.c

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdio.h>
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "satellite_types.h"
#include "tc_proc.h"
#include "eps_control.h"
#include "task_defs.h"
#include "watchdog.h"

void vCommandInjectionTask(void *pvParameters);
void vDataLoggerTask(void *pvParameters);

SystemMode_t g_current_mode = MODE_SAFE;
SemaphoreHandle_t xModeMutex;
QueueHandle_t xTelemetryQueue;
QueueHandle_t xCommandQueue;

void app_main(void) {
    printf("C&DH FSW Initialization Started...\n");

    xTelemetryQueue = xQueueCreate(5, sizeof(HK_Telemetry_t));

    if (xTelemetryQueue == NULL) {
        printf("CRITICAL ERROR: Failed to create Telemetry Queue! System HALT.\n");
        return;
    }
    
    xCommandQueue = xQueueCreate(3, sizeof(TelecommandPacket_t));
    if (xCommandQueue == NULL) {
        printf("CRITICAL ERROR: Failed to create Command Queue! System HALT.\n");
        return;
    }

    xModeMutex = xSemaphoreCreateMutex();
    if (xModeMutex == NULL) {
        printf("CRITICAL ERROR: Failed to create Mode Mutex! System HALT.\n");
        return;
    }

    xTaskCreate(vSoftwareWatchdogTask, "WDT_MON", 2048, NULL, 6, NULL);
    xTaskCreate(vCommandProcessorTask, "CMD_PROC", 4096, NULL, 5, NULL); 
    xTaskCreate(vEPSMonitoringTask, "EPS_MON", 2048, NULL, 4, NULL); 
    xTaskCreate(vTelemetryGeneratorTask, "TM_GEN", 2048, NULL, 3, NULL);
    xTaskCreate(vCommandInjectionTask, "CMD_INJECT", 2048, NULL, 1, NULL);
    xTaskCreate(vDataLoggerTask, "DATA_LOG", 2048, NULL, 4, NULL);
    
    printf("All tasks and communication channels launched. System running.\n");
}