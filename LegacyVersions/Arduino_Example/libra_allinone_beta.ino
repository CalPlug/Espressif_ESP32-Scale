//PROJECT LIBRA SCALE MEASUREMENT SYSTEM - BETA Model
//CODE BY MICHAEL KLOPFER (2015)

#include <SoftwareSerial.h> //For LCD (this is the Sparkfun LCD kit) (Built in Arduino Library)
#include <Hx711.h> //Load Cell A/D (HX711 library)
#include <Wire.h>  //For Accelemeter (Built in Arduino Library)
#include "MMA7660.h" // For Accelemeter (MMA7660 Library)
#include <avr/wdt.h> //For Watchdog Timer (Sometimes the damn thing locks-up and hangs, I think it is something to do with the accelerometer - the WDT sees the hangup and reboots the arduino)

MMA7660 accelemeter; // Initialize accelemeter object
Hx711 scale(A1, A2); // Hx711.DOUT - pin #A2 & Hx711.SCK - pin #A3 ---this sensor uses a propriatary synchronus serial communication, not I2C
SoftwareSerial lcd(13, 2);  // Initialize LCD RX port on PIN2, Pin 13 is TX, this is unused and unconnected to LCD

//----------------------------------
//Global Variables
float powerstatus=0; //variable to hold status of battery power as read by analog in
float temp=0; //variable to read thermistor temp sensor as read by analog in
float calibrated_scale_weight_g=0; //Holding variable for weight
float calibrated_scale_weight_g_1 = 0; //For Averaging
float calibrated_scale_weight_g_2 = 0; //For Averaging
float calibrated_scale_weight_g_3 = 0; //For Averaging
float tare_subtraction_factor=0;  //When TARE is processed, this value is subtracted for the displayed weight
bool overload=0;  //Overloaded Condition flag - this is an underweight or overweight condition
bool shake=0;  //Note unstable in Z condition flag

//Initial Calibration Factors and offsets
float scale_gain=-.0022103;   //Initial Sensor Calibration Offset Value
float scale_offset=36820; //Initial Sensor Calibration Offset Value
float inherent_offset=171;  //Weight of unloaded pan

//Aceleremeter holding values
float ax,ay,az; // Accelemeter Values for x, y, and z
//------------------------------------


//---------------------------------------------------------
void setup()
{
  wdt_enable(WDTO_8S); //Enable Watchdog Timer to reboot arduino on hangup over 8 seconds if PAT heartbeat signal is not received
  accelemeter.init(); //send I2C command to initialize accelemeter
  
  //User Interface Buttons Initialize
  pinMode(6, INPUT);      //Calibration Button     
  digitalWrite(3, LOW); 

  pinMode(3, INPUT);     //Select Button      
  digitalWrite(4, LOW);
  
  pinMode(5, INPUT);     //Tare Button      
  digitalWrite(5, LOW);  
  
  pinMode(4, INPUT);    //Home Button       
  digitalWrite(6, LOW);
  
  // User Interface LEDS Initialize
  pinMode(7, OUTPUT);      //Bottom Red Light      
  digitalWrite(7, HIGH); 

  pinMode(8, OUTPUT);    //Center Green Light         
  digitalWrite(8, HIGH);
  
  pinMode(9, OUTPUT);    //Right Red Light         
  digitalWrite(9, HIGH);  
  
  pinMode(10, OUTPUT);   //Top Red Light        
  digitalWrite(10, HIGH);
  
  pinMode(11, OUTPUT);      //Left Red Light     
  digitalWrite(11, HIGH);
  
  
  Serial.begin(9600);  //begin Serial for LCD
  lcd.begin(9600);  // Start the LCD at 9600 baud
  setLCDCursor(0);  // Set cursor to the 3rd spot, 1st line
  lcd.print(" Project Libra "); //Print top line of splash screen
  setLCDCursor(16);  // Set the cursor to the beginning of the 2nd line
  lcd.print("Loading... v1.0B"); //Print bottom line of splash screen
    // Flash the backlight for test:
  for (int i=0; i<3; i++)
  {
    setBacklight(0);
    delay(250);
    setBacklight(255);
    delay(250);
  }
  delay (2000); //wait for all the BS to complete 
  clearDisplay();
  
  //Set the offset and gain values
  scale.setScale(scale_gain); //Gain
  scale.setOffset(scale_offset); //Offset
  
}

