/* Arduino library for digital weight scale of hx711
 *
 * Hx711EXT.h
 *
 * Original 
 * Library design: Weihong Guan (@aguegu) (hosted: https://github.com/aguegu/Arduino)
 * http://aguegu.net
 * Hardware design: syyyd, available at http://syyyd.taobao.com
 * Created on: Oct 31, 2012
 *  Extended with work by LEMIO: https://github.com/lemio/HX711
 *
 * Modified and Extended by Michael J. Klopfer
 * University of California, Irvine  
 * Includes median functions (noise reduction for mis-reads and ESP32 Compatibility)
 *
 */

#ifndef HX711EXT_H_
#define HX711EXT_H_

#include "Arduino.h"

class Hx711
{
public:
	Hx711(uint8_t pin_din, uint8_t pin_slk);
	virtual ~Hx711();
	long getValue();
	long averageValue(byte times = 25);
	void setOffset(long offset);
	void setScale(float scale = 1992.f);
	float getGram(byte times = 25);
	float getMedianGram(byte times = 5);
	long medianValue();
	long averageMedianValue(byte times = 5);


private:
	const uint8_t _pin_dout;
	const uint8_t _pin_slk;
	long _offset;
	float _scale;
};

#endif /* HX711EXT_H_ */