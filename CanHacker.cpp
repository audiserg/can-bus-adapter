/*
 * CanHacker.cpp
 *
 *  Created on: 17 ���. 2015 �.
 *      Author: Dmitry
 */
#include <Arduino.h>
#include <stdio.h>
#include <ctype.h>

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

CanHackerLineReader::CanHackerLineReader(CanHacker *vCanHacker) {
    canHacker = vCanHacker;
    index = 0;
}

CanHacker::ERROR CanHackerLineReader::processChar(char rxChar) {
    switch (rxChar) {
        case '\r':
        case '\n':
            if (index > 0) {
                buffer[index] = '\0';
            
                CanHacker::ERROR error = canHacker->receiveCommand(buffer, index);
                if (error != CanHacker::ERROR_OK) {
                    return error;
                }
                index = 0;
            }
            break;
  
        case '\0':
            break;
  
        default:
            if (index < COMMAND_MAX_LENGTH) {
              buffer[index++] = rxChar;
            }
            break;
    }
    return CanHacker::ERROR_OK;
}

CanHacker::ERROR CanHacker::writeSerial(const char character) {
    char str[2];
    str[0] = character;
    str[1] = '\0';
    return writeSerial(str);
}

CanHacker::ERROR CanHacker::receiveCommand(const char *buffer, const int length) {
    switch (buffer[0]) {
        case COMMAND_GET_SERIAL: {
            return writeSerial(CANHACKER_SERIAL_RESPONSE);
        }

        case COMMAND_GET_SW_VERSION: {
            return writeSerial(CANHACKER_SW_VERSION_RESPONSE);
        }

        case COMMAND_GET_VERSION: {
            return writeSerial(CANHACKER_VERSION_RESPONSE);
        }

        case COMMAND_SEND_11BIT_ID:
        case COMMAND_SEND_29BIT_ID:
        case COMMAND_SEND_R11BIT_ID:
        case COMMAND_SEND_R29BIT_ID:
            return receiveTransmitCommand(buffer, length);

        case COMMAND_CLOSE_CAN_CHAN:
            return receiveCloseCommand(buffer, length);
            
        case COMMAND_OPEN_CAN_CHAN:
            return receiveOpenCommand(buffer, length);
            
        case COMMAND_SET_BITRATE:
            return receiveSetBitrateCommand(buffer, length);
            
        case COMMAND_SET_ACR:
            return receiveSetAcrCommand(buffer, length);
            
        case COMMAND_SET_AMR:
            return receiveSetAmrCommand(buffer, length);
        
        case COMMAND_SET_BTR: {
            if (isConnected()) {
                writeSerial(BEL);
                return ERROR_CONNECTED;
            }
            return writeSerial(CR);
        }
        
        case COMMAND_LISTEN_ONLY:
            return receiveListenOnlyCommand(buffer, length);
        
        case COMMAND_TIME_STAMP:
            return receiveTimestampCommand(buffer, length);
            
        case COMMAND_WRITE_REG:
        case COMMAND_READ_REG: {
            return writeSerial(CR);
        }
        
        case COMMAND_READ_STATUS:
        case COMMAND_READ_ECR:
        case COMMAND_READ_ALCR: {
            if (!isConnected()) {
                writeSerial(BEL);
                return ERROR_NOT_CONNECTED;
            }
            return writeSerial(CR);
        }

        default: {
            writeSerial(BEL);
            return ERROR_UNEXPECTED_COMMAND;
        }
    }
}

CanHacker::ERROR CanHacker::receiveCanFrame(const struct can_frame *frame) {
    char out[35];
    ERROR error = createTransmit(frame, out, 35);
    if (error != ERROR_OK) {
        return error;
    }
    return writeSerial(out);
}

CanHacker::ERROR CanHacker::parseTransmit(const char *buffer, int length, struct can_frame *frame) {
    if (length < MIN_MESSAGE_LENGTH) {
        return ERROR_INVALID_COMMAND;
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
            return ERROR_INVALID_COMMAND;

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
        return ERROR_INVALID_COMMAND;
    }
    frame->can_dlc = dlc;

    if (!isRTR) {
        for (int i=0; i<dlc; i++) {
            char hiHex = buffer[offset++];
            char loHex = buffer[offset++];
            frame->data[i] = hexCharToByte(loHex) + (hexCharToByte(hiHex) << 4);
        }
    }
    
    return ERROR_OK;
}

