#ifndef TimedErrorLog_h
#define TimedErrorLog_h

// This library is free software; you can redistribute it and/or 
// modify it under the terms of the GNU Lesser General Public 
// License as published by the Free Software Foundation; either 
// version 2.1 of the License, or (at your option) any later version.

#define TIMEDERRORLOGLIBVERSION "0.0.1"

#include <stdint-gcc.h>
#include <Print.h>
#include "../Templates/Templates.h"

#define TIMED_ERROR_LOG_ADDRESS 0


enum class alarm9Run: uint8_t {INVALID,OFF,BEFORE_START,ON};
extern enumStr<alarm9Run> alarm9RunList[];

enum class alarm9Repeat: uint8_t {INVALID,NONE,MINUT,HOUR,DAY};
extern enumStr<alarm9Repeat> alarm9RepeatList[];

enum class pinmode: uint8_t {INVALID,PM_INPUT,PM_OUTPUT};
extern enumStr<pinmode> pinmodeList[];


#define enumVar(enumClass, var) findEnum< enumClass ,enumStr< enumClass >>(#var,enumClass##List)
#define enumVarByStr(enumClass, var) findEnum< enumClass ,enumStr< enumClass >>(var,enumClass##List)
#define enumUInt32(enumClass, var) findEnumUInt32< enumClass ,enumStr< enumClass >>(var,enumClass##List)
#define enumStr(enumClass, var) findEnumStr< enumClass ,enumStr< enumClass >>(var,enumClass##List)


/**
 *  Byond reset, one item error log.
 *  A singleton which state is EEPROMED
 Setting a time:
	datetime AND secSReset is set
	errNr=0
	EEPromWrite
 Retrieving a time
	datetime + millis()/1000 - secSReset
 Setting And error
     errNr=error
	 secAtError = millis()/1000 - secSReset
	 EEPromWrite
 Retrieving an error time
	datetime+secAtError

  
  The saved time denoted error consisting of a single 8 bit number.
  The used 11 bytes EEPROM memory is the real object reestablished by default construction when arduino is booted. 
  An error number can only be set one time and survives reset. 
  To enable being able to recieve an error number, the time must be set.
  
  */
class TimedErrorLog {
	uint32_t secSReset; 	//!< seconds sinse reset, gets set on time setting. 
	uint32_t dateTime; 			//!< seconds in y2k epoch
	uint32_t secAtError;	//!< seconds adjustment to secSReset that indicates time of error.
	int8_t errNr;				//!< setted by setError and cleared by setting time
	
	enum timeIndex {TI_YY,TI_MM,TI_DD,TI_HH,TI_NN};
	
  	
    TimedErrorLog();
public: 	
	static bool hasBeenSet;
	enum showmode {SETTEDTIME,NOW,ERROR};

	/** Singleton that invokes constucters reading object from EPROM
	*/ static TimedErrorLog & instance();

	/**
	*/ uint32_t now();
  	
	/** get the error number using short(singleton error object)
  	*/ operator short() const;
  	
  	/* get info about a present error usinf if(singleton error object)
  	*/ operator bool() const;

  	/** Assign an error
	*/	int8_t operator = (short);

	/** Presents time_t as string, prefixed with labl
  	*/ void sendlabeledTime(uint32_t timestamp, char* format);

	/**  Construct TimedErrorLog object from a timestamp string an EEPROM persitent it.
      
      @param timestamp of form [yy.[mm.[dd.[hh]]]]:nn where non given default to prior and '.' can be any non digits
	
	*/ bool setTime(char timestamp[]);

	/** Shows error object - if no error just current time.
	    @param showNum is one off above enums
  	  - SETTEDTIME 
  	  - NOW
  	  - ERROR
  	  and reflected respectively time of construction, current time and time of logged error
	  parameters not used any more
  	*/ bool show(showmode showNum);
	
	/** epoch y2k second  from timespamp string

	  @param timestamp of form [yy.[mm.[dd.[hh]]]]:nn where non given default to prior and '.' can be any non digits
	
	*/	uint32_t timegmY2k(char timeStr[]);

private:  	

	 /** writing limiting conversion of string to int array
	  accepts:
	  	2204251212 	-> [22,04,25,12,12]
	  	12         	-> [-1,-1,-1,-1,12]
	  	9.12			-> [-1,-1,-1,9,12]
	  and folish
	  	113.12		-> [-1,-1,1,13,12]
	*/	void backwardsIntArrayBy2digitsFiller(char str[], int groups[], int groupLength);

	/** 
	*/ uint8_t monthLen(uint8_t year, uint8_t month);

  	/** Parses time and set array times
  		
  		@param timeStr is eg. 'yy.mm.dd.hh.nn' delimiter '.' is optional and must just be non digit(s)
  				only requirement is exact five groups of two digits.
  		@param times is set to times[YY],times[MM],times[DD],times[HH],times[NN] on succesfull parsing.
  		@return true on success
  	*/	bool parseTimeStr(int timeS[]);

	/** Based og datetime of object, conditional fill array
	   
	  Only prior values of -1 is replaced.
	 
	  @param times filled with yy,mm,dd,hh,nn in index 0,1,2,3,4

	*/	void replaceUnsetTime(int times[]);
  	
	/** second since 2000.01.01 00:00 
	*/ uint32_t timegmY2k(int timeFragments[]);
  	
};

#endif

