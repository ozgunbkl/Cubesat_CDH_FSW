// include/tc_proc.h

#ifndef TC_PROC_H
#define TC_PROC_H

#include "freertos/task.h"
#include "satellite_types.h"


void vCommandProcessorTask(void *pvParameters);

void vTC_SetSystemMode(int new_mode);

#endif // TC_PROC_H