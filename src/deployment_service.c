// src/deployment_service.c

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "state_manager.h"
#include "satellite_types.h"
#include <stdio.h>

void vDeploymentTask(void *pvParameters) {
    // REALITY: 30 Minutes (1800s)
    // SIMULATION: 10 Seconds for home testing
    const uint32_t DEPLOY_DELAY_MS = 10000; 

    printf("DEPLOY: Satellite ejected. Starting safety silence period (%d ms)...\n", DEPLOY_DELAY_MS);
    
    set_system_mode(MODE_INITIAL_BOOT);

    vTaskDelay(pdMS_TO_TICKS(DEPLOY_DELAY_MS));

    printf("DEPLOY: Silence period over. Deploying Antennas (Activating Burn Wire)...\n");
    
    // Simulate the physical deployment
    vTaskDelay(pdMS_TO_TICKS(2000)); 

    printf("DEPLOY: Antennas Deployed! Entering MODE_SAFE.\n");
    
    // Now we allow the satellite to start its normal life
    set_system_mode(MODE_SAFE);

    // Deployment is a one-time thing, so we delete this task to free RAM
    vTaskDelete(NULL); 
}