// This library is free software; you can redistribute it and/or 
// modify it under the terms of the GNU Lesser General Public 
// License as published by the Free Software Foundation; either 
// version 2.1 of the License, or (at your option) any later version.

#include "../Libraries/TimedErrorLog/TimedErrorLog.h"
#include "../Libraries/Templates/Templates.h"
#include <wiring.c>
#include "../Libraries/Sendf/Sendf.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

extern enumStr<alarm9Run> alarm9RunList[];
extern enumStr<alarm9Repeat> alarm9RepeatList[];


namespace slices {
extern uint16_t onTime;
extern uint32_t alarm9Start;
extern uint16_t alarm9Duration;
extern alarm9Run alarm9RunState;
extern alarm9Repeat alarm9RepeatState;

};

bool TimedErrorLog::hasBeenSet=false;
const time_t y2k = 24l*3600l*(365l*30l+7l);






/****************************
             _     _ _      
 _ __  _   _| |__ | (_) ___ 
| '_ \| | | | '_ \| | |/ __|
| |_) | |_| | |_) | | | (__ 
| .__/ \__,_|_.__/|_|_|\___|
|_|                         
 **/

static TimedErrorLog & TimedErrorLog::instance() {
	static TimedErrorLog retval;
	return retval;
}
uint32_t TimedErrorLog::now() {
	return dateTime+ millis()/1000-secSReset;
} 
TimedErrorLog::operator bool() const {
	return errNr!=0;
}
TimedErrorLog::operator short() const {
	return errNr;
}
int8_t TimedErrorLog::operator = (short errorNr) {
	//if ((errNr == 0 && errorNr != 0 ) || (errNr != 0 && errorNr == 0 && TimedErrorLog::hasBeenSet==true)) {
	if ((!errNr && errorNr ) || (errNr && !errorNr && TimedErrorLog::hasBeenSet)) {
		slices::onTime= errNr == 0 ? 50 : 500;
		errNr = errorNr;
		secAtError = millis()/1000-secSReset;
		sendf(F("writing EEPROM"));
		EEPROMWrite(TIMED_ERROR_LOG_ADDRESS,*this);
	}
	return errNr;
}
void TimedErrorLog::replaceUnsetTime(int times[]) {
	struct tm * ptm = gmtime(&dateTime);
	if (times[0] == -1)
		times[0] = ptm->tm_year-100;
	if (times[1] == -1)
		times[1] = ptm->tm_mon+1;
	if (times[2] == -1)
		times[2] = ptm->tm_mday;
	if (times[3] == -1) {
		times[3] = ptm->tm_hour;
		//sendf("hour was %d\n",ptm->tm_hour);
	}
	if (times[4] == -1) {
		times[4] = ptm->tm_min;
		//sendf("minut was %d\n",ptm->tm_min);
		//sendf("strftime gave %s\n",outbuf);
	}
}
void TimedErrorLog::sendlabeledTime(uint32_t timestamp, char* format) {
	char outbuf[25];
	strftime(outbuf,25,"%F %R:%S",gmtime(&timestamp));
	sendf(format, outbuf);
}
bool TimedErrorLog::setTime(char arg[]) { 
	uint32_t secs = timegmY2k(arg);
	if (secs) {
		secSReset = millis()/1000;
		secAtError = secSReset;
		dateTime = secs;
		errNr=0;
		TimedErrorLog::hasBeenSet=true;
		slices::onTime=500;
		sendf(F("writing EEPROM\n"));
		EEPROMWrite(TIMED_ERROR_LOG_ADDRESS,*this);
	}
	return bool(secs);
}
bool TimedErrorLog::show(showmode show) {
	
	if (!errNr || TimedErrorLog::hasBeenSet)
		sendlabeledTime(now(),"now %s\n");
	if (slices::alarm9Start) {
		sendlabeledTime(slices::alarm9Start,"alarm\n\tstart %s\n");
		sendf("\tduration %d seconds\n",slices::alarm9Duration);
		sendf("\trunstate ");
		sendf(enumStr(alarm9Run,slices::alarm9RunState));
		///sendf(findEnumStr<alarm9Run,enumStr<alarm9Run>>(slices::alarm9RunState,alarm9RunList));
		sendf("\n\trepeat type ");
		sendf(enumStr(alarm9Repeat,slices::alarm9RepeatState));
		sendf(findEnumStr<alarm9Repeat,enumStr<alarm9Repeat>>(slices::alarm9RepeatState,alarm9RepeatList));
		sendf(F("\n"));
	}
	if (errNr) {
		char buf[20];
		sprintf(buf,"%%s error %hd\n",errNr);
		sendlabeledTime(dateTime+(secAtError & 0xfff),buf);
		sendf(F("\nSet Time"));
		if (TimedErrorLog::hasBeenSet)
			sendf(F(" or set Error 0!\n")); 
		else
			sendf(F("!\n")); 
	}
	return true; 
}
uint32_t TimedErrorLog::timegmY2k(char timeStr[]) { 
	const int partCount=5;
	int timeParts[partCount];
	backwardsIntArrayBy2digitsFiller(timeStr,timeParts, partCount);
	replaceUnsetTime(timeParts);
	//bool parseSucces = parseTimeStr(arg,timeparts);
	if (!parseTimeStr(timeParts))
		return 0;
	return timegmY2k(timeParts);
}






