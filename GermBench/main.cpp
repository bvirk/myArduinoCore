
#include "../Libraries/Sendf/Sendf.h"
#include "../Libraries/CmdLoop/CmdLoop.h"
#include <EEPROM.h>
#include "../Libraries/TimedErrorLog/TimedErrorLog.h"
#include <avr/pgmspace.h>
#include "../Libraries/Templates/Templates.h"
#include "../Libraries/Utils/Utils.h"
#include "../Libraries/OneWire/OneWire.h"
#include "../Libraries/Arduino-Temperature-Control-Library/DallasTemperature.h"
#include <math.h>

#define USE_EARTH
#ifdef USE_EARTH
	#define DS1820_DEVICE_COUNT 2
#else
	#define DS1820_DEVICE_COUNT 1
#endif

#define ONE_WIRE_BUS 2
#define TEMPERATURE_PRECISION 9
#define EARTH_SERIAL 0x8102162CBF22EE28
#define AIR_SERIAL   0x36021616B726EE28

enum {ERROR_READ_DS18B22_DEVICE=1,ERROR_DALLASTEMPERATURE_GETADDRESS};

TimedErrorLog & error=TimedErrorLog::instance();

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

DeviceAddress devAddr[DS1820_DEVICE_COUNT];


#define FAN_CONTROL_PIN 11
#define HEAT_CONTROL_PIN 12
#define FAN_ON 1
#define FAN_OFF 0
#define HEAT_ON 1
#define HEAT_OFF 0

float heatSwitchTemp;
float  fanSwitchTemp;
//int error;

int8_t address64Index(uint64_t addr) {
	for (int8_t i=0; i < DS1820_DEVICE_COUNT;i++) 
		if (*(uint64_t*)&devAddr[i] == addr )
			return i;
	error = ERROR_READ_DS18B22_DEVICE;
	return -1;  
}

enumStr<alarm9Run> alarm9RunList[] = 
	{ {alarm9Run::INVALID}
	 ,{alarm9Run::OFF,"OFF"}
	 ,{alarm9Run::BEFORE_START,"BEFORE_START"}
	 ,{alarm9Run::ON,"ON"}
	 };	

enumStr<alarm9Repeat> alarm9RepeatList[] = 
	{ {alarm9Repeat::INVALID}
	 ,{alarm9Repeat::NONE,"NONE",0}
	 ,{alarm9Repeat::MINUT,"MINUT",60}
	 ,{alarm9Repeat::HOUR,"HOUR",3600}
	 ,{alarm9Repeat::DAY,"DAY",86400l}
	 };	

enumStr<pinmode> pinmodeList[] = 
	{ {pinmode::INVALID}
	 ,{pinmode::PM_INPUT,"INPUT",INPUT}
	 ,{pinmode::PM_OUTPUT,"OUTPUT",OUTPUT}
	 };	






namespace slices {

	uint16_t onTime=500;         //!< steady blinking on startup
	uint16_t offTime=500;
	uint32_t alarm9Start=0;
	uint16_t alarm9Duration=0; //seconds
	uint32_t waterOnTime=0;
	alarm9Run  alarm9RunState=alarm9Run::OFF;
	alarm9Repeat  alarm9RepeatState=alarm9Repeat::NONE;

	//! initial values to ensure no action when temperature measurement fails
	float airTemperature=0;
	float earthTemperature=200;
	bool fanIsOn=false;
	bool heatIsOn=false;
	bool toneIsOn=true;
	bool verbose=false;
	
};

void enabledTone(uint8_t pin, unsigned int frequency, unsigned long duration = 0) {
	if (slices::toneIsOn)
		tone(pin,frequency,duration);
}

namespace slices {
	
