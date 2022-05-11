// This library is free software; you can redistribute it and/or 
// modify it under the terms of the GNU Lesser General Public 
// License as published by the Free Software Foundation; either 
// version 2.1 of the License, or (at your option) any later version.

#include <Arduino.h>
#include "../Libraries/CmdLoop/CmdLoop.h"
#include "../Libraries/Sendf/Sendf.h"
#include <avr/pgmspace.h>
	
const uint32_t defaultDelay=1000;
int8_t pin13State=0;
PGM_P CmdLoop::autoExec;

	

/** 
  * main style arglist and return value 'commands' functions. The functions are meant to be used as remote shell comands, 
  * invoked by named equality of argv[0] 
  */

  CmdLoop::CmdLoop( CommandFuncBox & cmdFuncBox, void (*tslice[])(CmdLoop &), uint8_t tsliceCount 
  	) : cmdFuncs(cmdFuncBox.commands),cmdsCount(cmdFuncBox.size),forRepeatSaved('\0')
  	,sliceDelay(defaultDelay),timeSlices(tslice),timeSlicesCount(tsliceCount),timeSliceIndex(0)
  	{
  	sliceStart = millis();
	argv[0]=command;
	
}

inline void CmdLoop::getCmdAndTimeSlice() {
	uint8_t cmdNPos=0;
	while (true) {
		timeSlice();	
		
		if (Serial.available()) {
			uint8_t inp = Serial.read();
			if (inp != 10 && inp != 13 && cmdNPos < COMMAND_SIZE-1) 
				if (inp != 8)
					command[cmdNPos++] = inp;
				else
					if (cmdNPos)
						cmdNPos--;
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

inline void CmdLoop::timeSlice() {
	uint32_t millisNow = millis();
	if (sliceStart+sliceDelay > millisNow)
		return;
	(timeSlices[timeSliceIndex])(*this);
	sliceStart=millisNow;
}

void CmdLoop::setDelay(uint32_t delay) {
	sliceDelay=delay;
}

void CmdLoop::next() {
	timeSliceIndex++;
	if (timeSliceIndex < timeSlicesCount)
		return;
	timeSliceIndex=0;
}
void CmdLoop::prev() {
	timeSliceIndex--;
	if (timeSliceIndex <= 0)
		return;
	timeSliceIndex=timeSlicesCount-1;
}

void CmdLoop::loop(const __FlashStringHelper *autoEx) {
	CmdLoop::autoExec = reinterpret_cast<PGM_P>(autoEx);
	if (strlen_P(autoExec)) {
		strcpy_P(command,autoExec);
		argc=1;
	}
	while (true) {
		uint8_t cmdsPos;
		for (cmdsPos=0; cmdsPos < cmdsCount; cmdsPos++)
		  if (strcmp_P(argv[0],cmdFuncs[cmdsPos].name)==0) {
		  	  exitLevel = (cmdFuncs[cmdsPos].f)(argc,argv);
		  	  sendf(F(" %d\n"),exitLevel);
		  	  break;
		  }
		if (cmdsPos == cmdsCount && strcmp_P(argv[0],CmdLoop::autoExec))
			sendf(F("Command not found: %s\n"),command );
		getCmdAndTimeSlice();
	}
}

void CmdLoop::loop() { loop(F("")); }