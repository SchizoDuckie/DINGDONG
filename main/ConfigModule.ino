#include "ConfigModule.h"
#include "time.h"
#include "secrets.h"
#include <ESPmDNS.h>

// Web server
AsyncWebServer server(80);
Preferences preferences;

bool wifiConnected = false;

void setupWiFiAndServer(void* parameter) {
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.println("Connecting to WiFi...");

    while (WiFi.status() != WL_CONNECTED) {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        Serial.print(".");
    }
    Serial.println("\nConnected to WiFi");
    // Set DNS servers
    Serial.println("WiFi connected.");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    Serial.print("Gateway: ");
    Serial.println(WiFi.gatewayIP());
    Serial.print("Subnet: ");
    Serial.println(WiFi.subnetMask());
    IPAddress dns1(8, 8, 8, 8);  // Google DNS
    IPAddress dns2(8, 8, 4, 4);  // Google DNS
    WiFi.config(WiFi.localIP(), WiFi.gatewayIP(), WiFi.subnetMask(), dns1, dns2);

    configTime(0, 0, "pool.ntp.org"); // get UTC time via NTP
    time_t now = time(nullptr);
    while (now < 24 * 3600)
    {
      vTaskDelay(1000 / portTICK_PERIOD_MS);
      Serial.print(".");
      now = time(nullptr);
    }
    Serial.println(now);
    
    if (!MDNS.begin("doorbell")) {
      Serial.println("Error starting mDNS");
    }
    preferences.begin("doorbell", false);
    loadSettings(servoMinAngle, servoMaxAngle, servoStep, dingDongInterval, dingDongDuration);
    loadSubscribers(subscribers);
    setupServer();
    wifiConnected = true;
    
    setupTelegram();
    // Start Telegram task
    xTaskCreate(
        telegramTask,        // Task function
        "TelegramTask",      // Task name
        10000,               // Stack size
        NULL,                // Task input parameter
        1,                   // Task priority
        NULL                 // Task handle
    );

    vTaskDelete(NULL); // Delete the task once setup is done
}

void setupWiFi() {
    xTaskCreate(
        setupWiFiAndServer,   // Task function
        "WiFiSetup",          // Task name
        10000,                // Stack size
        NULL,                 // Task input parameter
        1,                    // Task priority
        NULL                  // Task handle
    );
}

void checkWiFiConnection() {
    if (WiFi.status() == WL_CONNECTED && !wifiConnected) {
        // WiFi is connected, proceed with server setup
        preferences.begin("doorbell", false);
        loadSettings(servoMinAngle, servoMaxAngle, servoStep, dingDongInterval, dingDongDuration);
        loadSubscribers(subscribers);
        setupServer();
        wifiConnected = true;
    } else if (WiFi.status() != WL_CONNECTED && wifiConnected) {
        // WiFi is disconnected, attempt to reconnect
        WiFi.reconnect();
        wifiConnected = false;
    }
}

void setupServer() {
    server.on("/", HTTP_GET, [=](AsyncWebServerRequest *request){
        request->send(200, "text/html", generateHTML(servoMinAngle, servoMaxAngle, servoStep, dingDongInterval, dingDongDuration, getSubscribersString()));
    });

    server.on("/save", HTTP_POST, [=](AsyncWebServerRequest *request){
        if (request->hasParam("servoMinAngle", true)) {
            servoMinAngle = request->getParam("servoMinAngle", true)->value().toInt();
        }
        if (request->hasParam("servoMaxAngle", true)) {
            servoMaxAngle = request->getParam("servoMaxAngle", true)->value().toInt();
        }
        if (request->hasParam("servoStep", true)) {
            servoStep = request->getParam("servoStep", true)->value().toInt();
        }
        if (request->hasParam("dingDongInterval", true)) {
            dingDongInterval = request->getParam("dingDongInterval", true)->value().toInt();
        }
        if (request->hasParam("dingDongDuration", true)) {
            dingDongDuration = request->getParam("dingDongDuration", true)->value().toInt();
        }
        if (request->hasParam("subscribers", true)) {
            String subs = request->getParam("subscribers", true)->value();
            subscribers.clear();
            int start = 0;
            int end = subs.indexOf(',');
            while (end != -1) {
                subscribers.push_back(subs.substring(start, end));
                start = end + 1;
                end = subs.indexOf(',', start);
            }
            subscribers.push_back(subs.substring(start));
            saveSubscribers(subscribers);
        }
        saveSettings(servoMinAngle, servoMaxAngle, servoStep, dingDongInterval, dingDongDuration);
        request->send(200, "text/plain", "Settings saved. Rebooting...");
        vTaskDelay(2000 / portTICK_PERIOD_MS);
        ESP.restart();
    });

    server.begin();
}

