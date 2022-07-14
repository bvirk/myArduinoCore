#ifndef eetimer_h
#define eetimer_h
#include <time.h>
#define EETIMER_LOG_ADDRESS 0

#define IDBITFIELDS 2 
#define STARTBITFIELDS 4 
#define PERIODBITFIELDS 2 

struct Bed {
	uint8_t id : IDBITFIELDS;
	uint8_t start :STARTBITFIELDS;
	uint8_t period :PERIODBITFIELDS;
};

#define BEDSCOUNT 32


class EETimer {

	enum timeIndex {TI_YY,TI_MM,TI_DD,TI_HH,TI_NN,TI_SS};

    uint32_t secSReset; 	//! seconds sinse reset, gets set on time setting. 
	time_t dateTime; 		//! seconds in y2k epoch
	uint32_t millisAtError;	//! seconds adjustment to secSReset that indicates time of error.
    uint8_t errNr;			//! setted by setError and cleared by setting time
    
    uint8_t bedssize;
	int8_t bedsCapacity;
	Bed bed[BEDSCOUNT];
	
    static char ctimeBuf[26];
    
    /**
     * singleton use
     **/
    EETimer(); 
    

        /**
     * index of a bed with a iven starttime
     * 
     * @param id of a bed
     * @param starttime is an index to one of 16 starttimes
     * @return index or -1 if the given bed,start don't exists
     **/
    int8_t index(uint8_t id, uint8_t start);

public:
//              _     _ _      
//  _ __  _   _| |__ | (_) ___ 
// | '_ \| | | | '_ \| | |/ __|
// | |_) | |_| | |_) | | | (__ 
// | .__/ \__,_|_.__/|_|_|\___|
// |_|                         


    static bool errHasBeenSet; // intial false to indicate obj has not been set to another value 
                               // than initial EEProm read
    
    /**
     * @param id of a bed
     * @param starttime is an index to one of 16 starttimes
     * @param period is an index for number some timeunit 
     * @return number of item added to list
     **/
    uint8_t added(uint8_t id, uint8_t start, uint8_t period);


    /**
     *  @param hour at the day
     *  @param displacement in multiplum of 270 secs
     *  @return time of at a gien hour at current date
     **/
    time_t atHour(uint8_t hour, uint8_t displacement); 


    /** 
     * writing limiting conversion of string to int array
	 *  accepts:
	 *  	2204251212 	-> [22,04,25,12,12]
	 *  	12         	-> [-1,-1,-1,-1,12]
	 *  	9.12			-> [-1,-1,-1,9,12]
	 *  and folish
	 *  	113.12		-> [-1,-1,1,13,12]
	 **/	
    void backwardsIntArrayBy2digitsFiller(char str[], int groups[], int groupLength);
 
    uint8_t bedsSize();
    Bed &bedRef(uint8_t bedsNr);

    /**
     *  @return time of error setted
     **/
    const char* cerrortime(); 
	
    
    /**
     *   @return
     **/
    char *ctime(time_t timestamp);


    /**
     *   @return
     **/
    char *ctime();
    

    /**
     * @return errNr
     **/
    uint8_t errorNr();


	/**
     * @return lists of beds full
     **/
    bool full();


    /**
     * @return true if both errNr <> 0 and errHasBeenSet is true. After a boot errNr could be 
     * different from zero both will be false;
     **/
    bool hasSettedError();


    /**
     * @return
     **/
    void init();
    

    /**
     * @return singleton object reference
     **/
    static EETimer & instance();
    
    
    /**
     * @return current time
     **/
    time_t now();
    
    
    /**
     * @param timeS[6] is [yy,mm,dd,hh,mm,ss], where yy is in y2k epoch
     * @return is true for a valid point in time
     */
    bool parseTimeStr(int16_t timeS[]);


    /**
     * @return number of removed beds starting at start
     */
    uint8_t removed(uint8_t id, uint8_t start);
	
    
    /**
     * 
     * @return number of removed beds
     */
    uint8_t removed(uint8_t id);
	

    /** 
     * replace missing point in time parts with those read from EEProp on instatiation
	 *  
	 * only values of -1 is replaced.
	 *
	 * @param times filled with yy,mm,dd,hh,nn,ss in index 0,1,2,3,4,5
	 **/
    void replaceUnsetTime(int16_t times[]);


    /**
     *  
     **/
    void setError(uint8_t err);


    /**
     * Construct TimedErrorLog object from a timestamp string an EEPROM persitent it.
     *
     * @param timestamp of form [[[[yy.[mm].dd].]hh].nn] ss where non given default to prior 
     *   and optional '.' can be any non digits
     * @return is true on succesfull time set by a valid input
	 **/


    /**
     *
     **/
    const char * setTime(char timestamp[]);


    /** 
     * epoch y2k second  from timespamp string
     * 
	 * @param timesStr of form [[[[yy.9mm.]dd.]hh.]nn.]ss where non given default to prior and '.' can be any non digits
     * @return seconds since 2000.01.01 00:00:00
     **/	
    time_t timegmY2k(char timeStr[]);


    /**
     * valid int[6] array
     * @return second since 2000.01.01 00:00 
	 **/ 
    time_t timegmY2k(int16_t timeFragments[]);


  	/**
     * experimental boot construction
     *
     **/
    void writeEEProm();

};


#endif