#include <SPI.h>
#include <mcp_can.h>

#include "lib.h"
#include "CanHackerArduino.h"

const int LED_PIN = 13;
const int SPI_CS_PIN = 10;

bool interrupt = false;

CanHackerArduinoLineReader *lineReader = NULL;
CanHackerArduino *canHacker = NULL;

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
        CanHacker::ERROR error = canHacker->processInterrupt();
        if (error != CanHacker::ERROR_OK) {
            stopAndBlink(error);
        }
    }
}

void serialEvent() {
    CanHacker::ERROR error = lineReader->process();
    if (error != CanHacker::ERROR_OK) {
        stopAndBlink(error);
    }
}
