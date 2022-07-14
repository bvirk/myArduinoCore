#include "../Libraries/Templates/Templates.h"
#include "../Libraries/EEPromObj/eetimer.h"
#include <ctype.h>
#include <stdlib.h>
#include <time.h>
#include <Arduino.h>
#define __ASSERT_USE_STDERR
#include <assert.h>


#include "../Libraries/Sendf/Sendf.h"



/****************************
             _     _ _      
 _ __  _   _| |__ | (_) ___ 
| '_ \| | | | '_ \| | |/ __|
| |_) | |_| | |_) | | | (__ 
| .__/ \__,_|_.__/|_|_|\___|
|_|                         
 **/

uint8_t EETimer::added(uint8_t id, uint8_t start, uint8_t period) {
	assert(id < (1 <<IDBITFIELDS) && start < (1 <<STARTBITFIELDS) && period < (1 <<PERIODBITFIELDS));
	//soln ( in added);
	int8_t ix = index(id,start);
	if (ix != -1) { 
		//soln( added existing! );
		bed[ix].period=period;
		return 0;
	}
	if (bedssize < bedsCapacity) {
		//soln ( still room for beds );
		uint8_t arg = id + (start << IDBITFIELDS) + (period << (IDBITFIELDS+STARTBITFIELDS));
		memmove(&bed[bedssize++],&arg,sizeof(Bed));
		return 1;
	}
	return 0;

}

time_t EETimer::atHour(uint8_t hour, uint8_t displacement) {
	time_t now = dateTime+millis()/1000-secSReset;
	time_t dayFrac = now % (time_t)ONE_DAY;
	return  now-dayFrac + (time_t)ONE_HOUR*hour+270*displacement;
}


void EETimer::backwardsIntArrayBy2digitsFiller(char str[], int16_t groups[], int groupLength) {
	for (int i=0; i<groupLength ; i++)
		groups[i] = -1;
	int grp = groupLength-1;
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
		groups[grp--]= (int16_t)atoi(buf);
	} while (grp >= 0 && digInp >= str);
}


uint8_t EETimer::bedsSize() { return bedssize; }


Bed &EETimer::bedRef(uint8_t bedsNr) {return bed[bedsNr];}


const char* EETimer::cerrortime() {
	return ctime(dateTime+millisAtError-secSReset);
}


char *EETimer::ctime(time_t timestamp) {
	tm* ptm = gmtime(&timestamp);
	ptm->tm_isdst=0;
	strcpy(EETimer::ctimeBuf,itoa(ptm->tm_year-100,EETimer::ctimeBuf,10));
	strftime(EETimer::ctimeBuf+2,sizeof(EETimer::ctimeBuf)-2,"-%m-%d %X",ptm);
	return EETimer::ctimeBuf;
} 


char *EETimer::ctime() {
	return ctime(dateTime+millis()/1000-secSReset);
} 


uint8_t EETimer::errorNr() {
	return errNr;
}


bool EETimer::full() {
	return bedssize == bedsCapacity;
}


bool EETimer::hasSettedError() {
	return errNr && errHasBeenSet;
}


void EETimer::init() {
	bedssize = 0;
	bedsCapacity = BEDSCOUNT;
}


EETimer & EETimer::instance() {
    static EETimer eETimer;
    return eETimer;
} 


time_t EETimer::now() {
	return dateTime+millis()/1000-secSReset;
}

bool EETimer::parseTimeStr(int16_t timeS[]) {
	return
		   timeS[TI_YY] >20 
		&& timeS[TI_MM] >0
		&& timeS[TI_MM] <13
		&& timeS[TI_DD] > 0
		&& timeS[TI_DD] <= month_length(timeS[TI_YY],timeS[TI_MM])
		&& timeS[TI_HH] < 25
		&& timeS[TI_HH] >= 0
		&& timeS[TI_NN] < 60
		&& timeS[TI_NN] >= 0
		&& timeS[TI_SS] < 60 
		&& timeS[TI_SS] >= 0;
}


uint8_t EETimer::removed(uint8_t id, uint8_t start) {
	assert(id < (1 <<IDBITFIELDS) && start < (1 <<STARTBITFIELDS));
	int8_t ix=index(id,start);
	//cout << "found bed " << (unsigned int)id << " start " << (unsigned int)start << " at index " << (unsigned int)ix << endl;
	if (ix !=  -1) {
		memmove(&bed[ix],&bed[ix+1],(bedssize-- -ix-1)*sizeof(Bed));
		return 1;
	}
	return 0;
}


uint8_t EETimer::removed(uint8_t id) {
	assert(id < (1 <<IDBITFIELDS));
	uint8_t rmCnt=0;
	for (int i=0; i<bedssize; i++)
		if (bed[i].id == id ) {
			memmove(&bed[i],&bed[i+1],(bedssize-- -i-1)*sizeof(Bed));
			i--;
			rmCnt++;
		}
	return rmCnt;
}


