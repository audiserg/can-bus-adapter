#ifndef CANHACKERARDUINO_H_
#define CANHACKERARDUINO_H_

#include <mcp_can.h>

#include "CanHacker.h"

class CanHackerArduino : public CanHacker {
    private:
        uint8_t _cs;
        MCP_CAN *mcp2515;
        CAN_SPEED bitrate;
        bool _isConnected = false;
        
        ERROR connectCan();
        ERROR disconnectCan();
        bool isConnected();
        ERROR writeCan(const struct can_frame *);
        ERROR writeSerial(const char *buffer);
        ERROR receiveSetBitrateCommand(const char *buffer, const int length);
        
    public:
        CanHackerArduino(uint8_t cs);
        ~CanHackerArduino();
        ERROR pollReceiveCan();
        ERROR receiveCan(const MCP_CAN::RXBn rxBuffer);
        uint16_t getTimestamp();
        MCP_CAN *getMcp2515();
        ERROR processInterrupt();
        ERROR setFilter(const uint32_t filter);
        ERROR setFilterMask(const uint32_t mask);
};

class CanHackerArduinoLineReader : public CanHackerLineReader {
    public:
        CanHackerArduinoLineReader(CanHackerArduino *vCanHacker) : CanHackerLineReader(vCanHacker) { };
        CanHacker::ERROR process();
};

#endif /* CANHACKERARDUINO_H_ */
