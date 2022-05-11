//! Graft - the CmdLoop using Graftplatform

/** The place for
  growing the libraries used in, but not exclusive for, CmdLoop.
  growing CmdLoop itself

ino files

	Graft.ino
		This info about the ino files
	
	Graft1-grows.ino
		Where classes start their life before going in libraries
	
	Graft2-slices.ino
		The functionality of business logic about electronic input output besides serial communication
	
	Graft3-cmdFuncs-Basic.ino
		Some basic commands that, in their simplicity, show what CmdLoop commands is. 
	
	GraftApp.ino
		The commands used in this CmdLoop application
	
*/

#include <CmdLoop.h>
#include <Sendf.h>
#include <EEPROM.h>
#include "TimedErrorLog.h"
#include <avr/pgmspace.h>
#include "Templates.h"
#include "Utils.h"

