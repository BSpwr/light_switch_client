/*
 * WebSocketClientSSL.ino
 *
 *  Created on: 10.12.2015
 *
 *  note SSL is only possible with the ESP8266
 *
 */

#include <Arduino.h>

#include <string>

#include "MATRIXVoiceOTA.h"

#include <WebSocketsClient.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>

#include <ArduinoJson.h>
#include <RBDdimmer.h>

extern "C" {
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/timers.h"
}

MATRIXVoiceOTA otaObj(WIFI_SSID, WIFI_PASS, HOSTNAME,
                      OTA_PASS);  // please see platformio.ini

WebSocketsClient webSocket;

// DIMMER SETUP
#define outputPinLight 26
#define outputPinFan 27
#define zerocross 25

dimmerLamp lightDimmer(outputPinLight, zerocross);
dimmerLamp fanDimmer(outputPinFan, zerocross);

/* ************************************************************************ *
      EASY DIMMER CONTROL
 * ************************************************************************ */
void setDimmer(dimmerLamp& dimmer, int intensity) {
    intensity =
        std::max(std::min(intensity, 94), 0);  // Gate value between 0 and 95
    if (intensity < 15) {
        dimmer.setMode(NORMAL_MODE);
        dimmer.setState(OFF);
    } else {
        dimmer.setMode(NORMAL_MODE);
        dimmer.setState(ON);
        dimmer.setPower(intensity);
    }
}

char* getJoinMessage(std::string lobby_name) {
    StaticJsonDocument<200> join_json;
    char* join_json_string = new char[512];
    join_json["topic"] = "light_switch:lobby";
    join_json["event"] = "phx_join";
    JsonObject payload = join_json.createNestedObject("payload");
    join_json["ref"] = 0;
    serializeJson(join_json, join_json_string, 512);
    join_json.clear();
    return join_json_string;
}

void webSocketEvent(WStype_t type, uint8_t* payload, size_t length) {
    if (type == WStype_DISCONNECTED) {
        Serial.printf("[WSc] Disconnected!\n");
    } else if (type == WStype_CONNECTED) {
        Serial.printf("[WSc] Connected to url: %s\n", payload);

        // send message to server when Connected
        char* join_json_string = getJoinMessage("light_switch:lobby");
        webSocket.sendTXT(join_json_string);
        delete[] join_json_string;
    } else if (type == WStype_TEXT) {
        StaticJsonDocument<512> response_json;
        DeserializationError error;
        error = deserializeJson(response_json, payload);
        if (error) {
            Serial.print(F("deserializeJson() failed: "));
            Serial.println(error.c_str());
            return;
        }
        const char* str = response_json["topic"];
        if (response_json["event"] == "set_light") {
            int lightVal = response_json["payload"]["value"];
            Serial.printf("Setting light to %i", lightVal);
            Serial.println();
            setDimmer(lightDimmer, lightVal);
        }
        response_json.clear();
    }
}

void setup() {
    Serial.begin(115200);

    Serial.setDebugOutput(true);

    // Enable dimmers
    lightDimmer.begin(NORMAL_MODE, OFF);
    fanDimmer.begin(NORMAL_MODE, OFF);

    otaObj.setup();

    webSocket.begin("192.168.1.71", 4000, "/socket/websocket");
    webSocket.onEvent(webSocketEvent);
}

void loop() {
    otaObj.loop();
    webSocket.loop();
}