CanHacker::ERROR CanHacker::createTransmit(const struct can_frame *frame, char *buffer, const int length) {
    int offset;
    int len = frame->can_dlc;

    int isRTR = frame->can_id & CAN_RTR_FLAG ? 1 : 0;
    
    if (frame->can_id & CAN_ERR_FLAG) {
        return ERROR_ERROR_FRAME_NOT_SUPPORTED;
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
    
    if (_timestampEnabled) {
        uint16_t ts = getTimestamp();
        put_hex_byte(buffer + offset, ts >> 8);
        offset += 2;
        put_hex_byte(buffer + offset, ts);
        offset += 2;
    }
    
    buffer[offset++] = CR;
    buffer[offset] = '\0';
    
    if (offset >= length) {
        return ERROR_BUFFER_OVERFLOW;
    }
    
    return ERROR_OK;
}

CanHacker::ERROR CanHacker::sendFrame(const struct can_frame *frame) {
    return writeCan(frame);
}

CanHacker::ERROR CanHacker::receiveTransmitCommand(const char *buffer, const int length) {
    if (!isConnected()) {
        return ERROR_NOT_CONNECTED;
    }
    
    if (_listenOnly) {
        return ERROR_LISTEN_ONLY;
    }
    
    struct can_frame frame;
    ERROR error = parseTransmit(buffer, length, &frame);
    if (error != ERROR_OK) {
        return error;
    }
    error = writeCan(&frame);
    if (error != ERROR_OK) {
        return error;
    }

    return writeSerial(CR);
}

CanHacker::ERROR CanHacker::receiveTimestampCommand(const char *buffer, const int length) {
    if (length != 2) {
        writeSerial(BEL);
        return ERROR_INVALID_COMMAND;
    }
    switch (buffer[1]) {
        case '0':
            _timestampEnabled = false;
            return writeSerial(CR);
        case '1':
            _timestampEnabled = true;
            return writeSerial(CR);
        default:
            writeSerial(BEL);
            return ERROR_INVALID_COMMAND;
    }

    return ERROR_OK;
}

CanHacker::ERROR CanHacker::receiveCloseCommand(const char *buffer, const int length) {
    if (!isConnected()) {
        return writeSerial(BEL);
    }
    ERROR error = disconnectCan();
    if (error != ERROR_OK) {
        return error;
    }
    return writeSerial(CR);
}

CanHacker::ERROR CanHacker::receiveOpenCommand(const char *buffer, const int length) {
    ERROR error = connectCan();
    if (error != ERROR_OK) {
        return error;
    }
    return writeSerial(CR);
}

CanHacker::ERROR CanHacker::receiveListenOnlyCommand(const char *buffer, const int length) {
    if (isConnected()) {
        writeSerial(BEL);
        return ERROR_CONNECTED;
    }
    _listenOnly = true;
    return writeSerial(CR);
}

CanHacker::ERROR CanHacker::receiveSetAcrCommand(const char *buffer, const int length) {
    if (length != 9) {
        writeSerial(BEL);
        return ERROR_INVALID_COMMAND;
    }
    uint32_t id = 0;
    for (int i=1; i<=8; i++) {
        id <<= 4;
        id += hexCharToByte(buffer[i]);
    }
    
    bool beenConnected = isConnected();
    ERROR error;
    
    if (beenConnected) {
        error = disconnectCan();
        if (error != ERROR_OK) {
            return error;
        }
    }
    
    error = setFilter(id);
    if (error != ERROR_OK) {
        return error;
    }
    
    if (beenConnected) {
        error = connectCan();
        if (error != ERROR_OK) {
            return error;
        }
    }
    
    return writeSerial(CR);
}

CanHacker::ERROR CanHacker::receiveSetAmrCommand(const char *buffer, const int length) {
    if (length != 9) {
        writeSerial(BEL);
        return ERROR_INVALID_COMMAND;
    }
    uint32_t id = 0;
    for (int i=1; i<=8; i++) {
        id <<= 4;
        id += hexCharToByte(buffer[i]);
    }
    
    bool beenConnected = isConnected();
    ERROR error;
    
    if (beenConnected) {
        error = disconnectCan();
        if (error != ERROR_OK) {
            return error;
        }
    }
    
    error = setFilterMask(id);
    if (error != ERROR_OK) {
        return error;
    }
    
    if (beenConnected) {
        error = connectCan();
        if (error != ERROR_OK) {
            return error;
        }
    }
    
    return writeSerial(CR);
}

CanHacker::ERROR CanHacker::setLoopbackEnabled(const bool value) {
    if (isConnected()) {
        return ERROR_CONNECTED;
    }
    
    _loopback = value;
    
    return ERROR_OK;
}
