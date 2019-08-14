/*
 * WebSocketClientSSL.ino
 *
 *  Created on: 10.12.2015
 *
 *  note SSL is only possible with the ESP8266
 *
 */

#include <Arduino.h>

#include "MATRIXVoiceOTA.h"

#include <WebSocketsClient.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>

#include <ArduinoJson.h>

MATRIXVoiceOTA otaObj(WIFI_SSID, WIFI_PASS, HOSTNAME,
                      OTA_PASS);  // please see platformio.ini

WebSocketsClient webSocket;

DynamicJsonDocument doc(200);
char join_channel[512];

#define USE_SERIAL Serial

void hexdump(const void* mem, uint32_t len, uint8_t cols = 16) {
    const uint8_t* src = (const uint8_t*)mem;
    USE_SERIAL.printf("\n[HEXDUMP] Address: 0x%08X len: 0x%X (%d)",
                      (ptrdiff_t)src, len, len);
    for (uint32_t i = 0; i < len; i++) {
        if (i % cols == 0) {
            USE_SERIAL.printf("\n[0x%08X] 0x%08X: ", (ptrdiff_t)src, i);
        }
        USE_SERIAL.printf("%02X ", *src);
        src++;
    }
    USE_SERIAL.printf("\n");
}

void webSocketEvent(WStype_t type, uint8_t* payload, size_t length) {
    DynamicJsonDocument ret(200);
    DeserializationError error;
    const char* str;
    switch (type) {
        case WStype_DISCONNECTED:
            USE_SERIAL.printf("[WSc] Disconnected!\n");
            break;
        case WStype_CONNECTED: {
            USE_SERIAL.printf("[WSc] Connected to url: %s\n", payload);

            // send message to server when Connected
            webSocket.sendTXT(join_channel);
        } break;
        case WStype_TEXT:
            Serial.println();
            error = deserializeJson(ret, payload);
            if (error) {
                Serial.print(F("deserializeJson() failed: "));
                Serial.println(error.c_str());
                return;
            }
            serializeJson(ret, Serial);
            Serial.println();
            str = ret["topic"];
            Serial.println(str);
            // USE_SERIAL.printf("[WSc] get binary length: %u\n", length);
            // USE_SERIAL.printf("[WSc] get text: %s\n", payload);

            // send message to server
            // webSocket.sendTXT("message here");
            break;
        case WStype_BIN:
            USE_SERIAL.printf("[WSc] get binary length: %u\n", length);
            hexdump(payload, length);

            // send data to server
            // webSocket.sendBIN(payload, length);
            break;
        case WStype_ERROR:
        case WStype_FRAGMENT_TEXT_START:
        case WStype_FRAGMENT_BIN_START:
        case WStype_FRAGMENT:
        case WStype_FRAGMENT_FIN:
            break;
    }
}

void setup() {
    // USE_SERIAL.begin(921600);
    USE_SERIAL.begin(115200);

    // Serial.setDebugOutput(true);
    USE_SERIAL.setDebugOutput(true);

    USE_SERIAL.println();
    USE_SERIAL.println();
    USE_SERIAL.println();

    for (uint8_t t = 4; t > 0; t--) {
        USE_SERIAL.printf("[SETUP] BOOT WAIT %d...\n", t);
        USE_SERIAL.flush();
        delay(1000);
    }

    otaObj.setup();

    webSocket.begin("192.168.1.71", 4000, "/socket/websocket");
    webSocket.onEvent(webSocketEvent);

    doc["topic"] = "light_switch:lobby";
    doc["event"] = "phx_join";
    JsonObject payload = doc.createNestedObject("payload");
    doc["ref"] = 0;

    serializeJson(doc, join_channel);
}

void loop() {
    otaObj.loop();
    webSocket.loop();
}