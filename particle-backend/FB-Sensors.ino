#include "DS18B20/DS18B20.h"
#include "DS18B20/Particle-OneWire.h"
#include "Particle.h"
#include "PulseSensor_Spark/SparkIntervalTimer.h"

//Varible setup 
DS18B20 ds18b20 = DS18B20(C5); //Sets Pin D2 for Water Temp Sensor
int led = D7;
char szInfo[64];
float pubTemp;
double celsius;
double fahrenheit;
unsigned int Metric_Publish_Rate = 7000;
unsigned int MetricnextPublishTime;
int DS18B20nextSampleTime;
int DS18B20_SAMPLE_INTERVAL = 6500;
int dsAttempts = 0;

void interruptSetup(void);
extern volatile int BPM;

//Choose webhook
const char *PUBLISH_EVENT_NAME = "FB-Sensors";

void setup() {
    Time.zone(-5);
    Particle.syncTime();
    pinMode(C5, INPUT);
    Serial.begin(115200);
    interruptSetup();
}

void loop() {
    if (millis() > DS18B20nextSampleTime){
        //BOOM
        ATOMIC_BLOCK(){
            getTemp();
        }
    }
    if (millis() > MetricnextPublishTime){
        Serial.println("Publishing now.");
        publishData();
    }
    noInterrupts();
}


void publishData(){
    if(!ds18b20.crcCheck()){
        return;
    }
    char tempArr[64];
    sprintf(tempArr, ",%d", BPM);
    sprintf(szInfo, "%2.2f", fahrenheit);
    strcat(szInfo, tempArr);
    //publish temp
    Particle.publish(PUBLISH_EVENT_NAME, szInfo, PRIVATE);
    //set time
    MetricnextPublishTime = millis() + Metric_Publish_Rate;
}

void getTemp(){
    if(!ds18b20.search()){
        ds18b20.resetsearch();
        celsius = ds18b20.getTemperature();
        Serial.println(celsius);
        while (!ds18b20.crcCheck() && dsAttempts < 4){
            Serial.println("Caught bad value.");
            dsAttempts++;
            Serial.print("Attempts to Read: ");
            Serial.println(dsAttempts);
        if (dsAttempts == 3){
            delay(1000);
        }
        ds18b20.resetsearch();
        celsius = ds18b20.getTemperature();
        continue;
      }
      dsAttempts = 0;
      fahrenheit = ds18b20.convertToFahrenheit(celsius);
      DS18B20nextSampleTime = millis() + DS18B20_SAMPLE_INTERVAL;
      Serial.println(fahrenheit);
    }
}
