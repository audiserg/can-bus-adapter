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

#define CANHACKER_OK 0
#define CANHACKER_ERROR -1

#define CANHACKER_VERSION       "1010"      // hardware version
#define CANHACKER_SW_VERSION    "0107"
#define CANHACKER_SERIAL        "0001"    // device serial number
#define CANHACKER_CR            (char)'\r'
#define CANHACKER_BEL           (char)7
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

typedef void (*SerialOutputCallbackType)(const char *);
typedef void (*CanOutputCallbackType)(const struct can_frame *);

class CanHacker {
    private:
        SerialOutputCallbackType serialOutputCallback;
        CanOutputCallbackType canOutputCallback;
        void canOutput(const struct can_frame *frame);
        void serialOutput(const char *buffer);
        void serialOutput(const char character);
    public:
        CanHacker(SerialOutputCallbackType vSerialOutputCallback, CanOutputCallbackType vCanOutputCallback);
        virtual ~CanHacker();

        int receiveCommand(const char *buffer, const int length);
        int receiveCanFrame(const struct can_frame *frame);
        int parseTransmit(const char *buffer, int length, struct can_frame *frame);
        int createTransmit(const struct can_frame *frame, char *buffer, const int length);
};

#endif /* CANHACKER_H_ */
