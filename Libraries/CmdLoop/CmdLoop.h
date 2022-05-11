#ifndef CmdLoop_h
#define CmdLoop_h

// This library is free software; you can redistribute it and/or 
// modify it under the terms of the GNU Lesser General Public 
// License as published by the Free Software Foundation; either 
// version 2.1 of the License, or (at your option) any later version.

#include <WString.h>

#define CMDLOOPLIBVERSION "0.0.3"

//! Pairs a function pointer and the name of the command.
/*!
  Due to absence of ability to used flashed strings in a brace initialization list, objects has to be initialized
*/
struct CommandFunc {
		int8_t (*f)(int,char**);
		PGM_P name;
		
		//! makes it possible to make an array of bogus content objects.
		CommandFunc() {}
		
		//! Fills object with content
		/*!
		  \param fp is pointer to function taken parameters int, char** and returning an int8_t
		  \param pName point to a compiled flashed string
		*/
		void init(int8_t (*fp)(int,char**), const __FlashStringHelper *pName) {
			f=fp;
			name = reinterpret_cast<PGM_P>(pName);
		}
};


//! Initialize and refers to array of CommandFunc objects
struct CommandFuncBox {
	static CommandFuncBox & instance() {	
		static CommandFuncBox cmdBox;
		return  cmdBox;
	}
	static CommandFunc commands[];
	uint8_t size;
private:	
	CommandFuncBox();
};


//! Combined remote command shell and timesliced execution of functions.
/*!
  Commands with their optional space delimited arguments are called over serial connection. Every command is a function in namespace cmdFuncs, a function which parameter and return value is like int main(int argc, char**argv) in the standard C desktop platform. 
  Namespace cmdFuncs contains a list of CommandFuncs that relates names to commands.
  The command's name and arguments is recieved in a buffer in which spaces are replaced with '\0', and char * to each argument are putted in array of pointer to array of chars prior use as argument for calling the function named by first string in the buffer.
  A mechnism for saving first char additional as forRepeatSaved makes a shortcut (a single dot) for repeating previous command with all parameters.
  The  command loop is optional initiated with the name of the autoexec command, a function which purpose is final setup. 
*/  
class CmdLoop {
	#define COMMAND_SIZE 64
	char command[COMMAND_SIZE];		//! buffer seriel input as that line that get '\0' bytes to split in args
	#define MAX_ARG_COUNT 6         //! the command name inclusive - the 5' argument will contain remaining with spaces
	
	char forRepeatSaved;			//!< A single char in command[0] acts as repeat last, replacing command[0] 
									//!< with the prior saved command[0] as forRepeatSaved
	                                   
	uint8_t argc;					//!< detected numbers of arguments
	char *argv[MAX_ARG_COUNT];		//!< array of pointer to '\0' bytes terminatet string(s) in command
	CommandFunc *cmdFuncs;			//!< pointer to first item in array of struct CommandFunc's
	uint8_t cmdsCount;              //!< number of CommandFunc objects   
	int8_t exitLevel;				//!< exitlevel of last executed command
	                                   
	
	void (**timeSlices)(CmdLoop &);	//!< Array of pointers to timeSlice functions
	uint8_t timeSlicesCount;        //!< Number of timeSlice functions    
	int8_t timeSliceIndex;			//!< index  of the timeSlice that will get thread of execution next time
	uint32_t sliceStart;    		//!< milliseconds sinse boot of last slice sceduling 
	uint32_t sliceDelay;			//!< delay before timeslice() is called again
	
	//! runs timeslice() and read from serial if characters of a line is avail
	void getCmdAndTimeSlice();
	
	
	
	//! schedules a slice on delay elapsed.
	/*!
	  the slice is responsible for calling next() for running next slice on next scheduling
	*/  
	void timeSlice();
  
  public:
  	//! Makes a CmdLoop object
  	/*!
  	  \param cmdFuncbox refers, through a cmdFuncbox, to initialized array of CommandFunc object and their number
  	  \param (*tslice[]) array of functions to be scheduled by timeSlice(); 
  	  \param tsliceCount of slices
  	*/
  	CmdLoop( CommandFuncBox &  cmdFuncbox
  			,void (*tslice[])(CmdLoop &), uint8_t tsliceCount
  			);
  	
  	//! Infinite loop of running getCmdAndTimeSlice();
  	/*!
  	  \param optional autoExec is put in command buffer for first execution. 
  	*/
  	void loop(const __FlashStringHelper *autoExec);
  	void loop();
  	
  	//! Sets delay until next slice running
  	
  	//! Delay before timeslice runs 
  	/*!
  	  \param delay in milliseconds timeslice gets thread of execution
  	*/
  	void setDelay(uint32_t delay);
  	
  	//! schedule next timeslice.
  	/*!
  	  Handle over being the next timeslice to run, to the timeslice following the current.
  	  It's a programming error if a timeslice unconditionally not calls any of next() or prev()
  	  Such error will block slicing as the current slice will get thead og execution on every scheduling
  	*/
  	void next();
  	
  	//! makes  previous slice the one that is called next.
  	void prev();
  	
  	static PGM_P autoExec;					//!< name of autoexec function

  	
};
	
#endif