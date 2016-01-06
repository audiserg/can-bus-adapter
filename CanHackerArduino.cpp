#include "CanHackerArduino.h"

CanHacker::ERROR CanHackerArduinoLineReader::process() {
    while (Serial.available()) {
        CanHacker::ERROR error = processChar(Serial.read());
        if (error != CanHacker::ERROR_OK) {
            return error;
        }
    }
    
    return CanHacker::ERROR_OK;
}

CanHackerArduino::CanHackerArduino(uint8_t _cs, MCP_CAN::MODE _mode) {
    cs = _cs;
    mode = _mode;
    mcp2551 = NULL;
}

CanHacker::ERROR CanHackerArduino::connectCan() {
    mcp2551 = new MCP_CAN(cs, mode);
    
    if (mcp2551->begin(bitrate) != MCP_CAN::ERROR_OK) {
        return ERROR_MCP2515_INIT;
    }
    return ERROR_OK;
}

CanHacker::ERROR CanHackerArduino::disconnectCan() {
    delete mcp2551;
    mcp2551 = NULL;
    return ERROR_OK;
}

bool CanHackerArduino::isConnected() {
    return mcp2551 != NULL;
}

CanHacker::ERROR CanHackerArduino::writeCan(const struct can_frame *frame) {
    if (mcp2551->sendMessage(frame) != MCP_CAN::ERROR_OK) {
        return ERROR_MCP2515_SEND;
    }
    
    return ERROR_OK;
}

CanHacker::ERROR CanHackerArduino::writeSerial(const char *buffer) {
    if (Serial.availableForWrite() < strlen(buffer)) {
        return ERROR_SERIAL_TX_OVERRUN;
    }
    Serial.print(buffer);
    return ERROR_OK;
}

CanHacker::ERROR CanHackerArduino::pollReceiveCan() {
    if (!isConnected()) {
        return ERROR_OK;
    }
    
    while (mcp2551->checkReceive()) {
        struct can_frame frame;
        if (mcp2551->readMessage(&frame) != MCP_CAN::ERROR_OK) {
            return ERROR_MCP2515_READ;
        }
    
        ERROR error = receiveCanFrame(&frame);
        if (error != ERROR_OK) {
            return error;
        }
    }
    
    return ERROR_OK;
}

CanHacker::ERROR CanHackerArduino::receiveCan(const MCP_CAN::RXBn rxBuffer) {
    if (!isConnected()) {
        return ERROR_OK;
    }
    
    bool received = false;
    do {
        struct can_frame frame;
        MCP_CAN::ERROR result = mcp2551->readMessage(rxBuffer, &frame);
        if (result == MCP_CAN::ERROR_NOMSG) {
            break;
        }
        
        if (result != MCP_CAN::ERROR_OK) {
            return ERROR_MCP2515_READ;
        }

        ERROR error = receiveCanFrame(&frame);
        if (error != ERROR_OK) {
            return error;
        }
    } while (received);
    
    return ERROR_OK;
}

MCP_CAN *CanHackerArduino::getMcp2515() {
    return mcp2551;
}

uint16_t CanHackerArduino::getTimestamp() {
    return millis() % TIMESTAMP_LIMIT;
}

CanHacker::ERROR CanHackerArduino::receiveSetBitrateCommand(const char *buffer, const int length) {
    if (isConnected()) {
        CanHacker::writeSerial(BEL);
        return ERROR_CONNECTED;
    }
    
    if (length < 2) {
        CanHacker::writeSerial(BEL);
        return ERROR_INVALID_COMMAND;
    }
    switch(buffer[1]) {
        case '0':
            bitrate = CAN_10KBPS;
            break;
        case '1':
            bitrate = CAN_20KBPS;
            break;
        case '2':
            bitrate = CAN_50KBPS;
            break;
        case '3':
            bitrate = CAN_100KBPS;
            break;
        case '4':
            bitrate = CAN_125KBPS;
            break;
        case '5':
            bitrate = CAN_250KBPS;
            break;
        case '6':
            bitrate = CAN_500KBPS;
            break;
        case '7':
            CanHacker::writeSerial(BEL);
            return ERROR_INVALID_COMMAND;
            break;
        case '8':
            bitrate = CAN_1000KBPS;
            break;
        default:
            CanHacker::writeSerial(BEL);
            return ERROR_INVALID_COMMAND;
            break;
    }
    
    CanHacker::writeSerial(CR);
    return ERROR_OK;
}
