# ESP8266_Switch
ESP8266_Switch.ino is for control up to 4 switches, using NodeMCU module. For use the module in local network only, url address in the browser should be:
http://ModuleIP (for example: http://192.168.1.123). For control the ESP8266 module globally, the listen port must be open in the router. 
That can be done automatically with ESP8266_Switch_UPNP.ino sketch. The port in the sketch is set to 5000, and can be changed if needed. The url address in the browser in this case should be: http://StaticIP:Port (for example: http://80.90.134.243:5000).
There is daily schedulle for every switch. Time zone can be changed in the sketch.
