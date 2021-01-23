

//#include <heltec.h>
#include <LoRa.h>

//Libraries for OLED Display
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

// LoRa signal band
#define BAND 866E6

//OLED pins
#define OLED_SDA 4
#define OLED_SCL 15 
#define OLED_RST 16
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Sensor pins
#define trigPin 13
#define echoPin 12

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RST);

float distance_cm;

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

  SPI.begin(SCK, MISO, MOSI, SS);
  LoRa.setPins(SS, RST, DIO0);
  
  if (!LoRa.begin(BAND)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
  
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

void loop() {
  distance_cm = takeMeasurement(10);

    //Send LoRa packet to receiver
  delay(100);
  LoRa.beginPacket();
  LoRa.print(distance_cm);
  LoRa.print(" cm");
  LoRa.endPacket();
  delay(100);
  //Serial.print("Distance: ");
  //Serial.print(distance_cm);
  //Serial.println(" cm");
  
  display.clearDisplay();
  display.setCursor(0,0);
  display.print("Ultrasonic Sensor ");
  display.setCursor(0,20);
  display.setTextSize(1);
  display.print("Distance: ");
  display.setCursor(0,30);
  display.print(distance_cm);
  display.setCursor(50,30);
  display.print(" cm");      
  display.display();


  
  delay(1000);
}
