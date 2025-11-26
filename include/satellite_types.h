// satellite_types.h

#ifndef SATELLITE_TYPES_H
#define SATELLITE_TYPES_H

#include <stdint.h> // For uint8_t, uint16_t, etc.

// --- I. TELEMETRY PACKET (TM) ---
// The structure sent down to the ground station.
typedef struct {
    uint32_t timestamp;         // 32 bits: Time of data acquisition
    uint16_t sequence_count;    // 16 bits: Incremented with every TM packet

    // Status Flags (8 bits total, using bit fields for efficiency)
    // This is the combined status of the C&DH and EPS.
    struct {
        unsigned int flg_low_voltage    : 1; // Bit 0: EPS fault flag
        unsigned int flg_antenna_armed  : 1; // Bit 1: ADCS/Mechanisms status
        unsigned int system_mode        : 3; // Bits 2-4: Current operating state (NOMINAL, SAFE, etc.)
        unsigned int flg_reset_pending  : 1; // Bit 5: FDIR status
        unsigned int reserved           : 2; // Bits 6-7: Padding for 8-bit alignment
    } status_flags;

    float bus_voltage;          // 32 bits: Main battery voltage (from EPS)
    float ext_temp_c;           // 32 bits: External temperature (from TCS)
    
    // Checksum must be the last field calculated over the *preceding* data.
    uint16_t crc_checksum;      // 16 bits: CRC-16 check value
} __attribute__((packed)) HK_Telemetry_t; // Total size: 14 bytes + 2 bytes for CRC = 16 bytes

// --- II. TELECOMMAND PACKET (TC) ---
// The structure received from the ground station (Uplink).
typedef struct {
    uint8_t command_id;         // 8 bits: Defines the action (e.g., CMD_SET_MODE)
    uint8_t parameter_length;   // 8 bits: Length of the data field
    uint32_t execution_time;    // 32 bits: When the command should be run (Time-Tagged)
    uint8_t data[8];            // 64 bits: Optional data payload (parameters)
    uint16_t checksum;          // 16 bits: For integrity check on arrival
} __attribute__((packed)) Command_t;

typedef enum {
    MODE_SAFE,
    MODE_NOMINAL,
    MODE_CRITICAL
} SystemMode_t;

// --- III. EPS INTERNAL STATUS ---
// Data passed internally from the EPS Task to the TM Generator Task via a Queue.
typedef struct {
    float state_of_charge;      // 32 bits: Remaining battery life (%)
    float current_draw;         // 32 bits: Total system current draw
    uint8_t fault_code;         // 8 bits: Detailed fault identifier (0x00=OK, 0x01=UVLO)
} EPS_Status_t;

// --- IV. Enumeration of Telecommand IDs ---
typedef enum {
    TC_NO_OP,        // Command for testing connectivity (no operation)
    TC_SET_MODE,     // Command to change the system mode
    TC_REQUEST_HK    // Command to request immediate Housekeeping Telemetry
} TelecommandID_t;

// --- V. Telecommand Packet Structure ---
typedef struct {
    uint32_t timestamp; 
    TelecommandID_t command_id;
    uint8_t payload[8]; // Simple payload for command arguments (e.g., the new mode value)
    uint16_t crc;       // CRC-16 for integrity
} TelecommandPacket_t;

typedef enum {
    WDT_TASK_TM_GEN,
    WDT_TASK_CMD_PROC,
    WDT_TASK_EPS_MON,
    WDT_TASK_COUNT
} WatchdogTaskID_t;

#endif // SATELLITE_TYPES_H