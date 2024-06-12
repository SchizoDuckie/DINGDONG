#include <WiFiClientSecure.h>
#include "TelegramModule.h"
#include "secrets.h"
#include "ConfigModule.h"

std::vector<String> subscribers;
long lastUpdateId = 0;  // Initialize the last update ID to 0

void setupTelegram() {
    // Load subscribers from config
    loadSubscribers(subscribers);
}

void sendTelegramMessage(const char* message, const String& chatID) {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("WiFi not connected");
        return;
    }

    WiFiClientSecure client;
    client.setInsecure();
    
    const char* server = "api.telegram.org";

    if (!client.connect(server, 443)) {
        Serial.println("Connection to Telegram failed");
        return;
    }

    String url = "/bot";
    url += TELEGRAM_BOT_TOKEN;
    url += "/sendMessage?chat_id=";
    url += chatID;
    url += "&text=";
    url += message;

    client.print(String("GET ") + url + " HTTP/1.1\r\n" +
                 "Host: " + server + "\r\n" +
                 "User-Agent: ESP32\r\n" +
                 "Connection: close\r\n\r\n");

    while (client.connected()) {
        String line = client.readStringUntil('\n');
        if (line == "\r") {
            break;
        }
    }
    String line = client.readStringUntil('\n');
    Serial.println(line);
}

void triggerNotification() {
    for (const auto& chatID : subscribers) {
        Serial.print("Notifying via Telegram, chatID: ");
        Serial.println(chatID);
        sendTelegramMessage("DING DONG! Er staat iemand voor de deur!.", chatID);
    }
}

void telegramTask(void* parameter) {
    while (true) {
        // Fetch updates from Telegram periodically
        getUpdates();
        vTaskDelay(15000 / portTICK_PERIOD_MS); 
    }
}

void getUpdates() {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("WiFi not connected");
        return;
    }

    WiFiClientSecure client;
    client.setInsecure();
    const char* server = "api.telegram.org";

    if (!client.connect(server, 443)) {
        Serial.println("Connection to api.telegram.org on port 443 failed");
        return;
    }

    String url = "/bot";
    url += TELEGRAM_BOT_TOKEN;
    url += "/getUpdates?offset=";
    url += String(lastUpdateId + 1);  // Fetch updates starting from the last update ID + 1

    client.print(String("GET ") + url + " HTTP/1.1\r\n" +
                 "Host: " + server + "\r\n" +
                 "User-Agent: ESP32\r\n" +
                 "Connection: close\r\n\r\n");

    String response = "";
    while (client.connected() || client.available()) {
        if (client.available()) {
            response += client.readStringUntil('\n');
        }
    }

    // Extract JSON part from the response
    int jsonStart = response.indexOf('{');
    if (jsonStart == -1) {
        Serial.println("Invalid response");
        return;
    }
    String jsonResponse = response.substring(jsonStart);

    StaticJsonDocument<2000> doc;
    DeserializationError error = deserializeJson(doc, jsonResponse);

    if (error) {
        Serial.print("deserializeJson() failed: ");
        Serial.println(error.c_str());
        Serial.println(jsonResponse);
        return;
    }

    JsonArray results = doc["result"].as<JsonArray>();
    for (JsonVariant result : results) {
        long update_id = result["update_id"];
        if (update_id > lastUpdateId) {
            lastUpdateId = update_id;  // Update the last update ID
        }

        String chat_id = result["message"]["chat"]["id"].as<String>();
        String text = result["message"]["text"].as<String>();

        if (text.equalsIgnoreCase("geef deurbel")) {
            addSubscriber(chat_id);
            saveSubscribers(subscribers);  // Save after adding
        } else if (text.equalsIgnoreCase("rot op") || text.equalsIgnoreCase("hou je bek") || text.equalsIgnoreCase("bakkes")) {
            removeSubscriber(chat_id);
            sendTelegramMessage("Okee doei.", chat_id);
            saveSubscribers(subscribers);  // Save after removing
        }
    }
}

void addSubscriber(const String& chatID) {
    if (std::find(subscribers.begin(), subscribers.end(), chatID) == subscribers.end()) {
        subscribers.push_back(chatID);
    }
}

void removeSubscriber(const String& chatID) {
    auto it = std::remove(subscribers.begin(), subscribers.end(), chatID);
    if (it != subscribers.end()) {
        subscribers.erase(it, subscribers.end());
    }
}
