// satellite_types.h

#ifndef SATELLITE_TYPES_H
#define SATELLITE_TYPES_H

#include <stdint.h> // For uint8_t, uint16_t, etc.

// --- I. CCSDS CONSTANTS ---
#define CCSDS_HEADER_SIZE 14  // Primary (6) + Secondary (8)

// --- II. SYSTEM MODES ---
typedef enum {
    MODE_INITIAL_BOOT,
    MODE_SAFE,
    MODE_NOMINAL,
    MODE_DEGRADED,
    MODE_EMERGENCY,
    MODE_CRITICAL
} SystemMode_t;

// --- III. TELECOMMAND PACKET STRUCTURE ---
// We use a "Union" or a "Raw Buffer" approach so tc_proc can 
// check the CRC of the whole packet before the Router reads the APID.
typedef struct {
    uint8_t raw_data[64]; // Maximum expected packet size (CCSDS Header + Payload + CRC)
    uint16_t length;      // Actual length of the received data
    uint16_t packet_crc;  // Extracted CRC for tc_proc to verify
} TelecommandPacket_t;

// --- IV. HOUSEKEEPING TELEMETRY ---
typedef struct {
    uint32_t timestamp;
    uint16_t sequence_count;

    struct {
        unsigned int flg_low_voltage   : 1; 
        unsigned int flg_antenna_armed : 1; 
        unsigned int system_mode       : 3; 
        unsigned int flg_reset_pending : 1; 
        unsigned int reserved          : 2; 
    } status_flags;

    float bus_voltage;
    float ext_temp_c;
    uint16_t crc_checksum;
} __attribute__((packed)) HK_Telemetry_t;

// --- V. INTERNAL COMMAND IDs (Used in Router/Subsystems) ---
typedef enum {
    TC_NO_OP = 0,
    TC_SET_MODE = 1,
    TC_REQUEST_HK = 2,
    CDH_CMD_REBOOT = 3,
    CDH_CMD_SYNC_TIME = 4
} TelecommandID_t;

// --- VI. WATCHDOG & MODES ---
typedef enum {
    WDT_TASK_TM_GEN,
    WDT_TASK_CMD_PROC,
    WDT_TASK_EPS_MON,
    WDT_TASK_DATA_LOG,
    WDT_TASK_ADCS,
    WDT_TASK_COUNT
} WatchdogTaskID_t;

typedef enum{
    DOWNLINK_INACTIVE,
    DOWNLINK_ACTIVE
} DownlinkMode_t;

#endif // SATELLITE_TYPES_H