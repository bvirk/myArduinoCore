#ifndef newenumspin_h
#define newenumspin_h

//enum class pin {MURBED=5,BEHOLDER,BAMBIBED,TONE,LAUBAER,SPIRE,FAN,HEAT,BLINK};
enum class pin {MURBED=10,BEHOLDER=6,BAMBIBED,TONE,LAUBAER,SPIRE=6,FAN=11,HEAT,BLINK};
enum pintobedindex {MURBED,BAMBIBED,LAUBAER,NONAVAIL};
#define sizeofArray(x) (sizeof(x)/sizeof(x[0]))
#define PINSCOUNT 9


#define toInt(enumVar) static_cast<int>(enumVar)
#define pinOut(x,level) digitalWrite(toInt(pin::x),level)  
#define pinsOut(x,level) ((pins.out(x,level)))
#define pinIn(x) digitalRead(toInt(pin::x))
#define pinsIn(x) pins.in(x,false)
#define pinsInFromOutput(x) pins.in(x)


struct pinProp {
	pin enumVar;
    const char *name;  
	bool isOutput;
	uint8_t bedindex;
    pinProp(pin enumVar, const char* name, bool isOutput,uint8_t bedindex);
	
};

struct Pin {
	uint8_t size;
	const pinProp * pProps[PINSCOUNT];
	
    Pin(pinProp elems[]);
	uint8_t pinNr(uint8_t i);
	void consoleOut();
	void setAllOutput(uint8_t level);
    void setBedPins(uint8_t level);
	void setPinMode(const char* pinStr, uint8_t dir);
	void setPinModes();
	const pinProp* operator()(const char* req, bool ignorecase=true);
	const pinProp* operator()(uint8_t enumIntValue);
	const pinProp* ofBedid(uint8_t bedId);
	void out(const char* pinStr, uint8_t level);
	uint8_t in(const char* pinStr,bool permitFromOutput=true);
};



#endif