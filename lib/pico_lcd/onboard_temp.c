#include "pico/stdlib.h"
#include "hardware/adc.h"

#include "onboard_temp.h"

float read_onboard_temperature(const char unit)
{
	const float conversionFactor = 3.3f / (1 << 12); 
	
	float adc = (float)adc_read() * conversionFactor;
	
	float tempC = 27.0f - (adc - 0.706f) / 0.001721f;

	if (unit == 'C') {
		return (int)tempC; 
	} else {
		return (int)(tempC * 9 / 5 + 32); 
	}

	return -1.0f; 
}

