#include <ESP8266WiFi.h>
#include <ESP8266WiFiMesh.h>
#include <WebSocketsServer.h>
#include <Hash.h>

unsigned int request_i = 0;
unsigned int response_i = 0;


const char* Data="Hello";


WebSocketsServer webSocket = WebSocketsServer(81);

#define USB_SERIAL Serial

String manageRequest(String request);

/* Create the mesh node object */
ESP8266WiFiMesh mesh_node = ESP8266WiFiMesh(ESP.getChipId(), manageRequest);


/**
 * Callback for when other nodes send you data
 *
 * @request The string received from another node in the mesh
 * @returns The string to send back to the other node
 */
String manageRequest(String request)
{
  /* Print out received message */
  Serial.print("received: ");
  Serial.println(request);

  /* return a string to send back */
  char response[60];
  sprintf(response, Data, response_i++, ESP.getChipId());
  return response;
}

void setup()
{
  Serial.begin(115200);
  delay(10);

  Serial.println();
  Serial.println();
  Serial.println("Setting up mesh node...");

  /* Initialise the mesh node */
  Serial.println(ESP.getChipId());
  mesh_node.begin();
  USB_SERIAL.setDebugOutput(true);

    USB_SERIAL.println();
    USB_SERIAL.println();
    USB_SERIAL.println();

    for(uint8_t t = 4; t > 0; t--) {
        USB_SERIAL.printf("[SETUP] BOOT WAIT %d...\n", t);
        USB_SERIAL.flush();
        delay(1000);
    }
    webSocket.begin();
    webSocket.onEvent(webSocketEvent);
}

void loop()
{
  /* Accept any incoming connections */
  mesh_node.acceptRequest();

  /* Scan for other nodes and send them a message */
  char request[60];
  sprintf(request, "Hello world request #%d from Mesh_Node%d.", request_i++, ESP.getChipId());
  mesh_node.attemptScan(request);
  webSocket.loop();
  delay(1000);
}
void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {

    switch(type) {
        case WStype_DISCONNECTED:
            USB_SERIAL.printf("[%u] Disconnected!\n", num);
            break;
        case WStype_CONNECTED:
            {
                IPAddress ip = webSocket.remoteIP(num);
                USB_SERIAL.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);
        
        // send message to client
        webSocket.sendTXT(num, "Connected");
            }
            break;
        case WStype_TEXT:
           //USE_SERIAL.printf("[%u] get Text: %s\n", num, payload);
            Data=(char *)payload;
           // USB_SERIAL.print(num);
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

