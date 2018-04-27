/* Arduino library for digital weight scale of hx711 (HX711EXT.CPP)
 *
 * Hx711EXT.cpp
 *
 * Original library design: Weihong Guan (@aguegu) (hosted: https://github.com/aguegu/Arduino)
 * 
 * http://aguegu.net
 * Hardware design: syyyd, available at http://syyyd.taobao.com
 * Created on: Oct 31, 2012
 *  Extended with work by LEMIO: https://github.com/lemio/HX711
 *
 * Modified and Extended by Michael J. Klopfer
 * University of California, Irvine  
 * Includes median functions (noise reduction for misreads and ESP32 Compatibility)
 *
 */

#include "Hx711EXT.h"
#define GAIN 1  //hard code GAIN as 1 (128 value), OK for most usage

#ifdef ESP_H
uint8_t shiftInSlow(uint8_t dataPin, uint8_t clockPin, uint8_t bitOrder) {
    uint8_t value = 0;
    uint8_t i;

    for(i = 0; i < 8; ++i) {
        digitalWrite(clockPin, HIGH);
        delayMicroseconds(1);
        if(bitOrder == LSBFIRST)
            value |= digitalRead(dataPin) << i;
        else
            value |= digitalRead(dataPin) << (7 - i);
        digitalWrite(clockPin, LOW);
        delayMicroseconds(1);
    }
    return value;
}
#define SHIFTIN_WITH_SPEED_SUPPORT(data,clock,order) shiftInSlow(data,clock,order)
#else
#define SHIFTIN_WITH_SPEED_SUPPORT(data,clock,order) shiftIn(data,clock,order)
#endif


Hx711::Hx711(uint8_t pin_dout, uint8_t pin_slk) :
		_pin_dout(pin_dout), _pin_slk(pin_slk)
{
	pinMode(_pin_slk, OUTPUT);
	pinMode(_pin_dout, INPUT);

	digitalWrite(_pin_slk, HIGH);
	delayMicroseconds(100);
	digitalWrite(_pin_slk, LOW);

	averageValue();
	this->setOffset(averageValue());
	this->setScale();
}

Hx711::~Hx711()
{

}

long Hx711::averageValue(byte times)
{
	long sum = 0;
	for (byte i = 0; i < times; i++)
	{
		sum += getValue();
	}

	return (sum / times);
}

long Hx711::averageMedianValue(byte times) //averages a median set:  Warning, each median call is 3 reads!
{
	long sum = 0;
	for (byte i = 0; i < times; i++)
	{
		sum += medianValue();
	}

	return (sum / times);
}

long Hx711::medianValue()  //Returns median of 3 readings, median noise filter
{
 long middle;
 long a = getValue();
 delayMicroseconds(100);  //delay before next read
 long b = getValue();
 delayMicroseconds(100); //delay before next read
 long c = getValue();

 if ((a <= b) && (a <= c))
 {
   middle = (b <= c) ? b : c;
 }
 else if ((b <= a) && (b <= c))
 {
   middle = (a <= c) ? a : c;
 }
 else
 {
   middle = (a <= b) ? a : b;
 }
 return middle;
}


long Hx711::getValue()
{
	// wait for the chip to become ready
	while (digitalRead(_pin_dout) != LOW) 
		// check if HX711 is ready
		// from the datasheet: When output data is not ready for retrieval, digital output pin data_out is high. Serial clock
		// input clock pin should be low. When data_out goes to low, it indicates data is ready for retrieval.
	{
		//If this is not implemented, there is the potential for watchdog based resets or lockup of critical code without a "yield" function implemented.  If set wrong, this can create exceptions at boot up
		// Will do nothing on Arduino but prevent resets of ESP8266 (Watchdog Issue) - in Arduino, this is part of the <Scheduler.h> library
		//yield();    //ESP8266 / Arduino (if neededd) version
		//vPortYield(); //ESP32 version
		//esp_task_wdt_feed(); //ESP32 version with RTOS
	}

	unsigned long value = 0;
	uint8_t data[3] = { 0 };
	uint8_t filler = 0x00;

	// pulse the clock pin 24 times to read the data
	data[2] = SHIFTIN_WITH_SPEED_SUPPORT(_pin_dout, _pin_slk, MSBFIRST);
	data[1] = SHIFTIN_WITH_SPEED_SUPPORT(_pin_dout, _pin_slk, MSBFIRST);
	data[0] = SHIFTIN_WITH_SPEED_SUPPORT(_pin_dout, _pin_slk, MSBFIRST);

	// set the channel and the gain factor for the next reading using the clock pin
	for (unsigned int i = 0; i < GAIN; i++) {
		digitalWrite(_pin_slk, HIGH);
		#ifdef ESP_H
		delayMicroseconds(1);
		#endif
		digitalWrite(_pin_slk, LOW);
		#ifdef ESP_H
		delayMicroseconds(1);
		#endif
	}

	// Replicate the most significant bit to pad out a 32-bit signed integer
	if (data[2] & 0x80) {
		filler = 0xFF;
	} else {
		filler = 0x00;
	}

	// Construct a 32-bit signed integer
	value = ( static_cast<unsigned long>(filler) << 24
			| static_cast<unsigned long>(data[2]) << 16
			| static_cast<unsigned long>(data[1]) << 8
			| static_cast<unsigned long>(data[0]) );

	return static_cast<long>(value);
}

void Hx711::setOffset(long offset)
{
	_offset = offset;
}

void Hx711::setScale(float scale)
{
	_scale = scale;
}

float Hx711::getGram(byte times)
{
	float val = (float(averageValue(times)) * _scale);
	return (float( val + _offset));
}

float Hx711::getMedianGram(byte times) //averages a median set:  Warning, each median call is 3 reads, so this should not be as small as the normal getGram()
{
	float val = (float(averageMedianValue(times)) * _scale);
	return (float( val + _offset));
}