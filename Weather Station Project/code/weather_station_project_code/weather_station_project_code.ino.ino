#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <Adafruit_BMP085.h>
#include <DHT.h>

const char* ssid = "HONOR 200 Pro"; 
const char* password = "prajithrockerz"; 

#define BOT_TOKEN "8734190184:AAEvrFARGtIclyxWX3iQe6PC4Q2wqMcZe70" 
#define CHAT_ID "8624400926"

WiFiClientSecure client;
UniversalTelegramBot bot(BOT_TOKEN, client);

#define MQ4_PIN 34
#define MQ135_PIN 35
#define RED_LED 19
#define GREEN_LED 18

#define DHTPIN 4
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);
Adafruit_BMP085 bmp;
int mq4Base = 0;
int mq135Base = 0;

float tempDHT, humidity, pressure, altitude, tempBMP;
int mq4Avg, mq135Avg;
float prevTemp = 0.0, prevPressure = 0.0;
int prevMq4 = 0, prevMq135 = 0;
const float LIMIT_TEMP = 35.0;     
const float LIMIT_PRESSURE = 1025.0; 
const int LIMIT_GAS = 80;
const float DELTA_TEMP = 3.0;      
const float DELTA_PRESSURE = 4.0;  
const int DELTA_GAS = 150;
volatile bool alertGas = false;
volatile bool alertTemp = false;
volatile bool alertPressure = false;
volatile bool spikeGas = false;
volatile bool spikeTemp = false;
volatile bool spikePressure = false;
volatile bool isReady = false; 
unsigned long lastAlertTime = 0;
const unsigned long ALERT_COOLDOWN = 300000; 
String getGasStatus(int current, int base) {
  int difference = current - base;
  if (difference < 40) return "Normal 🟢";
  if (difference < LIMIT_GAS) return "Elevated 🟡";
  return "DANGER 🔴";
}
String getPressureStatus(float press) {
  if (press < 970.0) return "Low (Stormy) ⛈️";
  if (press > 1010.0) return "High (Clear) ☀️";
  return "Normal (Fair) 🌤️";
}
void sensorTask(void * parameter) {
  for (;;) {
    int mq4Sum = 0;
    int mq135Sum = 0;

    for (int i = 0; i < 10; i++) {
      mq4Sum += analogRead(MQ4_PIN);
      mq135Sum += analogRead(MQ135_PIN);
      vTaskDelay(50 / portTICK_PERIOD_MS);
    }

    mq4Avg = mq4Sum / 10;
    mq135Avg = mq135Sum / 10;
    humidity = dht.readHumidity();
    tempDHT = dht.readTemperature();
    pressure = bmp.readPressure() / 100.0;
    altitude = bmp.readAltitude();
    tempBMP = bmp.readTemperature();
    alertGas = ((mq4Avg - mq4Base > LIMIT_GAS) || (mq135Avg - mq135Base > LIMIT_GAS));
    alertTemp = (tempDHT > LIMIT_TEMP);
    alertPressure = (pressure > LIMIT_PRESSURE);

    if (abs(tempDHT - prevTemp) >= DELTA_TEMP) spikeTemp = true;
    if (abs(pressure - prevPressure) >= DELTA_PRESSURE) spikePressure = true;
    if (abs(mq4Avg - prevMq4) >= DELTA_GAS || abs(mq135Avg - prevMq135) >= DELTA_GAS) spikeGas = true;
    prevTemp = tempDHT;
    prevPressure = pressure;
    prevMq4 = mq4Avg;
    prevMq135 = mq135Avg;

    if (alertGas || alertTemp || alertPressure || spikeGas || spikeTemp || spikePressure) {
      digitalWrite(RED_LED, HIGH);
      digitalWrite(GREEN_LED, LOW);
    } else {
      digitalWrite(RED_LED, LOW);
      digitalWrite(GREEN_LED, HIGH);
    }
    Serial.println("Sensor task running");
    vTaskDelay(2000 / portTICK_PERIOD_MS);
  }
}

