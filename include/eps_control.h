// include/eps_control.h

#ifndef EPS_CONTROL_H
#define EPS_CONTROL_H

#include "freertos/task.h"

// --- Public Function Prototypes ---

// 1. The FreeRTOS Task function (called by xTaskCreate in cdh_main.c)
void vEPSMonitoringTask(void *pvParameters);

// 2. The critical execution function for load shedding (called by the Command Processor)
// This is the function we will fully implement in your EPS project phase.
void vEPS_SetSafeModePower(int mode_id); 

#endif // EPS_CONTROL_H