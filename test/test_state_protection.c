// test/test_state_protection.c

#include <unity.h>               // Unity Test Framework
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"    // For SemaphoreHandle_t
#include "state_manager.h"       // Functions to test: get/set_system_mode
#include "satellite_types.h"     // Needed for SystemMode_t


// --- GLOBAL VARIABLE DEFINITIONS FOR HOST TESTING ---
// These declarations link to your actual functions in src/state_manager.c
SystemMode_t g_current_mode = MODE_SAFE;
SemaphoreHandle_t xModeMutex = NULL; 

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