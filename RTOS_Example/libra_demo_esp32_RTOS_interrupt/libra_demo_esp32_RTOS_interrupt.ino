#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"


#include <Hx711EXT.h> //Load Cell A/D (HX711 Extended library by M. J. Klopfer)
#include <Adafruit_TLC59711.h> //12 Ch LED Controller, uses SPI only MOSI and CLK
#include <SPI.h>
#include <EEPROM.h>  //this is needed to access EEPROM
 
#define BLINK_GPIO 13


//Constants
#define portTICK_PERIOD_MS 10


//Interface Assignments
#define HX711CLK 19
#define HX711DA 18
#define PIEZO 4
#define TAREBTN 21
#define TLC59711CLK 23
#define TLC59711DA 5

//Peripheral Setup
#define NUM_TLC59711 1  // How many boards do you have chained? (Only 1 in this example)

 //Define Objects
Hx711 scale(HX711DA, HX711CLK); // Object for scale - Hx711.DOUT - pin #A2 & Hx711.SCK - pin #A3 ---this sensor uses a propriatary synchronus serial communication, not I2C
Adafruit_TLC59711 tlc = Adafruit_TLC59711(NUM_TLC59711, TLC59711CLK, TLC59711DA);
portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;

//----------------------------------
//Global Variables
float calibrated_scale_weight_g=0; //Holding variable for weight
float tare_subtraction_factor=0;  //When TARE is processed, this value is subtracted for the displayed weight
volatile int interruptcounter = 0;

//Initial Calibration Factors and offsets
float scale_gain=-.0022103;   //Initial Sensor Calibration Offset Value (fill in the `ratio` value here) [ratio = (w - offset) / 1000]  where W is a known weight, say 1000 grams
float scale_offset=36820; //Initial Sensor Calibration Offset Value (fill in the `offset` value here)
float inherent_offset=30769.40;  //Weight of unloaded sensor

//Piezo Parameters
int freq1 = 2000;
int freq2 = 125;
int dutyCycle = 50;
int channel = 0;
int resolution = 8;
int piezoch = 3;

//------------------------------------


//---------------------------------------------------------
void setup()
{
  Serial.begin(115200);  //begin Serial for LCD
    
  //User Interface Buttons Initialize
  pinMode(TAREBTN, INPUT_PULLUP);      //Tare Button 
  attachInterrupt(digitalPinToInterrupt(TAREBTN), handleInterrupt, RISING);

  pinMode(PIEZO, OUTPUT);     //Select Button      
  digitalWrite(PIEZO, LOW);

  //Initalize Piezo
  ledcSetup(channel, freq1, resolution);
  ledcAttachPin(4, channel);

  //Initialize LED Driver
  tlc.begin();
  tlc.write();
  
  //Set the offset and gain values
  scale.setScale(scale_gain); //Gain
  scale.setOffset(scale_offset); //Offset
  
  Serial.println("****************END OF BIOS MESSAGES****************");
  Serial.println("");
}

void handleInterrupt()
  {
    portENTER_CRITICAL_ISR(&mux);
    interruptcounter++;
    portEXIT_CRITICAL_ISR(&mux);
  }
  
 void a_task(void *pvParameter)
{
    while(1)
  {
  //Read scale Value
  calibrated_scale_weight_g = scale.getMedianGram(byte(2)) - inherent_offset;  //subtract inherent offset from pan weight //return mean average from 3 sets of median of 3 readings

  if (interruptcounter > 0) //Look for Tare button Press
    {
     portENTER_CRITICAL(&mux);
     tare_subtraction_factor = calibrated_scale_weight_g;
     interruptcounter = 0;
     portEXIT_CRITICAL(&mux);
    }
    
  calibrated_scale_weight_g = calibrated_scale_weight_g - tare_subtraction_factor; //Calculate weight
  
  //Serial.println(scale.averageValue()); //output of value prior to gram calculation
  printf("Measured Weight: %.2f grams\n", calibrated_scale_weight_g);
  printf("Measured Weight: %.3f ounces\n", (calibrated_scale_weight_g*0.035274));
  printf("Measured Weight: %.4f Newtons\n", (calibrated_scale_weight_g*0.0098));
  printf("\n");
  
  vTaskDelay(500 / portTICK_RATE_MS);
  }
}
 
void hello_task(void *pvParameter)
{
 //Light LEDS, eventually will be used to make an LED bargraph
  while(1)
  {
    
 //Demo lights - ramp up to all on
      for (int i=0; i<65535; i = i + 100)
      {
        tlc.setLED(0, i, i, i);
        tlc.write();
        tlc.setLED(1, i, i, i);
        tlc.write();
        tlc.setLED(2, i, i, i);
        tlc.write();
        tlc.setLED(3, i, i, i);
        tlc.write();
      }
   //Make tone  
  //ledcWriteTone(channel, freq2);
        
   vTaskDelay(5000 / portTICK_PERIOD_MS);

//Demo lights - state 1 (all Off)
      tlc.setLED(0, 0, 0, 0);
      tlc.write();
      tlc.setLED(1, 0, 0, 0);
      tlc.write();
      tlc.setLED(2, 0, 0, 0);
      tlc.write();
      tlc.setLED(3, 0, 0, 0);
      tlc.write();
      
   //Make tone     
  //ledcWriteTone(channel, freq1);
  //ledcWrite(channel, dutyCycle);
   vTaskDelay(5000 / portTICK_RATE_MS);
  }
}
 
void loop()
{
    xTaskCreate(&a_task, "a_task", 4096, NULL, 5, NULL);
    xTaskCreate(&hello_task, "hello_task", 4096,NULL,5,NULL );
    while (1)
    {} //Just loop after first run, treat this as a main() function rather than a loop
}
