#ifndef SERVO_MODULE_H
#define SERVO_MODULE_H

#include <Arduino.h>

// Servo control constants
extern int servoPin;
extern int servoMinAngle;
extern int servoMaxAngle;
extern int servoStep;
const long servoInterval = 2;  // Interval for servo update in milliseconds
extern bool servoActive;
extern bool notificationSent = false;

// Function declarations
void setupServo();
void updateServo();
int servoPulse(int angleDegrees);

#endif // SERVO_MODULE_H
