#include <string.h>
#define __ASSERT_USE_STDERR
#include <assert.h>
#include <Arduino.h>

#include "../Libraries/RichEnums/enumpins.h"
#include "../Libraries/RichEnums/enumcmd.h"
#include "../Libraries/RichEnums/commands.h"
#define sendfFlashAlways
#include "../Libraries/Sendf/Sendf.h"
#include "../Libraries/Utils/strtokext.h"
#include "../Libraries/EEPromObj/eetimer.h"



extern Pin pins;
extern EETimer& eeTimer;


cmdProp::cmdProp(cmd enumVar, const char* name, uint8_t (*pCmd)(uint8_t, char*[])) 
	: enumVar(enumVar),name(name),pCmd(pCmd)  {}


cmdProp cmdArray[] = {              
	 {cmd::PINSTAT		,"pinstat"		,cmdFuncs::pinstat	    }
	,{cmd::ALLOUT		,"allout"		,cmdFuncs::setAllOut	}
	,{cmd::OUT			,"out"			,cmdFuncs::setOut		}
	,{cmd::IN			,"in"			,cmdFuncs::getIn		}
	,{cmd::SETPINMODES  ,"setpinmodes"	,cmdFuncs::setPinModes  }
	,{cmd::SETPINMODE	,"setpinmode"	,cmdFuncs::setPinMode   }
	,{cmd::PLAY			,"p"			,cmdFuncs::play		    }
	,{cmd::TIME			,"time"			,cmdFuncs::setTime	    }
	,{cmd::ERR			,"err"			,cmdFuncs::setError	    }
	,{cmd::FAN  		,"fan"			,cmdFuncs::fan       	}
    ,{cmd::BEDTID		,"bed"			,cmdFuncs::bedtid		}
    ,{cmd::TONE		    ,"tone"			,cmdFuncs::tone	    	}
    ,{cmd::VERBOSE		,"verbose"		,cmdFuncs::verbose		}
};
	

Cmd::Cmd(cmdProp eProps[]) : size(CMDSCOUNT), taskRunner(tasks::funcs),isLooping(true)  {
	assert(sizeofArray(cmdArray) == CMDSCOUNT);
    for (int i=0; i < size; i++)
		pProps[i] = &eProps[i];
    argv[0]=command;
}

void Cmd::consoleOut() {
    for (int i=0; i < size; i++)
        sln(pProps[i]->name);
}

void Cmd::getCmdAndTimeSlice() {
    uint8_t cmdNPos=0;
    while (isLooping) {
        if (eeTimer.hasSettedError()) {
            pinsSafe();
            isLooping=false;
            sendf("Error %u at %s\n",eeTimer.errorNr(),eeTimer.cerrortime());
            break;
        }

        taskRunner.run();	
        
        if (Serial.available()) {
            uint8_t inp = Serial.read();
            if (inp != 10 && inp != 13 && cmdNPos < COMMAND_SIZE-1) {
                if (inp != 8) 
                    command[cmdNPos++] = inp;
                else 
                    if (cmdNPos)
                        cmdNPos--;
            }
            if ( inp == 10 || inp == 13 || cmdNPos == COMMAND_SIZE-1) {
                // we got line with a command with possible arguments
                // cmdNPos is its length
                if ( forRepeatSaved && cmdNPos == 1 && command[0] == '.') {
                    command[0]=forRepeatSaved;
                    return;
                } else { // not a repeat command or no prior command[0] saved  	
                    command[cmdNPos] = '\0';
                    argc = 1;
                    for (uint8_t cmdPos=0; cmdPos < cmdNPos; cmdPos++)
                        if (command[cmdPos] == ' ')
                            command[cmdPos] = '\0';
                        else
                            if (cmdPos && command[cmdPos-1] == '\0' ) {
                                argv[argc++] = &command[cmdPos];
                                if (argc == MAX_ARG_COUNT)
                                    break; // for loop
                            }
                } // end else no repeat comannd
                if (cmdNPos > 1) 
                    forRepeatSaved = command[0];
                return;
            } // lineshift or buffer full
        } // serial.available
    } // end while
}    

void Cmd::runCmd() {
    if (strcmp(argv[0],"list")==0)
        consoleOut();
    else {
        const cmdProp * item = operator()(argv[0],true);
        if (item) {
            exitLevel = (item->pCmd)(argc,argv); 
            if (exitLevel)
                sendf("exitlevel=%d\n",exitLevel);
        } else
            soln( What? );
    }
}

void Cmd::loop() {
    while (isLooping) {
        getCmdAndTimeSlice();
        if (isLooping)
            runCmd();		
    }
}

const cmdProp* Cmd::operator()(const char* req, bool ignorecase) {
    int (*cmp)(const char*,const char*) = ignorecase ? strcasecmp : strcmp;
    for (int i=0; i < size; i++)
        if (cmp(pProps[i]->name,req)==0)
            return pProps[i];
    return nullptr;
}


void Cmd::loop(const char *line) {
    if (strlen(line) < COMMAND_SIZE ) {
        strcpy(command,line);
        if((argc=splittedCount(command," ")) > MAX_ARG_COUNT)
            argc = MAX_ARG_COUNT;
        catchSplits(argv, command, argc,true);
        runCmd();
    }
    loop();
}

void Cmd::pinsSafe() {
    pins.setAllOutput(0);
}

Cmd cmds(cmdArray);
