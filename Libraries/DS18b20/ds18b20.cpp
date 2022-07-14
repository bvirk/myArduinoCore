#include "../Libraries/OneWire/OneWire.h"
#include "../Libraries/Arduino-Temperature-Control-Library/DallasTemperature.h"
#include <math.h>
#define __ASSERT_USE_STDERR
#include <assert.h>
//#include "../Libraries/DS18b20/ds18b20.h"

#define DS1820_DEVICE_COUNT 2
#define ONE_WIRE_BUS 2
#define TEMPERATURE_PRECISION 9

/** 
* global objects for temperature reading 
*/
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
DeviceAddress devAddr[DS1820_DEVICE_COUNT];

void initTemperatureSensing() {
    sensors.begin();
	for (uint8_t i=0; i < DS1820_DEVICE_COUNT; i++) {
		assert(sensors.getAddress(devAddr[i], i));
		sensors.setResolution(devAddr[i], TEMPERATURE_PRECISION);
	}
}

float tempC(uint64_t addr) {
	sensors.requestTemperatures();
	uint8_t i;for (i=0; i < DS1820_DEVICE_COUNT;i++) 
		if (*(uint64_t*)&devAddr[i] == addr )
			return sensors.getTempC(devAddr[i]);
	assert(i < DS1820_DEVICE_COUNT);
	return  0; //dummy
}
