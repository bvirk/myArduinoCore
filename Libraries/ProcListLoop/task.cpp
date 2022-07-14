#include "../Libraries/ProcListLoop/task.h"
#include <Arduino.h>
#define sendfFlashAlways
#include "../Libraries/Sendf/Sendf.h"
#include "../Libraries/EEPromObj/eetimer.h"
#include "../Libraries/RichEnums/enumpins.h"
#include "../Libraries/DS18b20/ds18b20.h"

extern Pin pins;
extern EETimer& eeTimer;

namespace tasks  {

uint16_t onTime=50;         //!< steady blinking on startup
uint16_t offTime=500;
float fanSwitchTemp = 30.0;
bool toneOn=true;
bool verboseOn=false;


//  _          _                    __                  _   _                 
// | |__   ___| |_ __   ___ _ __   / _|_   _ _ __   ___| |_(_) ___  _ __  ___ 
// | '_ \ / _ \ | '_ \ / _ \ '__| | |_| | | | '_ \ / __| __| |/ _ \| '_ \/ __|
// | | | |  __/ | |_) |  __/ |    |  _| |_| | | | | (__| |_| | (_) | | | \__ \.
// |_| |_|\___|_| .__/ \___|_|    |_|  \__,_|_| |_|\___|\__|_|\___/|_| |_|___/
//              |_|                                                           


void tell(const char* fmt=nullptr, const char* mes=nullptr, uint16_t freq=3500, uint16_t dur=100) {
    if (mes != nullptr &&  verboseOn) {
        char buf[64];
        snprintf(buf,63,fmt,mes);
        sendf("%s: %s\n",eeTimer.ctime(),buf);
    }
    if (toneOn)
        tone(toInt(pin::TONE),freq,dur);
}





//                 _   _            _        
//  _ __ ___  __ _| | | |_ __ _ ___| | _____ 
// | '__/ _ \/ _` | | | __/ _` / __| |/ / __|
// | | |  __/ (_| | | | || (_| \__ \   <\__ 
// |_|  \___|\__,_|_|  \__\__,_|___/_|\_\___/


void onBlinks(TasksCtl & tc) {
    #define countdownstart 20
    static uint8_t cntDown=countdownstart;        		// 1.
    tc.permitAgainDelay = onTime;
    if (!cntDown-- ) { 		// 3.
        digitalWrite(13,!digitalRead(13));
        //sendf("%ld, 13=%hu not %hu\n",millis(),!digitalRead(13),digitalRead(13));
        cntDown = countdownstart;                 		// 6.   
    }
    tc.next();
    
}

void fanCtl(TasksCtl & tc) {
    static int8_t cd=40;
    if (!cd--) {
        cd=40;
        float t = tempC(AIR_SERIAL);
        if (pinIn(FAN)) { // FAN is on
            if (t  < fanSwitchTemp) {
                pinOut(FAN,0);
                tell();
            }
        } else
            if (t > fanSwitchTemp) {
                pinOut(FAN,1);
                tell();
            }
    }
    tc.next();
}

void waterctl(TasksCtl & tc) {
    uint8_t bs = eeTimer.bedsSize();
    if (bs) {
        static uint8_t i=0;
        static uint8_t advance=1;
        Bed b = eeTimer.bedRef(i);
        
        const char *name = pins.ofBedid(b.id)->name;
        uint8_t pinNr = toInt(pins.ofBedid(b.id)->enumVar);
        time_t now = eeTimer.now();
        time_t start = eeTimer.atHour(b.start+6,b.id);
        time_t stop = eeTimer.atHour(b.start+6,b.id)+60*(b.period+1);
        
        //if (verboseOn)
        //    sendf("%ld\n%ld\n%ld\n%d\n\n",start,now,stop,i);
        
        
        if (digitalRead(pinNr)) { // is on
            if ( now > stop ) {
                digitalWrite(pinNr,0);
                tell("pin %s off",name);
                advance=1;
            }
        } else {
            if ( now > start && now < stop) {
                digitalWrite(pinNr,1);
                tell("pin %s on",name);
                advance=0;
            } else
                advance=1; 
                

        }
        i += advance;
        if (i==bs)
            i=0;
    }
    tc.next();
}


void breakLoop(TasksCtl & tc) {
    if ( pinIn(BEHOLDER) == 0)
        eeTimer.setError(2);
    tc.next();
}

void offBlinks(TasksCtl & tc) {
    tc.next();
}

void (*funcs[])(TasksCtl &)={onBlinks,fanCtl,waterctl,breakLoop,offBlinks};

};

