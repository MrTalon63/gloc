
#define TINY_GSM_MODEM_SIM7000
#define APN "simbase"

#include "Arduino.h"
#include "TinyGsmClient.h"
#include "ESP32Time.h"
#include "BluetoothSerial.h"
#include "secrets.h"
#include <Wire.h>
#include <INA3221.h>

TinyGsm modem (Serial2);
TinyGsmClient client(modem);
ESP32Time rtc;
BluetoothSerial SerialBT;
INA3221 ina(INA3221_ADDR40_GND);

struct gpsData {
  double lat;
  double lng;
  float alt;
  float speed;
  uint8_t satellites;
  uint8_t signalQuality;
};

struct gnssData {
  float lat;
  float lon;
  float speed;
  float alt;
  int vsat;
  int usat;
  float accuracy;
  int year;
  int month;
  int day;
  int hour;
  int minute;
  int second;
  int signalQuality;
  double vbat;
  double vcc;
} data;

long unsigned int lastGpsWrite;
long unsigned int lastGsmWrite;
long unsigned int lastErrorWrite;
int lastError;
int connectionAttempts;
int networkAttempts;
boolean gpsUpdated;
gpsData lastGpsData;

float distanceBetween(float lat1, float long1, float lat2, float long2) {

  // returns distance in meters between two positions, both specified 
  // as signed decimal-degrees latitude and longitude. Uses great-circle 
  // distance computation for hypothetical sphere of radius 6372795 meters.
  // Because Earth is no exact sphere, rounding errors may be up to 0.5%.
  // Courtesy of Maarten Lamers

  float delta = radians(long1-long2);
  float sdlong = sin(delta);
  float cdlong = cos(delta);
  lat1 = radians(lat1);
  lat2 = radians(lat2);
  float slat1 = sin(lat1);
  float clat1 = cos(lat1);
  float slat2 = sin(lat2);
  float clat2 = cos(lat2);
  delta = (clat1 * slat2) - (slat1 * clat2 * cdlong); 
  delta = sq(delta); 
  delta += sq(clat2 * sdlong); 
  delta = sqrt(delta); 
  float denom = (slat1 * slat2) + (clat1 * clat2 * cdlong); 
  delta = atan2(delta, denom); 
  return delta * 6372795; 
};

int getNetworkType() {
  int networkType = modem.getNetworkMode();
  if (networkType == 2) return 1;
  return 3;
}

void setup() {
  Serial.begin(115200);
  Serial2.begin(115200);
  SerialBT.begin("ESP32TEST");
  SerialBT.setPin("2137");
  delay(1000);
  modem.restart();
  Serial.println("Enabling GPS...");
  SerialBT.println("Enabling GPS...");
  modem.enableGPS();
  Serial.println("Waiting for network...");
  SerialBT.println("Waiting for network...");
  modem.waitForNetwork();  
  Serial.println("Connecting to GPRS...");
  SerialBT.println("Connecting to GPRS...");
  modem.gprsConnect(APN);

  Serial.println("Initializing current sensor...");
  SerialBT.println("Initializing current sensor...");
  ina.begin(&Wire);
  ina.reset();
  ina.setShuntRes(10, 10, 10);
}