	//! time error depend on blink
	/*!
	  Error signaling by LED_BUILTIN on Time being 100
	  1. static for remembering values - 5 for 3 light on periods and 2 dark periods. =5 is compile time for static
	  2. bit 0 is used as LED_BUILTIN value
	  3. condition for error signaling is ==100 - after last light on cntDown is 0 and statement // 7. is chosen
	  4. switching light/dark of LED_BUILTIN on next onBlinks run and count down to 'pass the baton on' to offBlinks
	  5. necessary on first run after offBlinks(...), but is costs more to test than just setting each time.
	  6. Make ready for next 3 short light pulses on LED_BUILTIN after 2 seconds dark made by  offBlinks(...)
	  7. cheapest way to ensure long light beam when no error has occurred
	  8. 'pass the baton on' to offBlinks
	*/  
	void onBlinks(CmdLoop & cmdLoop) {
		static uint8_t cntDown=5;        		// 1.
		digitalWrite(LED_BUILTIN,1 & cntDown);	// 2.
		if (onTime==50 && cntDown ) {   		// 3.
			cntDown--;                   		// 4.
			cmdLoop.setDelay(onTime);    		// 5.
		} else {                         		
			cntDown = 5;                 		// 6.   
			cmdLoop.setDelay(onTime);    		// 7.
			cmdLoop.next();              		// 8.  
		}
	}
	
	void offBlinks(CmdLoop & cmdLoop) {
		digitalWrite(LED_BUILTIN,0);
		sensors.requestTemperatures();
		int8_t i = address64Index(AIR_SERIAL);
    	if ( i >= 0) {
    		float newAirTemp = sensors.getTempC(devAddr[i]);
			if (fabs (newAirTemp-airTemperature) > 0.5) {
				airTemperature = newAirTemp;
				if (verbose)
					sendff(F("Air now %f\n"),newAirTemp);
			}
		}
		#ifdef USE_EARTH
    		i = address64Index(EARTH_SERIAL);
    		if ( i >= 0) {
    			float newEarthTemp = sensors.getTempC(devAddr[i]);
				if (fabs(earthTemperature-newEarthTemp) >0.5 ) {
					earthTemperature = newEarthTemp;
					if (verbose)
						sendff(F("Earth now %f\n"),newEarthTemp);
				}
			}
		 #endif
    	if (airTemperature > fanSwitchTemp && !fanIsOn) { // 0.5 degree above 
    		digitalWrite(FAN_CONTROL_PIN, FAN_ON);
			fanIsOn=true;
			if (verbose)
				sendff(F("Fan Switched on at %f\n"),airTemperature);
		}
      
    	if (airTemperature <  fanSwitchTemp &&  fanIsOn ) { // 0.5 degree belove
    		digitalWrite(FAN_CONTROL_PIN, FAN_OFF);
			fanIsOn=false;
			if (verbose)
				sendff(F("Fan Switched off at %f\n"),airTemperature);
		}

		//if (!digitalRead(11)) {
		//	error = 1;
		//}
		if (error) {
			digitalWrite (12, !digitalRead (12));
			enabledTone(8,800,200);
		}
		if (digitalRead(10) && !waterOnTime) {
			enabledTone(8,2500,100);
			pinMode(10,OUTPUT);
			digitalWrite(10,1);
			waterOnTime=millis();
			if (verbose)
				sendf(F("on 0 ms Water opened\n"));
		}
		if (waterOnTime) {
			if (waterOnTime + 3000 < millis()) {
				if (waterOnTime > 10) {
					digitalWrite(10,0);
					if (verbose)
						sendf(F("on %lu ms Water closed\n"),millis()-waterOnTime);
					waterOnTime=10;
				} else {
					//sendf(F("Water pump delay coundowned to %d\n"),--waterOnTime);
					if (!--waterOnTime) {
						if (verbose)
							sendf(F("Water enabled\n"));
						pinMode(10,INPUT);
					}
				}
			} else
				enabledTone(8,2500,100);
			
			
		}


		if (fanIsOn)
			enabledTone(8,3500,100);

		if (alarm9RunState == alarm9Run::BEFORE_START) {
			if (error.now() >= alarm9Start  ) {
				alarm9RunState = alarm9Run::ON;
				digitalWrite(9,1);
				if (verbose)
					error.sendlabeledTime(error.now(),"on  %s\n");
			}
		}
		if (alarm9RunState == alarm9Run::ON) {
			if (error.now() >= alarm9Start + alarm9Duration  ) {
				digitalWrite(9,0);
				if (verbose) {
					error.sendlabeledTime(error.now(),"off %s\n");
					sendf("duration was %d secs, \n",alarm9Duration);
				}
				if (alarm9RepeatState == alarm9Repeat::NONE)
					alarm9RunState = alarm9Run::OFF;
				else {
					alarm9RunState = alarm9Run::BEFORE_START;
					alarm9Start += enumUInt32(alarm9Repeat,alarm9RepeatState);
					
				}

			} else
				enabledTone(8,4000,50);
		}
		
		#ifdef USE_EARTH
    		if (earthTemperature < heatSwitchTemp && !heatIsOn) { 
    			digitalWrite(HEAT_CONTROL_PIN, HEAT_ON);
				heatIsOn=true;
				if (verbose)
					sendff(F("Heat switched on at %f\n"),earthTemperature);
			
			}
			if (earthTemperature >  heatSwitchTemp && heatIsOn) {
    			digitalWrite(HEAT_CONTROL_PIN, HEAT_OFF);
				heatIsOn=false;
				if (verbose)
					sendff(F("Heat switched off at %f\n"),earthTemperature);
			}
    	#endif
    	cmdLoop.setDelay(offTime);
		cmdLoop.next();
	}