void telegramTask(void * parameter) {
  for (;;) {
    if (isReady && (alertGas || alertTemp || alertPressure) && (millis() - lastAlertTime > ALERT_COOLDOWN || lastAlertTime == 0)) {
      String alertMsg = "🚨 WARNING: LIMIT EXCEEDED 🚨\n\n";
      if (alertGas) alertMsg += "- High Gas/AQI Detected!\n";
      if (alertTemp) alertMsg += "- High Temp: " + String(tempDHT) + " °C\n";
      if (alertPressure) alertMsg += "- High Pressure: " + String(pressure) + " hPa\n";
      bot.sendMessage(CHAT_ID, alertMsg, "");
      lastAlertTime = millis(); 
    }
    if (isReady && (spikeGas || spikeTemp || spikePressure)) {
      String spikeMsg = "⚡ URGENT: SUDDEN SPIKE DETECTED ⚡\n\n";
      if (spikeGas) spikeMsg += "- Rapid increase in Gas/AQI!\n";
      if (spikeTemp) spikeMsg += "- Temperature jumped rapidly!\n";
      if (spikePressure) spikeMsg += "- Sudden change in air pressure!\n";
      spikeMsg += "\nSend /data for current readings.";
      bot.sendMessage(CHAT_ID, spikeMsg, "");
      spikeGas = false;
      spikeTemp = false;
      spikePressure = false;
    }
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    while (numNewMessages) {
      for (int i = 0; i < numNewMessages; i++) {
        String text = bot.messages[i].text;
        if (text == "/data") {
          if(!isReady){
            bot.sendMessage(CHAT_ID, "⏳ The sensors are still warming up and calibrating. Please wait a moment!", "");
          }else{
            String msg = "🌤️ Weather & Air Quality Station\n\n";
            msg += "💨 MQ4 (Methane): " + getGasStatus(mq4Avg, mq4Base) + " (" + String(mq4Avg) + ")\n";
            msg += "🌫️ MQ135 (Air Quality): " + getGasStatus(mq135Avg, mq135Base) + " (" + String(mq135Avg) + ")\n";
            msg += "🌡️ Temp: " + String(tempDHT) + " °C\n";
            msg += "💧 Humidity: " + String(humidity) + " %\n";
            msg += "⏱️ Pressure: " + getPressureStatus(pressure) + " (" + String(pressure) + " hPa)\n";
            msg += "⛰️ Altitude: " + String(altitude) + " m\n";
            bot.sendMessage(CHAT_ID, msg, "");
          }
        }
        else if (text == "/restart") {
          bot.sendMessage(CHAT_ID, "🔄 Restarting ESP32... Be back in a few seconds!", "");
          bot.getUpdates(bot.last_message_received + 1); 
          vTaskDelay(1000 / portTICK_PERIOD_MS); 
          ESP.restart(); 
        }
      }
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(RED_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  digitalWrite(RED_LED, HIGH);
  digitalWrite(GREEN_LED, LOW);
  dht.begin();
  bmp.begin();
  WiFi.begin(ssid, password);
  Serial.print("Connecting WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");
  client.setInsecure();
  bot.sendMessage(CHAT_ID, "⚙️ ESP32 Started!\nCalibrating gas sensors. This will take 10 seconds. Please wait...", "");

  long mq4Sum = 0;
  long mq135Sum = 0;
  for (int i = 0; i < 10; i++) {
    mq4Sum += analogRead(MQ4_PIN);
    mq135Sum += analogRead(MQ135_PIN);
    delay(1000); 
  }
  mq4Base = mq4Sum / 10;
  mq135Base = mq135Sum / 10;
  prevTemp = dht.readTemperature();
  prevPressure = bmp.readPressure() / 100.0;
  prevMq4 = mq4Base;
  prevMq135 = mq135Base;
  isReady = true;
  digitalWrite(RED_LED, LOW); 
  bot.sendMessage(CHAT_ID, "✅ Calibration complete!\nBaseline set.\nSend /data to get readings.", "");
  xTaskCreatePinnedToCore(telegramTask, "TelegramTask", 8000, NULL, 1, NULL, 0);
  xTaskCreatePinnedToCore(sensorTask, "SensorTask", 4000, NULL, 1, NULL, 1);
}

void loop() {
}