void loop() {
  if (millis() - lastGpsWrite > 1000 * 10) {
    Serial.println();
    Serial.println();
    Serial.println();
    Serial.println("Reading GPS data");
    SerialBT.println();
    SerialBT.println();
    SerialBT.println();
    SerialBT.println("Reading GPS data");
    
    modem.enableGPS();
    modem.getGPS(&data.lat, &data.lon, &data.speed, &data.alt, &data.vsat, &data.usat, &data.accuracy);
    modem.getGPSTime(&data.year, &data.month, &data.day, &data.hour, &data.minute, &data.second);
    data.signalQuality = modem.getSignalQuality();
    data.vbat = ina.getVoltage(INA3221_CH1);
    data.vcc = ina.getVoltage(INA3221_CH3);

    Serial.print("Latitude: ");
    Serial.println(data.lat, 6);
    Serial.print("Longitude: ");
    Serial.println(data.lon, 6);
    Serial.print("Speed: ");
    Serial.println(data.speed);
    Serial.print("Altitude: ");
    Serial.println(data.alt);
    Serial.print("Visible satellites: ");
    Serial.println(data.vsat);
    Serial.print("Used satellites: ");
    Serial.println(data.usat);
    Serial.print("Accuracy: ");
    Serial.println(data.accuracy);
    Serial.print("Raw GNSS data: ");
    Serial.println(modem.getGPSraw());
    Serial.print("Signal quality: ");
    Serial.println(data.signalQuality);
    Serial.print("Operator: ");
    Serial.println(modem.getOperator());
    Serial.print("Network mode: ");
    Serial.println(modem.getNetworkMode());
    Serial.print("VBat: ");
    Serial.println(data.vbat);
    Serial.print("VCC: ");
    Serial.println(data.vcc);
    Serial.print("Time: ");
    Serial.println(rtc.getTime("%H:%M:%S"));

    SerialBT.print("Latitude: ");
    SerialBT.println(data.lat, 6);
    SerialBT.print("Longitude: ");
    SerialBT.println(data.lon, 6);
    SerialBT.print("Speed: ");
    SerialBT.println(data.speed);
    SerialBT.print("Altitude: ");
    SerialBT.println(data.alt);
    SerialBT.print("Visible satellites: ");
    SerialBT.println(data.vsat);
    SerialBT.print("Used satellites: ");
    SerialBT.println(data.usat);
    SerialBT.print("Accuracy: ");
    SerialBT.println(data.accuracy);
    SerialBT.print("Raw GNSS data: ");
    SerialBT.println(modem.getGPSraw());
    SerialBT.print("Signal quality: ");
    SerialBT.println(data.signalQuality);
    SerialBT.print("Operator: ");
    SerialBT.println(modem.getOperator());
    SerialBT.print("Network mode: ");
    SerialBT.println(modem.getNetworkMode());
    SerialBT.print("VBat: ");
    SerialBT.println(data.vbat);
    SerialBT.print("VCC: ");
    SerialBT.println(data.vcc);
    SerialBT.print("Time: ");
    SerialBT.println(rtc.getTime("%H:%M:%S"));

    if (data.year > 2000) {
      rtc.setTime(data.second, data.minute, data.hour, data.day, data.month, data.year);
    }

    lastGpsWrite = millis();
  }
  
  if (millis() - lastGsmWrite > 1000 * 120 || (data.speed > 80 && millis() - lastGsmWrite > 1000 * 60)) {

    if (data.usat == 0) {
      if (lastErrorWrite + 1000 * 30 < millis() || lastError != 1) {
        Serial.println("No usable satellites. re-enabling GPS");
        SerialBT.println("No usable satellites. re-enabling GPS");
        modem.enableGPS();
        lastErrorWrite = millis();
        lastError = 1;
      }
      return;
    }

    if (lastGpsData.lat != 0.0 && distanceBetween(lastGpsData.lat, lastGpsData.lng, data.lat, data.lon) < 20 && millis() - lastGsmWrite < 1000 * 60 * 15) {
      if (lastErrorWrite + 1000 * 30 < millis() || lastError != 250) {
        Serial.println("GPS location not changed, vehicle is probably stationary. Time since last GSM write: " + String((millis() - lastGsmWrite) / 1000) + "s");
        SerialBT.println("GPS location not changed, vehicle is probably stationary. Time since last GSM write: " + String((millis() - lastGsmWrite) / 1000) + "s");
        lastError = 250;
        lastErrorWrite = millis();
      }
      gpsUpdated = false;
      return;
    }

    if (!modem.isNetworkConnected()) {
      Serial.println("Waiting for network");
      SerialBT.println("Waiting for network");
      if (!modem.waitForNetwork(5000L)) {
        networkAttempts++;
        if (networkAttempts > 10) {
          Serial.println("Network failed, too many attempts. Restarting modem");
          SerialBT.println("Network failed, too many attempts. Restarting modem");
          modem.restart();
          networkAttempts = 0;
        }
        if (lastErrorWrite + 1000 * 30 < millis() || lastError != 2) {
          Serial.println("Network failed.");
          SerialBT.println("Network failed.");
//        modem.restart();
          lastErrorWrite = millis();
          lastError = 2;
        }
        return;
      }
    }

    if (!modem.isGprsConnected()) {
      Serial.println("Connecting to GPRS");
      SerialBT.println("Connecting to GPRS");
      if (!modem.gprsConnect(APN)) {
        if (lastErrorWrite + 1000 * 30 < millis() || lastError != 3) {
          Serial.println("GPRS failed. Restarting modem");
          SerialBT.println("GPRS failed. Restarting modem");
          modem.restart();
          lastErrorWrite = millis();
          lastError = 3;
        }
        return;
      }
    }

    if (!client.connect(serverHostname, serverPort)) {
      if (lastErrorWrite + 1000 * 30 < millis() || lastError != 4) {
        Serial.println("Connection failed");
        SerialBT.println("Connection failed");
        connectionAttempts++;
        if (connectionAttempts > 3) {
          Serial.println("Too many connection attempts. Restarting modem");
          SerialBT.println("Too many connection attempts. Restarting modem");
          modem.restart();
          connectionAttempts = 0;
        }
        lastErrorWrite = millis();
        lastError = 4;
      }
      return;
    }

    while (!client.connected()) {
      delay(1);
    }
    
    client.printf("LG,0,%f,%f,%G,%f,%u,%f,%G,%G,%u,%u,%s", data.lat, data.lon, data.alt, data.speed, data.usat, data.accuracy, data.vbat, data.vcc, data.signalQuality, getNetworkType(), modem.getOperator());
    //client.printf("0,%f,%f,%f,%f,%u,%u", data.lat, data.lon, data.alt, data.speed, data.usat, data.signalQuality);
    delay(50);
    client.stop();
    Serial.print("Data sent. Payload: ");
    Serial.printf("LG,0,%f,%f,%G,%f,%u,%f,%G,%G,%u,%u,%s", data.lat, data.lon, data.alt, data.speed, data.usat, data.accuracy, data.vbat, data.vcc, data.signalQuality, getNetworkType(), modem.getOperator());
    SerialBT.print("Data sent. Payload: ");
    SerialBT.printf("LG,0,%f,%f,%G,%f,%u,%f,%G,%G,%u,%u,%s", data.lat, data.lon, data.alt, data.speed, data.usat, data.accuracy, data.vbat, data.vcc, data.signalQuality, getNetworkType(), modem.getOperator());
    lastGsmWrite = millis();
    lastError = 255;
    gpsUpdated = false;
    lastGpsData.lat = data.lat;
    lastGpsData.lng = data.lon;
  }
}

