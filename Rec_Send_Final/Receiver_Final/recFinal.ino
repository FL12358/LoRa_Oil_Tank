#include <SPI.h>
#include <heltec.h>
#include <arduino.h>

//LoRa Pins
#define SCK 5
#define MISO 19
#define MOSI 27
#define SS 18
#define RST 14
#define DIO0 26

#define BAND 868E6

String LoRaData;

void setup(){
    Serial.begin(115200);

    SPI.begin(SCK, MISO, MOSI, SS);
    Heltec.begin(false , true , true , true , BAND);

    LoRa.setSpreadingFactor(7);
    LoRa.setSignalBandwidth(125E3);
    LoRa.setCodingRate4(5);
    LoRa.setPreambleLength(8);
    //LoRa.disableCrc();
    LoRa.setSyncWord(0x12);
}

template<typename TYPE>
void loRaSendPacket(TYPE data){
    LoRa.beginPacket();
    LoRa.setTxPower(14, RF_PACONFIG_PASELECT_PABOOST);
    LoRa.print(data);
    LoRa.endPacket();
}

int checkData(String data){
    int num = atoi(data.c_str());
    return num;
}

void loop(){
    int packetSize = LoRa.parsePacket();
    delay(1);
    if(packetSize){
        while (LoRa.available()) {
            LoRaData = LoRa.readString();
            int validPacket = checkData(LoRaData);

            if(validPacket){
                Serial.println(LoRaData);
            }else{ // not valid packet
                Serial.print("ERROR: ");
                Serial.println(LoRaData);
            }
            // Send back handshake
            delay(50);
            loRaSendPacket(validPacket);

            delay(1000);
        }
        
    }
}
