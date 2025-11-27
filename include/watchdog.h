// include/watchdog.h

#ifndef WATCHDOG_H
#define WATCHDOG_H

#include "satellite_types.h" // Needed for WatchdogTaskID_t

// Public API for tasks to signal they are alive
void watchdog_pet(WatchdogTaskID_t task_id);

// Public API for FSW Initialization (called by main.c to launch the task)
void vSoftwareWatchdogTask(void *pvParameters);

#endif // WATCHDOG_H