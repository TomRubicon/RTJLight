#pragma once
#include <WebSocketsClient.h>

WebSocketsClient webSocket;
unsigned long    messageInterval =    30;
bool             wsConnected     = false;

void hexDump(const void *mem, uint32_t len, uint8_t cols = 16) {
  const uint8_t* src = (const uint8_t*) mem;
  Serial.printf("\n[HEXDUMP] Address: 0x%08x len: 0x%X (%d)", (ptrdiff_t)src, len, len);

  for(uint32_t i = 0; i <  len; i++) {
    if(i % cols == 0) {
      Serial.printf("\n[0x%08X] 0x%08X: ", (ptrdiff_t)src, i);
    }
    Serial.printf("%02x ", *src);
    src++;
  }
  Serial.print("\n");
}

void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {
  if (type == WStype_DISCONNECTED) {
    Serial.println("[WEBSOCKET] Disconnected!");
    wsConnected = false;

  } else if (type == WStype_CONNECTED) {
    Serial.printf("[WEBSOCKET] Connected to URL: %s\n", payload);
    wsConnected = true;

    Serial.println("[WEBSOCKET] SENT: Connected");
    //webSocket.sendTXT("Connected");

  } else if (type == WStype_TEXT) {
    //Serial.printf("[WEBSOCKET] Response: %s\n", payload); 
    int data = atoi((char *)payload); 
    int datamap = map(data, 0, 6000, 0 ,255);
    //Serial.println(datamap);
    // Serial.println("Sending data");
    fill_palette(g_LEDs, NUM_LEDS, datamap, datamap / NUM_LEDS, yankeeBrave, 100, LINEARBLEND);

    FastLED.show();

  } else if (type == WStype_BIN) {
    Serial.printf("[WEBSOCKET] get binary length: %u\n", length);
    hexDump(payload, length);

  } else if (type == WStype_PING) {
    Serial.println("[WEBSOCKET] Ping!");

  } else if (type == WStype_PONG) {
    Serial.println("[WEBSOCKET] Pong!");

  }

}

void initWebSocket() {
  webSocket.begin("192.168.0.51", WS_PORT, "/");
  webSocket.onEvent(webSocketEvent);
  webSocket.setReconnectInterval(5000);
  Serial.println("[WEBSOCKET] Websocket trying to connect");
}
