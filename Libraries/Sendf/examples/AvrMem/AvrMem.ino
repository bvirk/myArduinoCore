#include "Sendf.h"
#include "Utils.h"
#include <avr/pgmspace.h>


extern unsigned int __data_start;
extern unsigned int __data_end;
extern unsigned int __bss_start;
extern unsigned int __bss_end;


void setup() {
	Serial.begin(115200);
	uint16_t data_size = unsigned(&__data_end)-unsigned(&__data_start);
	uint16_t bss_size = unsigned(&__bss_end)-unsigned(&__bss_start);
	sendf(F("data: %x-%x : %x (%u)\n"),&__data_start,&__data_end,data_size,data_size);
	sendf(F("bss: %x-%x : %x (%u)\n"),&__bss_start,&__bss_end,bss_size,bss_size);
	sendf(F("\ndata dump\n"));
	Utils::hexdump(reinterpret_cast<const char *>(&__data_start),data_size);
	sendf(F("\nbss dump\n"));
	Utils::hexdump(reinterpret_cast<const char *>(&__bss_start),bss_size);
	
}

void loop() { }