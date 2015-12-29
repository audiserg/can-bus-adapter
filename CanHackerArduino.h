#ifndef CANHACKERARDUINO_H_
#define CANHACKERARDUINO_H_

#include <Arduino.h>
#include <mcp_can.h>
#include <mcp_can_dfs.h>

#include "CanHacker.h"

class CanHackerArduino : public CanHacker {
    private:
        INT8U cs;
        INT8U mode;
        MCP_CAN *mcp2551;
        
        CANHACKER_ERROR connectCan();
        CANHACKER_ERROR disconnectCan();
        bool isConnected();
        CANHACKER_ERROR writeCan(const struct can_frame *);
        CANHACKER_ERROR writeSerial(const char *buffer);
        
    public:
        CanHackerArduino(INT8U _cs, INT8U _mode);
        CANHACKER_ERROR receiveCan();
};

class CanHackerArduinoLineReader : public CanHackerLineReader {
    public:
        CanHackerArduinoLineReader(CanHackerArduino *vCanHacker) : CanHackerLineReader(vCanHacker) {};
        CANHACKER_ERROR process();
};

#endif /* CANHACKERARDUINO_H_ */
