// Rayko Panteleev
// ESP8266 Switch with server NTP clock and scheduler
  
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <EEPROM.h> 
#include <NTPClient.h>   // Include NTPClient library
#include <Timezone.h>    // https://github.com/JChristensen/Timezone
#include <ESPAsyncTCP.h> // https://github.com/me-no-dev/AsyncTCP
#include <ESPAsyncWebServer.h> // https://github.com/me-no-dev/ESPAsyncWebServer
#include <ESP8266mDNS.h>
#include <TinyUPnP.h> // by Ofek Pearl.

#define LISTEN_PORT 5000  // http://IP:LISTEN_PORT 
#define LEASE_DURATION 360000  // seconds
#define FRIENDLY_NAME "ESP8266Switch"  // this name will appear in Your router port forwarding section

TinyUPnP tinyUPnP(20000);  // -1 means blocking, preferably, use a timeout value (ms)
AsyncWebServer server(LISTEN_PORT); 
 
const char *ssid     = "YOUR_SSID"; //Replace with your Network ID
const char *password = "your_password"; //Replace with your Password

const int switch1 = 14;  // Pin D5 NodeMCU Active LOW 
const int switch2 = 12;  // Pin D6 NodeMCU Active LOW
const int switch3 = 13;  // Pin D7 NodeMCU Active LOW 
const int switch4 = 15;  // Pin D8 NodeMCU Active LOW 

const int button1 = 5;  // Pin D1 NodeMCU  Pull resistor not needed, button should be connected between input pin and GND.  
const int button2 = 4;  // Pin D2 NodeMCU  Pull resistor not needed, button should be connected between input pin and GND.
const int button3 = 0;  // Pin D3 NodeMCU  Pull resistor not needed, button should be connected between input pin and GND. Flash button is connected here.
const int button4 = 2;  // Pin D4 NodeMCU  Pull resistor not needed, button should be connected between input pin and GND. Build in Led is connected here.

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "time.nist.gov", 0, 60000); 

// Time Change Rules
TimeChangeRule CEST = {"CEST", Last, Sun, Mar, 3, 120};     // 120 EU Summer Time (UTC +3 Hours);  -240 (UTC -4 Hours)
TimeChangeRule CET = {"CET ", Last, Sun, Oct, 4, 60};       // 60 EU Standard Time (UTC +2 Hours); -300 (UTC -5 Hours)
Timezone CE(CEST, CET);

