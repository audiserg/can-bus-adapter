#include <SPI.h>
#include <mcp_can.h>
#include <mcp_can_dfs.h>
#include "lib.h"


#include "CanHackerArduino.h"

const int LED_PIN = 13;
const int SPI_CS_PIN = 10;

bool interrupt = false;

CanHackerArduinoLineReader *lineReader = NULL;
CanHackerArduino *canHacker = NULL;

void stopAndBlink(const int times) {
    while (1) {
        for (int i=0;i<times;i++) {
            digitalWrite(LED_PIN, HIGH);
            delay(300);
            digitalWrite(LED_PIN, LOW);
            delay(300);
        }
        
        delay(2000);
    }
}

void stopAndBlink(const CANHACKER_ERROR error) {
    int c = error;
    for (int i=0;i<c;i++) {
        digitalWrite(LED_PIN, HIGH);
        delay(500);
        digitalWrite(LED_PIN, LOW);
        delay(500);
    }
    
    delay(1000);
    
    while (1) {
        digitalWrite(LED_PIN, HIGH);
        delay(300);
        digitalWrite(LED_PIN, LOW);
        delay(300);
    } ;
}

void setup() {
    Serial.begin(115200);
    
    //  attachInterrupt(0, MCP2515_ISR, FALLING); // start interrupt
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);

    canHacker = new CanHackerArduino(SPI_CS_PIN, MODE_LOOPBACK);
    lineReader = new CanHackerArduinoLineReader(canHacker);
    
    attachInterrupt(0, irqHandler, FALLING);
}

void irqHandler() {
    interrupt = true;
}

void loop() {
    if (interrupt) {
        interrupt = false;
        
        MCP_CAN *mcp2551 = canHacker->getMcp2515();
        if (mcp2551 != NULL) {
            INT8U irq = mcp2551->getInterrupts();
            //Serial.print(irq, HEX);
            /*Serial.print("t0011");
            Serial.write(nibble2ascii(irq >> 4));
            Serial.write(nibble2ascii(irq));
            Serial.write('\r');*/
            
            if (irq & (MCP_RX0IF | MCP_RX1IF)) {
                CANHACKER_ERROR error = canHacker->receiveCan();
                if (error != CANHACKER_ERROR_OK) {
                    stopAndBlink(error);
                }
            }
            
            if (irq & (MCP_TX0IF | MCP_TX1IF | MCP_TX2IF)) {
                mcp2551->clearTXInterrupts();
                //Serial.print("MCP_TXxIF\r\n");
                //stopAndBlink(1);
            }
            
            if (irq & MCP_ERRIF) {
                Serial.print("MCP_ERRIF\r\n");
                stopAndBlink(2);
            }
            
            if (irq & MCP_WAKIF) {
                Serial.print("MCP_WAKIF\r\n");
                stopAndBlink(3);
            }
            
            if (irq & MCP_MERRF) {
                Serial.print("MCP_MERRF\r\n");
                stopAndBlink(4);
            }
        }
    }
}

void serialEvent() {
    CANHACKER_ERROR error = lineReader->process();
    if (error != CANHACKER_ERROR_OK) {
        stopAndBlink(error);
    }
}