	void (*slicefuncs[])(CmdLoop &) = {onBlinks,offBlinks};
};





namespace cmdFuncs {

	int8_t autoexec(int argc, char* argv[]);
	int8_t ds18b20(int argc, char* argv[]);
	int8_t fanSwitch(int argc, char *argv[]);
	int8_t heatSwitch(int argc, char *argv[]);
	bool isOutput(uint8_t pin);
	int8_t play(int argc, char *argv[]);
	int8_t out(int argc, char *argv[]);
	int8_t setAlarm(int argc, char *argv[]);
	int8_t setError(int argc, char* argv[]);
	int8_t setPinMode(int argc, char *argv[]);
	int8_t setTime(int argc, char* argv[]);
	int8_t setToneEnabled(int argc, char* argv[]);
	int8_t showStatus(int argc, char* argv[]);
	int8_t temp(int argc, char* argv[]);
	int8_t toogleToneEnabled(int argc, char* argv[]);
	int8_t toogleVerbose(int argc, char* argv[]);

	int8_t autoexec(int argc, char* argv[]) {
		char *t[] = {argv[0],"10","INPUT"};
		return setPinMode(3,t);

	}
	int8_t ds18b20(int argc, char* argv[]) {
		
    	for (int i=0; i < DS1820_DEVICE_COUNT; i++)
    		// would be lie to claim I don't experimentet with that format specifier! - llx=bogus
    		sendf("device %d: has address: %lx%lx\n",i,*((uint32_t *)devAddr[i]+1),*(uint32_t *)devAddr[i]);
    	return 0;
	}
	int8_t fanSwitch(int argc, char *argv[]) {
	  if (argc > 1) {
	  	  fanSwitchTemp = atof(argv[1]);
	  	  temp(argc,argv);
	  	  return 0;
	  }
	  return -1;  
  	}
	int8_t heatSwitch(int argc, char *argv[]) {
		if (argc > 1) {
			heatSwitchTemp = atof(argv[1]);
			temp(argc,argv);
			return 0;
		}
		return -1;  
	}
	bool isOutput(uint8_t pin) {
		uint8_t dptp = digitalPinToPort(pin);
		return bool((*portModeRegister(dptp)) & digitalPinToBitMask(pin)); 
	}  
	int8_t play(int argc, char *argv[]) {
		char *t[] = {argv[0],"10","INPUT"};
		return setPinMode(3,t);
  	}
	int8_t out(int argc, char *argv[]) {
		if (argc >2) {
			uint8_t pin= atoi(argv[1]);
			uint8_t level = atoi(argv[2]) == 1 ? 1 : 0;
			sendf(F("out %hu,%hu\n"),pin,level);
			digitalWrite(pin,level);
		}
		else
			sendf(F("out pin level\n")); 
	  	return 0;
  	}
	int8_t setAlarm(int argc, char *argv[]) { 
		//uint32_t start, uint16_t duration, alarm9Repeat repeat) {
		if (argc == 3 || argc == 4) {
			uint32_t start = error.timegmY2k(argv[1]);
			uint16_t duration = atoi(argv[2]);
			alarm9Repeat repeat =  
				enumVarByStr(alarm9Repeat,argc == 4 ? argv[3] : "NONE");
			error.sendlabeledTime(start,"start %s\n");
			sendf("duration=%hu, repeat=",duration);
			sendf(enumStr(alarm9Repeat,repeat));
			sendendl();	
			slices::alarm9Start=start;
			slices::alarm9Duration=duration;
			slices::alarm9RunState=alarm9Run::BEFORE_START;
			slices::alarm9RepeatState = repeat;
			return 0;
		} else {
				sendf(F("syntax:\n  alarm starttime duration [repeat]\n\n"));
				sendf(F("  startime is time complent of current time\n"));
				sendf(F("  duration is in seconds\n"));
				sendf(F("  repeat is one of NONE|MINUT|HOUR|DAY - NONE is default\n"));
			return 0;
		}
	}
	int8_t setError(int argc, char* argv[]) {
		if (argc > 1 ) { 
			error = atoi(argv[1]);
			error.show(TimedErrorLog::ERROR);
			return 0;
		}
		return -1;
	}
	int8_t setPinMode(int argc, char *argv[]) {
		if (argc >1) {
			uint8_t pin= atoi(argv[1]);
			if (pin >= 0 && pin <= 14) {
				if (argc >2) {
					pinmode pm = findEnum<pinmode,enumStr<pinmode>>(argv[2],pinmodeList);
					uint8_t newpinmode = (uint8_t)findEnumUInt32<pinmode,enumStr<pinmode>>(pm,pinmodeList);
					//sendf("setting new pinmode to %hd",newpinmode);
					pinMode(pin,newpinmode);
				}
				sendf("pin %hd has mode %s\n",pin,isOutput(pin) ? "output" : "input");
				//sendf("pin %hd was setted to input or output\n");
				return 0;
			}
		}
		sendf(F("pinmode pin INPUT|OUTPUT")); 
	  	return 0;
  	}
	int8_t setTime(int argc, char* argv[]) {
		if ( argc == 2 ) {
			if (error.setTime(argv[1])) {
				error.show(TimedErrorLog::NOW);
				return 0;
			} else 
				sendf(F("timestamp parse error\n"));
		} else
			sendf(F("requires 1. argument\n"));
		return -1;
	}
	int8_t showStatus(int argc, char* argv[]) {
		switch(	argc > 1 ? argv[1][0] & 0x5f : 'S' ) {
			case 'N':
				error.show(TimedErrorLog::NOW);
				break;
			case 'E':
				error.show(TimedErrorLog::ERROR);
				break;
			case 'S':
			default:
				error.show(TimedErrorLog::SETTEDTIME);
		} 
		return 0;
	}
	int8_t temp(int argc, char* argv[]) {
		
    	sendff(F("air temp:   %f dgr.\n"),slices::airTemperature);
    	#ifdef USE_EARTH
			sendff(F("earth temp: %f dgr.\n"),slices::earthTemperature);
    		sendff(F("\nSwitching temperatures:\n\nAir fan:\t\t%.1f\nHeat element: \t%.1f\n"),fanSwitchTemp,heatSwitchTemp);
		#else  
    		sendff(F("\nSwitching temperatures:\n\nAir fan:\t\t%.1f\n"),fanSwitchTemp);
		#endif	
		return 0;
	}
	int8_t toogleToneEnabled(int argc, char* argv[]) {
		if (argc > 1) {
			slices::toneIsOn = !slices::toneIsOn;
		}
		sendf(F("Tone is %s\n"),slices::toneIsOn ? "enabled": "disabled");
		return 0;
	}
	int8_t toogleVerbose(int argc, char* argv[]) {
		if (argc > 1) {
			slices::verbose = !slices::verbose;
		}
		sendf(F("Verbose writting is %s\n"),slices::verbose ? "enabled": "disabled");
		return 0;
	}

};
	

