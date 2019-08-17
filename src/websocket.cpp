#include "websocket.h"

#include <string.h>

WiFiClient tcpClient;
WebSocketClient webSocketClient;

TimerHandle_t websocketPingTimer = xTimerCreate("websocketPingTimer", pdMS_TO_TICKS(5000), pdTRUE, (void *)0,
                reinterpret_cast<TimerCallbackFunction_t>(pingPhoenixChannel));

char path[] = "/socket/websocket";
char host[] = "192.168.1.71";

void initWebsocket() {
    // webSocket.begin("192.168.1.71", 4000, "/socket/websocket");
    // webSocket.onEvent(webSocketEvent);
    // webSocket.setReconnectInterval(1000);

//   // Connect to the websocket server
//     while (!tcpClient.connect(host, 4000)) {}
//     Serial.println("Connected to WS TCP");


  if (tcpClient.connect(host, 4000)) {
    Serial.println("[WSc] TCP client Connected");
  } else {
    Serial.println("[WSc] TCP client Connection failed");
  }

  // Handshake with the server
    webSocketClient.path = path;
    webSocketClient.host = host;
    // while (!webSocketClient.handshake(tcpClient)) {}
    // Serial.println("Handshake successful");

  if (webSocketClient.handshake(tcpClient)) {
    Serial.println("[WSc] Handshake successful");
  } else {
    Serial.println("[WSc] Handshake failed");
  }
    // send message to server when Connected
    char* join_json_string = createJoinMessage("light_switch:lobby");
    webSocketClient.sendData(join_json_string);
    delete[] join_json_string;
}

char* createJoinMessage(std::string lobby_name) {
    StaticJsonDocument<200> join_json;
    size_t length = 200;
    char* join_json_string = new char[length];
    join_json["topic"] = "light_switch:lobby";
    join_json["event"] = "phx_join";
    JsonObject payload = join_json.createNestedObject("payload");
    join_json["ref"] = 0;
    serializeJson(join_json, join_json_string, length);
    join_json.clear();
    return join_json_string;
}

void handleWebsocketEvents() {
    String payload;
    if (tcpClient.connected()) {
    
    webSocketClient.getData(payload);
    if (payload.length() > 10) {
        StaticJsonDocument<512> response_json;
        DeserializationError error;
        error = deserializeJson(response_json, payload);
        if (error) {
            Serial.print(F("[WSc] deserializeJson() failed: "));
            Serial.println(error.c_str());
            // Something went out of sync
            // Clear the websocket buffer
            do
            {
               webSocketClient.getData(payload);
            } while (payload.length() > 0);
            return;
        }
        const char* str = response_json["topic"];
        if (response_json["event"] == "set_light") {
            int lightVal = response_json["payload"]["value"];
            Serial.printf("[WSc] Setting light to %i", lightVal);
            Serial.println();
            setDimmer(lightDimmer, lightVal);
        }
        response_json.clear();
    }
    
  } else {
    //Serial.println("Client disconnected.");
  }
}

// void webSocketEvent(WStype_t type, uint8_t* payload, size_t length) {
//     if (type == WStype_DISCONNECTED) {
//         Serial.printf("[WSc] Disconnected!");
//         Serial.println();
//     } else if (type == WStype_CONNECTED) {
//         Serial.printf("[WSc] Connected to url: %s", payload);
//         Serial.println();

//         // send message to server when Connected
//         char* join_json_string = getJoinMessage("light_switch:lobby");
//         webSocket.sendTXT(join_json_string);
//         delete[] join_json_string;
//     } else if (type == WStype_TEXT) {
//         StaticJsonDocument<512> response_json;
//         DeserializationError error;
//         error = deserializeJson(response_json, payload);
//         if (error) {
//             Serial.print(F("deserializeJson() failed: "));
//             Serial.println(error.c_str());
//             return;
//         }
//         const char* str = response_json["topic"];
//         if (response_json["event"] == "set_light") {
//             int lightVal = response_json["payload"]["value"];
//             Serial.printf("Setting light to %i", lightVal);
//             Serial.println();
//             setDimmer(lightDimmer, lightVal);
//         }
//         response_json.clear();
//     }
// }

void pingPhoenixChannel() {
    if (tcpClient.connected()) {
    Serial.println("[WSc] Ping Server");
    StaticJsonDocument<200> ping_json;
    size_t length = 200;
    char* ping_json_string = new char[length];
    ping_json["topic"] = "phoenix";
    ping_json["event"] = "heartbeat";
    JsonObject payload = ping_json.createNestedObject("payload");
    ping_json["ref"] = 0;
    serializeJson(ping_json, ping_json_string, length);
    ping_json.clear();
    webSocketClient.sendData(ping_json_string);
    }
    else {}
}