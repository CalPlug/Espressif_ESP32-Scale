# Functions

### saveCalibrate:float, long -> void

Saves calibration data to the EEPROM. If the SAVE button is pressed, the scale gain and scale offset
are saved to the EEPROM.  

### getFloatEeprom -> float

Reads from address zero to three of the EEPROM and converts the bytes to a float. This occurs at startup to 
read the saved scale data from the EEPROM.

### getLongEeprom -> void

Reads from address four to seven of the EEPROM and converts the bytes to a long. This occurs at startup to
read the saved offset data from the EEPROM.

### setup -> void

Starts the service and creates the BLE device, server, services and their respective characteristics.
Sets up the arduino for data transmission. Gets saved scale and offset data from the EEPROM and 
sets the scale using the respective data. Initializes tare button using pinMode -- input pin is pulled low if button is released. 
Attaches interrupt to tare button so that an interrupt counter is incremented by one when the tare button is released. 

### handleInterrupt -> void

A thread-safe interrupt handler that increments an interrupt counter by one when the tare button is released after a  button press.

### setCalZero -> void

Set current scale_offset to current HX711 raw value when no weight on the scale

### setCalHundred -> void

Set the scale_gain when 100 grams object is on the scale. The scale_gain is calculated by the linear equation:
100 grams = scale_gain * HX711_raw_value + scale_offset. 

### a_task:void* -> void

Continuously reads the weight from the scale and handles button presses from both user interface and physical tare button.

### loop-> void

Starts task , more information: 
https://docs.aws.amazon.com/freertos-kernel/latest/ref/reference5.html
