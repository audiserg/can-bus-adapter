/*
 * CanHacker.h
 *
 *  Created on: 17 ���. 2015 �.
 *      Author: Dmitry
 */

#ifndef CANHACKER_H_
#define CANHACKER_H_

#include "can.h"

#define CAN_MIN_DLEN 1
#define HEX_PER_BYTE 2
#define MIN_MESSAGE_DATA_HEX_LENGTH CAN_MIN_DLEN * HEX_PER_BYTE
#define MAX_MESSAGE_DATA_HEX_LENGTH CAN_MAX_DLEN * HEX_PER_BYTE
#define MIN_MESSAGE_LENGTH 5

#define CANHACKER_CMD_MAX_LENGTH 26

enum CANHACKER_ERROR { 
    CANHACKER_ERROR_OK, 
    CANHACKER_ERROR_CONNECTED, 
    CANHACKER_ERROR_NOT_CONNECTED, 
    CANHACKER_ERROR_UNEXPECTED_COMMAND,
    CANHACKER_ERROR_INVALID_COMMAND,
    CANHACKER_ERROR_ERROR_FRAME_NOT_SUPPORTED,
    CANHACKER_ERROR_BUFFER_OVERFLOW,
    CANHACKER_ERROR_MCP2515_INIT,
    CANHACKER_ERROR_MCP2515_SEND,
    CANHACKER_ERROR_MCP2515_READ
};

#define CANHACKER_VERSION       "1010"      // hardware version
#define CANHACKER_SW_VERSION    "0107"
#define CANHACKER_SERIAL        "0001"    // device serial number
#define CANHACKER_CR            13
#define CANHACKER_BEL           7
#define CANHACKER_SET_BITRATE     'S' // set CAN bit rate
#define CANHACKER_SET_BTR         's' // set CAN bit rate via
#define CANHACKER_OPEN_CAN_CHAN   'O' // open CAN channel
#define CANHACKER_CLOSE_CAN_CHAN  'C' // close CAN channel
#define CANHACKER_SEND_11BIT_ID   't' // send CAN message with 11bit ID
#define CANHACKER_SEND_29BIT_ID   'T' // send CAN message with 29bit ID
#define CANHACKER_SEND_R11BIT_ID  'r' // send CAN remote message with 11bit ID
#define CANHACKER_SEND_R29BIT_ID  'R' // send CAN remote message with 29bit ID
#define CANHACKER_READ_STATUS     'F' // read status flag byte
#define CANHACKER_SET_ACR         'M' // set Acceptance Code Register
#define CANHACKER_SET_AMR         'm' // set Acceptance Mask Register
#define CANHACKER_GET_VERSION     'V' // get hardware and software version
#define CANHACKER_GET_SW_VERSION  'v' // get software version only
#define CANHACKER_GET_SERIAL      'N' // get device serial number
#define CANHACKER_TIME_STAMP      'Z' // toggle time stamp setting
#define CANHACKER_READ_ECR        'E' // read Error Capture Register
#define CANHACKER_READ_ALCR       'A' // read Arbritation Lost Capture Register
#define CANHACKER_READ_REG        'G'   // read register conten from SJA1000
#define CANHACKER_WRITE_REG       'W'   // write register content to SJA1000
#define CANHACKER_LISTEN_ONLY     'L' // switch to listen only mode

#define TIME_STAMP_TICK 1000    // microseconds

#define CANHACKER_SERIAL_RESPONSE     "N" CANHACKER_SERIAL     "\r"
#define CANHACKER_SW_VERSION_RESPONSE "v" CANHACKER_SW_VERSION "\r"
#define CANHACKER_VERSION_RESPONSE    "V" CANHACKER_VERSION    "\r"

class CanHacker {
    private:
        CANHACKER_ERROR parseTransmit(const char *buffer, int length, struct can_frame *frame);
        CANHACKER_ERROR createTransmit(const struct can_frame *frame, char *buffer, const int length);
        
        virtual CANHACKER_ERROR connectCan();
        virtual CANHACKER_ERROR disconnectCan();
        virtual bool isConnected();
        virtual CANHACKER_ERROR writeCan(const struct can_frame *);
        virtual CANHACKER_ERROR writeSerial(const char *buffer);
        
        CANHACKER_ERROR writeSerial(const char character);
        
    public:
        CANHACKER_ERROR receiveCommand(const char *buffer, const int length);
        CANHACKER_ERROR receiveCanFrame(const struct can_frame *frame);
};

class CanHackerLineReader {
    private:
        static const int COMMAND_MAX_LENGTH = 30; // not including \r\0
        
        CanHacker *canHacker;
        char buffer[COMMAND_MAX_LENGTH + 2];
        int index;
    public:
        CanHackerLineReader(CanHacker *vCanHacker);
        CANHACKER_ERROR processChar(char rxChar);
};

#endif /* CANHACKER_H_ */