/**********************************
            _            _       
 _ __  _ __(_)_   ____ _| |_ ___ 
| '_ \| '__| \ \ / / _` | __/ _ \
| |_) | |  | |\ V / (_| | ||  __/
| .__/|_|  |_| \_/ \__,_|\__\___|
|_|                              
**/
void TimedErrorLog::backwardsIntArrayBy2digitsFiller(char str[], int groups[], int groupLength) {
	for (int i=0; i<groupLength ; i++)
		groups[i] = -1;
	int grp = 4;
	char buf[3];
	buf[2] = '\0';
	char *digInp = str+strlen(str)-1;
	do {
		while (!isdigit(*digInp))
			digInp--;
		if (digInp < str)
			return;
		buf[1] = *digInp--;
		buf[0] = '0';
		if (digInp >= str && isdigit(*digInp))
			buf[0] = *digInp--;
		groups[grp--]= atoi(buf);
	} while (grp >= 0 && digInp >= str);
}
uint8_t TimedErrorLog::monthLen(uint8_t year, uint8_t month) {
	static const uint8_t monthLenTbl[] = {31,28,31,30,31,30,31,31,30,31,30,31};
	return monthLenTbl[month-1]+(year%4==0 && month==2);
}
bool TimedErrorLog::parseTimeStr(int timeS[]) {
	return
		   timeS[TI_YY] <20 
		|| timeS[TI_MM] <1
		|| timeS[TI_MM] > 12
		|| timeS[TI_DD] < 1
		|| timeS[TI_DD] > monthLen(timeS[TI_YY],timeS[TI_MM])
		|| timeS[TI_HH] > 24
		|| timeS[TI_NN] > 59 ? false : true;
}

uint32_t TimedErrorLog::timegmY2k(int timeFragments[]) {
	int ycopy = timeFragments[TI_YY];
	int days = 365*ycopy;
	if  (days) {
		while (ycopy) {
			if (((ycopy-1) % 4) == 0)
				days++;
			ycopy--;
		}
	} 
	int mcopy = timeFragments[TI_MM];
	while ( mcopy > 1)
		days += monthLen(timeFragments[TI_YY],--mcopy);

	return 3600l*24l*(days+timeFragments[TI_DD]-1)
	+3600l*timeFragments[TI_HH]
	+60l*timeFragments[TI_NN];
}
TimedErrorLog::TimedErrorLog() {
	// sendf(F("reading EEPROM\n")); FYFY
	EEPROMRead<TimedErrorLog>(TIMED_ERROR_LOG_ADDRESS,*this);
	slices::onTime = errNr ? 50 : 500;
}












//bool TimedErrorLog::parseTimeStr(char timeStr[],int timeS[]) {
//	uint8_t timeRemains = strlen(timeStr);
//	char buf[3];
//	//sendf("parseing %s with length=%d\n",timeStr,timeRemains);
//	for (uint8_t grp=TI_YY; grp <=TI_NN; grp++) {
//		//sendf("loopindex %d\n",grp);
//		while (!isdigit(*timeStr)) {
//			timeStr++;
//			timeRemains--;
//			if (timeRemains <=0) {
//				sendf("timeRemains became %d",timeRemains);
//				return false;
//			}
//		}
//		buf[0] = *timeStr++;
//		buf[1] = *timeStr++;
//		if (!isdigit(buf[1])) {
//			sendf("buf[1] became %c",buf[1]);
//			return false;
//		}
//		timeRemains -=2;
//		buf[2] = '\0';
//		//sendf("buf contains %s\n",buf);
//		timeS[grp]= atoi(buf);
//
//	}
//	return parseTimeStr(timeS);
//}




 
