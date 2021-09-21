/*
 * WebSocketClient.ino
 *
 *  Created on: 24.05.2015
 *
 */

#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <WebSocketsServer.h>
#include <WebSocketsClient.h>

#include <Hash.h>

String Data="message";

const char* ssid = "SEEKR1";
const char* password = "12345678";

IPAddress Ip(192, 168, 1, 1);
IPAddress NMask(255, 255, 255, 0);

ESP8266WiFiMulti WiFiMulti;
WebSocketsClient webSocket;

WebSocketsServer webSocketServer = WebSocketsServer(81);
 

#define USB_SERIAL Serial

void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {

  switch(type) {
    case WStype_DISCONNECTED:
      USB_SERIAL.printf("[WSc] Disconnected!\n");
      break;
    case WStype_CONNECTED: {
      USB_SERIAL.printf("[WSc] Connected to url: %s\n", payload);

      // send message to server when Connected
     // webSocket.sendTXT("Connected");
    }
      break;
    case WStype_TEXT:
      USB_SERIAL.printf("[WSc] get text: %s\n", payload);

      // send message to server
     // webSocket.sendTXT("message here");
      break;
    case WStype_BIN:
      USB_SERIAL.printf("[WSc] get binary length: %u\n", length);
      hexdump(payload, length);

      // send data to server
      // webSocket.sendBIN(payload, length);
      break;
  }

}

void setup() {
  // USE_SERIAL.begin(921600);
  USB_SERIAL.begin(115200);

  WiFi.softAPConfig(Ip, Ip, NMask);
  WiFi.softAP(ssid, password); //begin WiFi connection
 if (!WiFi.softAP( ssid, password))
  {
   Serial.println("WiFi.softAP failed.(Password too short?)");
   return;
  }
  IPAddress myIP = WiFi.softAPIP();
  Serial.println();
  Serial.print("AP IP address: ");
  Serial.println(myIP);
  Serial.println("");
 

  //Serial.setDebugOutput(true);
  USB_SERIAL.setDebugOutput(true);

  USB_SERIAL.println();
  USB_SERIAL.println();
  USB_SERIAL.println();

  for(uint8_t t = 4; t > 0; t--) {
    USB_SERIAL.printf("[SETUP] BOOT WAIT %d...\n", t);
    USB_SERIAL.flush();
    delay(1000);
  }

  WiFiMulti.addAP("SEEKR3", "12345678");

  //WiFi.disconnect();
  while(WiFiMulti.run() != WL_CONNECTED) {
    delay(100);
  }

  // server address, port and URL
  webSocket.begin("192.168.3.1", 81, "/");

  // event handler
  webSocket.onEvent(webSocketEvent);
  webSocketServer.begin();
  webSocketServer.onEvent(webSocketServerEvent);
  // use HTTP Basic Authorization this is optional remove if not needed
  //webSocket.setAuthorization("user", "Password");

  // try ever 5000 again if connection has failed
  webSocket.setReconnectInterval(5000);
  Serial.println("Web server started!");

}

void loop() {
  webSocket.loop();
  webSocketServer.loop();
}
void webSocketServerEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {

    switch(type) {
        case WStype_DISCONNECTED:
            USB_SERIAL.printf("[%u] Disconnected!\n", num);
            break;
        case WStype_CONNECTED:
            {
                IPAddress ip = webSocketServer.remoteIP(num);
                USB_SERIAL.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);
        
        // send message to client
        //webSocketServer.sendTXT("Connected");
            }
            break;
        case WStype_TEXT:
           USB_SERIAL.printf("[%u] get Text: %s\n", num, payload);
            Data=String(((char*)payload));
           // USB_SERIAL.print(num);
            webSocket.sendTXT(Data);
            webSocket.loop();
            USB_SERIAL.println(Data);
            // send message to client
            // webSocket.sendTXT(num, "message here");

            // send data to all connected clients
            // webSocket.broadcastTXT("message here");
            break;
        case WStype_BIN:
            USB_SERIAL.printf("[%u] get binary length: %u\n", num, length);
            hexdump(payload, length);

            // send message to client
            // webSocket.sendBIN(num, payload, length);
            break;
    }

}
