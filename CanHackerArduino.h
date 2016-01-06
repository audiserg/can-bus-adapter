#ifndef CANHACKERARDUINO_H_
#define CANHACKERARDUINO_H_

#include <mcp_can.h>
#include <mcp_can_dfs.h>

#include "CanHacker.h"

class CanHackerArduino : public CanHacker {
    private:
        uint8_t cs;
        MCP_CAN::MODE mode;
        MCP_CAN *mcp2551;
        
        CANHACKER_ERROR connectCan();
        CANHACKER_ERROR disconnectCan();
        bool isConnected();
        CANHACKER_ERROR writeCan(const struct can_frame *);
        CANHACKER_ERROR writeSerial(const char *buffer);
        
    public:
        CanHackerArduino(uint8_t _cs, MCP_CAN::MODE _mode);
        CANHACKER_ERROR pollReceiveCan();
        CANHACKER_ERROR receiveCan(const MCP_CAN::RXBn rxBuffer);
        MCP_CAN *getMcp2515();
};

class CanHackerArduinoLineReader : public CanHackerLineReader {
    public:
        CanHackerArduinoLineReader(CanHackerArduino *vCanHacker) : CanHackerLineReader(vCanHacker) { };
        CANHACKER_ERROR process();
};

#endif /* CANHACKERARDUINO_H_ */
