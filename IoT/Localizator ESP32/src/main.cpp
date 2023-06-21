//#define TINY_GSM_DEBUG Serial

#define TINY_GSM_MODEM_SIM7000
#define APN "simbase"

#include "Arduino.h"
#include "TinyGsmClient.h"
#include "StreamDebugger.h"
#include "ESP32Time.h"
#include "secrets.h"

//StreamDebugger debugger(Serial2, Serial);
TinyGsm modem (Serial2);
TinyGsmClient client(modem);
ESP32Time rtc;

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
} data;

long unsigned int lastGpsWrite;
long unsigned int lastGsmWrite;
long unsigned int lastErrorWrite;
int lastError;
int connectionAttempts;
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

void setup() {
  Serial.begin(115200);
  Serial2.begin(115200);
  delay(1000);
  modem.init();
  Serial.println("Enabling GPS...");
  modem.enableGPS();
  Serial.println("Waiting for network...");
  modem.waitForNetwork();  
  Serial.println("Connecting to GPRS...");
  modem.gprsConnect(APN);
}

void loop() {
  if (millis() - lastGpsWrite > 1000 * 10) {
    Serial.println();
    Serial.println();
    Serial.println();
    Serial.println("Reading GPS data");
    
    modem.getGPS(&data.lat, &data.lon, &data.speed, &data.alt, &data.vsat, &data.usat, &data.accuracy);
    modem.getGPSTime(&data.year, &data.month, &data.day, &data.hour, &data.minute, &data.second);
    data.signalQuality = modem.getSignalQuality();

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
    Serial.print("Time: ");
    Serial.println(rtc.getTime("%H:%M:%S"));


    if (data.year > 2000) {
      rtc.setTime(data.second, data.minute, data.hour, data.day, data.month, data.year);
    }

    lastGpsWrite = millis();
  }
  
  if (millis() - lastGsmWrite > 1000 * 120 || (data.speed > 80 && millis() - lastGsmWrite > 1000 * 60)) {

    if (data.usat == 0) {
      if (lastErrorWrite + 1000 * 30 < millis() || lastError != 1) {
        Serial.println("No usable satellites. re-enabling GPS");
        modem.enableGPS();
        lastErrorWrite = millis();
        lastError = 1;
      }
      return;
    }

//    if (lastGpsData.lat != 0.0 && distanceBetween(lastGpsData.lat, lastGpsData.lng, data.lat, data.lon) < 25 && millis() - lastGsmWrite < 1000 * 60 * 10) {
//      Serial.println("GPS location not changed, vehicle is probably stationary. Returning...");
//      lastGsmWrite = millis();
//      lastError = 255;
//      gpsUpdated = false;
//      return;
//    }

    if (!modem.isNetworkConnected()) {
      Serial.println("Waiting for network");
      if (!modem.waitForNetwork(15000L)) {
        if (lastErrorWrite + 1000 * 30 < millis() || lastError != 2) {
        Serial.println("Network failed. Restarting modem");
//        modem.restart();
        lastErrorWrite = millis();
        lastError = 2;
        }
        return;
      }
    }

//    modem.sendAT(GF("+CDNSCFG=\"8.8.8.8\",\"8.8.4.4\""));

    if (!modem.isGprsConnected()) {
      Serial.println("Connecting to GPRS");
      if (!modem.gprsConnect(APN)) {
        if (lastErrorWrite + 1000 * 30 < millis() || lastError != 3) {
          Serial.println("GPRS failed. Restarting modem");
          modem.restart();
          lastErrorWrite = millis();
          lastError = 3;
        }
        return;
      }
    }

    if (!client.connect("server", 1111)) {
      if (lastErrorWrite + 1000 * 30 < millis() || lastError != 4) {
        Serial.println("Connection failed");
        connectionAttempts++;
        if (connectionAttempts > 3) {
          Serial.println("Too many connection attempts. Restarting modem");
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
    
    //client.println("Hejka test");
    client.printf("1,%f,%f,%f,%f,%u,%u,%u", data.lat, data.lon, data.alt, data.speed, data.usat, data.signalQuality, modem.getNetworkMode());
    client.stop();
    modem.gprsDisconnect();
    Serial.println("Data sent. Disconnecting and turning off modem");
    lastGsmWrite = millis();
    lastError = 255;
    gpsUpdated = false;
    lastGpsData.lat = data.lat;
    lastGpsData.lng = data.lon;
  }
}

