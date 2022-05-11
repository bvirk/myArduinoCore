#include "Sendf.h"

void setup() {
	Serial.begin(115200);
	sendf(F("Serial.print\tsendff\t\tprecision\n"));
	sendf(F("------------------------------------------------\n"));
	int maxlen = -1;
	char fmt[] = "%.pf";
	for (int i=30; i; i--) {
        float f = ((float)(500000-(random() % 1000001)))/500000.0*pow(10,(rand() % 21)-10);
        uint8_t preci=rand() % 8;
        Serial.print(f,preci);
        sendf("\t\t");
        fmt[2] = char(preci+48);
        int8_t len = sendff(fmt,f);
        sendf(F("\t\t(len=%d presision=%d)\n"),len,preci);
        if (len > maxlen) 
            maxlen=len;
    }
    sendf(("\nmaximal length of sendff: %d\n"),maxlen);
}

void loop() { }