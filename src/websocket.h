#ifndef WEBSOCKET_H
#define WEBSOCKET_H

#include <Arduino.h>

#include <string>

#include <WebSocketClient.h>
#include <WiFi.h>

#include <ArduinoJson.h>

#include "dimmer.h"
#include "global.h"

extern WiFiClient tcpClient;
extern WebSocketClient webSocketClient;
extern TimerHandle_t websocketPingTimer;

void connectWebsocket();
char* createJoinMessage(std::string lobby_name);

void handleWebsocketEvents(void* p);

void pingPhoenixChannel();

// void handleWebsocketEvents();
// void webSocketEvent(WStype_t type, uint8_t* payload, size_t length);
// void pingPhoenixChannel();

#endif