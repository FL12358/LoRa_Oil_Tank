#include <SPI.h>
#include <heltec.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

//LoRa Pins
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

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RST);

String LoRaData;

void setup(){
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
    display.setTextSize(1);
    display.setCursor(0,0);
    display.print("LORA RECEIVER ");
    display.display();
    
    //initialize Serial Monitor
    Serial.begin(115200);
    
    //SPI LoRa pins
    SPI.begin(SCK, MISO, MOSI, SS);
    //setup LoRa transceiver module
    //LoRa.setPins(SS, RST, DIO0);

    Heltec.begin(true , true , true , true , BAND);

    LoRa.setSpreadingFactor(7);
    LoRa.setSignalBandwidth(125E3);
    LoRa.setCodingRate4(5);
    LoRa.setPreambleLength(8);
    //LoRa.disableCrc();
    LoRa.setSyncWord(0x12);

    Serial.println("LoRa Initialising OK!");
    display.setCursor(0,10);
    display.println("LoRa Initialising OK!");
    display.display();
}

void loop() {
    int packetSize = LoRa.parsePacket();
    delay(1);
    if(packetSize){
        while (LoRa.available()) {
            LoRaData = LoRa.readString();
            Serial.println(LoRaData);
        }
        delay(1000);
        display.clearDisplay();
        display.setCursor(0,0);
        display.print("LORA RECEIVER");
        display.setCursor(0,20);
        display.print("Received packet:");
        display.setCursor(0,30);
        display.print(LoRaData);
        display.setCursor(0,40);
        display.print("Packet Size: ");
        display.print(packetSize);
        display.display();
    }
}
