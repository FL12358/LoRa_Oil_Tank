#include <heltec.h>
#include <Wire.h>

#include "WiFi.h" 
#include "driver/adc.h"
#include <esp_wifi.h>
#include <esp_bt.h>
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"

//LoRa pins
#define SCK 5
#define MISO 19
#define MOSI 27
#define SS 18
#define RST 14
#define DIO0 26

#define BAND 868E6



// Sensor pins
#define trigPin 13
#define echoPin 12

#define uS_TO_S_FACTOR 1000000
#define SLEEPTIME 10


float takeMeasurement(int samples){ // returns mm
    unsigned long duration_us = 0;
    for(int i=0; i<samples; ++i){
        digitalWrite(trigPin, HIGH);
        delayMicroseconds(20);
        digitalWrite(trigPin, LOW);
        
        duration_us += pulseIn(echoPin, HIGH);
        delayMicroseconds(10);
    }
    duration_us /= samples;

    Serial.println((17*duration_us)/100);

    return  (17*duration_us)/100;
}

template<typename TYPE>
void loRaSendPacket(TYPE data){
    LoRa.beginPacket();
    LoRa.setTxPower(14, RF_PACONFIG_PASELECT_PABOOST);
    LoRa.print(data);
    LoRa.endPacket();
}

void setupDeepSleep(){
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
    btStop();
    esp_bt_controller_disable();
    LoRa.end();
    LoRa.sleep();
    adc_power_off();
    
    pinMode(5,INPUT);
    pinMode(14,INPUT);
    pinMode(15,INPUT);
    pinMode(16,INPUT);
    pinMode(17,INPUT);
    pinMode(18,INPUT);
    pinMode(19,INPUT);
    pinMode(26,INPUT);
    pinMode(27,INPUT);

    delay(100);
    esp_sleep_enable_timer_wakeup(SLEEPTIME * uS_TO_S_FACTOR);
    esp_deep_sleep_start();
}

void setup(){
    // Wakes from sleep...
    Serial.begin(115200);

    pinMode(trigPin, OUTPUT);
    pinMode(echoPin, INPUT);
    Heltec.begin(false , true , true , true , BAND);

    LoRa.setSpreadingFactor(7);
    LoRa.setSignalBandwidth(125E3);
    LoRa.setCodingRate4(5);
    LoRa.setPreambleLength(8);
    LoRa.disableCrc();
    LoRa.setSyncWord(0x12);

    delay(100);

    unsigned long distance = takeMeasurement(10);
    loRaSendPacket(distance);
    setupDeepSleep();
}



void loop() {
    // Not needed due to sleep
}