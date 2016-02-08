#include <SPI.h>
#include <mcp_can.h>
#include <SoftwareSerial.h>

#include "lib.h"
#include "CanHacker.h"
#include "CanHackerLineReader.h"

const int SPI_CS_PIN = 10;

const int SS_RX_PIN = 5;
const int SS_TX_PIN = 4;

bool interrupt = false;

CanHackerLineReader *lineReader = NULL;
CanHacker *canHacker = NULL;

SoftwareSerial softwareSerial(SS_RX_PIN, SS_TX_PIN);

void setup() {
    Serial.begin(115200);
    SPI.begin();
    softwareSerial.begin(38400);

    Stream *interfaceStream = &Serial;
    Stream *debugStream = &softwareSerial;
    
    
    canHacker = new CanHacker(interfaceStream, debugStream, SPI_CS_PIN);
    canHacker->setLoopbackEnabled(true);
    lineReader = new CanHackerLineReader(interfaceStream, canHacker);
    
    attachInterrupt(0, irqHandler, FALLING);
}

void loop() {
    if (interrupt) {
        interrupt = false;
        CanHacker::ERROR error = canHacker->processInterrupt();
        handleError(error);
    }
}

void serialEvent() {
    CanHacker::ERROR error = lineReader->process();
    handleError(error);
}

void irqHandler() {
    interrupt = true;
}

void handleError(const CanHacker::ERROR error) {

    switch (error) {
        case CanHacker::ERROR_OK:
        case CanHacker::ERROR_UNKNOWN_COMMAND:
        case CanHacker::ERROR_NOT_CONNECTED:
        case CanHacker::ERROR_MCP2515_ERRIF:
        case CanHacker::ERROR_INVALID_COMMAND:
            return;

        default:
            break;
    }
  
    Serial.print("Failure (code ");
    Serial.print((int)error);
    Serial.println(")");

    digitalWrite(SPI_CS_PIN, HIGH);
    pinMode(LED_BUILTIN, OUTPUT);
  
    while (1) {
        int c = (int)error;
        for (int i=0; i<c; i++) {
            digitalWrite(LED_BUILTIN, HIGH);
            delay(500);
            digitalWrite(LED_BUILTIN, LOW);
            delay(500);
        }
        
        delay(2000);
    } ;
}