void EETimer::replaceUnsetTime(int16_t times[]) {

    struct tm * ptm = gmtime(&dateTime);
	if (times[TI_YY] == -1)
		times[TI_YY] = (int16_t)ptm->tm_year-100;
	if (times[TI_MM] == -1)
		times[TI_MM] = (int16_t)ptm->tm_mon+1;
	if (times[TI_DD] == -1)
		times[TI_DD] = (int16_t)ptm->tm_mday;
	if (times[TI_HH] == -1) {
		times[TI_HH] = (int16_t)ptm->tm_hour;
		//sendf("hour was %d\n",ptm->tm_hour);
	}
	if (times[TI_NN] == -1) {
		times[TI_NN] = (int16_t)ptm->tm_min;
		//sendf("minut was %d\n",ptm->tm_min);
		//sendf("strftime gave %s\n",outbuf);
	}
    if (times[TI_SS] == -1) {
		times[TI_SS] = (int16_t)ptm->tm_sec;
		
	}
}


void EETimer::setError(uint8_t err) {
	errHasBeenSet=true;
	errNr = err;
	millisAtError = millis()/1000;
	writeEEProm();
}


const char* EETimer::setTime(char timestamp[]) {
	const int partCount=6;
	int16_t timeParts[partCount];
	backwardsIntArrayBy2digitsFiller(timestamp,timeParts, partCount);
	//for (int i=0; i<partCount; i++)
	//	sendf("%d,",timeParts[i]);
	//newln; 
	replaceUnsetTime(timeParts);
	//for (int i=0; i<partCount; i++)
	//	sendf("%d,",timeParts[i]);
	//newln;
	if (parseTimeStr(timeParts)) {
		dateTime = timegmY2k(timeParts);
		secSReset = millis()/1000;
		writeEEProm();
		return ctime(dateTime);
	} else
		return const_cast<char*>("parse error");
}


void EETimer::writeEEProm() {
    EEPROMWrite(EETIMER_LOG_ADDRESS,*this);
}


time_t EETimer::timegmY2k(char timeStr[]) { 
	const int partCount=6;
	int16_t timeParts[partCount];
	backwardsIntArrayBy2digitsFiller(timeStr,timeParts, partCount);
	replaceUnsetTime(timeParts);
	//bool parseSucces = parseTimeStr(arg,timeparts);
	if (!parseTimeStr(timeParts))
		return 0;
	return timegmY2k(timeParts);
}


time_t EETimer::timegmY2k(int16_t timeFragments[]) {
	int ycopy = timeFragments[TI_YY];
	int daysE = 365*ycopy;
	if  (daysE) {
		while (ycopy) {
			if (((ycopy-1) % 4) == 0)
				daysE++;
			ycopy--;
		}
	} 
	int16_t mcopy = timeFragments[TI_MM];
	while ( mcopy > 1)
		daysE += month_length(timeFragments[TI_YY],--mcopy);

	//sendf("%d days\n",daysE+timeFragments[TI_DD]-1);
	return (time_t)(
		 (uint32_t)ONE_DAY*(daysE+timeFragments[TI_DD]-1)
		+(uint32_t)ONE_HOUR*timeFragments[TI_HH]
		+60*timeFragments[TI_NN]
		+timeFragments[TI_SS]);
}



/**********************************
            _            _       
 _ __  _ __(_)_   ____ _| |_ ___ 
| '_ \| '__| \ \ / / _` | __/ _ \
| |_) | |  | |\ V / (_| | ||  __/
| .__/|_|  |_| \_/ \__,_|\__\___|
|_|                              
**/

EETimer::EETimer() {
    EEPROMRead<EETimer>(EETIMER_LOG_ADDRESS,*this);
}


int8_t EETimer::index(uint8_t id, uint8_t start) {
		for (int i=0; i <bedssize; i++)
			if (id==bed[i].id && start==bed[i].start)
				return i;
		return -1;
}
	


/**************************************************************************
       _       _           _                   _       _        _   _      
  __ _| | ___ | |__   __ _| |   __ _ _ __   __| |  ___| |_ __ _| |_(_) ___ 
 / _` | |/ _ \| '_ \ / _` | |  / _` | '_ \ / _` | / __| __/ _` | __| |/ __|
| (_| | | (_) | |_) | (_| | | | (_| | | | | (_| | \__ \ || (_| | |_| | (__ 
 \__, |_|\___/|_.__/ \__,_|_|  \__,_|_| |_|\__,_| |___/\__\__,_|\__|_|\___|
 |___/                                                                     
**/

bool EETimer::errHasBeenSet=false;
char EETimer::ctimeBuf[26];

EETimer& eeTimer = EETimer::instance(); 