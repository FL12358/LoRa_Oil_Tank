#include "Arduino.h"
#include "LoRaWan_APP.h"


const int pwPin1 = GPIO0;
long sensor, mm;

void setup()
{
    Serial.begin(9600);
    pinMode(pwPin1, INPUT);
}

void read_sensor()
{
    sensor = pulseIn(pwPin1, HIGH);
    mm = sensor/5.79;
    //mm = sensor;
}

void print_range()
{
    Serial.print("S1");
    Serial.print("=");
    Serial.println(mm);
}

void loop()
{
    read_sensor();
    print_range();
    delay(100);
}