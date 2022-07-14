#include <string.h>
#define __ASSERT_USE_STDERR
#include <assert.h>
#include <Arduino.h>
#include "../Libraries/RichEnums/enumpins.h"
#define sendfFlashAlways
#include "../Libraries/Sendf/Sendf.h"


pinProp::pinProp(pin enumVr, const char* name, bool isOutput, uint8_t bedindex) 
    : enumVar(enumVr),name(name),isOutput(isOutput),bedindex(bedindex) {}

#define mkpinProp(enumName,out,bedindex) pin::enumName,#enumName,out,bedindex

pinProp pinArray[] = {
	 {mkpinProp(MURBED		,true ,MURBED   )}
	,{mkpinProp(BEHOLDER	,false,NONAVAIL )}
	,{mkpinProp(BAMBIBED   	,true ,BAMBIBED )}
	,{mkpinProp(TONE		,true ,NONAVAIL )}
	,{mkpinProp(LAUBAER		,true ,LAUBAER	)}
	,{mkpinProp(SPIRE		,false,NONAVAIL )}
	,{mkpinProp(FAN			,true ,NONAVAIL )} 
	,{mkpinProp(HEAT		,true ,NONAVAIL )}
	,{mkpinProp(BLINK		,true ,NONAVAIL )}
};

Pin::Pin(pinProp eProp[]) : size(PINSCOUNT) {
    for (int i=0; i < size; i++)
			pProps[i] = &eProp[i];
} 

uint8_t Pin::pinNr(uint8_t i) {
    return static_cast<uint8_t>(this->pProps[i]->enumVar);

}

void Pin::consoleOut() {
	for (int i=0; i < this->size; i++)
        if (strlen(pProps[i]->name))
            sendf("%s,\t%s\n",pProps[i]->name,pProps[i]->isOutput ? "output" : "input");
    
}

void Pin::setAllOutput(uint8_t level) {
    assert(level == 1 || level == 0);
    for( uint8_t i=0; i< this->size; i++)
        if (this->pProps[i]->isOutput)
            digitalWrite(pinNr(i),level);
}

void Pin::setPinMode(const char* pinStr, uint8_t dir) { 
    assert(dir == INPUT || dir == OUTPUT);
    pinProp * p = const_cast<pinProp *>(this->operator()((pinStr)));
    assert(p != nullptr);
    p->isOutput = dir == OUTPUT ? true : false;
    pinMode(toInt(p->enumVar),p->isOutput);
}

const pinProp* Pin::operator()(const char* req, bool ignorecase) {
    int (*cmp)(const char*, const char*) = ignorecase ? strcasecmp : strcmp;
    for (int i=0; i < size; i++)
        if (cmp(pProps[i]->name,req)==0)
            return pProps[i];
    return nullptr;
}

const pinProp* Pin::operator()(uint8_t enumIntValue) {
    for (int i=0; i < size; i++)
        if (static_cast<uint8_t>(pProps[i]->enumVar) == enumIntValue)
            return pProps[i];
    return nullptr;
}

const pinProp* Pin::ofBedid(uint8_t bedId) {
    for (int i=0; i < size; i++)
        if (pProps[i]->bedindex == bedId)
            return pProps[i];
    return nullptr;
}

void Pin::setPinModes() { 
    for( uint8_t i=0; i< this->size; i++)
        pinMode(pinNr(i),(uint8_t)this->pProps[i]->isOutput);
}

void Pin::out(const char* pinStr, uint8_t level) {
    assert(level == 1 || level == 0);
    const pinProp * p = this->operator()((pinStr));
    assert(p != nullptr && p->isOutput == true);
    digitalWrite(toInt(p->enumVar),level);
}

uint8_t Pin::in(const char* pinStr,bool permitFromOutput) {
    const pinProp * p = this->operator()((pinStr));
    if (!permitFromOutput)
        assert(p != nullptr && p->isOutput == false);
    return digitalRead(toInt(p->enumVar));
}


Pin pins(pinArray);

