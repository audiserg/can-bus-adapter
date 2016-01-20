#include <SPI.h>
#include <mcp_can.h>

#include "lib.h"
#include "CanHackerArduino.h"

const int SPI_CS_PIN = 10;

bool interrupt = false;

CanHackerArduinoLineReader *lineReader = NULL;
CanHackerArduino *canHacker = NULL;

void stopAndBlink(const CanHacker::ERROR error) {
  
    Serial.print("Failure (code ");
    Serial.print((int)error);
    Serial.println(")");

    digitalWrite(SPI_CS_PIN, HIGH);    
    pinMode(LED_BUILTIN, OUTPUT);
  
    while (1) {
        int c = (int)error;
        for (int i=0;i<c;i++) {
            digitalWrite(LED_BUILTIN, HIGH);
            delay(500);
            digitalWrite(LED_BUILTIN, LOW);
            delay(500);
        }
        
        delay(2000);
    } ;
}

void setup() {
    Serial.begin(115200);
    SPI.begin();
    
    canHacker = new CanHackerArduino(SPI_CS_PIN);
    canHacker->setLoopbackEnabled(true);
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
