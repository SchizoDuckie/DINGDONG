#include <M5StickC.h>
#include "secrets.h"    // Include the secrets module
#include "ServoModule.h"    // Include the servo module
#include "ServoModule.h"    // Include the servo module
#include "DoorbellModule.h" // Include the doorbell module
#include "ConfigModule.h"   // Include the config module
#include "TelegramModule.h" // Include the Telegram module

// Variables to hold settings
int servoMinAngle = 70;
int servoMaxAngle = 110;
int servoPin = 0;
int servoStep = 10;
long dingDongInterval = 50;
int dingDongDuration = 3000;

// Global variables for doorbell detection
int graphVal = 0;
bool dingDongActive = false;

void setup() {
    // Initialize M5Stick-C
    M5.begin();
    M5.Lcd.setRotation(1);  // Set the screen rotation
    M5.Lcd.fillScreen(BLACK);  // Clear the screen
    

    // Initialize serial communication
    Serial.begin(115200);

    // Initialize modules
    setupServo();
    setupDoorbell();
    setupTelegram();

    // Load settings
    loadSettings(servoMinAngle, servoMaxAngle, servoStep, dingDongInterval, dingDongDuration);

    // Start WiFi and Web Server asynchronously on Core 1
    xTaskCreatePinnedToCore(
        setupWiFiAndServer,  /* Function to implement the task */
        "WiFiTask",          /* Name of the task */
        8192,                /* Stack size in words */
        NULL,                /* Task input parameter */
        1,                   /* Priority of the task */
        NULL,                /* Task handle. */
        1);                  /* Core where the task should run */

    // Start Telegram task asynchronously on Core 1
    xTaskCreatePinnedToCore(
        telegramTask,        /* Function to implement the task */
        "TelegramTask",      /* Name of the task */
        4096,                /* Stack size in words */
        NULL,                /* Task input parameter */
        1,                   /* Priority of the task */
        NULL,                /* Task handle. */
        1);                  /* Core where the task should run */
}

void loop() {
    // Update doorbell detection and servo on Core 0
    updateDoorbell(servoMinAngle, servoMaxAngle, servoStep, dingDongInterval, dingDongDuration);

    // Update servo position if active
    if (servoActive) {
        updateServo();
    }
}