const char htmlTemplate[] PROGMEM = R"rawliteral(
<!DOCTYPE html><html lang='en'><head><meta charset='UTF-8'><meta name='viewport' content='width=device-width, initial-scale=1.0'>
<link href='https://stackpath.bootstrapcdn.com/bootstrap/4.3.1/css/bootstrap.min.css' rel='stylesheet'>
<title>Doorbell Config</title></head><body><div class='container'>
<h1>Doorbell Configuration</h1><form action='/save' method='post'>
<div class='form-group'><label for='servoMinAngle'>Servo Min Angle:</label>
<input type='number' class='form-control' name='servoMinAngle' value='%d'></div>
<div class='form-group'><label for='servoMaxAngle'>Servo Max Angle:</label>
<input type='number' class='form-control' name='servoMaxAngle' value='%d'></div>
<div class='form-group'><label for='servoStep'>Servo Step:</label>
<input type='number' class='form-control' name='servoStep' value='%d'></div>
<div class='form-group'><label for='dingDongInterval'>Ding Dong Interval:</label>
<input type='number' class='form-control' name='dingDongInterval' value='%ld'></div>
<div class='form-group'><label for='dingDongDuration'>Ding Dong Duration:</label>
<input type='number' class='form-control' name='dingDongDuration' value='%d'></div>
<div class='form-group'><label for='subscribers'>Subscribers (comma-separated chat IDs):</label>
<input type='text' class='form-control' name='subscribers' value='%s'></div>
<button type='submit' class='btn btn-primary'>Save</button></form></div>
<script src='https://code.jquery.com/jquery-3.3.1.slim.min.js'></script>
<script src='https://cdnjs.cloudflare.com/ajax/libs/popper.js/1.14.7/umd/popper.min.js'></script>
<script src='https://stackpath.bootstrapcdn.com/bootstrap/4.3.1/js/bootstrap.min.js'></script>
</body></html>
)rawliteral";

String generateHTML(int servoMinAngle, int servoMaxAngle, int servoStep, long dingDongInterval, int dingDongDuration, const String& subscribers) {
    char buffer[2048];
    snprintf_P(buffer, sizeof(buffer), htmlTemplate, servoMinAngle, servoMaxAngle, servoStep, dingDongInterval, dingDongDuration, subscribers.c_str());
    return String(buffer);
}

String getSubscribersString() {
    String subscriberList = "";
    for (size_t i = 0; i < subscribers.size(); i++) {
        if (i > 0) {
            subscriberList += ",";
        }
        subscriberList += subscribers[i];
    }
    return subscriberList;
}

void loadSettings(int& servoMinAngle, int& servoMaxAngle, int& servoStep, long& dingDongInterval, int& dingDongDuration) {
    servoMinAngle = preferences.getInt("servoMinAngle", 70);
    servoMaxAngle = preferences.getInt("servoMaxAngle", 110);
    servoStep = preferences.getInt("servoStep", 10);
    dingDongInterval = preferences.getLong("dingDongInterval", 50);
    dingDongDuration = preferences.getInt("dingDongDuration", 3000);
}

void saveSettings(int servoMinAngle, int servoMaxAngle, int servoStep, long dingDongInterval, int dingDongDuration) {
    preferences.putInt("servoMinAngle", servoMinAngle);
    preferences.putInt("servoMaxAngle", servoMaxAngle);
    preferences.putInt("servoStep", servoStep);
    preferences.putLong("dingDongInterval", dingDongInterval);
    preferences.putInt("dingDongDuration", dingDongDuration);
}

void loadSubscribers(std::vector<String>& subscribers) {
    preferences.begin("doorbell", true); // read-only
    size_t size = preferences.getBytesLength("subscribers");
    if (size > 0) {
        char* buffer = new char[size];
        preferences.getBytes("subscribers", buffer, size);
        String subscriberList(buffer);
        delete[] buffer;

        int start = 0;
        int end = subscriberList.indexOf(',');
        while (end != -1) {
            subscribers.push_back(subscriberList.substring(start, end));
            start = end + 1;
            end = subscriberList.indexOf(',', start);
        }
        subscribers.push_back(subscriberList.substring(start));
    }
    preferences.end();
}

void saveSubscribers(const std::vector<String>& subscribers) {
    preferences.begin("doorbell", false); // read-write
    String subscriberList = "";
    for (const auto& subscriber : subscribers) {
        if (!subscriberList.isEmpty()) {
            subscriberList += ",";
        }
        subscriberList += subscriber;
    }
    preferences.putBytes("subscribers", subscriberList.c_str(), subscriberList.length() + 1);
    preferences.end();
}
