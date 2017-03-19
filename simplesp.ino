#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>
#include <WiFiUdp.h>
#include <WiFiManager.h>
#include <ArduinoOTA.h>
#include <Button.h>

#define RELAY_PIN 15
#define LED_PIN 2 // LOW is on
Button btn(13);
WiFiManager wifiManager;

/* For use with the KAB outlet
    --------------------------------
nodemcuv2.menu.FlashSize.1M64=1M (64K SPIFFS)
nodemcuv2.menu.FlashSize.1M64.build.flash_size=1M
nodemcuv2.menu.FlashSize.1M64.build.flash_ld=eagle.flash.1m64.ld
nodemcuv2.menu.FlashSize.1M64.build.spiffs_start=0xEB000
nodemcuv2.menu.FlashSize.1M64.build.spiffs_end=0xFB000
nodemcuv2.menu.FlashSize.1M64.build.spiffs_blocksize=4096
nodemcuv2.menu.FlashSize.1M64.upload.maximum_size=958448
*/

char host[10];
bool state = false;

#define RELAY_PIN 15
#define LED_PIN 2 // LOW is on
#define BUTTON_PIN 13

ESP8266WebServer server ( 80 );

void handleRoot() {
  if (state) {
    server.send ( 200, "text/html", "status: on<br>version: v1.1");
  } else {
    server.send ( 200, "text/html", "status: off<br>version: v1.1" );
  }
}

void turnOn() {
  state = true;
  digitalWrite(RELAY_PIN, HIGH);
  server.send ( 200, "text/html", "on");
}

void turnOff() {
  state = false;
  digitalWrite(RELAY_PIN, LOW);
  server.send ( 200, "text/html", "off");
}


void setup() {
  pinMode(LED_PIN, OUTPUT);
  pinMode(RELAY_PIN, OUTPUT);
  btn.begin();
  Serial.begin(115200);
  Serial.println("Booting");

  sprintf(host, "kab-%06x", ESP.getChipId());

  wifiManager.autoConnect(host);

  ArduinoOTA.setHostname(host);

  ///ArduinoOTA.setPassword((const char *)"123");

  ArduinoOTA.onStart([]() {
    Serial.println("Start");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();

  server.on ( "/", handleRoot );
  server.on ( "/on", turnOn );
  server.on ( "/off", turnOff );

  server.begin();

  Serial.print("Ready: "); Serial.println(host);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  digitalWrite(LED_PIN, LOW);
}

void loop() {
  ArduinoOTA.handle();
  server.handleClient();

  if (btn.pressed()) {
    if (state) {
      turnOff();
    } else {
      turnOn();
    }
  }  
}
