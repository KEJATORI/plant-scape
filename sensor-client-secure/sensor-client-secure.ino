#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ArduinoWebsockets.h>
#include "DHT.h"
#include <Servo.h>
#include <ArduinoJson.h>

using namespace websockets;

#define DHTPIN 4
#define DHTTYPE DHT22
#define soilMoisturePin A0
#define servoPin 0
#define waterPumpPin 16
#define ledPin 14

// wifi settings
const char* ssid = "Rivera_WIFI";
const char* pass = "Rivera$$$1234";
const char* websockets_connection_string = "wss://plant-scape.herokuapp.com/"; //Enter server adress
const char echo_org_ssl_fingerprint[] PROGMEM = "99 e0 da 5f a7 92 41 75 1b d5 41 fd a5 da ea f7 e4 a7 0a 72";

// values
String clientType = "sensor";
float humidity = 0;
float temperature = 0;
float heatIndex = 0;
int soilMoisture = 0; // 0 - 100

// settingValues
int soilMoistureThreshold = 10; // 0 - 100
bool isAutoWater = false;
bool isWater = false;
bool isCover = false;

// initialized
ESP8266WiFiMulti WiFiMulti;
WebsocketsClient client;
DHT dht(DHTPIN, DHTTYPE);
Servo servo;
StaticJsonDocument<300> doc;

void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  pinMode(ledPin, OUTPUT);
  pinMode(waterPumpPin, OUTPUT);
  servo.attach(servoPin);
  dht.begin();
  
  for(uint8_t t = 4; t > 0; t--) {
    Serial.printf("[SETUP] BOOT WAIT %d...\n", t);
    Serial.flush();
    delay(1000);
  }
  
  WiFiMulti.addAP(ssid, pass);

  while(WiFiMulti.run() != WL_CONNECTED) {
    delay(100);
  }
 
  // run callback when message recieved
  client.onMessage(onMessageCallback);
  // run callback when events are occuring
  client.onEvent(onEventsCallback);
  // Before connecting, set the ssl fingerprint of the server
  client.setFingerprint(echo_org_ssl_fingerprint);
  // Connect to server
  client.connect(websockets_connection_string);
  client.ping();
}

void loop() {
  client.poll();
  readSensors();
  coverPlant(isCover);
  waterPlant(isWater, isAutoWater, soilMoistureThreshold);
  updateServer();
  delay(1000);
}

void onMessageCallback(WebsocketsMessage message) {
    Serial.print("Got Message: ");
    Serial.println(message.data());
    DeserializationError error = deserializeJson(doc, message.data());
    if (error) {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.c_str());
      return;
    }
    soilMoistureThreshold = doc["soilMoistureThreshold"];
    isAutoWater = doc["isAutoWater"];
    isWater = doc["isWater"];
    isCover = doc["isCover"];
}

void onEventsCallback(WebsocketsEvent event, String data) {
    if(event == WebsocketsEvent::ConnectionOpened) {
        Serial.println("Connnection Opened");
        digitalWrite(ledPin, HIGH);
        char buffer[300];
        doc["clientType"] = clientType;
        serializeJson(doc, buffer);
        client.send(buffer);
    } else if(event == WebsocketsEvent::ConnectionClosed) {
        Serial.println("Connnection Closed");
        digitalWrite(ledPin, LOW);
        humidity = 0;
        temperature = 0;
        heatIndex = 0;
        soilMoisture = 0;
        soilMoistureThreshold = 10;
        isAutoWater = false;
        isWater = false;
        isCover = false;
    } else if(event == WebsocketsEvent::GotPing) {
        Serial.println("Got a Ping!");
    } else if(event == WebsocketsEvent::GotPong) {
        Serial.println("Got a Pong!");
    }
}

void readSensors() {
  soilMoisture = analogRead(soilMoisturePin);
  soilMoisture = map(soilMoisture, 0, 1023, 100, 0);
  humidity = dht.readHumidity();
  temperature = dht.readTemperature();
  delay(20);
  if (isnan(humidity) || isnan(temperature)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }
  heatIndex = dht.computeHeatIndex(temperature, humidity, false);
}

void coverPlant(bool isCover){
  int servoPosition = servo.read();
  if(isCover == true && servoPosition < 180){
      servo.writeMicroseconds(2300);
  }else if(isCover == false && servoPosition > 0){
      servo.writeMicroseconds(700); 
  }
}

void waterPlant(bool isWater1, bool isAutoWater, int soilMoistureThreshold){
  if(isWater1 || isAutoWater){
    int soilMoisture = analogRead(soilMoisturePin);
    soilMoisture = map(soilMoisture, 0, 1023, 100, 0);
    if(soilMoisture < soilMoistureThreshold){
      isWater = true;
      digitalWrite(waterPumpPin, HIGH);
    }
    if(soilMoisture > soilMoistureThreshold){
      isWater = false;
      digitalWrite(waterPumpPin, LOW);
    }
  }
}

void updateServer(){
  char buffer[300];
  doc["clientType"] = clientType;
  doc["humidity"] = humidity;
  doc["temperature"] = temperature;
  doc["heatIndex"] = heatIndex;
  doc["soilMoisture"] = soilMoisture; // 0 - 100
  doc["soilMoistureThreshold"] = soilMoistureThreshold; // 0 - 100
  doc["isAutoWater"] = isAutoWater;
  doc["isWater"] = isWater;
  doc["isCover"] = isCover;
  serializeJson(doc, buffer);
  client.send(buffer);
}
