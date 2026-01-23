#include "cdhs_router.h"
#include "ccsds_packet.h"
#include <stdio.h>

// I will also include other subsystem headers here 

void CDHS_RoutePacket(const uint8_t* packet_buffer, uint16_t length){
    // 1. Extract the APID
    uint16_t apid = CCSDS_GetAPID(packet_buffer);

    
    // 2. Skip Primary (6) + Secondary (8) = 14 bytes to get to User Data
    const uint8_t* userData = packet_buffer + 14; 
    uint16_t userDataLen = length - 14;

    // 3. Post office switch
    switch (apid){
        case APID_ADCS:
            printf("CDHS: Routing packet to ADCS...\n");
            // ADCS_ProcessCommand(payload, payload_len);
            break;
        
        case APID_EPS:
            printf("CDHS: Routing packet to EPS...\n");
            // EPS_ProcessCommand(payload, payload_len); 
            break;

        case APID_HK:
            printf("CDHS: Routing packet to HK...\n");
            // HK_ProcessCommand(payload, payload_len);
            break;

        case APID_ARCHIVE:
            printf("CDHS: Routing packet to ARCHIVE...\n");
            // ADCS_ProcessCommand(payload, payload_len);
            break;
        
        case APID_FDIR:
            printf("CDHS: Routing packet to FDIR...\n");
            // ADCS_ProcessCommand(payload, payload_len);
            break;

        case APID_PAYLOAD:
            printf("CDHS: Routing packet to PAYLOAD...\n");
            // ADCS_ProcessCommand(payload, payload_len);
            break;

        case APID_CDHS:
            printf("CDHS: Routing packet to CDHS...\n");
            // ADCS_ProcessCommand(payload, payload_len);
            break;

        case APID_IDLE:
            
            break;
        
        default:
            printf("CDHS WARNING: Unknown APID 0x%03X recieved!\n", apid);
            // FDIR "Invalid Command"
            break;
    }
}