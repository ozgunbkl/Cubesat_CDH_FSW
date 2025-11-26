// src/utils.c

#include "utils.h"
#include <stdint.h>
#include <stddef.h>

// Standard CRC-16 CCITT polynomial (0x1021)
#define CRC16_POLY 0x1021 
#define CRC16_INITIAL 0xFFFF

uint16_t crc16_ccitt(const uint8_t *data, size_t length) {
    uint16_t crc = CRC16_INITIAL;
    size_t i, j;

    if (data == NULL) {
        return 0; // Handle null pointer case
    }
    
    for (i = 0; i < length; i++) {
        crc ^= ((uint16_t)data[i] << 8);
        for (j = 0; j < 8; j++) {
            if (crc & 0x8000) {
                crc = (crc << 1) ^ CRC16_POLY;
            } else {
                crc <<= 1;
            }
        }
    }
    return crc;
}