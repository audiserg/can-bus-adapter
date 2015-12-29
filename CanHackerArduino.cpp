#include "CanHackerArduino.h"

CANHACKER_ERROR CanHackerArduinoLineReader::process() {
    while (Serial.available()) {
        CANHACKER_ERROR error = processChar(Serial.read());
        if (error != CANHACKER_ERROR_OK) {
            return error;
        }
    }
    
    return CANHACKER_ERROR_OK;
}

CanHackerArduino::CanHackerArduino(INT8U _cs, INT8U _mode) {
    cs = _cs;
    mode = _mode;
    mcp2551 = NULL;
}

CANHACKER_ERROR CanHackerArduino::connectCan() {
    mcp2551 = new MCP_CAN(cs, mode);
    
    if (mcp2551->begin(CAN_125KBPS) != MCP2515_OK) {
        return CANHACKER_ERROR_MCP2515_INIT;
    }
    return CANHACKER_ERROR_OK;
}

CANHACKER_ERROR CanHackerArduino::disconnectCan() {
    delete mcp2551;
    mcp2551 = NULL;
    return CANHACKER_ERROR_OK;
}

bool CanHackerArduino::isConnected() {
    return mcp2551 != NULL;
}

CANHACKER_ERROR CanHackerArduino::writeCan(const struct can_frame *frame) {
    INT8U isExtended = frame->can_id & CAN_EFF_FLAG ? 1 : 0;
    INT8U isRTR = frame->can_id & CAN_RTR_FLAG ? 1 : 0;
    canid_t id = frame->can_id & (isExtended ? CAN_EFF_MASK : CAN_SFF_MASK);

    if (mcp2551->sendMsgBuf(id, isExtended, isRTR, frame->can_dlc, frame->data) != CAN_OK) {
        return CANHACKER_ERROR_MCP2515_SEND;
    }
    
    return CANHACKER_ERROR_OK;
}

CANHACKER_ERROR CanHackerArduino::writeSerial(const char *buffer) {
    Serial.print(buffer);
    return CANHACKER_ERROR_OK;
}

CANHACKER_ERROR CanHackerArduino::receiveCan() {
    if (!isConnected()) {
        return CANHACKER_ERROR_OK;
    }
    
    if (CAN_MSGAVAIL != mcp2551->checkReceive()) {
        return CANHACKER_ERROR_OK;
    }
    struct can_frame frame;
    if (mcp2551->readMsgBuf(&frame.can_dlc, frame.data) != CAN_OK) {
        return CANHACKER_ERROR_MCP2515_READ;
    }
    canid_t id = mcp2551->getCanId();
    if (mcp2551->isRemoteRequest()) {
        id |= CAN_RTR_FLAG;
    }
    if (mcp2551->isExtendedFrame()) {
        id |= CAN_EFF_FLAG;
    }

    frame.can_id = id;

    return receiveCanFrame(&frame);
}
