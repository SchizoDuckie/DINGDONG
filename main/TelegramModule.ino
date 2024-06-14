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
void sendTelegramMessageTask(void* parameter) {
    MessageParams* params = static_cast<MessageParams*>(parameter);
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("WiFi not connected");
        delete params; // Clean up allocated memory
        vTaskDelete(NULL);
        return;
    }

    WiFiClientSecure client;
    client.setInsecure();

    const char* server = "api.telegram.org";

    if (!client.connect(server, 443)) {
        Serial.println("Connection to Telegram failed");
        delete params; // Clean up allocated memory
        vTaskDelete(NULL);
        return;
    }

    String url = "/bot";
    url += TELEGRAM_BOT_TOKEN;
    url += "/sendMessage?chat_id=";
    url += params->chatID;
    url += "&text=";
    url += params->message;

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

    delete params; // Clean up allocated memory
    vTaskDelete(NULL);
}

void triggerNotification() {
    for (auto it = subscribers.begin(); it != subscribers.end(); ) {
        const auto& chatID = *it;
        if (chatID.isEmpty()) {
            it = subscribers.erase(it); // Remove empty chatID and update iterator
            continue;
        }
        Serial.print("Notifying via Telegram, chatID: ");
        Serial.println(chatID);

        MessageParams* params = new MessageParams{"DING DONG! Er staat iemand voor de deur!.", chatID};
        xTaskCreate(sendTelegramMessageTask, "SendTelegramMessage", 4096, params, 1, NULL);
        ++it; // Move to the next subscriber
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

        if (text.equalsIgnoreCase("geef deurbel") || text.equalsIgnoreCase("/start")) {
            addSubscriber(chat_id);
            saveSubscribers(subscribers);  // Save after adding
        } else if (text.equalsIgnoreCase("rot op") || text.equalsIgnoreCase("hou je bek") || text.equalsIgnoreCase("bakkes") || text.equalsIgnoreCase("/stop")) {
            removeSubscriber(chat_id);
            saveSubscribers(subscribers);  // Save after removing
        }
    }
}

void addSubscriber(const String& chatID) {
    if (!chatID.isEmpty() && std::find(subscribers.begin(), subscribers.end(), chatID) == subscribers.end()) {
        subscribers.push_back(chatID);
        MessageParams* params = new MessageParams{"Okay, je krijgt nu deurbel notificaties", chatID};
        xTaskCreate(sendTelegramMessageTask, "SendTelegramMessage", 4096, params, 1, NULL);
    }
}

void removeSubscriber(const String& chatID) {
    auto it = std::remove(subscribers.begin(), subscribers.end(), chatID);
    MessageParams* params = new MessageParams{"Okay doei", chatID};
    xTaskCreate(sendTelegramMessageTask, "SendTelegramMessage", 4096, params, 1, NULL);
    if (it != subscribers.end()) {
        subscribers.erase(it, subscribers.end());
    }
}
