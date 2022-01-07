# LoRa_Oil_Tank
Using LoRa and a ultrasonic sensor to meaure oil level in tank and share inforation via a raspberry pi web server 

Sensor Device:
 - WiFi_Lora_32_V2 microcontroller
 - Ultrasonic sensor (Temp sensor? decrease noise?)
 - LoRa connection
 - Battery powered
 - Wakes up, measures distance, transmits, goes to sleep...
 - Sleep mode needs heltec.h package
 
Server Device:
 - raspberry pi 
 - connected to second WiFi_Lora_32_V2 via usb serial
 - accepts data transmitted at set interval, records/sanity check for data, adds time stamp?
 - pyserial package to get data from Heltec
 - Measuremnt storage? (two files alternating when full so not "memory leak" by logging?)
 - MQTT protocol? 

Webpage:
 - Grafana dashboard with InfluxDB backend
   
 
 
