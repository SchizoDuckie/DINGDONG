#ifndef CONFIG_MODULE_H
#define CONFIG_MODULE_H

#include <WiFi.h>
#include "secrets.h"
#include <ESPAsyncWebServer.h>
#include <Preferences.h>
#include <pgmspace.h>
#include <vector>

// WiFi credentials are loaded from secrets.h:
// const char* WIFI_SSID = "XXXXXXXX";
// const char* WIFI_PASSWORD = "XXXXXXXX";

extern AsyncWebServer server;
extern Preferences preferences;
extern std::vector<String> subscribers;

void setupWiFiAndServer(void* parameter);
String generateHTML(int servoMinAngle, int servoMaxAngle, int servoStep, long dingDongInterval, int dingDongDuration, const String& subscribers);
void loadSettings(int& servoMinAngle, int& servoMaxAngle, int& servoStep, long& dingDongInterval, int& dingDongDuration);
void saveSettings(int servoMinAngle, int servoMaxAngle, int servoStep, long dingDongInterval, int dingDongDuration);
void loadSubscribers(std::vector<String>& subscribers);
void saveSubscribers(const std::vector<String>& subscribers);
String getSubscribersString();

#endif // CONFIG_MODULE_H
