#include "websocket.h"

#include <string.h>

WiFiClient tcpClient;
WebSocketClient webSocketClient;

char path[] = "/socket/websocket";
char host[] = "192.168.1.71";

void connectWebsocket() {
    if (!tcpClient.connected()) {
        if (tcpClient.connect(host, 4000)) {
            Serial.println("[WSc] TCP client Connected");
        } else {
            Serial.println("[WSc] TCP client Connection failed");
            return;
        }
    }

    // Handshake with the server
    webSocketClient.path = path;
    webSocketClient.host = host;
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

// WEBSOCKET TASK
void handleWebsocketEvents(void* p) {
    String payload;
    while (true) {
        if (xSemaphoreTake(wbSemaphore, (TickType_t)5000) == pdTRUE) {
            // Serial.println("[WSc] Handling Websocket");
            // Serial.println(ESP.getFreeHeap());
            payload = "";
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
                        do {
                            payload = "";
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

            } else
                connectWebsocket();
        }
        xSemaphoreGive(wbSemaphore);
        vTaskDelay(1);
    }
    vTaskDelete(NULL);
}

// PING PING
void pingPhoenixChannel() {
    while (true) {
        if (xSemaphoreTake(wbSemaphore, (TickType_t)5000) == pdTRUE) {
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
            } else {
                connectWebsocket();
            }
            xSemaphoreGive(wbSemaphore);
            break;
        }
    }
}