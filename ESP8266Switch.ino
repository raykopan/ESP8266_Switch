/* Rayko Panteleev
   For use with ESP8266 Switch Android App */
   
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>

#define STASSID "SSID" // Replace with your wireless network ID
#define STAPSK  "password" // Replace with your password

//Outputs for control the switches
const int switch1 = 5;   // Pin D1 NodeMCU
const int switch2 = 4;  // Pin D2 NodeMCU 
const int switch3 = 0;   // Pin D3 NodeMCU 
const int switch4 = 2;  // Pin D4 NodeMCU 

const char* ssid = STASSID;
const char* password = STAPSK;

ESP8266WebServer server(80); // Server port

 void handleRoot() {  
  server.send(200, "text/plain", "Hello from ESP8266 Switch!"); 
}

 void handleNotFound() {   
   server.send(404, "text/plain", "Not Found!");
 }

 void switch1On(){
    Serial.println("Switch 1 ON");
    server.send(200, "text/plain", "Switch 1 ON!");
    digitalWrite(switch1, HIGH);
             }
             
 void switch1Off() {
    Serial.println("Switch 1 OFF");
    server.send(200, "text/plain", "Switch 1 OFF!");
    digitalWrite(switch1, LOW);
             }

 void switch2On() {
    Serial.println("Switch 2 ON");
    server.send(200, "text/plain", "Switch 2 ON!");
    digitalWrite(switch2, HIGH);
             }

 void switch2Off() {
    Serial.println("Switch 2 OFF");
    server.send(200, "text/plain", "Switch 2 OFF!");
    digitalWrite(switch2, LOW);
             }  

void switch3On() {
    Serial.println("Switch 3 ON");
    server.send(200, "text/plain", "Switch 3 ON!");
    digitalWrite(switch3, HIGH);
             }

 void switch3Off() {
    Serial.println("Switch 3 OFF");
    server.send(200, "text/plain", "Switch 3 OFF!");
    digitalWrite(switch3, LOW);
             } 

  void switch4On() {
    Serial.println("Switch 4 ON");
    server.send(200, "text/plain", "Switch 4 ON!");
    digitalWrite(switch4, HIGH);
             } 

   void switch4Off() {
    Serial.println("Switch 4 OFF");
    server.send(200, "text/plain", "Switch 4 OFF!");
    digitalWrite(switch4, LOW);
             } 

void setup(void) {
  
  // Initialize the outputs  
  pinMode(switch1, OUTPUT);
  pinMode(switch2, OUTPUT);
  pinMode(switch3, OUTPUT);
  pinMode(switch4, OUTPUT);
  // Set outputs to LOW
  digitalWrite(switch1, LOW);
  digitalWrite(switch2, LOW);
  digitalWrite(switch3, LOW);
  digitalWrite(switch4, LOW);
      
  Serial.begin(115200);
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
}

void loop(void) {
  server.handleClient();
}
