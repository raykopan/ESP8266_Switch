/* Rayko Panteleev
   For use with ESP8266 Switch Android App */
   
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <TinyUPnP.h> // by Ofek Pearl.

#define STASSID "NetworkID" // Replace with your wireless network ID
#define STAPSK  "password" // Replace with your password

#define LISTEN_PORT 5000  // http://IP:LISTEN_PORT 
#define LEASE_DURATION 360000  // seconds
#define FRIENDLY_NAME "ESP8266Switch"  // this name will appear in your router port forwarding section

//Outputs for control the switches
const int switch1 = 5;   // Pin D1 NodeMCU
const int switch2 = 4;  // Pin D2 NodeMCU 
const int switch3 = 0;   // Pin D3 NodeMCU 
const int switch4 = 2;  // Pin D4 NodeMCU 

const char* ssid = STASSID;
const char* password = STAPSK;

TinyUPnP tinyUPnP(20000);  // -1 means blocking, preferably, use a timeout value (ms)

ESP8266WebServer server(LISTEN_PORT); // 

 void connectWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  WiFi.setAutoReconnect(true);
  WiFi.persistent(true);
  }

 void handleRoot() {  
  server.send(200, "text/plain", "Hello from ESP8266 Switch!"); 
}

 void handleNotFound() {   
   server.send(404, "text/plain", "Not Found!");
 }

 void switch1On(){
    Serial.println("Switch 1 ON");
    server.send(200, "text/plain", "Switch 1 ON!");
    digitalWrite(switch1, LOW); // Active is LOW
             }
             
 void switch1Off() {
    Serial.println("Switch 1 OFF");
    server.send(200, "text/plain", "Switch 1 OFF!");
    digitalWrite(switch1, HIGH);
             }

 void switch2On() {
    Serial.println("Switch 2 ON");
    server.send(200, "text/plain", "Switch 2 ON!");
    digitalWrite(switch2, LOW); // Active is LOW
             }

 void switch2Off() {
    Serial.println("Switch 2 OFF");
    server.send(200, "text/plain", "Switch 2 OFF!");
    digitalWrite(switch2, HIGH);
             }  

void switch3On() {
    Serial.println("Switch 3 ON");
    server.send(200, "text/plain", "Switch 3 ON!");
    digitalWrite(switch3, LOW); // Active is LOW
             }

 void switch3Off() {
    Serial.println("Switch 3 OFF");
    server.send(200, "text/plain", "Switch 3 OFF!");
    digitalWrite(switch3, HIGH);
             } 

  void switch4On() {
    Serial.println("Switch 4 ON");
    server.send(200, "text/plain", "Switch 4 ON!");
    digitalWrite(switch4, LOW); // Active is LOW
             } 

   void switch4Off() {
    Serial.println("Switch 4 OFF");
    server.send(200, "text/plain", "Switch 4 OFF!");
    digitalWrite(switch4, HIGH);
             } 

void setup(void) {
  
  // Initialize the outputs  
  pinMode(switch1, OUTPUT);
  pinMode(switch2, OUTPUT);
  pinMode(switch3, OUTPUT);
  pinMode(switch4, OUTPUT);
  // Set outputs to HIGH
  digitalWrite(switch1, HIGH); 
  digitalWrite(switch2, HIGH);
  digitalWrite(switch3, HIGH);
  digitalWrite(switch4, HIGH);
      
  Serial.begin(115200);
  connectWiFi(); 

  server.on("/1/on", switch1On); 
  server.on("/1/off", switch1Off);               
  server.on("/2/on", switch2On);
  server.on("/2/off", switch2Off); 
  server.on("/3/on", switch3On); 
  server.on("/3/off", switch3Off);               
  server.on("/4/on", switch4On);
  server.on("/4/off", switch4Off); 
  
  server.on("/", handleRoot);
  
  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started");

  portMappingResult portMappingAdded;
  tinyUPnP.addPortMappingConfig(WiFi.localIP(), LISTEN_PORT, RULE_PROTOCOL_TCP, LEASE_DURATION, FRIENDLY_NAME);
  while (portMappingAdded != SUCCESS && portMappingAdded != ALREADY_MAPPED) {
    portMappingAdded = tinyUPnP.commitPortMappings();
    Serial.println("");
  
    if (portMappingAdded != SUCCESS && portMappingAdded != ALREADY_MAPPED) {
      // for debugging, you can see this in your router too under forwarding or UPnP
      tinyUPnP.printAllPortMappings();
      Serial.println(F("This was printed because adding the required port mapping failed"));
      delay(30000);  // 30 seconds before trying again
    }
  }
  
  Serial.println("UPnP done");
  
  // server
  if (MDNS.begin("esp8266")) {
    Serial.println(F("MDNS responder started"));
  }
}

void loop(void) {
  delay(5);
  tinyUPnP.updatePortMappings(600000, &connectWiFi);  // 10 minutes
  server.handleClient();
}
