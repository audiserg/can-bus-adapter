#include <SPI.h>

#include <mcp_can.h>
#include <mcp_can_dfs.h>

#include "CanHacker.h"
#include "can.h"
#include "lib.h"

const int COMMAND_MAX_LENGTH = 30; // not including \r\0

const int LED_PIN = 13;
const int SPI_CS_PIN = 10;

void serialOutputCallback(const char *str);
void canOutputCallback(const struct can_frame *frame);

CanHacker canHacker(serialOutputCallback, canOutputCallback);
MCP_CAN CAN(SPI_CS_PIN);

char commandBuffer[COMMAND_MAX_LENGTH + 2];  // command buffer
int commandBufferIndex = 0;

void stopAndBlink(void) {
    digitalWrite(LED_PIN, HIGH);
    /*while (1) {
        digitalWrite(LED_PIN, HIGH);
        delay(500);
        digitalWrite(LED_PIN, LOW);
        delay(500);
    }*/
}

void setup() {
    Serial.begin(115200);
    CAN.begin(CAN_125KBPS);
    //  attachInterrupt(0, MCP2515_ISR, FALLING); // start interrupt
    pinMode(LED_PIN, OUTPUT); 
    digitalWrite(LED_PIN, LOW);
}

void loop() {
    /*if (CAN.checkError()) {
        stopAndBlink();
    }*/
    
    /*while (Serial.available()) {
        processChar(Serial.read());
    }*/
  
    if (CAN_MSGAVAIL == CAN.checkReceive()) {
        struct can_frame frame;
        if (CAN.readMsgBuf(&frame.can_dlc, frame.data) != CAN_OK) {
            stopAndBlink();
        }
        frame.can_id = CAN.getCanId();
        
        if (canHacker.receiveCanFrame(&frame) != CANHACKER_OK) {
            stopAndBlink();
        }
    }
}

void processChar(char rxChar) {
    switch (rxChar) {
        case '\r':
        case '\n':
            if (commandBufferIndex > 0) {
                commandBuffer[commandBufferIndex] = '\0';

                if (canHacker.receiveCommand(commandBuffer, commandBufferIndex) != CANHACKER_OK) {
                    stopAndBlink();
                }
                commandBufferIndex = 0; 
            }
            break;
            
        case '\0':
            break;
            
        default:
            if (commandBufferIndex < COMMAND_MAX_LENGTH) {
                commandBuffer[commandBufferIndex++] = rxChar;
            }
            break;
    }
}

void serialEvent() {
    while (Serial.available()) {
        processChar(Serial.read());
    }
}

void serialOutputCallback(const char *str) {
    Serial.print(str);
}

void canOutputCallback(const struct can_frame *frame) {
    INT8U isExtended = frame->can_id & CAN_EFF_FLAG ? 1 : 0;
    INT8U isRTR = frame->can_id & CAN_RTR_FLAG ? 1 : 0;
    canid_t id = frame->can_id & (isExtended ? CAN_EFF_MASK : CAN_SFF_MASK);

    if (CAN.sendMsgBuf(id, isExtended, isRTR, frame->can_dlc, frame->data) != CAN_OK) {
        stopAndBlink();
    }
}
