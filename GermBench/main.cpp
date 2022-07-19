#include <Arduino.h>
#include <EEPROM.h>

#define sendfFlashAlways
#include "../Libraries/Sendf/Sendf.h"
#define __ASSERT_USE_STDERR
#include <assert.h>
#include "../Libraries/RichEnums/enumpins.h"
#include "../Libraries/RichEnums/enumcmd.h"
#include "../Libraries/EEPromObj/eetimer.h"
#include "../Libraries/DS18b20/ds18b20.h"


extern Pin pins;
extern EETimer& eeTimer;
extern Cmd cmds;


int main(void) {
    init();
	initTemperatureSensing();
	Serial.begin(115200);
	pins.setPinModes();
	
	sendf("GermBench v0.3 %s\nbed periodes: %u\n",eeTimer.ctime(),eeTimer.bedsSize());
	uint32_t err =  eeTimer.errorNr();
	if (err)
		sendf("Pending error %u setted at  %s\n",(uint16_t)err,eeTimer.cerrortime());
	cmds.loop("verbose on");
	for (int i=10; i; i--) {
		tone(8,3000,50);
		delay(150);
	}
	while(true) {
		digitalWrite(LED_BUILTIN,!digitalRead(LED_BUILTIN));
		delay(100);
		//tone(8,3000,50);
	}
}
void __assert(const char *__func, const char *__file, int __lineno, const char *__sexp) {
    sendf("%s:%s:%d: Assertion `%s' failed.\nprogram aborted\n",__func,__file,__lineno,__sexp);
	abort();
}
