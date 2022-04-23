# ESP8266_Switch
ESP8266Switch.ino is for control up to 4 switches, using NodeMCU module. For use the module in local network only, url address should be:
http://LocalIP (for example: http://192.168.1.123). For control the ESP8266 module globally, the listen port must be open in the router. 
That can be done automatically with ESP8266SwitchUPNP.ino sketch. The port in the sketch is set to 5000, and can be changed if needed. The url address in the application in this case should be: http://StaticIP:Port (for example: http://80.90.134.243:5000).
Android application: https://play.google.com/store/apps/details?id=com.esp8266.remote

