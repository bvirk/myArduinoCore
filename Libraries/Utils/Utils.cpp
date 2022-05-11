// This library is free software; you can redistribute it and/or 
// modify it under the terms of the GNU Lesser General Public 
// License as published by the Free Software Foundation; either 
// version 2.1 of the License, or (at your option) any later version.

#include "../Libraries/Utils/Utils.h"
#include "../Libraries/Sendf/Sendf.h"
#include <EEPROM.h>

namespace utils {
	
	uint8_t readPGM(unsigned int p) {
		return pgm_read_byte(p);
	}
	uint8_t readDATA(unsigned int p) {
		return *(uint8_t *)p;
	}
	uint8_t readEEPROM(unsigned int p) {
		return EEPROM.read(p);
	}
	
	void hexdump(const void *p, uint16_t size,uint16_t memHigestAddrBitMask, uint8_t (*content)(unsigned int address),__FlashStringHelper * typeLabel)  {
		sendf(typeLabel);
		sendf(F("0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F\n"));
		char ascii[0x11];
		ascii[0x10]='\0';
		for (unsigned int lineStart = (unsigned int)p & memHigestAddrBitMask & 0xfff0; (lineStart < (unsigned int)p+size) && (lineStart < memHigestAddrBitMask); lineStart += 0x10) {
			//printHex(lineStart,8,": ");
			sendf(F("%.8x: "),lineStart);
			for (int8_t col=0; col < 0x10; col++) 
				if (lineStart+col < (unsigned int)p || lineStart+col >= (unsigned int)p+size) {
					sendf(F(" . "));
					ascii[col]='.';
				} else {
					//uint8_t dumpCh = *(uint8_t *)(lineStart+col);
					uint8_t dumpCh = (content)(lineStart+col);
					sendf(F("%.2x "),dumpCh);
					ascii[col] = dumpCh < 0x20 ? '.' : dumpCh;
				}
			sendf(F(" |%s|\n"),ascii);
		}
	}
	void dataHexdump(const void *p, uint16_t size)  { hexdump(p,size,0x7ff,readDATA,F("DATA Addr  ")); }
	
	void pgmHexdump(const void *p, uint16_t size)  { hexdump(p,size,0x7fff,readPGM  ,F("PGM Addr   ")); }
	
	void eepromHexdump(uint16_t start, uint16_t length) { hexdump(start,length,0x3ff,readEEPROM,F("EEPROM add ")); }
	
	int8_t strcmp_PP(PGM_P s1, PGM_P s2) { 
		uint8_t b1 = 1;
		uint8_t b2 = 1;
		s1--;
		s2--;
		while (b1 && b2 && b1 == b2) {
			b1 = pgm_read_byte(++s1);
			b2 = pgm_read_byte(++s2);
		}
		return b1-b2;
	}


};