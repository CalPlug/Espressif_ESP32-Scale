#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <Hx711EXT.h> //Load Cell A/D (HX711 Extended library by M. J. Klopfer)
#include <SPI.h>
#include <EEPROM.h>  //this is needed to access EEPROM (not implemented yet)
 

//----------------------------------------HX711 AND TARE PARAMETERS-------------------------------
//Interface Assignments
#define HX711CLK 19
#define HX711DA 18
#define TAREBTN 21

 //Define Objects
Hx711 scale(HX711DA, HX711CLK); // Object for scale - Hx711.DOUT - pin #A2 & Hx711.SCK - pin #A3 ---this sensor uses a propriatary synchronus serial communication, not I2C
portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;

//Global Variables
float calibrated_scale_weight_g=0; //Holding variable for weight
volatile int interruptcounter = 0;  //Volatile as this is accessed by multiple threaded activities  //(see: https://techtutorialsx.com/2017/09/30/esp32-arduino-external-interrupts/)

//Initial Calibration Factors and offsets
float scale_gain = .0022103; //Initial Sensor Calibration Offset Value (fill in the `offset` value here) - working copy, can be changed and overwritten
float scale_offset = 30769.40;  //ADC Value of unloaded sensor - working copy, can be changed and overwritten
float tare_subtraction_factor=-6128;

//---------------------------------------EEPROM READ AND WRITE FUNCTIONS----------------------------
// ONLY FOR EEPROM WRITE FUNCTION AND READ FUNCTION
// FLOAT- BYTE type conversion
union FData{   
  float flData;
  byte DataA[4];
};
//LONG - BYTE type conversion
union LData{   
  long lgData;
  byte DataB[4];
};
  
//I only use 8 bytes of EEPROM. If you changed the size here, you also need to change the function below 
#define EEPROM_SIZE 8 
//Save float number and long number into EEPROM
void saveCalibrate(float num, long num2){
  union FData fdata;
  fdata.flData = num; 
  byte* fbyte = fdata.DataA;

  union LData ldata;
  ldata.lgData = num2;
  byte* lbyte = ldata.DataB;
  
  for (int i = 0; i < 4; i++){
      EEPROM.write(i, fbyte[i]);
      EEPROM.write(i+4, lbyte[i]);
  }
  EEPROM.commit();
}

//Get float number from EEPROM
float getFloatEeprom(){
  byte temp[4];
  union FData fdata;
  for (int i = 0; i< 4 ; i++){
     temp[i] = EEPROM.read(i);
     fdata.DataA[i] = temp[i];
  }
  Serial.printf("%f \n",fdata.flData);
  return fdata.flData;
 }

//Get long number from EEPROM
long getLongEeprom(){
  byte temp[4];
  union LData ldata;
  for (int i = 0; i< 4 ; i++){
     temp[i] = EEPROM.read(i+4);
     ldata.DataB[i] = temp[i];
  }
  Serial.printf("%ld \n", ldata.lgData);
  return ldata.lgData;
}
//-----------------------------------------BLE FUNCTION-----------------------------------------------
//BLE parameters
BLECharacteristic *pCharacteristic;
BLECharacteristic *pCharacteristic2;
bool deviceConnected = false;

#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

#define SERVICE2_UUID        "45ecddcf-c316-488d-8558-3222e5cb9b3c"
#define CHARACTERISTIC2_UUID "4a78b8dd-a43d-46cf-9270-f6b750a717c8"

//Convert float data into raw byte data (little endian) 
//ONLY FOR Bluetooth float data transfer 
union Data{   
  float flData;
  uint8_t DataA[4];
} ;
class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
    }
};


//------------------------------------------PROGRAM START ----------------------------------------
void setup()
{
  Serial.begin(115200); 
    
  //Read scale setting from EEPROM

   if (!EEPROM.begin(EEPROM_SIZE))
  {
    Serial.println("failed to initialise EEPROM, using default settings"); 
    scale.setScale(scale_gain);  //Set Scale Gain from working value
    scale.setOffset(scale_offset); //set scale offset from working value
  } 
  else {
     Serial.println(" bytes read from Flash . Values are:");
    for (int i = 0; i < EEPROM_SIZE; i++)
    {
      Serial.print(byte(EEPROM.read(i))); Serial.print(" ");
    }
    Serial.println("Loading setting from EEPROM"); 
    scale.setScale(getFloatEeprom());
    scale.setOffset(getLongEeprom());
  }
   
  tare_subtraction_factor=(scale.getMedianGram(byte(2))-scale_offset);  //default power on tare
  
  //User Interface Buttons Initialize
  pinMode(TAREBTN, INPUT_PULLUP);      //Tare Button 
  attachInterrupt(digitalPinToInterrupt(TAREBTN), handleInterrupt, RISING);
  
  
  Serial.println("****************END OF BIOS MESSAGES****************");
  Serial.println("");
    // Create the BLE Device
  BLEDevice::init("BLE_LIBRA");

  // Create the BLE Server
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE Service
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Create a BLE Characteristic
  //BLE_READ Characteristic
  pCharacteristic = pService->createCharacteristic(
                      CHARACTERISTIC_UUID,
                      BLECharacteristic::PROPERTY_WRITE  |
                      BLECharacteristic::PROPERTY_INDICATE
                    );

  pCharacteristic->addDescriptor(new BLE2902());

  //Set up default value for calibration, tare function
  uint8_t inniNum = 25;
  pCharacteristic->setValue(&inniNum, 1);
  //Calibration and Tare Characteristic
  pCharacteristic2 = pService->createCharacteristic(
                    CHARACTERISTIC2_UUID,
                    BLECharacteristic::PROPERTY_READ   |
                    BLECharacteristic::PROPERTY_NOTIFY |
                    BLECharacteristic::PROPERTY_INDICATE
                  );
  pCharacteristic2->addDescriptor(new BLE2902());

  // Start the service
  pService->start();
  // Start advertising
  pServer->getAdvertising()->start();
  union Data data;
  Serial.println("Waiting a client connection to notify...");
}

