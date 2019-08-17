#ifndef WEBSOCKET_H
#define WEBSOCKET_H

#include <Arduino.h>

#include <string>

#include <WebSocketClient.h>
#include <WiFi.h>

#include <ArduinoJson.h>

#include "dimmer.h"

extern WiFiClient tcpClient;
extern WebSocketClient webSocketClient;
extern TimerHandle_t websocketPingTimer;

void initWebsocket();
char* createJoinMessage(std::string lobby_name);

void handleWebsocketEvents();
// void webSocketEvent(WStype_t type, uint8_t* payload, size_t length);
void pingPhoenixChannel();

#endif