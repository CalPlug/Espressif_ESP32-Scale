/* Arduino library for digital weight scale of hx711
 *
 * Hx711EXT.cpp
 *
 * Original library design: Weihong Guan (@aguegu) (hosted: https://github.com/aguegu/Arduino)
 * 
 * http://aguegu.net
 * Hardware design: syyyd, available at http://syyyd.taobao.com
 * Created on: Oct 31, 2012
 *
 * Modified and Extended by Michael J. Klopfer
 * University of California, Irvine  
 * Includes median functions (noise reduction for misreads and ESP32 Compatibility)
 *
 */

#include "Hx711EXT.h"


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
	byte data[3];

	while (digitalRead(_pin_dout))
		;

	for (byte j = 0; j < 3; j++)
	{
		for (byte i = 0; i < 8; i++)
		{
			digitalWrite(_pin_slk, HIGH);
			#ifdef ESP_H
			delayMicroseconds(1);  //Slow Shift for ESP Microcontrollers
			#endif
			bitWrite(data[2 - j], 7 - i, digitalRead(_pin_dout));
			digitalWrite(_pin_slk, LOW);
			#ifdef ESP_H
			delayMicroseconds(1);  //Slow Shift for ESP Microcontrollers
			#endif
		}
	}

	digitalWrite(_pin_slk, HIGH);
	digitalWrite(_pin_slk, LOW);

	return ((long) data[2] << 16) | ((long) data[1] << 8) | (long) data[0];
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