#include "DoorbellModule.h"

// Variable declarations
int val = 0;
int nullCounter = 0;
int graphBuffer[graphBufferLength] = {0};
int graphIndex = 0;
unsigned long dingDongStartMillis = 0;
unsigned long dingDongPreviousMillis = 0;
unsigned long cooldownStartMillis = 0;  // For cooldown
int dingDongX = 160;  // Initial position for "DING DONG"
int previousDingDongX = 160;

// Declare the external variables from the header file
extern bool dingDongActive;
extern bool servoActive;

void setupDoorbell() {
    pinMode(analogPin, INPUT);
    nullCounter = 0; // Reset nullCounter to avoid false positives on startup
    dingDongActive = false;
    servoActive = false;
    M5.Lcd.fillScreen(BLACK);  // Clear the screen initially
}

void updateDoorbell(int servoMinAngle, int servoMaxAngle, int servoStep, long dingDongInterval, int dingDongDuration) {
    // Read the analog value
    val = analogRead(analogPin);
    int graphVal = map(val, 0, 4095, 0, maxGraphValue);

    // Update graph buffer
    graphBuffer[graphIndex] = graphVal;
    graphIndex = (graphIndex + 1) % graphBufferLength;

    // Plot the running graph
    plotGraph();

    // Check for cooldown period
    unsigned long currentMillis = millis();
    if (currentMillis - cooldownStartMillis < cooldownPeriod) {
        return; // Skip detection during cooldown
    }

    // Detection logic
    if (graphVal < 10) {  // Adjust threshold for your specific use case
        nullCounter++;
        if (nullCounter > 5 && !servoActive && !dingDongActive) {
            Serial.println("DING DONG!");
            dingDongActive = true;
            dingDongStartMillis = millis();
            nullCounter = 0;
            servoActive = true;  // Activate the servo
            digitalWrite(servoPin, HIGH); // Power on the servo
            cooldownStartMillis = currentMillis; // Start cooldown
            triggerNotification();
        }
    } else {
        nullCounter = 0;
    }

    // Flash "DING DONG" if active
    if (dingDongActive) {
        if (currentMillis - dingDongStartMillis < dingDongDuration) {
            if (currentMillis - dingDongPreviousMillis >= dingDongInterval) {
                dingDongPreviousMillis = currentMillis;
                updateDingDong();
            }
        } else {
            dingDongActive = false;
        }
    }
}

void updateDingDong() {
    M5.Lcd.setTextColor(YELLOW);  // Yellow text
    M5.Lcd.setTextSize(4);  // Use the largest possible text size
    M5.Lcd.setCursor(1, 30);  // Adjust position to keep text fully visible
    M5.Lcd.print("DING DONG!");

}

void plotGraph() {
    int screenWidth = 160;
    int screenHeight = 78;

    // Clear the graph area before plotting
    M5.Lcd.fillRect(0, 0, screenWidth, screenHeight, BLACK);  // Clear the entire screen area

    int graphWidth = screenWidth;
    int graphHeight = screenHeight;
String pixelCoordinates = "";  // String to hold the coordinates

    for (int i = 0; i < graphBufferLength; i++) {
        int x = i;
        int y = screenHeight - map(graphBuffer[(graphIndex + i) % graphBufferLength], 0, maxGraphValue, 0, graphHeight);
        M5.Lcd.drawPixel(x, y, ORANGE);

        // Append the coordinates to the string
        if(graphBuffer[(graphIndex + i)] > 0) {
          pixelCoordinates += "(" + String(x) + "," + String(graphBuffer[(graphIndex + i)]) +  ">" + String(y) + ")";  
        }
        
        if (i < graphBufferLength - 1) {
            pixelCoordinates += ", ";
        }
    }

    // Print the coordinates to the Serial console
    //Serial.println(pixelCoordinates);
}
