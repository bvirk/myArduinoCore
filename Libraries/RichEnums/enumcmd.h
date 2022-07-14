#include "../ProcListLoop/taskrunner.h"
#include "../ProcListLoop/task.h"
enum class cmd {PINSTAT,ALLOUT, OUT,IN,SETPINMODES,SETPINMODE,PLAY,TIME,ERR,BEDTID,FAN,TONE,VERBOSE};

#define CMDSCOUNT 13

struct cmdProp {
	cmd enumVar;
	const char *name;
	uint8_t (*pCmd)(uint8_t, char*[]);
	cmdProp(cmd enumVar, const char* name, uint8_t (*pCmd)(uint8_t, char*[]));
};

struct Cmd {
	int size;
	const cmdProp * pProps[CMDSCOUNT];
	
	TaskRunner<sizeofArray(tasks::funcs)> taskRunner;
	volatile bool isLooping;
	#define COMMAND_SIZE 64
	char command[COMMAND_SIZE];		//! buffer seriel input as that line that get '\0' bytes to split in args
	#define MAX_ARG_COUNT 6         //! the command name inclusive - the 5' argument will contain remaining with spaces
	uint8_t argc;					//!< detected numbers of arguments
	char *argv[MAX_ARG_COUNT];		//!< array of pointer to '\0' bytes terminatet string(s) in command
	char forRepeatSaved;			//!< A single char in command[0] acts as repeat last, replacing command[0] 
	uint8_t exitLevel;
	
	
	Cmd(cmdProp elems[]);
	
	void runCmd();
	void consoleOut();
	void getCmdAndTimeSlice();
	void loop();
	void loop(const char *line);
	void pinsSafe();
	const cmdProp* operator()(const char* req, bool ignorecase=true);
	
};

