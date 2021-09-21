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
String DataString="";
String Info;

WiFiServer server(80);


const char* ssid = "SEEKR3";
const char* password = "12345678";

IPAddress Ip(192, 168, 3, 1);
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
  pinMode(D7,OUTPUT);
  pinMode(D8,OUTPUT);
  digitalWrite(D8,HIGH);

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

  WiFiMulti.addAP("SEEKR2", "12345678");

  //WiFi.disconnect();
  while(WiFiMulti.run() != WL_CONNECTED) {
    delay(100);
  }

  // server address, port and URL
  webSocket.begin("192.168.2.1", 81, "/");

  // event handler
  webSocket.onEvent(webSocketEvent);
  webSocketServer.begin();
  webSocketServer.onEvent(webSocketServerEvent);
  // use HTTP Basic Authorization this is optional remove if not needed
  //webSocket.setAuthorization("user", "Password");

  // try ever 5000 again if connection has failed
  webSocket.setReconnectInterval(5000);
  Serial.println("Web server started!");
  ConnectSetup();
}

void loop() {
  HTTP();
  webSocket.loop();
  webSocketServer.loop();
  webSocket.sendTXT(Info);
  
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
           HTTP();
           digitalWrite(D7,HIGH);
            if(num<2)
              Data=String(((char*)payload));
            else
              Info=String(((char*)payload));  
              
              
              DataString=Data;
              HTTP();
           // USB_SERIAL.print(num);
            if(Info!=Data)
              webSocket.sendTXT(Info);
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
void ConnectSetup()
{
  Serial.println("scan start");

  // WiFi.scanNetworks will return the number of networks found
  int n = WiFi.scanNetworks();
  Serial.println("scan done");
  if (n == 0)
    Serial.println("no networks found");
  else
  {
    Serial.print(n);
    Serial.println(" networks found");
    DataString="";
    for (int i = 0; i < n; ++i)
    {
      // Print SSID and RSSI for each network found
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.print(WiFi.SSID(i));
      String ssid =WiFi.SSID(i).substring(0,5);
     
      if(ssid == "SEEKR")
      {
        DataString=DataString+WiFi.SSID(i)+WiFi.RSSI(i);
        
         

      }
      Serial.print(" (");
      Serial.print(WiFi.RSSI(i));
      Serial.print(")");
      Serial.println((WiFi.encryptionType(i) == ENC_TYPE_NONE)?" ":"*");
      Serial.println(ssid);
      delay(10);
    }
    Info=DataString;
  }
  Serial.println("");
}
void HTTP()
{
  // Check if a client has connected
      WiFiClient client = server.available();
      if (!client) {
        return;
      }
     
      // Wait until the client sends some data
      Serial.println("new client");
      while(!client.available()){
        delay(1);
      }
      
     
      // Read the first line of the request
      String request = client.readStringUntil(';');
      Serial.println(request);

      client.flush();
     


   //String responseBodyString = "{\"contacts\": [{\"id\": \"c200\",\"name\": \"Ravi Tamada\",\"email\": \"ravi@gmail.com\",\"address\": \"xx-xx-xxxx,x - street, x - country\",\"gender\" : \"male\",\"phone\": {\"mobile\": \"+91 0000000000\",\"home\": \"00 000000\",\"office\": \"00 000000\"}},{\"id\": \"c201\",\"name\": \"Johnny Depp\",\"email\": \"johnny_depp@gmail.com\",\"address\": \"xx-xx-xxxx,x - street, x - country\",\"gender\" : \"male\",\"phone\": {\"mobile\": \"+91 0000000000\",\"home\": \"00 000000\",\"office\": \"00 000000\"}},{\"id\": \"c202\",\"name\": \"Leonardo Dicaprio\",\"email\": \"leonardo_dicaprio@gmail.com\",\"address\": \"xx-xx-xxxx,x - street, x - country\",\"gender\" : \"male\",\"phone\": {\"mobile\": \"+91 0000000000\",\"home\": \"00 000000\",\"office\": \"00 000000\"}},{\"id\": \"c203\",\"name\": \"John Wayne\",\"email\": \"john_wayne@gmail.com\",\"address\": \"xx-xx-xxxx,x - street, x - country\",\"gender\" : \"male\",\"phone\": {\"mobile\": \"+91 0000000000\",\"home\": \"00 000000\",\"office\": \"00 000000\"}},{\"id\": \"c204\",\"name\": \"Angelina Jolie\",\"email\": \"angelina_jolie@gmail.com\",\"address\": \"xx-xx-xxxx,x - street, x - country\",\"gender\" : \"female\",\"phone\": {\"mobile\": \"+91 0000000000\",\"home\": \"00 000000\",\"office\": \"00 000000\"}},{\"id\": \"c205\",\"name\": \"Dido\",\"email\": \"dido@gmail.com\",\"address\": \"xx-xx-xxxx,x - street, x - country\",\"gender\" : \"female\",\"phone\": {\"mobile\": \"+91 0000000000\",\"home\": \"00 000000\",\"office\": \"00 000000\"}},{\"id\": \"c206\",\"name\": \"Adele\",\"email\": \"adele@gmail.com\",\"address\": \"xx-xx-xxxx,x - street, x - country\",\"gender\" : \"female\",\"phone\": {\"mobile\": \"+91 0000000000\",\"home\": \"00 000000\",\"office\": \"00 000000\"}},{\"id\": \"c207\",\"name\": \"Hugh Jackman\",\"email\": \"hugh_jackman@gmail.com\",\"address\": \"xx-xx-xxxx,x - street, x - country\",\"gender\" : \"male\",\"phone\": {\"mobile\": \"+91 0000000000\",\"home\": \"00 000000\",\"office\": \"00 000000\"}},{\"id\": \"c208\",\"name\": \"Will Smith\",\"email\": \"will_smith@gmail.com\",\"address\": \"xx-xx-xxxx,x - street, x - country\",\"gender\" : \"male\",\"phone\": {\"mobile\": \"+91 0000000000\",\"home\": \"00 000000\",\"office\": \"00 000000\"}},{\"id\": \"c209\",\"name\": \"Clint Eastwood\",\"email\": \"clint_eastwood@gmail.com\",\"address\": \"xx-xx-xxxx,x - street, x - country\",\"gender\" : \"male\",\"phone\": {\"mobile\": \"+91 0000000000\",\"home\": \"00 000000\",\"office\": \"00 000000\"}},{\"id\": \"c2010\",\"name\": \"Barack Obama\",\"email\": \"barack_obama@gmail.com\",\"address\": \"xx-xx-xxxx,x - street, x - country\",\"gender\" : \"male\",\"phone\": {\"mobile\": \"+91 0000000000\",\"home\": \"00 000000\",\"office\": \"00 000000\"}},{\"id\": \"c2011\",\"name\": \"Kate Winslet\",\"email\": \"kate_winslet@gmail.com\",\"address\": \"xx-xx-xxxx,x - street, x - country\",\"gender\" : \"female\",\"phone\": {\"mobile\": \"+91 0000000000\",\"home\": \"00 000000\",\"office\": \"00 000000\"}},{\"id\": \"c2012\",\"name\": \"Eminem\",\"email\": \"eminem@gmail.com\",\"address\": \"xx-xx-xxxx,x - street, x - country\",\"gender\" : \"male\",\"phone\": {\"mobile\": \"+91 0000000000\",\"home\": \"00 000000\",\"office\": \"00 000000\"}}]}"; 

      // Return the response
      if(WiFi.status() != WL_CONNECTED)
      {
        client.println("HTTP/1.1 200 OK");
        client.println("Content-Type: text/html");
        client.println(""); //  do not forget this one
        client.println(DataString);

      }
      else
      {
        client.println("HTTP/1.1 200 OK");
        client.println("Content-Type: text/html");
        client.println(""); //  do not forget this one
        client.println("Connected");
        
      }
      delay(1);
      Serial.println("Client disonnected");
      Serial.println("");
}

