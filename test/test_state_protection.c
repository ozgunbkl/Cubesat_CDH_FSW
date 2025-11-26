// test/test_state_protection.c

#include <unity.h>               // Unity Test Framework
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"    // For SemaphoreHandle_t
#include "state_manager.h"       // Functions to test: get/set_system_mode
#include "satellite_types.h"     // Needed for SystemMode_t


// --- GLOBAL VARIABLE DEFINITIONS FOR HOST TESTING ---
// These declarations link to your actual functions in src/state_manager.c
SystemMode_t g_current_mode = MODE_SAFE;
SemaphoreHandle_t xModeMutex;

// --- A. SAFELY READ THE CURRENT MODE (Mutex Take/Give) ---
// PASTE THE ENTIRE BODY OF get_system_mode HERE
SystemMode_t get_system_mode(void) {
    SystemMode_t mode_copy = MODE_SAFE;
    
    // Attempt to acquire the Mutex (Wait indefinitely: portMAX_DELAY)
    if (xSemaphoreTake(xModeMutex, portMAX_DELAY) == pdTRUE) {
        mode_copy = g_current_mode;
        xSemaphoreGive(xModeMutex);
    }
    return mode_copy;
}

// --- B. SAFELY CHANGE THE MODE (Mutex Take/Give) ---
// PASTE THE ENTIRE BODY OF set_system_mode HERE
void set_system_mode(SystemMode_t new_mode) {
    
    // Attempt to acquire the Mutex (Wait for 10 ticks, then give up)
    if (xSemaphoreTake(xModeMutex, pdMS_TO_TICKS(10)) == pdTRUE) {
        g_current_mode = new_mode;
        
        // Removed printf for cleaner test compile, but you can leave it if you prefer
        
        // Release the Mutex
        xSemaphoreGive(xModeMutex);
    } 
}


// --- TEST FUNCTION ---

void test_mode_change_to_nominal_is_successful() {
    
    // 1. ARRANGE: Set the state to an initial known value
    g_current_mode = MODE_SAFE; 
    
    // 2. ACT: Call the protected function to change the mode
    // This verifies the xSemaphoreTake() and xSemaphoreGive() logic.
    set_system_mode(MODE_NOMINAL);
    
    // 3. ASSERT: Use the protected getter function to check the result
    SystemMode_t current = get_system_mode();
    
    TEST_ASSERT_EQUAL(MODE_NOMINAL, current);
    TEST_ASSERT_NOT_EQUAL(MODE_SAFE, current);
}

// -----------------------------------------------------------
// Standard PlatformIO/Unity Test Runner Boilerplate

void setUp(void) {
    // This runs before EACH test case. We ensure the Mutex object is created here.
    if (xModeMutex == NULL) {
        xModeMutex = xSemaphoreCreateMutex();
    }
} 

void tearDown(void) {
    // This runs after EACH test case.
}

int main(int argc, char **argv) {
    UNITY_BEGIN(); 
    
    RUN_TEST(test_mode_change_to_nominal_is_successful); 
    
    return UNITY_END(); 
}