# Doorbell Notification System with M5Stick-C

This project uses an M5Stick-C to create a doorbell notification system. It includes an analog sensor to detect doorbell presses, a servo to perform an action, and a WiFi module to send notifications via Telegram. The system also displays sensor values on the M5Stick-C screen.

## Requirements

- **M5Stick-C**: ESP32-based development board with a built-in display.
- **Analog Sensor**: Connected to GPIO 36.
- **Servo Motor**: Connected to GPIO 0.
- **WiFi Connection**: For sending notifications via Telegram.
- **Telegram Bot**: To send notifications.

## Setup and Installation
- Wire your analog input to the GPIO36 port
- Wire your servo input to the GPIO0 port

### Hardware Connections

1. **Analog Sensor**: Connect the signal wire to GPIO 36, the power wire to 3.3V, and the ground wire to GND on the M5Stick-C.
2. **Servo Motor**: Connect the signal wire to GPIO 0, the power wire to an external 5V source (do not use M5Stick-C's 5V pin directly), and the ground wire to GND.

### Software Installation

1. **Arduino IDE**: Ensure you have the latest version of the Arduino IDE installed.
2. **ESP32 Board Support**: Install the ESP32 board package via the Arduino IDE Board Manager.
3. **M5Stick-C Library**: Install the M5StickC library from the Library Manager in Arduino IDE.
4. **Additional Libraries**:
    - `ESPAsyncWebServer`
    - `AsyncTCP`
    - `WiFi`
    - `ArduinoJson`
    - `ESP32Servo`
    - `UniversalTelegramBot`

### Setting Up the Telegram Bot

1. **Create a Telegram Bot**: Use the BotFather on Telegram to create a new bot and get the bot token.
2. **Subscribe to the bot**: Connect to the bot using Telegram and say 'geef deurbel'
3. **Unsubscribing from the bot**: Connect to the bot using Telegram and say 'rot op'

### Configure the Project

1. Copy `secrets.sh.example` to `secrets.sh`
2. **WiFi Credentials**: Update your WiFi credentials in the `secrets.h` file.
2. **Telegram Bot Token**: Add your bot token in the `secrets.h` file.

### Directory Structure

Ensure your project directory has the following structure:

```
/main
   main.ino
   secrets.h
   secrets.h.example
   DoorbellModule.h
   DoorbellModule.ino
   ServoModule.h
   ServoModule.ino
   ConfigModule.h
   ConfigModule.ino
   TelegramModule.h
   TelegramModule.ino
``` 

You will need to 

### Usage

1. **Upload the code**: Open `main.ino` in Arduino IDE and upload it to the M5Stick-C.
2. **Power the devices**: Ensure the analog sensor and servo motor are properly powered.
3. **Check serial monitor**: Open the serial monitor to check for WiFi connection status and other debug information.

### Main Components

#### main.ino

This is the entry point of the program. It sets up the necessary components and runs the main loop.

#### DoorbellModule

Handles the doorbell detection and updates the display with sensor values.

#### ServoModule

Controls the servo motor.

#### ConfigModule

Handles WiFi configuration and serves a web interface for setting parameters.

#### TelegramModule

Sends notifications to predefined Telegram chat IDs when the doorbell is pressed.

### WiFi and Web Server

The system sets up a web server that allows you to configure parameters like servo angles and notification settings. You can access this web interface by connecting to the M5Stick-C's IP address on your local network.

### Notifications

When the doorbell is pressed, a notification is sent to the predefined Telegram chat IDs. 


## Troubleshooting

1. **WiFi Connection Issues**: Ensure that the SSID and password are correct and the device is within range of the WiFi network.
2. **Servo Not Moving**: Check the servo connections and ensure it is powered by an appropriate external power source.
3. **No Display Output**: Verify the connections and ensure the M5Stick-C is properly initialized.

For further assistance, refer to the respective library documentation or the M5Stick-C support community.