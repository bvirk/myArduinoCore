#include "../Libraries/RichEnums/commands.h"
#include "../Libraries/RichEnums/enumpins.h"
#include <Arduino.h>
#define sendfFlashAlways
#include "../Libraries/Sendf/Sendf.h"
#define __ASSERT_USE_STDERR
#include <assert.h>
#include "../Libraries/EEPromObj/eetimer.h"
#include "../Libraries/DS18b20/ds18b20.h"
#include "../Libraries/ProcListLoop/task.h"



extern Pin pins;
extern EETimer& eeTimer;
extern float tasks::fanSwitchTemp;
extern bool tasks::toneOn;
extern bool tasks::verboseOn;


namespace cmdFuncs {

uint8_t pinstat(uint8_t argc, char *argv[]) {
	pins.consoleOut();
	return 0;
}

uint8_t setAllOut(uint8_t argc, char *argv[]) {
	if (argc > 1) 
		pins.setAllOutput(atoi(argv[1]));
	else
		soln( syntax:setout 1|0 );
	return 0;
}

uint8_t setOut(uint8_t argc, char *argv[]) {
	if (argc > 2)
		pinsOut(argv[1],atoi(argv[2]));
	else
		soln( syntax:setout 1|0 );
	return 0;
}

uint8_t getIn(uint8_t argc, char *argv[]) {
	if (argc > 1) 
		dln(pinsInFromOutput(argv[1]));
	else
		soln( syntax: in pinNr );
	return 0;
}

uint8_t setPinModes(uint8_t argc, char *argv[]) {
	pins.setPinModes();
	return 0;
}

uint8_t setPinMode(uint8_t argc, char *argv[]) {
	if (argc > 2) 
		pins.setPinMode(argv[1],atoi(argv[2]));
	else
		soln( syntax: pinmode pinNr level );
	return 0;
}

uint8_t play(uint8_t argc, char *argv[]) {
	int8_t bs = eeTimer.bedsSize();
	if (bs) {
		for (int i=0; i <bs; i++) {
			Bed b = eeTimer.bedRef(i);
			dln((unsigned)b.id);
			sendf("\tstart: %02d:00\n", b.start+6);
			sendf("\tperiod: %d minutter\n\n",b.period+1); 
		}

	} else
		soln (listen er tom);
	return 0;
}

uint8_t bedtid(uint8_t argc, char *argv[]) {
	if (argc == 1) {
		uint8_t bs = eeTimer.bedsSize();
		if (bs) {
			for (uint8_t i=0; i <bs; i++) {
				Bed b = eeTimer.bedRef(i);
				sendf("%s %s-",pins.ofBedid(b.id)->name,eeTimer.ctime(eeTimer.atHour(b.start+6,b.id)));
				sln(eeTimer.ctime(eeTimer.atHour(b.start+6,b.id)+60*(b.period+1)));
			}
		} else
			soln (listen er tom);
		return 0;
	}
	if (argc == 2) {
		if ( strcmp(argv[1],"init")==0) {
			eeTimer.init();
			soln( bed timer nulstillet );
			return 0;
		}
		if ( strcmp(argv[1],"write")==0) {
			eeTimer.writeEEProm();
			soln( bed timer skrevet til eeprom );
		} else {
			soln( syntax: bed MURBUD|BAMBIBED|LAUBAER starttime minuts );
			soln( syntax: bed MURBUD|BAMBIBED|LAUBAER starttime 0 );
			soln( syntax: bed);
			soln( syntax: bed init|write);
			newln;
			soln( 6 <= start time <= 21);
			soln( 1 <= minuts <= 4);
			soln( minuts == 0 mean remove);
		}
		return 0;
	}
	if (argc == 3) {
		soln ( kigge facilitet under opbryning);
		return 0;
	} else /* argc >= 3*/ {
		uint8_t starttid = atoi(argv[2]) -6;
		int8_t period = atoi(argv[3]) -1;
		if (starttid > 15 || period > 3) {
			soln (invalid starttime or period);
			return 1;
		}
		//sendf("trying %hd,%hd,%hd\n",pins(argv[1])->bedindex,starttid,period);
		const pinProp *p =pins(argv[1]);
		if (!
			(strcmp(p->name,"MURBED")
			&& strcmp(p->name,"BAMBIBED")
			&& strcmp(p->name,"LAUBAER"))
			
		) {
			uint8_t adjust = period == -1
				? eeTimer.removed(p->bedindex,starttid)
				: eeTimer.added(p->bedindex,starttid,(uint8_t)period);
			sendf("%d %s\n",adjust,period == -1 ? "removed" : "added");
		} else
			sendf("pin %s not a bed\n",p->name);
	}
	return 0;
}

uint8_t setTime(uint8_t argc, char *argv[]) {
	if (argc > 1)
		sln(eeTimer.setTime(argv[1]));
	else
		sln(eeTimer.ctime());
	return 0;
}


uint8_t setError(uint8_t argc, char *argv[]) {
	if (argc > 1)
		eeTimer.setError((uint8_t)atoi(argv[1]));
	sendf("error %hd at %s\n",eeTimer.errorNr(),eeTimer.cerrortime());
	return 0;
}

uint8_t fan(uint8_t argc, char *argv[]) {
	if (argc > 1) 
		tasks::fanSwitchTemp = atof(argv[1]);
	sendff(F("fan swiching: %f gr,\n"),tasks::fanSwitchTemp);
	sendff(F("Air %f gr.\n"),tempC(AIR_SERIAL));
	return 0;
}

uint8_t tone(uint8_t argc, char *argv[]) {
	if (argc > 1) 
		tasks::toneOn = !tasks::toneOn;
	sln(tasks::toneOn ? "on" : "off");
	return 0;
}

uint8_t verbose(uint8_t argc, char *argv[]) {
	if (argc > 1) 
		tasks::verboseOn = !tasks::verboseOn;
	sln(tasks::verboseOn ? "on" : "off");
	return 0;
}

uint8_t water(uint8_t argc, char *argv[]) {
	if (argc > 1) {
		if (isdigit(*argv[1])) 
			pins.setBedPins(atoi(argv[1]) & 1);
		else {
			tasks::waterOn = !tasks::waterOn;
			if (!tasks::verboseOn)
				pins.setBedPins(0);
		}
	}
	sln(tasks::waterOn ? "on" : "off");
	return 0;
}



};