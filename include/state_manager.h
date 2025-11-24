// include/state_manager.h

#ifndef STATE_MANAGER_H
#define STATE_MANAGER_H

#include "satellite_types.h"


SystemMode_t get_system_mode(void);
void set_system_mode(SystemMode_t new_mode);

#endif // STATE_MANAGER_H