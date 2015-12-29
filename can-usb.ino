#include <SPI.h>
#include <mcp_can.h>
#include <mcp_can_dfs.h>

#include "CanHackerArduino.h"

const int LED_PIN = 13;
const int SPI_CS_PIN = 10;

CanHackerArduinoLineReader *lineReader;
CanHackerArduino *canHacker;

void stopAndBlink(CANHACKER_ERROR error) {
    while (1) {
        digitalWrite(LED_PIN, HIGH);
        delay(500);
        digitalWrite(LED_PIN, LOW);
        delay(500);
    }
}

void setup() {
    Serial.end();
    Serial.begin(115200);
    
    //  attachInterrupt(0, MCP2515_ISR, FALLING); // start interrupt
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);

    canHacker = new CanHackerArduino(SPI_CS_PIN, MODE_LOOPBACK);
    lineReader = new CanHackerArduinoLineReader(canHacker);
}

void loop() {
    CANHACKER_ERROR error = canHacker->receiveCan();
    if (error != CANHACKER_ERROR_OK) {
        stopAndBlink(error);
    }
}

void serialEvent() {
    CANHACKER_ERROR error = lineReader->process();
    if (error != CANHACKER_ERROR_OK) {
        stopAndBlink(error);
    }
}
