##### Concepts

A certain standpoint about user interfaces for devices like arduino shall here be preached. Prior in history such things would not have worked without buttons or keyboard as input and some visual output. Today things has a serial connection - wired or air carried. Buttons and screens costs as hardware, design and programming time.


When programming a microprocessor in c/c++, the function is the name of the elementary composed action. Remote procedure calls is calling such a brick remotely.


Such an interface has the benefit of potentially being a brick in a bigger system, where communication is automated, layered or filtered to other concepts made possible by the more powerful connected devices.


A simple scheme is make procedure like commands in linux terminal or windows console. Main functions taken array of strings as parameter and returning an exit level. A configuration can make it choice-able to call an autoexec command after reset. 

##### Nearer implementation

Terminal talking can be done in character (with remote echo) or line unit. Because I chose line, and a simple dump terminal is used, a convenient 'repeat last' command (being a single dot) is implemented.


An arduino solves jobs dealing with reading sensors and controlling attached electronics. That involves doing many things at the same time following some time involving logic.


We use libraries which depends on interrupt. The most simple OS, is avoid interrupt at the level where we ties things together, and just extend the loop with millis() value depend branching in code. 


In CmdLoop, I made a mechanism with slices that have the responsibility to 'pass the baton on' so that next slice gets thread of execution on next millis() value depend invocation in main loop. The slice can adjust the delay formed by the value millis() compares to in the main loop. Slices is an array of function pointers and scheduling goes in ring when each slice calls next() as last statement.


Due to that slices also has prev(), setDelay(uint32_t delay), and state preserving ability of static variables, the slice  can be seen as a virtual instructions of a virtual engine with adjustable clock frequency - loops and conditional selections can be made using those prev() and next() that acts as program counter. 


Serial communication is part of that loop, but where slices is millis() value depend invoked, the branching to instructions of receiving characters of an arriving line is selected by Serial.available(). You could say that the top main loop is program counter constantly tumbling around millis() value and Serial.available() depend branching. 


I have the idea that an Arduino application shall be reset robust - in other words - a reset must have a predicted place in the big run. It has relevance to the way i have implemented error logging.


Error conditions can occur and includes, on an Arduino, all from hardware conditions to programming error. No room for c++ exceptions on a 2k bytes ram device and no user to take verbose action when Arduino controls a mechanical job.


The simplest is a single error with only a number as error cause identification. It can be registered because a timestamp has been sent from terminal and it can not be overwritten before a new timestamp is sent. It survives reset, as opposed to time readout, with an intact time of when the error number was set. It uses 11 bytes of EEPROM and would, if the numbers of registered error events should be more than one, use 4 bytes more for each persistent error.


Some sort of signaling an error must be performed. The 'blink' led on pin13 is dedicated that use. The blink led is given a double information job, by signaling 'all is good' by a slow steady blinking - an indication of that thread of execution travels through those array of function pointers that forms slices.     
