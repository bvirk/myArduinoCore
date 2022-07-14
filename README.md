## What is it?
Timed pin level activation that draws relais. Mechanism for falling out of loop on error condition and readout the time on next boot.


## Style of coding
A principle I have called rich enum is used. 
It is 
```
enum class
```
embedded in list of objects embedded in futher one object

The two object having a list of objects each having an enum and som related properties, is

- enumpins
- enumcmd

representing the i/o pins on the arduino uno and the commands invokeable from serial connection. 

Despite being c++, a very c way programming style is used. Its a principle to avoid any heap usage in running code. Such an arduino uno is meant to run forever in contrast to PC's, and the few 30% of 2k bytes ram free could soon be eated by a heap being corrupted by the moving down stack pointer.

Inital heap use would be ok, but I newer used any in this project.

I have not looked deep in the string heap usage many online examples and free source uses, instead i wrote my own sendf(F(string),...) which together with some really homebrewed macroes  offers lot more condensed expressivity.  
Loves the most ugly one.  

file:Sendf.h
```
int8_t sendDirect(const __FlashStringHelper *);
#define soln(x,...) sendDirect(F(#x #__VA_ARGS__ "\n"))
```

in code
```
soln( in whi,st, solo nolo rocks);
```

## template workarounds
I wasn't able to link tree level, in each other specified, template parameters, being shareable among translation units.
Well, i havn't the time to test blow code size anyway and settled for some ugly constants. Nothing works if manual typed given sizes for enumpins and enumcmd  are not correct - funny how bufferoverflow in translation unit not even used can hang an arduino uno.



