#include "LoRaWan_APP.h"
#include "Arduino.h"

#define RF_FREQUENCY                    868E6
#define TX_OUTPUT_POWER                 14       
#define LORA_BANDWIDTH                  0
#define LORA_SPREADING_FACTOR           7         
#define LORA_CODINGRATE                 1
#define LORA_PREAMBLE_LENGTH            8
#define LORA_FIX_LENGTH_PAYLOAD_ON      false
#define LORA_IQ_INVERSION_ON            false
#define BUFFER_SIZE                     30

#define US_SAMPLES                      40
#define timetillwakeup                  1*1000     // ms value

#define trigPin                         GPIO5
#define echoPin                         GPIO1

static TimerEvent_t wakeUp;
static RadioEvents_t RadioEvents;

typedef enum{
    LOWPOWER,
    TX
}States_t;

States_t state;
char txpacket[BUFFER_SIZE];


int sort_desc(const void *cmp1, const void *cmp2){
  unsigned int a = *((unsigned int *)cmp1);
  unsigned int b = *((unsigned int *)cmp2);
  //return a > b ? -1 : (a < b ? 1 : 0);
  return b - a;
}

unsigned int takeMeasurement(){ // returns mm
    digitalWrite(GPIO6, LOW);
    delay(50); // wait for sensor to power up
    unsigned int durationMed = 0;
    unsigned int durationArr[US_SAMPLES];
    int numZero = 0; // used to remove 0 values

    for(int i=0; i<US_SAMPLES; ++i){ // Take measurements
        digitalWrite(trigPin, LOW);
        delayMicroseconds(2);
        digitalWrite(trigPin, HIGH);
        delayMicroseconds(20);
        digitalWrite(trigPin, LOW);
        
        durationArr[i] = pulseIn(echoPin, HIGH, 1000);
        if(durationArr[i] == 0) numZero++;
        delayMicroseconds(50);
    }

    int validArrSize = US_SAMPLES - numZero;
    qsort(durationArr, validArrSize, sizeof(durationArr[0]), sort_desc);

    if(US_SAMPLES%2==0){
        durationMed = (durationArr[validArrSize/4 -1] + durationArr[validArrSize/4]) /2;
    }else{
        durationMed = durationArr[validArrSize/4];
    }

    digitalWrite(GPIO6, HIGH);
    return (17*durationMed)/100;
}

void SleepSetup(){
    Radio.Sleep();
    state = LOWPOWER;
    TimerSetValue( &wakeUp, timetillwakeup );
    TimerStart( &wakeUp );
    delay(50);
}

void onWakeUp(){
    state = TX;
}

void RadioTxSetup(){
    Radio.Init( &RadioEvents );
    Radio.SetChannel( RF_FREQUENCY );
    Radio.SetTxConfig(  MODEM_LORA, TX_OUTPUT_POWER, 0, LORA_BANDWIDTH,
                        LORA_SPREADING_FACTOR, LORA_CODINGRATE,
                        LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
                        true, 0, 0, LORA_IQ_INVERSION_ON, 3000 ); 
}

void setup(){
    boardInitMcu();

    pinMode(trigPin, OUTPUT);
    pinMode(echoPin, INPUT);

    TimerInit(&wakeUp, onWakeUp);
    state = TX;
}

void loop(){
    if(state == LOWPOWER){
        lowPowerHandler();
    }
    if(state == TX){
        RadioTxSetup();
        int distance = takeMeasurement();
        sprintf(txpacket,"%d",distance);  //start a package
        Radio.Send((uint8_t*)txpacket, strlen(txpacket)); //send the package out 
        delay(100); // this delay allows for packet to send properly
        SleepSetup();
    }
}