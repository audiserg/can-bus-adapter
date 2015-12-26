/*
 * CanHacker.cpp
 *
 *  Created on: 17 ���. 2015 �.
 *      Author: Dmitry
 */
#include <stdio.h>
#include <ctype.h>
#include <Arduino.h>

#include "CanHacker.h"
#include "lib.h"

const char hex_asc_upper[] = "0123456789ABCDEF";

#define hex_asc_upper_lo(x)    hex_asc_upper[((x) & 0x0F)]
#define hex_asc_upper_hi(x)    hex_asc_upper[((x) & 0xF0) >> 4]

static inline void put_hex_byte(char *buf, __u8 byte)
{
    buf[0] = hex_asc_upper_hi(byte);
    buf[1] = hex_asc_upper_lo(byte);
}

static inline void _put_id(char *buf, int end_offset, canid_t id)
{
    /* build 3 (SFF) or 8 (EFF) digit CAN identifier */
    while (end_offset >= 0) {
        buf[end_offset--] = hex_asc_upper[id & 0xF];
        id >>= 4;
    }
}

#define put_sff_id(buf, id) _put_id(buf, 2, id)
#define put_eff_id(buf, id) _put_id(buf, 7, id)

CanHacker::CanHacker (SerialOutputCallbackType vSerialOutputCallback, CanOutputCallbackType vCanOutputCallback)
{
    serialOutputCallback = vSerialOutputCallback;
    canOutputCallback = vCanOutputCallback;
}

CanHacker::~CanHacker ()
{

}

void CanHacker::canOutput(const struct can_frame *frame) {
    canOutputCallback(frame);
}

void CanHacker::serialOutput(const char *str) {
    serialOutputCallback(str);
}

void CanHacker::serialOutput(const char character) {
    char str[2];
    str[0] = character;
    str[1] = '\0';
    serialOutput(str);
}

int CanHacker::receiveCommand(const char *buffer, const int length) {
    switch (buffer[0]) {
        case CANHACKER_GET_SERIAL:
            serialOutput(CANHACKER_SERIAL_RESPONSE);
            return CANHACKER_OK;

        case CANHACKER_GET_SW_VERSION:
            serialOutput(CANHACKER_SW_VERSION_RESPONSE);
            return CANHACKER_OK;

        case CANHACKER_GET_VERSION:
            serialOutput(CANHACKER_VERSION_RESPONSE);
            return CANHACKER_OK;

        case CANHACKER_SEND_11BIT_ID:
        case CANHACKER_SEND_29BIT_ID:
        case CANHACKER_SEND_R11BIT_ID:
        case CANHACKER_SEND_R29BIT_ID:
            struct can_frame frame;
            if (parseTransmit(buffer, length, &frame) == CANHACKER_ERROR) {
                return CANHACKER_ERROR;
            }
            canOutput(&frame);

            serialOutput(CANHACKER_CR);
            return CANHACKER_OK;

        case CANHACKER_CLOSE_CAN_CHAN:
        case CANHACKER_OPEN_CAN_CHAN:
        case CANHACKER_SET_BITRATE:
        case CANHACKER_SET_BTR:
        case CANHACKER_LISTEN_ONLY:
        case CANHACKER_WRITE_REG:
        case CANHACKER_TIME_STAMP:
        case CANHACKER_SET_ACR:
        case CANHACKER_SET_AMR:
        case CANHACKER_READ_ECR:
        case CANHACKER_READ_ALCR:
        case CANHACKER_READ_REG:
            serialOutput(CANHACKER_CR);
            return CANHACKER_OK;

        default:
            return CANHACKER_BEL;
    }
}

int CanHacker::receiveCanFrame(const struct can_frame *frame) {
    char out[30];
    createTransmit(frame, out, 30);
    serialOutput(out);
    return CANHACKER_OK;
}

int CanHacker::parseTransmit(const char *buffer, int length, struct can_frame *frame) {
    if (length < MIN_MESSAGE_LENGTH) {
        return CANHACKER_ERROR;
    }

    int isExended = 0;
    int isRTR = 0;

    switch (buffer[0]) {
        case 't':
            break;
        case 'T':
            isExended = 1;
            break;
        case 'r':
            isRTR = 1;
            break;
        case 'R':
            isExended = 1;
            isRTR = 1;
            break;
        default:
            return CANHACKER_ERROR;

    }
    
    int offset = 1;

    canid_t id = 0;
    int idChars = isExended ? 8 : 3;
    for (int i=0; i<idChars; i++) {
        id <<= 4;
        id += hexCharToByte(buffer[offset++]);
    }
    if (isRTR) {
        id |= CAN_RTR_FLAG;
    }
    if (isExended) {
        id |= CAN_EFF_FLAG;
    }
    frame->can_id = id;
    
    __u8 dlc = hexCharToByte(buffer[offset++]);
    if (dlc > 8) {
        return CANHACKER_ERROR;
    }
    frame->can_dlc = dlc;

    if (!isRTR) {
        for (int i=0; i<dlc; i++) {
            char hiHex = buffer[offset++];
            char loHex = buffer[offset++];
            frame->data[i] = hexCharToByte(loHex) + (hexCharToByte(hiHex) << 4);
        }
    }

    return CANHACKER_OK;
}

int CanHacker::createTransmit(const struct can_frame *frame, char *buffer, const int length) {
    int offset;
    int len = frame->can_dlc;

    int isRTR = frame->can_id & CAN_RTR_FLAG ? 1 : 0;
    
    if (frame->can_id & CAN_ERR_FLAG) {
        return CANHACKER_ERROR;
    } else if (frame->can_id & CAN_EFF_FLAG) {
        buffer[0] = isRTR ? 'R' : 'T';
        put_eff_id(buffer+1, frame->can_id & CAN_EFF_MASK);
        offset = 9;
    } else {
        buffer[0] = isRTR ? 'r' : 't';
        put_sff_id(buffer+1, frame->can_id & CAN_SFF_MASK);
        offset = 4;
    }

    buffer[offset++] = hex_asc_upper_lo(frame->can_dlc);

    if (!isRTR) {
        int i;
        for (i = 0; i < len; i++) {
            put_hex_byte(buffer + offset, frame->data[i]);
            offset += 2;
        }
    }

    buffer[offset++] = CANHACKER_CR;
    buffer[offset] = '\0';
    
    return CANHACKER_OK;
}
