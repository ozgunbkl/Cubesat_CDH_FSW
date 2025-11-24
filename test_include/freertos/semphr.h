#include "FreeRTOS.h"
// Mock the Mutex creation and access functions
#define xSemaphoreCreateMutex() ((SemaphoreHandle_t)1)
#define xSemaphoreTake(x, t) pdTRUE
#define xSemaphoreGive(x) pdTRUE