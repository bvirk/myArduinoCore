//! Command functions
/*!
  Syntax of each function is in CommandFuncBox::CommandFuncBox() below
  Macro NUM_COMMANDS has to be manually edited when inserting and removing commands.
  Having autoExec sending some info and being the last in CommandFuncBox::CommandFuncBox() 
  shows visual on start in terminal that NUM_COMMANDS made room for all initialized CommandFunc objects.
  If it is too small, memory will be corrupted - if it is too big, that amount time 4 bytes is wasted in bss segment.
*/
namespace cmdFuncs {

	int8_t showTime(int argc, char* argv[]) {
		switch(	argc > 1 ? argv[1][0] & 0x5f : 'S' ) {
			case 'N':
				TimedErrorLog::show(TimedErrorLog::NOW);
				break;
			case 'E':
				TimedErrorLog::show(TimedErrorLog::ERROR);
				break;
			case 'S':
			default:
				TimedErrorLog::show(TimedErrorLog::SETTEDTIME);
		} 
		return 0;
	}

	int8_t setTime(int argc, char* argv[]) {
		if ( argc >=2 ) {
			TimedErrorLog::setTime(strtol(argv[1],0,10));
			TimedErrorLog::show(TimedErrorLog::NOW);
			return 0;
		}
		return -1;
	}
	
	
	int8_t setError(int argc, char* argv[]) {
		if (argc > 1 ) { 
			error = atoi(argv[1]);
			TimedErrorLog::show(TimedErrorLog::ERROR);
			return 0;
		}
		return -1;
	}

	int8_t r8(int argc, char* argv[]) {
		if (argc >=2) {
			uint16_t address=strtol(argv[1],0,16);
			uint8_t b = eeprom_read_byte(address);
			sendf("*%03x=%02x\n",address,b);
			return 0;
		} 
		return -1;
	}

	int8_t r16(int argc, char* argv[]) {
		if (argc >=2) {
			uint16_t address=strtol(argv[1],0,16);
			uint16_t w = EEPROMValue<uint16_t>(address);
			sendf("*%03x=%04x\n",address,w);
			return 0;
		} 
		return -1;
	}
	
	int8_t w8(int argc, char* argv[]) {
		if (argc >=3 ) {
			uint16_t address=strtol(argv[1],0,16);
			uint8_t value = strtol(argv[2],0,16);
			EEPROMWrite(address,value);
			r8(argc,argv);
			return 0;
		} 
		return -1;
	}

	int8_t w16(int argc, char* argv[]) {
		if (argc >=3 ) {
			uint16_t address=strtol(argv[1],0,16);
			uint16_t value = strtol(argv[2],0,16);
			EEPROMWrite(address,value);
			r16(argc,argv);
			return 0;
		} 
		return -1;
	}
		
	int8_t EEDump(int argc, char* argv[]) {
		uint16_t start = 0;
		uint16_t len = 0x400;
		if (argc > 1)
			start = strtol(argv[1],0,16);
		if (argc > 2)
			len = strtol(argv[2],0,16);
		utils::eepromHexdump(start,len);
		return 0;
	}

	int8_t autoexec(int argc, char* argv[]) {
		if (error)
			TimedErrorLog::show(TimedErrorLog::ERROR);
		else {
			TimedErrorLog::show(TimedErrorLog::SETTEDTIME);
		}
		sendf(F("Set time using:\n    settime yymmddhhnn\n"));
		sendf(F("Use list to see all commands\n"));
		return 0;
	}

	int8_t schnellStart(int argc, char* argv[]) {
		sendf("Schnell start\n");	
		
		return 0;
	}
};

//! EDIT THIS WHEN INSERTING/REMOVING COMMANDS - ONE MORE THEN LAST INDEX OF PUTF
#define NUM_COMMANDS 13

CommandFunc CommandFuncBox::commands[NUM_COMMANDS];

namespace cmdFuncs {
	
	int8_t  list(int argc, char *argv[]) {
  	  sendf(F("Available commands:\n\n"));
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

	
	PUTF(0) ver				,F("ver"
		  
							//! demo of printing arguments
	)) PUTF(1) printArgs,		F("printargs" // [arg [...]]
	
							//! reads 8 bit unit from EEPROM - address in hex
	)) PUTF(2) r8				,F("r8" // address

							//! writes 8 bit unit to EEPROM - address and value in hex
	)) PUTF(3) w8				,F("w8" // address value

							//! reads 16 bit unit from EEPROM - address in hex	
	)) PUTF(4) r16				,F("r16" // address
		
							//! writes 16 bit unit to EEPROM - address and value in hex
	)) PUTF(5) w16				,F("w16" // address value
		
							//! heximal dump of, if no arguments given, entire, EEPROM. address and length in hex 
	)) PUTF(6) EEDump			,F("eedump" // [address [length]]
		
							//! puts a 32 bit timestamp in EEPROM. The timestamp is an integer which digits is yymmddhhnn
							//! eg. 2005121330 for time of day 13:30 on maj the 12' year 2020. 
	)) PUTF(7) setTime			,F("settime" // timestamp
		
							//! shows timestamp in EEPROM. The showing, when optional argument now is used, is, 
							//! for obvious reasons, not true after a reset.	
	)) PUTF(8) showTime		,F("showtime" // [now]
		
							//! shows all commands
	)) PUTF(9) list			,F("list" // value		

							//! sets an error value in range {-128 ... 127}
	)) PUTF(10) setError		,F("seterror" // value
		
							//! For testing
	)) PUTF(11) schnellStart	,F("ss"  		

	// KEEPS AUTOEXEC AS LAST    
							//! auto executed on startup.
	)) PUTF(12) autoexec		,F("autoexec" 		
	));
}



CmdLoop cmdLoop( 
	 CommandFuncBox::instance()
	,slices::slicefuncs,sizeof(slices::slicefuncs)/sizeof(slices::slicefuncs[0])
	);

void setup() {
	Serial.begin(115200);
	pinMode(LED_BUILTIN,OUTPUT);
	cmdLoop.loop(F("autoexec"));
}

void loop() { }