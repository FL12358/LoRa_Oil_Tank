//#include <SPI.h>
#include <heltec.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include "WiFi.h" 
#include "driver/adc.h"
#include <esp_wifi.h>
#include <esp_bt.h>


//LoRa pins
#define SCK 5
#define MISO 19
#define MOSI 27
#define SS 18
#define RST 14
#define DIO0 26

#define BAND 868E6

//OLED pins
#define OLED_SDA 4
#define OLED_SCL 15 
#define OLED_RST 16
#define SCREEN_WIDTH 128 
#define SCREEN_HEIGHT 64 

// Sensor pins
#define trigPin 13
#define echoPin 12

#define uS_TO_S_FACTOR 1000000



Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RST);

void setup(){
    setCpuFrequencyMhz(80);

    pinMode(trigPin, OUTPUT);
    pinMode(echoPin, INPUT);

    Heltec.begin(false , true , true , true , BAND);
    delay(500);


    float distance_cm = takeMeasurement(10);
    LoRaSendPacket(distance_cm);

    setupDeepSleep(100);
    delay(100);
    esp_sleep_enable_timer_wakeup(10 * uS_TO_S_FACTOR);
    esp_deep_sleep_start();
}

float takeMeasurement(int samples){
    float duration_us = 0;
    for(int i=0;i<samples;++i){
        digitalWrite(trigPin, HIGH);
        delayMicroseconds(10);
        digitalWrite(trigPin, LOW);

        duration_us += pulseIn(echoPin, HIGH);
        delay(50);
    }

    return (0.017 * duration_us)/samples;
}

template<typename TYPE>
void LoRaSendPacket(TYPE data){
    LoRa.beginPacket();
    LoRa.setTxPower(14, RF_PACONFIG_PASELECT_PABOOST);
    LoRa.print(data);
    LoRa.endPacket();
}

void setupDeepSleep(double time){
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
    btStop();
    //esp_wifi_stop(); // Causes issues entering deep sleep
    esp_bt_controller_disable();
    LoRa.sleep();

    adc_power_off();
    
    display.ssd1306_command(0x8D); //into charger pump set mode
    display.ssd1306_command(0x10); //turn off charger pump
    display.ssd1306_command(0xAE); //set OLED sleep
}

void loop() {

}