byte second_, minute_, hour_, day_, month_, last_second = 60;
int hours, minutes, year_;
String minutesString, status1, status2, status3, status4;
int startHour, startHour1, startHour2, startHour3, startHour4, startMin, startMin1, startMin2, startMin3, startMin4;
int stopHour, stopHour1, stopHour2, stopHour3, stopHour4, stopMin, stopMin1, stopMin2, stopMin3, stopMin4;
String startHourString = "01", startMinString = "00", stopHourString = "23", stopMinString = "00";
String startHourString1 = "01", startMinString1 = "00", stopHourString1 = "23", stopMinString1 = "00";
String startHourString2 = "01", startMinString2 = "00", stopHourString2 = "23", stopMinString2 = "00";
String startHourString3 = "01", startMinString3 = "00", stopHourString3 = "23", stopMinString3 = "00";
String startHourString4 = "01", startMinString4 = "00", stopHourString4 = "23", stopMinString4 = "00";
String href, href1 = "/1/", href2 = "/2/", href3 = "/3/", href4 = "/4/", result;
int output1State, output2State, output3State, output4State; // the current state of the output pin
int buttonState1,buttonState2,buttonState3,buttonState4;  // last flickerable button state
int lastButtonState1 = HIGH, lastButtonState2 = HIGH, lastButtonState3 = HIGH, lastButtonState4 = HIGH;   // the previous steady state from the input pin
unsigned long lastDebounceTime1 = 0, lastDebounceTime2 = 0, lastDebounceTime3 = 0, lastDebounceTime4 = 0;  // the last time the output pin was toggled
unsigned long debounceDelay1 = 50, debounceDelay2 = 50, debounceDelay3 = 50, debounceDelay4 = 50;  // the debounce time; increase if the output flickers
boolean stopped1 = true, stopped2 = true, stopped3 = true, stopped4 = true, stopped = true;
String schedulerStatus1 = "OFF", schedulerStatus2 = "OFF", schedulerStatus3 = "OFF", schedulerStatus4 = "OFF", schedulerStatus = "OFF";
int delayPeriod = 5;

 // HTML web page
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html><html><head>
<meta name="viewport" content="width=device-width, initial-scale=1.0, user-scalable=no">
<title>ESP8266 Switch</title>
<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}
body{margin-top: 50px;} h1 {color: #000000; margin: 50px auto 30px;}
p {font-size: 24px; text-align: center; color: #000000; margin-bottom: 10px; margin-right: 37px; margin-left: 37px; display: inline;}
a {font-size: 20px; color: #000000; margin-right: 15px; margin-left: 15px;}
button { background-color: #a81a1a; border: none;  border-radius:10px; color: white; padding: 10px 10px; text-align: center; text-decoration: none; display: inline-block; font-size: 22px;}
</style></head><body> 
<h1>ESP8266 Switch</h1>
<p id="displayTime">Server time is -:-- h</p><br><br><br>
<a><button id="sw1">-----</button></a>
<a><button id="sw2">-----</button></a><br>
<p>1</p><p>2</p><br><br>       
<a><button id="sw3">-----</button></a>
<a><button id="sw4">-----</button><br></a>
<p>3</p><p>4</p><br><br>
<a id="scheduler1" href="/1/scheduler">Switch 1 Scheduler<br></a><br>
<a id="scheduler2" href="/2/scheduler">Switch 2 Scheduler<br></a><br>
<a id="scheduler3" href="/3/scheduler">Switch 3 Scheduler<br></a><br>
<a id="scheduler4" href="/4/scheduler">Switch 4 Scheduler<br></a>
<script>
window.onload = onLoad;
async function onLoad() {

const sw1 = document.getElementById('sw1');
sw1.addEventListener('click', changeState1);
const sw2 = document.getElementById('sw2');
sw2.addEventListener('click', changeState2);
const sw3 = document.getElementById('sw3');
sw3.addEventListener('click', changeState3);
const sw4 = document.getElementById('sw4');
sw4.addEventListener('click', changeState4);
const displayTime = document.getElementById('displayTime');
const scheduler1 = document.getElementById('scheduler1');
const scheduler2 = document.getElementById('scheduler2');
const scheduler3 = document.getElementById('scheduler3');
const scheduler4 = document.getElementById('scheduler4');

await getButtonsStates();

setInterval(async function ( ){
  await getButtonsStates();
}, 2000); // every 2 seconds

async function getButtonsStates() {
try { 
 const [result1, result2, result3, result4] = await Promise.all([fetch('/1/').then(response => response.text()), 
      fetch('/2/').then(response => response.text()), fetch('/3/').then(response => response.text()), fetch('/4/').then(response => response.text())]);
if(result1.includes("ON")){
   sw1.innerText = "ON";
   sw1.style.backgroundColor = "green";   
   } 
else if(result1.includes("OFF")){
   sw1.innerText = "OFF";
   sw1.style.backgroundColor = "#a81a1a";   
  }
if(result2.includes("ON")){
   sw2.innerText = "ON";
   sw2.style.backgroundColor = "green";   
   } 
else if(result2.includes("OFF")){
   sw2.innerText = "OFF";
   sw2.style.backgroundColor = "#a81a1a";    
   }
if(result3.includes("ON")){
   sw3.innerText = "ON";
   sw3.style.backgroundColor = "green";
   } 
else if(result3.includes("OFF")){
   sw3.innerText = "OFF";
   sw3.style.backgroundColor = "#a81a1a";
   }
if(result4.includes("ON")){
   sw4.innerText = "ON";
   sw4.style.backgroundColor = "green";   
   } 
else if(result4.includes("OFF")){
   sw4.innerText = "OFF";
   sw4.style.backgroundColor = "#a81a1a";   
   }
if(result1.includes("Stopped=0")){
  scheduler1.style.color = "green";  
   } 
else if(result1.includes("Stopped=1")){   
   scheduler1.style.color = "black";   
   }
if(result2.includes("Stopped=0")){
   scheduler2.style.color = "green";    
   } 
else if(result2.includes("Stopped=1")){
   scheduler2.style.color = "black";   
   }
   if(result3.includes("Stopped=0")){
   scheduler3.style.color = "green";     
   } 
else if(result3.includes("Stopped=1")){
   scheduler3.style.color = "black";     
   }
   if(result4.includes("Stopped=0")){
   scheduler4.style.color = "green";    
   } 
else if(result4.includes("Stopped=1")){
   scheduler4.style.color = "black";    
   }
 const timeResponse = await fetch('/time');
 timeResult = await timeResponse.text();
 displayTime.innerText = `Server time is ${timeResult} h`;
}catch (err){
   console.log(err);
  }
}
async function changeState1(ev) {
   ev.preventDefault();                  
   let url = ""; 
   if(sw1.innerText.includes("ON")){
   url = "/1/off";
   }else if(sw1.innerText.includes("OFF")){
   url = "/1/on";
   }    
try {              
 const response = await fetch(url);
 const result = await response.text();
if(result.includes("ON") && result.includes("Stopped=1")){
   sw1.innerText = "ON";
   sw1.style.backgroundColor = "green";
   }
   else if(result.includes("OFF") && result.includes("Stopped=1")){
   sw1.innerText = "OFF";
   sw1.style.backgroundColor = "#a81a1a";
   }
else if(result.includes("ON") || result.includes("OFF") && result.includes("Stopped=0")){
    alert("Scheduler is ON!");
   }
} catch { 
  }
}
async function changeState2(ev) {
   ev.preventDefault();                  
   let url = ""; 
 if(sw2.innerText.includes("ON")){
   url = "/2/off";
 }else if(sw2.innerText.includes("OFF")){
   url = "/2/on";
 } 
try {              
 const response = await fetch(url);
 const result = await response.text();
if(result.includes("ON") && result.includes("Stopped=1")){
   sw2.innerText = "ON";
   sw2.style.backgroundColor = "green";
   }
   else if(result.includes("OFF") && result.includes("Stopped=1")){
   sw2.innerText = "OFF";
   sw2.style.backgroundColor = "#a81a1a";
   }
else if(result.includes("ON") || result.includes("OFF") && result.includes("Stopped=0")){
    alert("Scheduler is ON!");
   }
} catch { 
  }
}
async function changeState3(ev) {
   ev.preventDefault();                  
   let url = ""; 
   if(sw3.innerText.includes("ON")){
   url = "/3/off";
}else if(sw3.innerText.includes("OFF")){
   url = "/3/on";
   }     
try {              
   const response = await fetch(url);
   const result = await response.text();
if(result.includes("ON") && result.includes("Stopped=1")){
   sw3.innerText = "ON";
   sw3.style.backgroundColor = "green";}
else if(result.includes("OFF") && result.includes("Stopped=1")){
   sw3.innerText = "OFF";
   sw3.style.backgroundColor = "#a81a1a";
   }else if(result.includes("ON") || result.includes("OFF") && result.includes("Stopped=0")){
    alert("Scheduler is ON!");
   }
} catch { 
  }
}
async function changeState4(ev) {
   ev.preventDefault();                  
   let url = ""; 
   if(sw4.innerText.includes("ON")){
   url = "/4/off";
 }else if(sw4.innerText.includes("OFF")){
   url = "/4/on";
   }       
try {              
   const response = await fetch(url);
   const result = await response.text();
if(result.includes("ON") && result.includes("Stopped=1")){
   sw4.innerText = "ON";
   sw4.style.backgroundColor = "green";
   }
else if(result.includes("OFF") && result.includes("Stopped=1")){
   sw4.innerText = "OFF";
   sw4.style.backgroundColor = "#a81a1a";
   }else if(result.includes("ON") || result.includes("OFF") && result.includes("Stopped=0")){
    alert("Scheduler is ON!");
   }
} catch { 
  }
 } 
}
</script></body></html>    
)rawliteral";

 // HTML web page
void schedulerSet_html(AsyncResponseStream *response){
response->print("<!DOCTYPE html><html>");
response->print("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">");
response->print("<meta id=\"meta\" http-equiv=\"refresh\" content=\"2; URL='"); // redirect after 2 seconds to Scheduler page if there is no javaScript 
response->print(href); // URL redirect.
response->print("scheduler'\">"); // URL redirect.
response->print("<title>ESP8266 Switch</title>");
response->print("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
response->print("body{margin-top: 50px;} h1 {color: #000000;margin: 50px auto 30px;}");
response->print("p {font-size: 24px;color: #000000;margin-bottom: 10px;}");
response->print("</style></head><body>");
response->print("<h1>ESP8266 Switch</h1>");
response->print("<p>Scheduler is ");
response->print(schedulerStatus);
response->print("!</p><script>window.setTimeout(function(){window.location.href='/'}, 1800);</script></body></html>"); // redirect after 2 seconds to Home page if there is javaScript 
}

void notFound(AsyncWebServerRequest *request) {
  request->send(404, "text/plain", "Not found");
}

void setScheduler(AsyncWebServerRequest *request){  

  startHourString = request->getParam("startHour")->value(); //this lets you access a query param
  startMinString = request->getParam("startMin")->value();
  stopHourString = request->getParam("stopHour")->value();
  stopMinString = request->getParam("stopMin")->value(); 
 
  startHour = startHourString.toInt();
  if(startHour < 0 || startHour > 23){
    startHour = 0;
    startHourString = "00";
  }  
  startMin = startMinString.toInt();
  if(startMin < 0 || startMin > 59){
    startMin = 0;
    startMinString = "00";
  }  
  stopHour = stopHourString.toInt();
  if(stopHour < 0 || stopHour > 23){
    stopHour = 0;
    stopHourString = "00";
  }  
  stopMin = stopMinString.toInt();  
  if(stopMin < 0 || stopMin > 59){
    stopMin = 0;
    stopMinString = "00";
  }    
  Serial.println("Scheduler is started!");
  Serial.print("Start Time ");
  Serial.print(startHour);
  Serial.print(":");
  Serial.println(startMinString);
  Serial.print("Stop Time ");
  Serial.print(stopHour);
  Serial.print(":");
  Serial.println(stopMinString);
  stopped = false;  
  schedulerStatus = "ON";  
}

 // HTML web page
void scheduler_html(AsyncResponseStream *response){

  response->print("<!DOCTYPE html><html>\n");
    response->print("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n");
    response->print("<meta http-equiv=\"refresh\" content=\"60\">\n");
    response->print("<title>ESP8266 Switch</title>\n");
    response->print("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}\n");
    response->print("body{margin-top: 50px;} h1 {color: #000000;margin: 50px auto 30px;}\n");
    response->print("p {font-size: 24px;color: #000000;margin-bottom: 10px;}\n");    
    response->print("</style>\n");
    response->print("</head><body>\n"); 
    response->print("<h1>ESP8266 Switch</h1>\n");
    response->print("<p>Server time is ");
    response->print(hours);
    response->print(":");
    response->print(minutesString);
    response->print(" h");
    response->print("</p>");
    response->print("<p>Scheduler is ");
    response->print(schedulerStatus);
    response->print("!</p>"); 
    response->print("<h2>Scheduler Settings</h2>\n");
    response->print("<form  action=\"");
    response->print(href);
    response->print("set\" method=\"get\">\n");
    response->print("<label for=\"aname\">Start time </label><input type=\"text\" name=\"startHour\" value=");
    response->print(startHourString);
    response->print(" maxlength=\"2\" size=\"1\"><label for=\"bname\">:</label><input type=\"text\" name=\"startMin\" value=");
    response->print(startMinString);
    response->print(" maxlength=\"2\" size=\"1\"><br><br>\n");
    response->print("<label for=\"cname\">Stop time </label><input type=\"text\" name=\"stopHour\" value=");
    response->print(stopHourString);
    response->print(" maxlength=\"2\" size=\"1\"><label for=\"dname\">:</label><input type=\"text\" name=\"stopMin\" value=");
    response->print(stopMinString);
    response->print(" maxlength=\"2\" size=\"1\"><br><br>\n");
    response->print("<input type=\"submit\" value=\"Start\"></form><br><a href=\"");
    response->print(href);
    response->print("stop\"><button>Stop</button></a>\n");         
    response->print("</body></html>\n");    
}

void switch1Status() {
    status1 = "Switch 1 ";
    String sw1 = "";
  if(digitalRead(switch1)==1){
       sw1 = "OFF!";
  }
  if(digitalRead(switch1)==0) {
       sw1 = "ON!";
  }
  status1 += sw1;
  status1 += " Stopped="+(String)stopped1; // Switch 1 OFF! Stopped=1    
 }

void switch2Status() {
    status2 = "Switch 2 ";
    String sw2 = "";
  if(digitalRead(switch2)==1){
       sw2 = "OFF!";
  }
  if(digitalRead(switch2)==0) {
       sw2 = "ON!";
  }
  status2 += sw2;
  status2 += " Stopped="+(String)stopped2;       
 }

void switch3Status() {
    status3 = "Switch 3 ";
    String sw3 = "";
  if(digitalRead(switch3)==1){
       sw3 = "OFF!";
  }
  if(digitalRead(switch3)==0) {
       sw3 = "ON!";
  }
  status3 += sw3;
  status3 += " Stopped="+(String)stopped3;      
 }

void switch4Status() {
    status4 = "Switch 4 ";
    String sw4 = "";
  if(digitalRead(switch4)==1){
       sw4 = "OFF!";
  }
  if(digitalRead(switch4)==0) {
       sw4 = "ON!";
  }
  status4 += sw4;
  status4 += " Stopped="+(String)stopped4;      
 }  

  void ntpClock(){
  
  timeClient.update();
  unsigned long unix_epoch = CE.toLocal(timeClient.getEpochTime()); // timeClient.getEpochTime() Get Unix epoch time from the NTP server // CE.toLocal - Timezone + daylight saving correction from Timezone.h
 
  second_ = second(unix_epoch);
  if (last_second != second_) { 
      
    minute_ = minute(unix_epoch);
    hour_   = hour(unix_epoch);
    day_    = day(unix_epoch);
    month_  = month(unix_epoch);
    year_   = year(unix_epoch);
  
    hours = hour_;
    minutes = minute_; 
    last_second = second_;               
  }  
  // Print time on serial monitor
    Serial.println("The time is:");
    Serial.print(hours);
    Serial.print(":");
   if (minutes < 10) {
     minutesString = "0"+(String)minutes;        
   }
   else minutesString = (String)minutes;
    Serial.println(minutesString); 
  }

 void scheduler1(){ // Start or stop the switch automatically
  if(!stopped1){
    if (hours * 60 + minutes >= startHour1 * 60 + startMin1 && hours * 60 + minutes < stopHour1 * 60 + stopMin1){ 
          digitalWrite(switch1, LOW); // Start, Active LOW 
    } else if (startHour1 == stopHour1 && startMin1 == stopMin1){
          digitalWrite(switch1, LOW);   
    } else{ 
          digitalWrite(switch1, HIGH); // Stop          
          }        
  }
}

 void scheduler2(){ // Start or stop the switch automatically
  if(!stopped2){
    if (hours * 60 + minutes >= startHour2 * 60 + startMin2 && hours * 60 + minutes < stopHour2 * 60 + stopMin2){ 
          digitalWrite(switch2, LOW); // Start, Active LOW 
    } else if (startHour2 == stopHour2 && startMin2 == stopMin2){
          digitalWrite(switch2, LOW);   
    } else{ 
          digitalWrite(switch2, HIGH); // Stop          
          }        
  }
}

 void scheduler3(){ // Start or stop the switch automatically
  if(!stopped3){
    if (hours * 60 + minutes >= startHour3 * 60 + startMin3 && hours * 60 + minutes < stopHour3 * 60 + stopMin3){ 
          digitalWrite(switch3, LOW); // Start, Active LOW 
    } else if (startHour3 == stopHour3 && startMin3 == stopMin3){
          digitalWrite(switch3, LOW);   
    } else{ 
          digitalWrite(switch3, HIGH); // Stop          
          }        
  }
}

 void scheduler4(){ // Start or stop the switch automatically
  if(!stopped4){
    if (hours * 60 + minutes >= startHour4 * 60 + startMin4 && hours * 60 + minutes < stopHour4 * 60 + stopMin4){ 
          digitalWrite(switch4, LOW); // Start, Active LOW 
    } else if (startHour4 == stopHour4 && startMin4 == stopMin4){
          digitalWrite(switch4, LOW);   
    } else{ 
          digitalWrite(switch4, HIGH); // Stop          
          }        
  }
}

void eepromInit(){
  
  EEPROM.begin(24);
  
  if(EEPROM.read(0)<24){
  startHour1 = EEPROM.read(0);  
  stopHour1 = EEPROM.read(1); 
  startMin1 = EEPROM.read(2);  
  stopMin1 = EEPROM.read(3);  
  stopped1 = EEPROM.read(4); 
   
  startHourString1 = (String)startHour1;
   if(startHour1 < 10){
    startHourString1 = 0+(String)startHour1;
   }
  startMinString1 =  (String)startMin1; 
    if(startMin1 < 10){
    startMinString1 = 0+(String)startMin1;
    }
  stopHourString1 = (String)stopHour1;
   if(stopHour1 < 10){
    stopHourString1 = 0+(String)stopHour1;
   }
  stopMinString1 = (String)stopMin1; 
    if(stopMin1 < 10){
    stopMinString1 = 0+(String)stopMin1; 
    }
    if(stopped1){
  schedulerStatus1 = "OFF";
    } else {
  schedulerStatus1 = "ON";   
    }
  }
  if(EEPROM.read(5)<24){
  startHour2 = EEPROM.read(5);  
  stopHour2 = EEPROM.read(6); 
  startMin2 = EEPROM.read(7);  
  stopMin2 = EEPROM.read(8);  
  stopped2 = EEPROM.read(9);
    startHourString2 = (String)startHour2;
   if(startHour2 < 10){
    startHourString2 = 0+(String)startHour2;
   }
  startMinString2 =  (String)startMin2; 
    if(startMin2 < 10){
    startMinString2 = 0+(String)startMin2;
    }
  stopHourString2 = (String)stopHour2;
   if(stopHour2 < 10){
    stopHourString2 = 0+(String)stopHour2;
   }
  stopMinString2 = (String)stopMin2; 
    if(stopMin2 < 10){
    stopMinString2 = 0+(String)stopMin2; 
    }
    if(stopped2){
  schedulerStatus2 = "OFF";
    } else {
  schedulerStatus2 = "ON";   
    }
  }
  if(EEPROM.read(10)<24){
  startHour3 = EEPROM.read(10);  
  stopHour3 = EEPROM.read(11); 
  startMin3 = EEPROM.read(12);  
  stopMin3 = EEPROM.read(13);   
  stopped3 = EEPROM.read(14);
    startHourString3 = (String)startHour3;
   if(startHour3 < 10){
    startHourString3 = 0+(String)startHour3;
   }
  startMinString3 =  (String)startMin3; 
    if(startMin3 < 10){
    startMinString3 = 0+(String)startMin3;
    }
  stopHourString3 = (String)stopHour3;
   if(stopHour3 < 10){
    stopHourString3 = 0+(String)stopHour3;
   }
  stopMinString3 = (String)stopMin3; 
    if(stopMin3 < 10){
    stopMinString3 = 0+(String)stopMin3; 
    }
    if(stopped3){
  schedulerStatus3 = "OFF";
    } else {
  schedulerStatus3 = "ON";   
    }
  }
  if(EEPROM.read(15)<24){
  startHour4 = EEPROM.read(15);  
  stopHour4 = EEPROM.read(16); 
  startMin4 = EEPROM.read(17);  
  stopMin4 = EEPROM.read(18);  
  stopped4 = EEPROM.read(19);
    startHourString4 = (String)startHour4;
   if(startHour4 < 10){
    startHourString4 = 0+(String)startHour4;
   }
  startMinString4 =  (String)startMin4; 
    if(startMin4 < 10){
    startMinString4 = 0+(String)startMin4;
    }
  stopHourString4 = (String)stopHour4;
   if(stopHour4 < 10){
    stopHourString4 = 0+(String)stopHour4;
   }
  stopMinString4 = (String)stopMin4; 
    if(stopMin4 < 10){
    stopMinString4 = 0+(String)stopMin4; 
    }
    if(stopped4){
  schedulerStatus4 = "OFF";
    } else {
  schedulerStatus4 = "ON";   
    }
   } 
  if(EEPROM.read(20) == 0) {
    digitalWrite(switch1, LOW);  
  }
  if(EEPROM.read(21) == 0) {
    digitalWrite(switch2, LOW);  
  }
  if(EEPROM.read(22) == 0) {
    digitalWrite(switch3, LOW);  
  }
  if(EEPROM.read(23) == 0) {
    digitalWrite(switch4, LOW);  
  }
 }
void btn1(){
  
 output1State = digitalRead(switch1); // read the state of the output  
 int reading = digitalRead(button1);  // read the state of the button

 if(stopped1){

  // If the switch changed, due to noise or pressing:
  if (reading != lastButtonState1) {
    // reset the debouncing timer
    lastDebounceTime1 = millis();
  }
  if ((millis() - lastDebounceTime1) > debounceDelay1) {
    // whatever the reading is at, it's been there for longer than the debounce
    // delay, so take it as the actual current state:

    // if the button state has changed:
    if (reading != buttonState1) {
      buttonState1 = reading;

      // only toggle the output if the new button state is LOW
      if (buttonState1 == LOW) {
        output1State = !output1State;
        digitalWrite(switch1, output1State);
        EEPROM.write(20, output1State);
        EEPROM.commit();        
        if(output1State == LOW){
          Serial.println("Switch 1 ON!");          
        }
        else{
          Serial.println("Switch 1 OFF!");
        }
      }
    }
  }   
  digitalWrite(switch1, output1State);  // set the output  
  lastButtonState1 = reading; // save the reading. Next time through the loop, it'll be the lastButtonState
  }
 }
  void btn2(){
  
 output2State = digitalRead(switch2); // read the state of the output  
 int reading = digitalRead(button2);  // read the state of the button

 if(stopped2){

  // If the switch changed, due to noise or pressing:
  if (reading != lastButtonState2) {
    // reset the debouncing timer
    lastDebounceTime2 = millis();
  }
  if ((millis() - lastDebounceTime2) > debounceDelay2) {
    // whatever the reading is at, it's been there for longer than the debounce
    // delay, so take it as the actual current state:

    // if the button state has changed:
    if (reading != buttonState2) {
      buttonState2 = reading;

      // only toggle the output if the new button state is LOW
      if (buttonState2 == LOW) {
        output2State = !output2State;
        digitalWrite(switch2, output2State);
        EEPROM.write(21, output2State);
        EEPROM.commit();        
        if(output2State == LOW){
          Serial.println("Switch 2 ON!");          
        }
        else{
          Serial.println("Switch 2 OFF!");
        }
      }
    }
  }   
  digitalWrite(switch2, output2State);  // set the output  
  lastButtonState2 = reading; // save the reading. Next time through the loop, it'll be the lastButtonState:
  }
 }
  void btn3(){
  
 output3State = digitalRead(switch3); // read the state of the output  
 int reading = digitalRead(button3);  // read the state of the button

 if(stopped3){

  // If the switch changed, due to noise or pressing:
  if (reading != lastButtonState3) {
    // reset the debouncing timer
    lastDebounceTime3 = millis();
  }
  if ((millis() - lastDebounceTime3) > debounceDelay3) {
    // whatever the reading is at, it's been there for longer than the debounce
    // delay, so take it as the actual current state:

    // if the button state has changed:
    if (reading != buttonState3) {
      buttonState3 = reading;
      
      // only toggle the output if the new button state is LOW
      if (buttonState3 == LOW) {
        output3State = !output3State;
        digitalWrite(switch3, output3State);
        EEPROM.write(22, output3State);
        EEPROM.commit();        
        if(output3State == LOW){
          Serial.println("Switch 3 ON!");          
        }
        else{
          Serial.println("Switch 3 OFF!");
        }
      }
    }
  }   
  digitalWrite(switch3, output3State);  // set the output  
  lastButtonState3 = reading; // save the reading. Next time through the loop, it'll be the lastButtonState
  }
 }
void btn4(){
  
 output4State = digitalRead(switch4); // read the state of the output  
 int reading = digitalRead(button4);  // read the state of the button 
 if(stopped4){
  // If the switch changed, due to noise or pressing:
  if (reading != lastButtonState4) {
    // reset the debouncing timer    
    lastDebounceTime4 = millis();
  }
  if ((millis() - lastDebounceTime4) > debounceDelay4) {
    // whatever the reading is at, it's been there for longer than the debounce
    // delay, so take it as the actual current state:

    // if the button state has changed:
    if (reading != buttonState4) {
      buttonState4 = reading;

      // only toggle the output if the new button state is LOW
      if (buttonState4 == LOW) {
        output4State = !output4State;
        digitalWrite(switch4, output4State);
        EEPROM.write(23, output4State);
        EEPROM.commit();        
        if(output4State == LOW){
          Serial.println("Switch 4 ON!");          
        }
        else{
          Serial.println("Switch 4 OFF!");
        }
      }
    }
  }   
  digitalWrite(switch4, output4State);  // set the output  
  lastButtonState4 = reading; // save the reading. Next time through the loop, it'll be the lastButtonState
  }
 }

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

void setup() {
 
  pinMode(switch1, OUTPUT);
  digitalWrite(switch1, HIGH);
  pinMode(switch2, OUTPUT);
  digitalWrite(switch2, HIGH);
  pinMode(switch3, OUTPUT);
  digitalWrite(switch3, HIGH);
  pinMode(switch4, OUTPUT);
  digitalWrite(switch4, HIGH);
  pinMode(button1, INPUT_PULLUP);
  pinMode(button2, INPUT_PULLUP);
  pinMode(button3, INPUT_PULLUP);
  pinMode(button4, INPUT_PULLUP);
    
  Serial.begin(115200);
 
 connectWiFi();
 timeClient.begin();
  
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
  
  //Init and read EEPROM
  eepromInit();
  
   // Send web page to client
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html);
  });

    server.onNotFound(notFound);
   
    // Send web page to client
    server.on("/1/scheduler", HTTP_GET, [](AsyncWebServerRequest * request) {
    AsyncResponseStream *response = request->beginResponseStream("text/html");
    href = href1;    
    startHourString = startHourString1;
    startMinString = startMinString1;
    stopHourString = stopHourString1;
    stopMinString = stopMinString1;
    schedulerStatus = schedulerStatus1;    
    scheduler_html(response); 
    request->send(response);    
  });

     // Send web page to client
    server.on("/2/scheduler", HTTP_GET, [](AsyncWebServerRequest * request) {
    AsyncResponseStream *response = request->beginResponseStream("text/html");
    href = href2; 
    startHourString = startHourString2;
    startMinString = startMinString2;
    stopHourString = stopHourString2;
    stopMinString = stopMinString2;
    schedulerStatus = schedulerStatus2; 
    scheduler_html(response); 
    request->send(response);
  });

     // Send web page to client
    server.on("/3/scheduler", HTTP_GET, [](AsyncWebServerRequest * request) {
    AsyncResponseStream *response = request->beginResponseStream("text/html");
    href = href3; 
    startHourString = startHourString3;
    startMinString = startMinString3;
    stopHourString = stopHourString3;
    stopMinString = stopMinString3;
    schedulerStatus = schedulerStatus3; 
    scheduler_html(response); 
    request->send(response);
  });

     // Send web page to client
    server.on("/4/scheduler", HTTP_GET, [](AsyncWebServerRequest * request) {
    AsyncResponseStream *response = request->beginResponseStream("text/html");
    href = href4; 
    startHourString = startHourString4;
    startMinString = startMinString4;
    stopHourString = stopHourString4;
    stopMinString = stopMinString4;
    schedulerStatus = schedulerStatus4; 
    scheduler_html(response); 
    request->send(response);
  });
    
   // Send web page to client
    server.on("/1/set", HTTP_GET, [](AsyncWebServerRequest * request) {
    AsyncResponseStream *response = request->beginResponseStream("text/html");
    setScheduler(request);
    href = href1;         
    stopped1 = stopped;    
    schedulerStatus1 = schedulerStatus;
    startHourString1 = startHourString;
    startMinString1 = startMinString;
    stopHourString1 = stopHourString;
    stopMinString1 = stopMinString;
    stopMin1 = stopMin;
    startHour1 = startHour;
    startMin1 = startMin;
    stopHour1 = stopHour;
    //Write data into EEPROM
    EEPROM.write(0, startHour1);
    EEPROM.write(1, stopHour1);
    EEPROM.write(2, startMin1);
    EEPROM.write(3, stopMin1);    
    EEPROM.write(4, stopped1);
    EEPROM.commit();         
    schedulerSet_html(response);
    request->send(response);   
  });

    // Send web page to client
    server.on("/2/set", HTTP_GET, [] (AsyncWebServerRequest *request) {
    AsyncResponseStream *response = request->beginResponseStream("text/html");    
    setScheduler(request);
    href = href2; 
    stopped2 = stopped;
    schedulerStatus2 = schedulerStatus;
    startHourString2 = startHourString;
    startMinString2 = startMinString;
    stopHourString2 = stopHourString;
    stopMinString2 = stopMinString;
    stopMin2 = stopMin;
    startHour2 = startHour;
    startMin2 = startMin;
    stopHour2 = stopHour;
    //Write data into EEPROM
    EEPROM.write(5, startHour2);
    EEPROM.write(6, stopHour2);
    EEPROM.write(7, startMin2);
    EEPROM.write(8, stopMin2);
    EEPROM.write(9, stopped2);
    EEPROM.commit();       
    schedulerSet_html(response);
    request->send(response);  
  });

    // Send web page to client
    server.on("/3/set", HTTP_GET, [] (AsyncWebServerRequest *request) {
    AsyncResponseStream *response = request->beginResponseStream("text/html"); 
    setScheduler(request);
    href = href3; 
    stopped3 = stopped;
    schedulerStatus3 = schedulerStatus;
    startHourString3 = startHourString;
    startMinString3 = startMinString;
    stopHourString3 = stopHourString;
    stopMinString3 = stopMinString;
    stopMin3 = stopMin;
    startHour3 = startHour;
    startMin3 = startMin;
    stopHour3 = stopHour;
    //Write data into EEPROM
    EEPROM.write(10, startHour3);
    EEPROM.write(11, stopHour3);
    EEPROM.write(12, startMin3);
    EEPROM.write(13, stopMin3);
    EEPROM.write(14, stopped3);
    EEPROM.commit();       
    schedulerSet_html(response);
    request->send(response);  
  });

    // Send web page to client
    server.on("/4/set", HTTP_GET, [] (AsyncWebServerRequest *request) {
    AsyncResponseStream *response = request->beginResponseStream("text/html");    
    setScheduler(request);
    href = href4;
    stopped4 = stopped;
    schedulerStatus4 = schedulerStatus;
    startHourString4 = startHourString;
    startMinString4 = startMinString;
    stopHourString4 = stopHourString;
    stopMinString4 = stopMinString;
    stopMin4 = stopMin;
    startHour4 = startHour;
    startMin4 = startMin;
    stopHour4 = stopHour;
    //Write data into EEPROM
    EEPROM.write(15, startHour4);
    EEPROM.write(16, stopHour4);
    EEPROM.write(17, startMin4);
    EEPROM.write(18, stopMin4);
    EEPROM.write(19, stopped4);
    EEPROM.commit();       
    schedulerSet_html(response);
    request->send(response);  
  });

  // Send web page to client
  server.on("/1/stop", HTTP_GET, [] (AsyncWebServerRequest *request) {
  AsyncResponseStream *response = request->beginResponseStream("text/html");    
  Serial.println("Scheduler 1 is stopped!");
  digitalWrite(switch1, HIGH);
  href = href1;
  stopped1 = "true";
  schedulerStatus = "OFF";
  schedulerStatus1 = "OFF";
  EEPROM.write(4, stopped1);
  EEPROM.commit(); 
  schedulerSet_html(response);
  request->send(response);  
  });

  // Send web page to client
  server.on("/2/stop", HTTP_GET, [] (AsyncWebServerRequest *request) {
  AsyncResponseStream *response = request->beginResponseStream("text/html");     
  Serial.println("Scheduler 2 is stopped!");
  digitalWrite(switch2, HIGH);
  href = href2;
  stopped2 = "true";
  schedulerStatus = "OFF";
  schedulerStatus2 = "OFF";
  EEPROM.write(9, stopped2);
  EEPROM.commit(); 
  schedulerSet_html(response);
  request->send(response);  
  });

  // Send web page to client
  server.on("/3/stop", HTTP_GET, [] (AsyncWebServerRequest *request) {
  AsyncResponseStream *response = request->beginResponseStream("text/html");
  Serial.println("Scheduler 3 is stopped!");
  digitalWrite(switch3, HIGH);
  href = href3;
  stopped3 = "true";  
  schedulerStatus = "OFF";
  schedulerStatus3 = "OFF";
  EEPROM.write(14, stopped3);
  EEPROM.commit(); 
  schedulerSet_html(response);
  request->send(response);  
  });

  // Send web page to client
  server.on("/4/stop", HTTP_GET, [] (AsyncWebServerRequest *request) {
  AsyncResponseStream *response = request->beginResponseStream("text/html");    
  Serial.println("Scheduler 4 is stopped!");
  digitalWrite(switch4, HIGH);
  href = href4;
  stopped4 = "true";
  schedulerStatus = "OFF";
  schedulerStatus4 = "OFF";
  EEPROM.write(19, stopped4);
  EEPROM.commit(); 
  schedulerSet_html(response);
  request->send(response);  
  });
  
  // Receive an HTTP GET request
  server.on("/1/on", HTTP_GET, [] (AsyncWebServerRequest *request) {
    Serial.println("Switch 1 ON!");    
    digitalWrite(switch1, LOW); // Active LOW
    EEPROM.write(20, 0); // write to EEPROM
    EEPROM.commit();   
    if(stopped1){result = "Switch 1 ON! Stopped=1";} 
    else {result = "Switch 1 ON! Stopped=0";}   
    request->send(200, "text/plain", result);
  });

  // Receive an HTTP GET request
  server.on("/1/off", HTTP_GET, [] (AsyncWebServerRequest *request) {
    Serial.println("Switch 1 OFF!");    
    digitalWrite(switch1, HIGH); // Active LOW
    EEPROM.write(20, 1); // write to EEPROM
    EEPROM.commit(); 
    if(stopped1){result = "Switch 1 OFF! Stopped=1";} 
    else {result = "Switch 1 OFF! Stopped=0";}         
    request->send(200, "text/plain", result);
  });

  // Receive an HTTP GET request
  server.on("/2/on", HTTP_GET, [] (AsyncWebServerRequest *request) {
    Serial.println("Switch 2 ON");    
    digitalWrite(switch2, LOW); // Active LOW
    EEPROM.write(21, 0); // write to EEPROM
    EEPROM.commit(); 
    if(stopped2){result = "Switch 2 ON! Stopped=1";} 
    else {result = "Switch 2 ON! Stopped=0";}       
    request->send(200, "text/plain", result);
  });

 // Receive an HTTP GET request
  server.on("/2/off", HTTP_GET, [] (AsyncWebServerRequest *request) {
    Serial.println("Switch 2 OFF!");    
    digitalWrite(switch2, HIGH); // Active LOW
    EEPROM.write(21, 1); // write to EEPROM
    EEPROM.commit(); 
    if(stopped2){result = "Switch 2 OFF! Stopped=1";} 
    else {result = "Switch 2 OFF! Stopped=0";}   
    request->send(200, "text/plain", result);
  });

  // Receive an HTTP GET request
  server.on("/3/on", HTTP_GET, [] (AsyncWebServerRequest *request) {
    Serial.println("Switch 3 ON");    
    digitalWrite(switch3, LOW); // Active LOW
    EEPROM.write(22, 0); // write to EEPROM
    EEPROM.commit(); 
    if(stopped3){result = "Switch 3 ON! Stopped=1";} 
    else {result = "Switch 3 ON! Stopped=0";}        
    request->send(200, "text/plain", result);
  });

   // Receive an HTTP GET request
  server.on("/3/off", HTTP_GET, [] (AsyncWebServerRequest *request) {
    Serial.println("Switch 3 OFF!");    
    digitalWrite(switch3, HIGH); // Active LOW
    EEPROM.write(22, 1); // write to EEPROM
    EEPROM.commit(); 
    if(stopped3){result = "Switch 3 OFF! Stopped=1";} 
    else {result = "Switch 3 OFF! Stopped=0";}    
    request->send(200, "text/plain", result);
  });

  // Receive an HTTP GET request
  server.on("/4/on", HTTP_GET, [] (AsyncWebServerRequest *request) {
    Serial.println("Switch 4 ON");    
    digitalWrite(switch4, LOW); // Active LOW
    EEPROM.write(23, 0); // write to EEPROM
    EEPROM.commit(); 
    if(stopped4){result = "Switch 4 ON! Stopped=1";} 
    else {result = "Switch 4 ON! Stopped=0";}   
    request->send(200, "text/plain", result);
  });

   // Receive an HTTP GET request
  server.on("/4/off", HTTP_GET, [] (AsyncWebServerRequest *request) {
    Serial.println("Switch 4 OFF!");    
    digitalWrite(switch4, HIGH); // Active LOW
    EEPROM.write(23, 1); // write to EEPROM
    EEPROM.commit(); 
    if(stopped4){result = "Switch 4 OFF! Stopped=1";} 
    else {result = "Switch 4 OFF! Stopped=0";}     
    request->send(200, "text/plain", result);
  });    

     // Receive an HTTP GET request
  server.on("/time", HTTP_GET, [] (AsyncWebServerRequest *request) {
    String getTimeString = (String)hours+":"+minutesString;    
    request->send(200, "text/plain", getTimeString);
  });

   // Receive an HTTP GET request
  server.on("/1/", HTTP_GET, [] (AsyncWebServerRequest *request) {      
    switch1Status();
    request->send(200, "text/plain", status1);    
  });

   // Receive an HTTP GET request
  server.on("/2/", HTTP_GET, [] (AsyncWebServerRequest *request) {     
    switch2Status();
    request->send(200, "text/plain", status2);
  });

   // Receive an HTTP GET request
  server.on("/3/", HTTP_GET, [] (AsyncWebServerRequest *request) {       
    switch3Status();
    request->send(200, "text/plain", status3);
  });

   // Receive an HTTP GET request
  server.on("/4/", HTTP_GET, [] (AsyncWebServerRequest *request) {
    switch4Status();    
    request->send(200, "text/plain", status4);
  });
    
  server.begin();
 }
 
void loop() {
 
 ntpClock();   
   for (int i = 0; i < 6000; i++){      
     delay(delayPeriod);
     tinyUPnP.updatePortMappings(600000, &connectWiFi);  // 10 minutes
     scheduler1();
     scheduler2(); 
     scheduler3(); 
     scheduler4();
     btn1();
     btn2();  
     btn3();  
     btn4();    
   }
 }


 
