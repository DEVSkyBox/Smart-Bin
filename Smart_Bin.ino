#include <WiFi.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <ESP32Servo.h> 
#include <APDS9960.h>
//#include <TinyGPS++.h>
//#include <HardwareSerial.h>
#include <Wire.h>

// WiFi Credentials
const char* ssid = "your ssid";
const char* password = "your password";

// Database details
const char* databaseUrl = "your database url";
const char* databaseKey = "your database key";
const char* tableName = "SmartBinData";

// Sensor Pins
const int triggerpin = 12;
const int echopin = 13;
const int servopin =14 ;
const int gps_RXpin = 27;
const int gps_TXpin = 26;
const int batterypin = 34;  // Analog pin for battery monitoring

int pos;
int proximity;
int Wastepercentage;
float Batteryvoltage;
int Batterypercentage;

unsigned long lastsendtime = 0;
const unsigned long sendinterval = 5000;


Servo binservo;
APDS9960 sensor;


void setup() {
  // put your setup code here, to run once:
Serial.begin(9600);
connectWiFi();
//gesture_setup();
sensor.begin();
sensor.enableProximitySensing(true);
sensor.setProximitySensitivity(3);
sensor.setProximitySensorRange(3); 
  
pinMode(triggerpin,OUTPUT);
pinMode(echopin,INPUT);
pinMode(batterypin,INPUT);
binservo.attach(14);

}

void loop() {
  // put your main code here, to run repeatedly:

  /*
  Code for waste level/height
  */
  long duration;
  float distance;
  digitalWrite(triggerpin, LOW);
  delayMicroseconds(2);
  digitalWrite(triggerpin, HIGH);
  delayMicroseconds(10);
  digitalWrite(triggerpin, LOW);

  duration = pulseIn(echopin,HIGH);
  distance = (duration*0.0342)/2;
  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.println(" cm");

  /*
  Code for waste percentage level
  */
   Wastepercentage = (1-(distance/23.))*100;// bin height = 21.85
  //return constrain(percentagelevel,0,100);
  Serial.print("percentagelevel: ");
  Serial.print(Wastepercentage);
  Serial.println("%");

  /*
  Code for battery level and percentage
  */
  int volts = analogRead(batterypin);
   Batteryvoltage = volts*(3.3/4095.)*2;
  float batterypercentage = (Batteryvoltage - 1.2)/(4.35 - 1.2);
  Batterypercentage = batterypercentage*100;
  Serial.print("Voltage: ");
  Serial.print(Batteryvoltage);
  Serial.println("v");
  Serial.print("Battery: ");
  Serial.print(Batterypercentage);
  Serial.println("%"); 

 /*/ void SendRequest();
 if (WiFi.status() == WL_CONNECTED) {

    // Create JSON payload
    DynamicJsonDocument doc(1024);
    doc["Wastepercentage"] = Wastepercentage;
    doc["Batterypercentage"] = Batterypercentage;

    String jsonPayload;
    serializeJson(doc, jsonPayload);

    // Send to Supabase
    HTTPClient http;
    String endpoint = String(databaseUrl) + "/rest/v1/" + tableName;

    http.begin(endpoint);
    http.addHeader("Content-Type", "application/json");
    http.addHeader("apikey", databaseKey);
    http.addHeader("Authorization", "Bearer " + String(databaseKey));
    http.addHeader("Prefer", "return=minimal"); // Suppress response body

    int httpCode = http.POST(jsonPayload);

    if (httpCode == HTTP_CODE_CREATED) {
      Serial.println("Data sent to Database!");
    } else {
      Serial.printf("Error: %s\n", http.errorToString(httpCode).c_str());
    }

    http.end();
  }
  */
  unsigned long currenttime = millis();
  if (currenttime - lastsendtime >= sendinterval){
    SendRequest();
    lastsendtime = currenttime;
  }

  /*
  Code for opening and closing bin lid 
  */
  proximity = sensor.readProximity();
  Serial.println(proximity);
  cover_motion();

 delay(1000);
}

// ✅ FUNCTION: Connect WiFi
void connectWiFi() {
  Serial.print("Connecting to WiFi...");
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.print(".");
    }
    Serial.println("Connected!");
    }



// ✅ FUNCTION: Open Bin Lid
void openlid(){
  //Serial.println("Opening Lid...");
  for (pos = 90; pos >= 0; pos -=2){
    binservo.write(pos);
    delay(20);
  }
}

// ✅ FUNCTION: Close Bin Lid
void closelid(){
  //Serial.println("Closing Lid...");
  for (pos = 0; pos <= 90; pos += 1){
    binservo.write(pos);
    delay(30);
}
}

// ✅ FUNCTION: Gesture Sensor Setup
void gesture_setup(){
  sensor.begin();
  sensor.enableProximitySensing(true);
  sensor.setProximitySensitivity(3);
  sensor.setProximitySensorRange(3); 
}

void cover_motion(){
  if (proximity >= 5 ){
    openlid();
   delay(7000);
    closelid();
  }
}
 
 
 void SendRequest(){
   if (WiFi.status() == WL_CONNECTED) {

    // Create JSON payload
    DynamicJsonDocument doc(1024);
    doc["Wastepercentage"] = Wastepercentage;
    doc["Batterypercentage"] = Batterypercentage;

    String jsonPayload;
    serializeJson(doc, jsonPayload);

    // Send to Supabase
    HTTPClient http;
    String endpoint = String(databaseUrl) + "/rest/v1/" + tableName;

    http.begin(endpoint);
    http.addHeader("Content-Type", "application/json");
    http.addHeader("apikey", databaseKey);
    http.addHeader("Authorization", "Bearer " + String(databaseKey));
    http.addHeader("Prefer", "return=minimal"); // Suppress response body

    int httpCode = http.POST(jsonPayload);

    if (httpCode == HTTP_CODE_CREATED) {
      Serial.println("Data sent to Database!");
    } else {
      Serial.printf("Error: %s\n", http.errorToString(httpCode).c_str());
    }

    http.end();
  }
 }


