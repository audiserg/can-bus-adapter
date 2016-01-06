#include <SPI.h>
#include <mcp_can.h>

#include "lib.h"
#include "CanHackerArduino.h"

const int LED_PIN = 13;
const int SPI_CS_PIN = 10;

bool interrupt = false;

CanHackerArduinoLineReader *lineReader = NULL;
CanHackerArduino *canHacker = NULL;
unsigned long lastTime = 0;
unsigned long uartRxCount = 0;

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

void stopAndBlink(const CanHacker::ERROR error) {
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

    canHacker = new CanHackerArduino(SPI_CS_PIN, MCP_CAN::MODE_LOOPBACK);
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
            uint8_t irq = mcp2551->getInterrupts();
            //INT8U canStat = mcp2551->getCanStat();
            //Serial.print(irq, HEX);
            /*Serial.print("t0012");
            Serial.write(nibble2ascii(irq >> 4));
            Serial.write(nibble2ascii(irq));
            Serial.write(nibble2ascii(canStat >> 4));
            Serial.write(nibble2ascii(canStat));
            Serial.write('\r');*/
            
            if (irq & MCP_CAN::CANINTF_RX0IF) {
                CanHacker::ERROR error = canHacker->receiveCan(MCP_CAN::RXB0);
                if (error != CanHacker::ERROR_OK) {
                    stopAndBlink(error);
                }
            }
            
            if (irq & MCP_CAN::CANINTF_RX1IF) {
                CanHacker::ERROR error = canHacker->receiveCan(MCP_CAN::RXB1);
                if (error != CanHacker::ERROR_OK) {
                    stopAndBlink(error);
                }
            }
            
            if (irq & (MCP_CAN::CANINTF_TX0IF | MCP_CAN::CANINTF_TX1IF | MCP_CAN::CANINTF_TX2IF)) {
                //mcp2551->clearTXInterrupts();
                //Serial.print("MCP_TXxIF\r\n");
                //stopAndBlink(1);
            }
            
            if (irq & MCP_CAN::CANINTF_ERRIF) {
                Serial.print("MCP_ERRIF\r\n");
                stopAndBlink(2);
            }
            
            if (irq & MCP_CAN::CANINTF_WAKIF) {
                Serial.print("MCP_WAKIF\r\n");
                stopAndBlink(3);
            }
            
            if (irq & MCP_CAN::CANINTF_MERRF) {
                Serial.print("MCP_MERRF\r\n");
                stopAndBlink(4);
            }
        }
    }
    
    const unsigned int interval = 500;
    
    unsigned long time = millis();
    if (time - lastTime > interval) {
        lastTime = time;
        
        unsigned int uartRxSpeed = uartRxCount * 8 / interval;
        
        
        MCP_CAN *mcp2551 = canHacker->getMcp2515();
        if (mcp2551 != NULL) {
            //INT8U irq = mcp2551->getInterrupts();
            //INT8U canStat = mcp2551->getCanStat();
            
            struct can_frame frame;
            
            frame.can_id = 1;
            frame.can_dlc = 6;
            frame.data[0] = 0;//irq;
            frame.data[1] = 0; //canStat;
            frame.data[2] = 0;
            frame.data[3] = uartRxCount >> 8;
            frame.data[4] = uartRxCount;
            frame.data[5] = uartRxSpeed;
            canHacker->receiveCanFrame(&frame);
        }
        
        uartRxCount = 0;
    }
}

void serialEvent() {
    CanHacker::ERROR error = lineReader->process();
    if (error != CanHacker::ERROR_OK) {
        stopAndBlink(error);
    }
}