//! EDIT THIS WHEN INSERTING/REMOVING COMMANDS - ONE MORE THEN LAST INDEX OF PUTF
#define NUM_COMMANDS 15

CommandFunc CommandFuncBox::commands[NUM_COMMANDS];

namespace cmdFuncs {
	
	int8_t  list(int argc, char *argv[]) {
  	  sendf(F("GermBench v.1.0\nAvailable commands:\n\n"));
  	  for (int i=0; i < NUM_COMMANDS; i++) {
  	  	  if (utils::strcmp_PP(CommandFuncBox::commands[i].name,CmdLoop::autoExec)) {
  	  	  	sendf(FP(CommandFuncBox::commands[i].name));
  	  	  	sendf("\n");
  	  	  }
  	  }
  	  return 0;
  	}
};

CommandFuncBox::CommandFuncBox() : size(NUM_COMMANDS) {	

#define PUTF(num) ;commands[num].init(cmdFuncs:: 

							//! shows connected dallas ds18b20 devices
	   PUTF(0) ds18b20		,F("ds18b20"  		

							//! set fan switching temperature
	)) PUTF(1) fanSwitch	,F("fan" // switching-temperature  		

							//! set heat switching temperature
	)) PUTF(2) heatSwitch	,F("heat" // switching-temperature  		

							//! shows all commands
	)) PUTF(3) list			,F("list" 		

							//! sandbox
	)) PUTF(4) play			,F("play" // args 		

							//! set output level of pin
	)) PUTF(5) out			,F("out" // pin level 		

							//! set alarm which is hardwired to pin 9 going high 
	)) PUTF(6) setAlarm			,F("a9"  		

							//! sets or clears an error level
	)) PUTF(7) setError		,F("error" // value

							//! sets or clears an error level
	)) PUTF(8) setPinMode		,F("pinmode" // value

							//! settime. merging in current 
	)) PUTF(9) setTime		,F("time" // [yy.[mm.[dd.[hh.]]]]nn
		
							//! current time - after reset reflecting bogous millis() diff
	)) PUTF(10) showStatus	,F("stat" // 
		
							//! Report earth and air temperature and switching temperatures for heat element and cooling fan 
	)) PUTF(11) temp			,F("temp"  		

							//! toggles between tone enabled or disabled on dummy arg 
	)) PUTF(12) toogleToneEnabled,F("tone" // [whatever]

							//! toggles between verbose on dummy arg 
	)) PUTF(13) toogleVerbose	,F("verbose" // [whatever]

	// KEEPS AUTOEXEC AS LAST    
							//! auto executed on startup.
	)) PUTF(14) autoexec		,F("autoexec" 		
	));
}



