#include "LoRaWan_APP.h"
#include "Arduino.h"

#define RF_FREQUENCY                                868E6
#define TX_OUTPUT_POWER                             14       
#define LORA_BANDWIDTH                              0
#define LORA_SPREADING_FACTOR                       7         
#define LORA_CODINGRATE                             1
#define LORA_PREAMBLE_LENGTH                        8
#define LORA_SYMBOL_TIMEOUT                         0
#define LORA_FIX_LENGTH_PAYLOAD_ON                  false
#define LORA_IQ_INVERSION_ON                        false


#define RX_TIMEOUT_VALUE                            1000
#define BUFFER_SIZE                                 30
#define US_SAMPLES                                  10

#define trigPin GPIO5
#define echoPin GPIO1


#define timetillwakeup 60*1000
static TimerEvent_t sleep;
static TimerEvent_t wakeUp;
uint8_t lowpower=1;
int hasSent = 0;

char txpacket[BUFFER_SIZE];
char rxpacket[BUFFER_SIZE];

static RadioEvents_t RadioEvents;
int16_t rxSize;


int sort_desc(const void *cmp1, const void *cmp2)
{
  unsigned long a = *((unsigned long *)cmp1);
  unsigned long b = *((unsigned long *)cmp2);
  return a > b ? -1 : (a < b ? 1 : 0);
  //return b - a;
}

unsigned long takeMeasurement(){ // returns mm
    unsigned long durationMed = 0;
    unsigned long durationArr[US_SAMPLES];
    int numZero = 0; // used to remove 0 values

    for(int i=0; i<US_SAMPLES; ++i){ // Take measurements
        digitalWrite(trigPin, LOW);
        delayMicroseconds(2);
        digitalWrite(trigPin, HIGH);
        delayMicroseconds(20);
        digitalWrite(trigPin, LOW);
        
        durationArr[i] = pulseIn(echoPin, HIGH, 10000);
        Serial.print(durationArr[i]);
        Serial.print(",");
        if(durationArr[i] == 0) numZero++;
        delay(50);
    }

    int validArrSize = US_SAMPLES - numZero;
    Serial.print(" : ");
    Serial.print(validArrSize);
    qsort(durationArr, validArrSize, sizeof(durationArr[0]), sort_desc);

    if(US_SAMPLES%2==0){
        durationMed = (durationArr[validArrSize/2 -1] + durationArr[validArrSize/2]) /2;
    }else{
        durationMed = durationArr[validArrSize/2];
    }

    Serial.println("");
    return (17*durationMed)/100;
}

void SleepSetup(){
    Radio.Sleep();
    lowpower = 1;

    //timetillwakeup ms later wake up;
    TimerSetValue( &wakeUp, timetillwakeup );
    TimerStart( &wakeUp );

    delay(50);
}

void onWakeUp(){
    lowpower = 0;
    hasSent = 0;
}

void setup(){
    boardInitMcu();
    Serial.begin(115200);

    pinMode(trigPin, OUTPUT);
    pinMode(echoPin, INPUT);

    Radio.Init( &RadioEvents );
    Radio.SetChannel( RF_FREQUENCY );
    Radio.SetTxConfig(  MODEM_LORA, TX_OUTPUT_POWER, 0, LORA_BANDWIDTH,
                        LORA_SPREADING_FACTOR, LORA_CODINGRATE,
                        LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
                        true, 0, 0, LORA_IQ_INVERSION_ON, 3000 ); 


    TimerInit(&wakeUp, onWakeUp);
    SleepSetup();
}

void loop(){
    if(lowpower){
        lowPowerHandler();
    }
    if(!hasSent){
        digitalWrite(GPIO6, LOW);
        Radio.Init( &RadioEvents );
        Radio.SetChannel( RF_FREQUENCY );
        Radio.SetTxConfig(  MODEM_LORA, TX_OUTPUT_POWER, 0, LORA_BANDWIDTH,
                        LORA_SPREADING_FACTOR, LORA_CODINGRATE,
                        LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
                        true, 0, 0, LORA_IQ_INVERSION_ON, 3000 ); 

        int distance = takeMeasurement();
        sprintf(txpacket,"%d",distance);  //start a package
        Radio.Send( (uint8_t *)txpacket, strlen(txpacket) ); //send the package out 
        Serial.printf("\"%s\" , length %d\r\n",txpacket, strlen(txpacket));
        digitalWrite(GPIO6, HIGH);
        delay(100);
        hasSent = 1;
        SleepSetup();
    }
}