void handleInterrupt()
  {
    portENTER_CRITICAL_ISR(&mux);
    interruptcounter++;
    portEXIT_CRITICAL_ISR(&mux);
  }

  void setCalZero()
  {
    long readval;
    readval = scale.averageMedianValue(byte(2)); //READ THE MEDIAN VALUE OF 3 THEN AVERAGE WITH A SECOND MEDIAN OF 3
  scale_offset = readval; //Set the value of the unloaded scale as the offset value
  scale.setOffset(scale_offset); //set scale offset from working value, that is updated from the read
  }
  
 void setCalHundred()
  {
  //the offset should be done before this
    long readval;
    readval = scale.averageMedianValue(byte(2)); //READ THE MEDIAN VALUE OF 3 THEN AVERAGE WITH A SECOND MEDIAN OF 3, when loaded with the 100.0g calibration weight.
  float gainval = (float)(readval - scale_offset)/(100-0); //calculate the factor for the gain using the distance formula, offset is assumed to be 0g of weight
  scale_gain = 1/gainval; //Set the value of the calculated gain as the working gain valueunloaded scale as the offset value
  scale.setScale(scale_gain); //update the new gain value
  tare_subtraction_factor=(scale.getMedianGram(byte(2))-scale_offset)-100;
  }
  
 void a_task(void *pvParameter)
{
  float firstValue;
  float secondValue;
    while(1)
  {
  
  //Read scale Value
  calibrated_scale_weight_g = scale.getMedianGram(byte(2))-scale_offset;  //  Read scale value in grams, return mean average from 3 sets of median of 3 readings

  if (interruptcounter > 0) //Look for Tare button Press
    {
     portENTER_CRITICAL(&mux);
     tare_subtraction_factor = calibrated_scale_weight_g;
     interruptcounter = 0;
     portEXIT_CRITICAL(&mux);
    }
  else
    {}
  calibrated_scale_weight_g =calibrated_scale_weight_g - tare_subtraction_factor;//Calculate weight factoring in TARE value

  
  //Serial.println(scale.averageValue()); //output of value prior to gram calculation
  printf("Measured Weight: %f grams\n", calibrated_scale_weight_g);
  printf("Measured Weight: %.3f ounces\n", (calibrated_scale_weight_g*0.035274));
  printf("Measured Weight: %.4f Newtons\n", (calibrated_scale_weight_g*0.0098));
  
  vTaskDelay(500 / portTICK_RATE_MS);
    
  //there should be a function here to save to the EEPROM after the update to gain and offset are made and verified as OK.

  //BLE Code
  union Data data;
  if (deviceConnected) {
    
    //Convert float number to byte[] 
    data.flData = calibrated_scale_weight_g;
    uint8_t* datas = data.DataA;
    pCharacteristic2->setValue(datas, 4);
    pCharacteristic2->notify();
    
    //Check input data from BLE to start Interrupt
    std::string great = pCharacteristic->getValue();
    if (great == "%"){
      handleInterrupt();
      uint8_t inniNum = 25;
      pCharacteristic->setValue(&inniNum, 1);
      Serial.printf("Data received, Interrupt activated\n");
    }
    else if (great == "$"){
      setCalZero();  // first innitial no weight on the bar
      uint8_t inniNum = 25;
      pCharacteristic->setValue(&inniNum, 1);
      Serial.printf("Data received, for offset calibration \n");
      
    }
    else if (great == "#"){
      setCalHundred();  //Try to reset the gain (w-innitial)/1000 
      uint8_t inniNum = 25;
      pCharacteristic->setValue(&inniNum, 1);
      Serial.printf("Data received, for gain calibration \n");
    }
    else if (great == "&"){
      saveCalibrate(scale_gain, scale_offset);
      uint8_t inniNum = 25;
      pCharacteristic->setValue(&inniNum, 1);
      Serial.printf("SAVE FUNCTION COMMAND RECEIVED, new setting saved \n");
    }
  }  
  printf("\n");
  }
}
 
void loop()
{
    xTaskCreate(&a_task, "a_task", 4096, NULL, 5, NULL);  //Details:  http://web.ist.utl.pt/~ist11993/FRTOS-API/group___task_ctrl.html
    while (1)
    {  
  } //Just loop after first run, treat this as a main() function rather than a loop
}
