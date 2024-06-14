#ifndef DOORBELL_MODULE_H
#define DOORBELL_MODULE_H

#include <M5StickC.h>

const int analogPin = 36;

const int graphBufferLength = 160;
const int maxGraphValue = 4095;
const int graphHeight = 160;
const int graphWidth = 160;    // Screen width
const long cooldownPeriod = 3000;  // Cooldown period in milliseconds

// External variable declarations
extern bool dingDongActive;
extern bool servoActive;

void setupDoorbell();
void updateDoorbell(int servoMinAngle, int servoMaxAngle, int servoStep, long dingDongInterval, int dingDongDuration);
void updateDingDong();
void plotGraph();

#endif
