#include "Utils.h"
#include "Sendf.h"

#define SHOWMYRAMUSAGE "Shakespeare's total work would fill flash memory"
void * dummy(__FlashStringHelper * p) { return p;}

void setup() {
	Serial.begin(115200);
	utils::dataHexdump("Hello world",12);
	sendf("\n");
	utils::pgmHexdump(dummy(F(SHOWMYRAMUSAGE)),49);
	sendf("\n");
	utils::eepromHexdump(0,64);
}
void loop() {}
