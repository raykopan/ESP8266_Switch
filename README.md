# ESP8266_Switch
ESP8266Switch.ino is for control up to 4 switches, using ESP8266 module and ESP8266 Switch Android App. 
ESP-01_Switch.ino is for control up to 2 switches with single ESP-01 module and up to 4 switches with 2 or more modules.
For use the module in local network only, url address in the application must be set to: http://LocalIP/1/on (for example: http://192.168.1.123/1/on).
For control the ESP8266 module globally, the listen port must be open in the router. That can be done automatically with ESP8266SwitchUPNP.ino sketch. The port in the sketch is set to 5000, and can be changed if needed. The url address in the application in this case must be set to: http://StaticIP:Port/1/on (for example: http://80.90.134.243:5000/1/on).
In the application settings menu, all labels can be changed. When the button is red, URL address for state OFF can be set. When the button is green, URL address for state ON can be set. 
Android application: https://play.google.com/store/apps/details?id=com.esp8266.remote

