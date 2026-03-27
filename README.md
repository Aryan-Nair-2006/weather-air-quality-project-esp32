# IoT-Based Weather and Air Quality Monitoring System using ESP32
An IoT-based environmental monitoring system that measures temperature, humidity, air pressure, and detects hazardous gas using ESP32. The system detects hazardous conditions and sends real-time alerts through a Telegram bot.

## Features
- Temperature and humidity monitoring using DHT11 sensor
- Air pressure monitoring using BMP085 sensor
- Hazardous gas detection using MQ4 (Methane) and MQ135 (Air Quality)
- Real-time alerts using Telegram bot
- Sudden spike detection for abnormal conditions
- LED-based warning system

## Hardware Components
- ESP32 DevKit V1
- MQ4 Gas Sensor (Methane detection)
- MQ135 Air Quality Sensor (Carbon Monoxide and other gases detection)
- DHT11 Temperature & Humidity Sensor
- BMP085 Pressure Sensor
- LEDs(Red and Green)
- Breadboard and Wires

## Software Used
- Arduino IDE
- ESP32 Board Package
-  WiFi Library
- Telegram Bot API
- DHT Sensor Library
- Adafruit BMP085 Library

## How It Works
1. ESP32 connects to a WiFi network
2. Green LED turns on by default if gas levels are safe
3. Sensors collect environmental data:
   - Gas levels (MQ4, MQ135)
   - Temperature & Humidity (DHT11)
   - Pressure (BMP085)
4. ESP32 processes data and checks threshold values
5. If hazardous gas conditions are detected:
   - Red LED turns ON
   - Telegram alert is sent to user
6. User can request data using Telegram command `/data`

## Project Type
Group Project

## My Contribution
- Handled hardware integration using ESP32
- Connected and configured all sensors (MQ4, MQ135, DHT11, BMP085)
- Interfaced hardware components with Arduino IDE code
- Assisted in testing and debugging system functionality

## Work Completed
- ESP32 hardware setup completed
- Sensors successfully interfaced
- WiFi connectivity established
- Telegram bot integrated
- Real-time monitoring implemented
- Sudden spike detection logic implemented

## Future Improvements
- Improve gas sensor calibration accuracy
- LED alert system implementation
- Implement cloud-based data storage
- Optimize power consumption

## Cost Analysis
Approximate total cost: ₹1330

## References
- https://randomnerdtutorials.com
- https://arduino.cc
- https://iotdesignpro.com
- https://lastminuteengineers.com

## Author
Aryan
