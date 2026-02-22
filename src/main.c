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
#include "eps_telemetry.h"
#include "state_manager.h"
#include "esp_log.h"
#include <stdlib.h>
#include <time.h>


// --- 1. GLOBAL RESOURCE DEFINITIONS (The Single Source of Truth) ---

// EPS Data
EPS_Telemetry_t g_eps_telemetry = {
    .f_BusVoltage = 4.0f, 
    .f_StateOfCharge = 50.0f,
    .f_SolarInputPower = 0.0f,
    .ul_PDU_Register = (PDU_LOAD_PAYLOAD_BIT | PDU_LOAD_HEATER_BIT | PDU_LOAD_COMMS_BIT),
};

// System State
SystemMode_t g_current_mode = MODE_SAFE;

// Handles
SemaphoreHandle_t xEPSDataMutex = NULL;
SemaphoreHandle_t xModeMutex = NULL;
QueueHandle_t xTelemetryQueue = NULL;
QueueHandle_t xCommandQueue = NULL;

// --- 2. EXTERNAL TASK PROTOTYPES ---
extern void vSolarHarvest_Task(void *pvParameters);
extern void vEPS_Manager_Task(void *pvParameters);
extern void vCommandInjectionTask(void *pvParameters);
extern void vDataLoggerTask(void *pvParameters);
extern void vTelemetryGeneratorTask(void *pvParameters);
extern void vADCS_Manager_Task(void *pvParameters); 
extern void vDeploymentTask(void *pvParameters);

// --- 3. MASTER INITIALIZATION AND LAUNCH ---

void app_main(void) {
    printf("--- Integrated Satellite FSW Initialization Started ---\n");

    // Initialize Random Seed for Solar Simulation
    srand((unsigned int)time(NULL));

    // A. Create Queues
    xTelemetryQueue = xQueueCreate(10, sizeof(HK_Telemetry_t));
    if (xTelemetryQueue == NULL) {
        printf("CRITICAL ERROR: Telemetry Queue Failed!\n");
        return;
    }

    xCommandQueue = xQueueCreate(5, sizeof(TelecommandPacket_t));
    if (xCommandQueue == NULL) {
        printf("CRITICAL ERROR: Command Queue Failed!\n");
        return;
    }

    // B. Create Mutexes
    xEPSDataMutex = xSemaphoreCreateMutex();
    if (xEPSDataMutex == NULL) {
        ESP_LOGE("MAIN", "Failed to create EPS Mutex!");
        return;
    }

    xModeMutex = xSemaphoreCreateMutex();
    if (xModeMutex == NULL) {
        printf("CRITICAL ERROR: Mode Mutex Failed!\n");
        return;
    }

    // --- 4. TASK CREATION (Priority Hierarchy) ---

    // [PRIO 6] - System Safety: Watchdog must be highest
    xTaskCreate(vSoftwareWatchdogTask, "WDT_MON", 2048, NULL, 6, NULL);

    xTaskCreate(vDeploymentTask, "DEPLOY", 2048, NULL, 5, NULL);

    // [PRIO configMAX_PRIORITIES - 1] - Power Safety: EPS Manager
    xTaskCreate(vEPS_Manager_Task, "EPS_MGMT", 4096, NULL, configMAX_PRIORITIES - 1, NULL);

    // [PRIO 5] - Communications: Command Processor
    xTaskCreate(vCommandProcessorTask, "CMD_PROC", 4096, NULL, 5, NULL);

    // [PRIO 4] - Maintenance: Global EPS Monitoring & Data Logging
    xTaskCreate(vEPSMonitoringTask, "EPS_MON", 2048, NULL, 4, NULL); 
    xTaskCreate(vDataLoggerTask, "DATA_LOG", 4096, NULL, 4, NULL);

    xTaskCreate(vADCS_Manager_Task, "ADCS_MGMT", 4096, NULL, 4, NULL);

    // [PRIO 3] - Telemetry Generation
    xTaskCreate(vTelemetryGeneratorTask, "TM_GEN", 2048, NULL, 3, NULL);

    // [PRIO 1] - Simulation Tools: Command Injection & Solar Harvesting
    xTaskCreate(vCommandInjectionTask, "CMD_INJECT", 2048, NULL, 1, NULL);
    xTaskCreate(vSolarHarvest_Task, "SOLAR_HARV", 2048, NULL, 1, NULL);

    printf("--- All Systems Integrated and Running. Start Ground Comms Simulation. ---\n");
}