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
#define LORA_SYMBOL_TIMEOUT             0
#define BUFFER_SIZE                     30

#define US_SAMPLES                      40
#define timetillwakeup                  10000

#define trigPin                         GPIO5
#define echoPin                         GPIO1

static TimerEvent_t sleep;
static TimerEvent_t wakeUp;
static TimerEvent_t timeOut;
static RadioEvents_t RadioEvents;


typedef enum{
    LOWPOWER,
    RX,
    TX
}States_t;

States_t state;
int numRetries = 0;
char txpacket[BUFFER_SIZE];
char rxpacket[BUFFER_SIZE];


int sort_desc(const void *cmp1, const void *cmp2){
  unsigned long a = *((unsigned long *)cmp1);
  unsigned long b = *((unsigned long *)cmp2);
  return a > b ? -1 : (a < b ? 1 : 0);
  //return b - a;
}

unsigned long takeMeasurement(){ // returns mm
    digitalWrite(GPIO6, LOW);
    delay(50); // wait for sensor to powre up
    unsigned long durationMed = 0;
    unsigned long durationArr[US_SAMPLES];
    int numZero = 0; // used to remove 0 values

    for(int i=0; i<US_SAMPLES; ++i){ // Take measurements
        digitalWrite(trigPin, LOW);
        delayMicroseconds(2);
        digitalWrite(trigPin, HIGH);
        delayMicroseconds(20);
        digitalWrite(trigPin, LOW);
        
        durationArr[i] = pulseIn(echoPin, HIGH, 1000);
        //Serial.print(durationArr[i]);
        //Serial.print(",");
        if(durationArr[i] == 0) numZero++;
        delayMicroseconds(50);
    }

    int validArrSize = US_SAMPLES - numZero;
    //Serial.print(" : ");
    //Serial.print(validArrSize);
    qsort(durationArr, validArrSize, sizeof(durationArr[0]), sort_desc);

    if(US_SAMPLES%2==0){
        durationMed = (durationArr[validArrSize/4 -1] + durationArr[validArrSize/4]) /2;
    }else{
        durationMed = durationArr[validArrSize/4];
    }

    //Serial.println("");
    digitalWrite(GPIO6, HIGH);
    return (17*durationMed)/100;
}

void SleepSetup(){
    Radio.Sleep();
    state = LOWPOWER;
    TimerSetValue( &wakeUp, timetillwakeup );
    TimerStart( &wakeUp );
    //Serial.println("SleepSetup Done");
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
/*
void RadioRxSetup(){
    Radio.Init( &RadioEvents );
    Radio.SetChannel( RF_FREQUENCY );
    Radio.SetRxConfig( MODEM_LORA, LORA_BANDWIDTH, LORA_SPREADING_FACTOR,
                                   LORA_CODINGRATE, 0, LORA_PREAMBLE_LENGTH,
                                   LORA_SYMBOL_TIMEOUT, LORA_FIX_LENGTH_PAYLOAD_ON,
                                   0, true, 0, 0, LORA_IQ_INVERSION_ON, true );
}

void OnTxDone( void ){
	//Serial.print("TX done......\n");
	state = RX;
}

void OnTxTimeout( void ){
    //Serial.print("TX Timeout......\n");
    state = LOWPOWER;
}

void OnRxDone(uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr){
    memcpy(rxpacket, payload, size );
    rxpacket[size]='\0';

    //Serial.printf("\r\nreceived packet \"%s\"\r\n",rxpacket);
    //Serial.println("wait to send next packet");
    // Check for valid reply here...
}
*/
void setup(){
    //Serial.begin(115200);
    boardInitMcu();

    pinMode(trigPin, OUTPUT);
    pinMode(echoPin, INPUT);

    //RadioEvents.TxDone = OnTxDone;
    //RadioEvents.TxTimeout = OnTxTimeout;
    //RadioEvents.RxDone = OnRxDone;

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
        //Serial.printf("\"%s\" , length %d\r\n",txpacket, strlen(txpacket));
        delay(100); // this delay allows for packet to send properly

        state = RX;
    }

    if(state == RX){ // waiting for confirmation message
        //Serial.println("into RX mode");
        //Radio.Rx( 0 );
        //delay(100);
        SleepSetup();
    }
    //Radio.IrqProcess();
}