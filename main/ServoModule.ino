#include "ServoModule.h"

int servoPos = servoMinAngle;
bool servoMovingUp = true;
bool servoActive = false;
unsigned long previousMillis = 0;

void setupServo() {
    pinMode(servoPin, OUTPUT);
    digitalWrite(servoPin, LOW); // Ensure servo is not powered initially
}

void updateServo() {
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= servoInterval) {
        previousMillis = currentMillis;

        if (servoMovingUp) {
            servoPos += servoStep;
            if (servoPos >= servoMaxAngle) {
                servoMovingUp = false;
            }
        } else {
            servoPos -= servoStep;
            if (servoPos <= servoMinAngle) {
                servoMovingUp = true;
                servoActive = false;  // Deactivate the servo after one complete cycle
                digitalWrite(servoPin, LOW); // Power off the servo
                return;
            }
        }

        int pulseWidth = servoPulse(servoPos);
        digitalWrite(servoPin, HIGH);
        delayMicroseconds(pulseWidth);
        digitalWrite(servoPin, LOW);
        delayMicroseconds(20000 - pulseWidth);
    }
}

int servoPulse(int angleDegrees) {
    return map(angleDegrees, 0, 180, 544, 2400);
}
