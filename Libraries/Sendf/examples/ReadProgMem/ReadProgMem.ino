#include "Sendf.h"
#include "Utils.h"

const char mes[] PROGMEM = {"Hello world\n"};

const char w0[] PROGMEM = {"frede "};
const char w1[] PROGMEM = {"fart "};
const char w2[] PROGMEM = {"fast\n"};

const char * const ss[] PROGMEM = {w0,w1,w2};

extern unsigned int __data_start;
extern unsigned int __data_end;
extern unsigned int __bss_start;
extern unsigned int __bss_end;

const char s1[] PROGMEM = {"kurt"};
const char s2[] PROGMEM = {"kurtt"};
const char s3[] PROGMEM = {"kur"};
const char s4[] PROGMEM = {"kurs"};
const char s5[] PROGMEM = {"kuru"};
const char s6[] PROGMEM = {"kursa"};
const char s7[] PROGMEM = {"kurub"};

const char *sp[] = {s1,s2,s3,s4,s5,s6,s7};

const char *sa[] = {"kurt","kurtt","kur","kurs","kuru","kursa","kurub"};

void showcmp(const char *  s1, const char *  s2, int8_t (*cmp)(const char * ,const char * ), const char * fname) {
	  
	sendf("%s(\"%s,%s\") = %d\n",fname,s1,s2,(cmp)(s1,s2));
}

void showcmp_P(PGM_P  s1, PGM_P  s2, int8_t (*cmp)(PGM_P ,PGM_P ), const char * fname) {
	  
	sendf("%s(\"",fname);
	sendf(FP(s1));
	sendf("\",\"");
	sendf(FP(s2));
	sendf(") = %d\n",(cmp)(s1,s2));
}




void setup() {
	Serial.begin(115200);
	sendf(FP(mes));
	for (PGM_P s : ss)
		sendf(FP(s));
	sendf(F("\ndata dump\n"));
	Utils::hexdump(reinterpret_cast<const char *>(&__data_start),unsigned(&__data_end)-unsigned(&__data_start));
	sendf(F("\nbss dump\n"));
	Utils::hexdump(reinterpret_cast<const char *>(&__bss_start),unsigned(&__bss_end)-unsigned(&__bss_start));
	
	for (const char * s : sa)
		showcmp(sa[0],s,strcmp,"strcmp  ");
	for (PGM_P s : sp) 
		showcmp_P(sp[0],s,Utils::strcmp_PP,"stcmp_P");

}
void loop() { }

