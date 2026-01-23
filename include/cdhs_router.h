#ifndef CDHS_ROUTER_H
#define CDHS_ROUTER_H

#include <stdint.h>

/**
 * @brief The main entry point for the "Brain". 
 * Comms calls this when it has a valid CCSDS packet.
 */
void CDHS_RoutePacket(const uint8_t* packet_buffer, uint16_t length);

#endif