//---------------------------------------------------------
void loop() 
{
  accelemeter.getAcceleration(&ax,&ay,&az); //read accelemeter
  overload=0; //reset scale weight overload flag
  calibrated_scale_weight_g_1=scale.getGram() - inherent_offset;  //subtract inherent offset from pan weight (Read 1)
  calibrated_scale_weight_g_2=scale.getGram() - inherent_offset;  //subtract inherent offset from pan weight (Read 2)
  calibrated_scale_weight_g_3=scale.getGram() - inherent_offset;  //subtract inherent offset from pan weight (Read 3)
  calibrated_scale_weight_g = (calibrated_scale_weight_g_1 + calibrated_scale_weight_g_2 + calibrated_scale_weight_g_3)/3;  //return mean average from 3 readings

  if (digitalRead(6)!=0)  //Placeholder for not implemented function (CALIBRATION)
  {
      clearDisplay();
      lcd.print("CAL NOT");
      setLCDCursor(16);
      lcd.print("IMPLEMENTED YET");
      delay(750);
      clearDisplay();
  }
  
  if (digitalRead(3)!=0)  //Placeholder for not implemented function (SELECT)
  {
      clearDisplay();
      lcd.print("SELECT NOT");
      setLCDCursor(16);
      lcd.print("IMPLEMENTED YET");
      delay(750);
      clearDisplay();
  }

  if (calibrated_scale_weight_g>4000 || calibrated_scale_weight_g<-4000) //Detect overload condition at 4kg - current loadcell is 5KG max, way over specs for device use
    {
      clearDisplay();
      overload=1;
      lcd.print("SCALE");
      setLCDCursor(16);
      lcd.print("OVERLOAD");
      delay(350); //Flash message
      clearDisplay();
    }
    else
    {
      
    if (shake==1) //Show when scale is unsteady, ignore in overload ondition (this is more serious) - unsteady measured by abnormal change in Z AXIS
    {
      clearDisplay();
      lcd.print("HOLD STEADY");
      setLCDCursor(16);
      lcd.print("****************");
      delay(350); //Hold message to Read
      clearDisplay();
    }
      
      if (digitalRead(4) != 0) //Reset Tare when Home Button is Pressed
      {
        tare_subtraction_factor=0;
      }
      if (digitalRead(5) != 0) //Look for Tare button Press
      {
        tare_subtraction_factor = calibrated_scale_weight_g;
      }
      calibrated_scale_weight_g = calibrated_scale_weight_g - tare_subtraction_factor;
      setLCDCursor(1);  // set cursor to 2nd spot, 1st row
      //lcd.print(calibrated_scale_weight_g, 1);  // Print weight on scale
      
   if (tare_subtraction_factor != 0) //Check for Tare - correct display accordingly
   {
    //lcd.print(" g      "); //we have removed the grams reading from the top line of the screen
    lcd.print("        ");
    setLCDCursor(15);
    lcd.print("T"); //This symbol comes up then the tare is activated and persistant until cleared
   }
   else
   {
     //lcd.print("POWER: ");
     //powerstatus = (analogRead(0));      //Power level to read battery level - tis was deactivated as it was difficult to read
     //lcd.print(powerstatus);
     
     setLCDCursor(15);
     lcd.print(" ");
   }
    setLCDCursor(17); //Place curser to bottom place where screen update will occur
    lcd.print(calibrated_scale_weight_g*0.035274, 1);  // Display Weight
    lcd.print(" Oz      "); //Read only in Oz
    }
   delay(1);
   
   //Show Tilt Values on LEDs the lights will change as the pan is tilted--------------------
   
   //X AXIS
  if (ax>.12)
   {         
    digitalWrite(9, HIGH);
   }
   else
   {
     digitalWrite(9, LOW);
   }
   
  if (ax<-.1)
   {         
    digitalWrite(11, HIGH);
   }
   else
   {
     digitalWrite(11, LOW);
   }
   
   //Y AXIS
  if (ay>.32)
   {         
    digitalWrite(10, HIGH);
   }
   else
   {
    digitalWrite(10, LOW);
   }
   
   
  if (ay<-.22)//sensor on a slight tilt in mounting in handle, this is to compensate
   {         
    digitalWrite(7, HIGH);
   }
   else
   {
     digitalWrite(7, LOW);
   }
   
   // Center LED is Illuminated if the scale is straight
   
  if (digitalRead(7)==LOW && digitalRead(9)==LOW && digitalRead(10)==LOW && digitalRead(11)==LOW)
   {
     digitalWrite(8, HIGH);
   }
   else
   {
     digitalWrite(8, LOW);
   }
   
   //Z-AXIS
  if (az>1.3 || az<.6)
   {         
    shake=1;
   }
   else
   {
     shake=0;
   }
     
     wdt_reset(); //Watchdog "Pat" Signal - reset ucontroller if it locks up
}




// ---------------LCD FUNCTIONS--------------------
void setBacklight(byte brightness)
{
  lcd.write(0x80);  // send the backlight command
  lcd.write(brightness);  // send the brightness value
}

void clearDisplay()
{
  lcd.write(0xFE);  // send the special command
  lcd.write(0x01);  // send the clear screen command
}

void setLCDCursor(byte cursor_position)
{
  lcd.write(0xFE);  // send the special command
  lcd.write(0x80);  // send the set cursor command
  lcd.write(cursor_position);  // send the cursor position
}

