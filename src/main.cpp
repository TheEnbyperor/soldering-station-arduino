#include "Arduino.h"
#include <Thermocouple.h>

#define thermoCS 10
#define zcPin 2
#define triacPIN 7

Thermocouple tc1 = Thermocouple(thermoCS);

void zcISR();

void setup() {
    Serial.begin(9600);
    pinMode(zcPin, INPUT_PULLUP);
    pinMode(triacPIN, OUTPUT);
    attachInterrupt(digitalPinToInterrupt(zcPin), zcISR, FALLING);

    delay(1000);
}

volatile unsigned int numCrossings = 0;
volatile unsigned int crossingThreshold = 0;
volatile double wantTemp = 0;
volatile double curTemp = 0;
volatile bool shouldRead = false;
volatile bool hasRead = false;

void zcISR() {
    if (shouldRead) {
        if (!hasRead) {
            return;
        }
    }
    if (numCrossings < 20) {
        numCrossings++;

        if (curTemp == -1.0) {
            digitalWrite(triacPIN, LOW);
        } else {
            if (curTemp < wantTemp) {
                digitalWrite(triacPIN, HIGH);
            } else {
                digitalWrite(triacPIN, LOW);
            }
        }
    } else {
        digitalWrite(triacPIN, LOW);
        detachInterrupt(digitalPinToInterrupt(zcPin));
        shouldRead = true;
        hasRead = false;
        numCrossings = 0;
    }
}

unsigned long lastPrint = 0;

void loop() {
    if (Serial.available() > 0) {
        wantTemp = Serial.parseFloat();
    }
    if ((millis() - lastPrint) > 500) {
        Serial.print(crossingThreshold);
        Serial.print(",");
        Serial.print(wantTemp);
        Serial.print(",");
        Serial.println(curTemp);
        lastPrint = millis();
    }
    if (shouldRead) {
        // Two mains cycles
        delay(40);
        curTemp = tc1.readC();
        hasRead = true;
        shouldRead = false;

        crossingThreshold = sqrt(wantTemp - curTemp);
        if (crossingThreshold < 1) {
            crossingThreshold = 1;
        }
        attachInterrupt(digitalPinToInterrupt(zcPin), zcISR, FALLING);
    }
}
