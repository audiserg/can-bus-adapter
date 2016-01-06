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

CanHackerArduino::CanHackerArduino(uint8_t _cs, MCP_CAN::MODE _mode) {
    cs = _cs;
    mode = _mode;
    mcp2551 = NULL;
}

CANHACKER_ERROR CanHackerArduino::connectCan() {
    mcp2551 = new MCP_CAN(cs, mode);
    
    if (mcp2551->begin(CAN_125KBPS) != MCP_CAN::ERROR_OK) {
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
    uint8_t isExtended = frame->can_id & CAN_EFF_FLAG ? 1 : 0;
    uint8_t isRTR = frame->can_id & CAN_RTR_FLAG ? 1 : 0;
    canid_t id = frame->can_id & (isExtended ? CAN_EFF_MASK : CAN_SFF_MASK);

    if (mcp2551->sendMessage(id, isExtended, isRTR, frame->can_dlc, frame->data) != MCP_CAN::ERROR_OK) {
        return CANHACKER_ERROR_MCP2515_SEND;
    }
    
    return CANHACKER_ERROR_OK;
}

CANHACKER_ERROR CanHackerArduino::writeSerial(const char *buffer) {
    if (Serial.availableForWrite() < strlen(buffer)) {
        return CANHACKER_ERROR_SERIAL_TX_OVERRUN;
    }
    Serial.print(buffer);
    return CANHACKER_ERROR_OK;
}

CANHACKER_ERROR CanHackerArduino::pollReceiveCan() {
    if (!isConnected()) {
        return CANHACKER_ERROR_OK;
    }
    
    while (mcp2551->checkReceive()) {
        struct can_frame frame;
        canid_t id;
        bool rtr, ext;
        if (mcp2551->readMessage(&id, &frame.can_dlc, frame.data, &rtr, &ext) != MCP_CAN::ERROR_OK) {
            return CANHACKER_ERROR_MCP2515_READ;
        }
        if (rtr) {
            id |= CAN_RTR_FLAG;
        }
        if (ext) {
            id |= CAN_EFF_FLAG;
        }
    
        frame.can_id = id;
    
        CANHACKER_ERROR error = receiveCanFrame(&frame);
        if (error != CANHACKER_ERROR_OK) {
            return error;
        }
    }
    
    return CANHACKER_ERROR_OK;
}

CANHACKER_ERROR CanHackerArduino::receiveCan(const MCP_CAN::RXBn rxBuffer) {
    if (!isConnected()) {
        return CANHACKER_ERROR_OK;
    }
    
    bool received = false;
    do {
        struct can_frame frame;
        canid_t id;
        bool rtr, ext;
        MCP_CAN::ERROR result = mcp2551->readMessage(rxBuffer, &id, &frame.can_dlc, frame.data, &rtr, &ext);
        
        if (result == MCP_CAN::ERROR_NOMSG) {
            break;
        }
        
        if (result != MCP_CAN::ERROR_OK) {
            return CANHACKER_ERROR_MCP2515_READ;
        }

        if (rtr) {
            id |= CAN_RTR_FLAG;
        }
        if (ext) {
            id |= CAN_EFF_FLAG;
        }
    
        frame.can_id = id;
    
        CANHACKER_ERROR error = receiveCanFrame(&frame);
        if (error != CANHACKER_ERROR_OK) {
            return error;
        }
    } while (received);
    
    return CANHACKER_ERROR_OK;
}

MCP_CAN *CanHackerArduino::getMcp2515() {
    return mcp2551;
}
