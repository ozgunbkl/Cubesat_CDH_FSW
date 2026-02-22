#include "cdhs_router.h"
#include "ccsds_packet.h"
#include "adcs_service.h"
#include "eps_commands.h"
#include "hk_service.h"
#include "archive_service.h"
#include "fdir_service.h"
#include "payload_service.h"
#include "commands.h"
#include "time_service.h"

#include <stdio.h>

 

void CDHS_RoutePacket(const uint8_t* packet_buffer, uint16_t length){
    if (length < 14) {
        FaultReport_t len_fault = {
            .source = SRC_CDH,
            .severity = FAULT_WARNING,
            .fault_code = FAULT_CDH_PACKET_TOO_SHORT,
            .timestamp = Time_GetSeconds()
        };
        FDIR_ReportFault(len_fault);
        printf("CDHS: Packet too short (%d bytes). Dropping.\n", length);
        return;
    }
    
    // 1. Extract the APID
    uint16_t apid = CCSDS_GetAPID(packet_buffer);

    
    // 2. Skip Primary (6) + Secondary (8) = 14 bytes to get to User Data
    const uint8_t* userData = packet_buffer + 14; 
    uint16_t userDataLen = length - 14;

    // 3. Post office switch
    switch (apid){
        case APID_ADCS:
            printf("CDHS: Routing packet to ADCS...\n");
            ADCS_ProcessCommand(userData, userDataLen);
            break;
        
        case APID_EPS:
            printf("CDHS: Routing packet to EPS...\n");
            EPS_ProcessCommand(userData, userDataLen); 
            break;

        case APID_HK:
            printf("CDHS: Routing packet to HK...\n");
            HK_ProcessCommand(userData, userDataLen);
            break;

        case APID_ARCHIVE:
            printf("CDHS: Routing packet to ARCHIVE...\n");
            ARCHIVE_ProcessCommand(userData, userDataLen);
            break;
        
        case APID_FDIR:
            printf("CDHS: Routing packet to FDIR...\n");
            FDIR_ProcessCommand(userData, userDataLen);
            break;

        case APID_PAYLOAD:
            printf("CDHS: Routing packet to PAYLOAD...\n");
            PAYLOAD_ProcessCommandWrapper(userData, userDataLen);
            break;

        case APID_CDHS:
            printf("CDHS: Routing packet to CDHS...\n");
            handle_internal_cdh_cmd(userData, userDataLen);
            break;

        case APID_IDLE:
            
            break;
        
        default:
            printf("CDHS WARNING: Unknown APID 0x%03X recieved!\n", apid);
            FaultReport_t unknown_apid_fault = {
                .source = SRC_CDH,
                .severity = FAULT_WARNING,
                .fault_code = FAULT_CDH_UNKNOWN_APID,
                .timestamp = Time_GetSeconds()
            };
            FDIR_ReportFault(unknown_apid_fault);
            break;
    }
}


static void handle_internal_cdh_cmd(const uint8_t* payload, uint16_t len) {
    if (len < 1) return;

    switch (payload[0]) {
        case CDH_CMD_REBOOT:
            printf("CDHS: System Rebooting...\n");
            // If using ESP32: esp_restart();
            break;
        case CDH_CMD_SYNC_TIME:
            // Update your internal RTC or software clock
            printf("CDHS: Clock synchronized.\n");
            break;
    }
}