CmdLoop cmdLoop( 
	 CommandFuncBox::instance()
	,slices::slicefuncs,sizeof(slices::slicefuncs)/sizeof(slices::slicefuncs[0])
	);



int main(void) {
    init();
	Serial.begin(115200);
	
	
	pinMode(9,OUTPUT); 					// kirsebær laubær
	pinMode(10, OUTPUT); 				// Germesh
	digitalWrite(10,0);
	pinMode(FAN_CONTROL_PIN, OUTPUT);   // pin 11
	pinMode(HEAT_CONTROL_PIN,OUTPUT);    // pin 12
	pinMode(LED_BUILTIN,OUTPUT);
	sensors.begin();
	sendf("Starting with DS1820_DEVICE_COUNT=%d\n",DS1820_DEVICE_COUNT);
	sendf("device count %d\n",sensors.getDeviceCount());
	
	for (uint8_t i=0; i < DS1820_DEVICE_COUNT; i++)
		if (!sensors.getAddress(devAddr[i], i)) {
			sendf(F("Unable to find address for Device %d\n"),i);
			error = ERROR_DALLASTEMPERATURE_GETADDRESS;
		}
		else 
			sensors.setResolution(devAddr[i], TEMPERATURE_PRECISION);
	heatSwitchTemp=10.0;
	fanSwitchTemp=29.5;
	//int8_t i = address64Index(AIR_SERIAL);
	//sendf("i=%d,error=,%d\n",i,error);
	//sensors.requestTemperatures();
	//float airtemp = sensors.getTempC(devAddr[i]);
	//sendff(F("air temp:   %f dgr.\n"),airtemp);
	
    //for (;;true) {digitalWrite(LED_BUILTIN,!digitalRead(LED_BUILTIN));delay(1000);}
	
	//
	cmdLoop.loop(F("autoexec"));
}

/*
water contact
high water: contact open
low water;  contact closed
tynd til tyk
blå		sort
grøn	hvid
brun	rød
orange	gul


*/

