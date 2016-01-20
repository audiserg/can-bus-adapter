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

CanHackerArduino::CanHackerArduino(uint8_t cs) {
    _cs = cs;
    mcp2515 = new MCP_CAN(_cs);
    mcp2515->reset();
    mcp2515->setConfigMode();
}

CanHackerArduino::~CanHackerArduino() {
    delete mcp2515;
}

CanHacker::ERROR CanHackerArduino::connectCan() {
    if (mcp2515->setBitrate(bitrate) != MCP_CAN::ERROR_OK) {
        return ERROR_MCP2515_INIT_BITRATE;
    }
    
    MCP_CAN::ERROR error;
    if (_loopback) {
        error = mcp2515->setLoopbackMode();
    } else if (_listenOnly) {
        error = mcp2515->setListenOnlyMode();
    } else {
        error = mcp2515->setNormalMode();
    }
    
    if (error != MCP_CAN::ERROR_OK) {
        return ERROR_MCP2515_INIT_SET_MODE;
    }
    
    _isConnected = true;
    return ERROR_OK;
}

CanHacker::ERROR CanHackerArduino::disconnectCan() {
    _isConnected = false;
    mcp2515->setConfigMode();
    return ERROR_OK;
}

bool CanHackerArduino::isConnected() {
    return _isConnected;
}

CanHacker::ERROR CanHackerArduino::writeCan(const struct can_frame *frame) {
    if (mcp2515->sendMessage(frame) != MCP_CAN::ERROR_OK) {
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
    
    while (mcp2515->checkReceive()) {
        struct can_frame frame;
        if (mcp2515->readMessage(&frame) != MCP_CAN::ERROR_OK) {
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
    
    struct can_frame frame;
    MCP_CAN::ERROR result = mcp2515->readMessage(rxBuffer, &frame);
    if (result == MCP_CAN::ERROR_NOMSG) {
        return ERROR_OK;
    }
    
    if (result != MCP_CAN::ERROR_OK) {
        return ERROR_MCP2515_READ;
    }

    return receiveCanFrame(&frame);
}

MCP_CAN *CanHackerArduino::getMcp2515() {
    return mcp2515;
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
    
    return CanHacker::writeSerial(CR);
}

CanHacker::ERROR CanHackerArduino::processInterrupt() {
    if (!isConnected()) {
        return ERROR_NOT_CONNECTED;
    }

    uint8_t irq = mcp2515->getInterrupts();
    
    if (irq & MCP_CAN::CANINTF_RX0IF) {
        ERROR error = receiveCan(MCP_CAN::RXB0);
        if (error != ERROR_OK) {
            return error;
        }
    }
    
    if (irq & MCP_CAN::CANINTF_RX1IF) {
        ERROR error = receiveCan(MCP_CAN::RXB1);
        if (error != ERROR_OK) {
            return error;
        }
    }
    
    /*if (irq & (MCP_CAN::CANINTF_TX0IF | MCP_CAN::CANINTF_TX1IF | MCP_CAN::CANINTF_TX2IF)) {
        Serial.print("MCP_TXxIF\r\n");
        stopAndBlink(1);
    }*/
     
    if (irq & MCP_CAN::CANINTF_ERRIF) {
        Serial.print("MCP_ERRIF\r\n");
        return ERROR_MCP2515_ERRIF;
    }
     
    /*if (irq & MCP_CAN::CANINTF_WAKIF) {
        Serial.print("MCP_WAKIF\r\n");
        stopAndBlink(3);
    }*/
     
    if (irq & MCP_CAN::CANINTF_MERRF) {
        Serial.print("MCP_MERRF\r\n");
        return ERROR_MCP2515_MERRF;
    }
    
    return ERROR_OK;
}

CanHacker::ERROR CanHackerArduino::setFilter(const uint32_t filter) {
    if (isConnected()) {
        return ERROR_CONNECTED;
    }
    
    MCP_CAN::RXF filters[] = {MCP_CAN::RXF0, MCP_CAN::RXF1, MCP_CAN::RXF2, MCP_CAN::RXF3, MCP_CAN::RXF4, MCP_CAN::RXF5};
    for (int i=0; i<6; i++) {
        MCP_CAN::ERROR result = mcp2515->setFilter(filters[i], false, filter);
        if (result != MCP_CAN::ERROR_OK) {
            return ERROR_MCP2515_FILTER;
        }
    }
    
    return ERROR_OK;
}

CanHacker::ERROR CanHackerArduino::setFilterMask(const uint32_t mask) {
    if (isConnected()) {
        return ERROR_CONNECTED;
    }
    
    MCP_CAN::MASK masks[] = {MCP_CAN::MASK0, MCP_CAN::MASK1};
    for (int i=0; i<2; i++) {
        MCP_CAN::ERROR result = mcp2515->setFilterMask(masks[i], false, mask);
        if (result != MCP_CAN::ERROR_OK) {
            return ERROR_MCP2515_FILTER;
        }
    }
    
    return ERROR_OK;
}
