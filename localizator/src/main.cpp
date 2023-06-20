#define TINY_GSM_MODEM_SIM800
#define APN "simbase"

#include <Arduino.h>
#include <SoftwareSerial.h>
#include <TinyGPSPlus.h>
#include <TinyGsmClient.h>
#include <ESP8266WiFi.h>

SoftwareSerial GSM (5, 4);
SoftwareSerial GPS (13, 15);
TinyGPSPlus gps;
TinyGsm modem (GSM);
TinyGsmClient client(modem);

struct gpsData {
  double lat;
  double lng;
  float alt;
  float speed;
  uint8_t satellites;
  uint8_t signalQuality;
};

long unsigned int lastGpsWrite;
long unsigned int lastGsmWrite;
long unsigned int lastErrorWrite;
int lastError;
boolean gpsUpdated;
gpsData lastGpsData;

void setup() {
  Serial.begin(9600);
  GSM.begin(9600);
  GPS.begin(9600);
  WiFi.mode(WIFI_OFF);
  delay(1000);
  modem.restart();
  modem.sendAT("+CNETSCAN=1");
  Serial.println("Waiting for network...");
  modem.waitForNetwork();  
  Serial.println("Connecting to GPRS...");
  modem.gprsConnect(APN);
}

void smartDelay(unsigned long ms) {
  unsigned long start = millis();
  do {
    while (GPS.available())
      gps.encode(GPS.read());
  } while (millis() - start < ms);
};

void loop() {
  if (GPS.available()) {
    gps.encode(GPS.read());
  }
  if (millis() - lastGpsWrite > 1000 * 10) {
    Serial.println("GPS data:");
    Serial.print("Latitude = "); 
    Serial.print(gps.location.lat(), 6);
    Serial.print(" Longitude = "); 
    Serial.println(gps.location.lng(), 6);
    Serial.print("Altitude = ");
    Serial.println(gps.altitude.meters());
    Serial.print("Speed = ");
    Serial.println(gps.speed.kmph());
    Serial.print("Satellites = ");
    Serial.println(gps.satellites.value());
    Serial.print("Signal quality = ");
    Serial.println(modem.getSignalQuality());
    Serial.print("Registration status = ");
    Serial.println(modem.getRegistrationStatus());
    Serial.print("Network operator = ");
    Serial.println(modem.getOperator());
    //Serial.print("Current station ID = ");
    //modem.sendAT("+CNETSCAN");
    //String response;
    //modem.waitResponse(20000L, response);
    //Serial.println(response);
    lastGpsWrite = millis();
  }

  if (gps.location.isUpdated() && gps.location.isValid()) {
    gpsUpdated = true;
  }
  
  if (millis() - lastGsmWrite > 1000 * 120 || (gps.speed.kmph() > 80 && millis() - lastGsmWrite > 1000 * 60)) {
    
    if (gpsUpdated == false) {
      if (lastErrorWrite + 1000 * 30 < millis() || lastError != 1) {
        Serial.println("GPS location not updated. Returning...");
        lastErrorWrite = millis();
        lastError = 1;
      }
      return;
    }

    Serial.println("GPS location updated. Turning on modem");

    if (lastGpsData.lat != 0.0 && gps.distanceBetween(lastGpsData.lat, lastGpsData.lng, gps.location.lat(), gps.location.lng()) < 25 && millis() - lastGsmWrite < 1000 * 60 * 15) {
      Serial.println("GPS location not changed, vehicle is probably stationary. Returning...");
      lastGsmWrite = millis();
      lastError = 255;
      gpsUpdated = false;
      return;
    }

    modem.init();

    if (!modem.isNetworkConnected()) {
      Serial.println("Waiting for network");
      if (!modem.waitForNetwork(15000L)) {
        if (lastErrorWrite + 1000 * 30 < millis() || lastError != 2) {
        Serial.println("Network failed. Restarting modem");
        modem.restart();
        lastErrorWrite = millis();
        lastError = 2;
        }
        return;
      }
    }

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

    if (!client.connect("server", 2137)) {
      if (lastErrorWrite + 1000 * 30 < millis() || lastError != 4) {
        Serial.println("Connection failed");
        lastErrorWrite = millis();
        lastError = 4;
      }
      return;
    }

    client.printf("0,%f,%f,%f,%f,%u,%u\n", gps.location.lat(), gps.location.lng(), gps.altitude.meters(), gps.speed.kmph(), gps.satellites.value(), modem.getSignalQuality());
    client.stop();
    modem.gprsDisconnect();
    Serial.println("Data sent. Disconnecting and turning off modem");
    lastGsmWrite = millis();
    lastError = 255;
    gpsUpdated = false;
    lastGpsData.lat = gps.location.lat();
    lastGpsData.lng = gps.location.lng();
  }
}

