//#include <SPI.h>
#include <heltec.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

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

#define uS_TO_S_FACTOR 10E6
#define SLEEP_TIME 5



Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RST);

void setup(){
    pinMode(trigPin, OUTPUT);
    pinMode(echoPin, INPUT);

    pinMode(OLED_RST, OUTPUT);
    digitalWrite(OLED_RST, LOW);
    delay(20);
    digitalWrite(OLED_RST, HIGH);

    //initialize OLED
    Wire.begin(OLED_SDA, OLED_SCL);
    if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3c, false, false)) { // Address 0x3C for 128x32
        Serial.println(F("SSD1306 allocation failed"));
        for(;;);
    }
    
    display.clearDisplay();
    display.setTextColor(WHITE);
    display.setTextSize(1);
    display.setCursor(0,0);
    display.print("LORA SENDER ");
    display.display();
    
    //initialize Serial Monitor
    Serial.begin(115200);

    //SPI.begin(SCK, MISO, MOSI, SS);
    Heltec.begin(true , true , true , true , BAND);


    
    Serial.println("LoRa Initialising OK!");
    display.setCursor(0,10);
    display.print("LoRa Initialising OK!");
    display.display();
    delay(2000);
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

template<typename TYPE>
void OledWritePacket(TYPE data){
    display.clearDisplay();
    display.setCursor(0,0);
    display.print("Ultrasonic Sensor ");
    display.setCursor(0,20);
    display.setTextSize(1);
    display.print("Distance: ");
    display.setCursor(0,30);
    display.print(data);
    display.setCursor(50,30);
    display.print(" cm");      
    display.display();
}

void engageDeepSleep(){
    esp_sleep_enable_timer_wakeup(SLEEP_TIME * uS_TO_S_FACTOR);
    esp_deep_sleep_start();
}

void loop() {


    float distance_cm = takeMeasurement(10);
    LoRaSendPacket(distance_cm);
    OledWritePacket(distance_cm);
    engageDeepSleep();

    //Wait until confirm signal, then sleep (or try again)
    //delay(1000);
}
