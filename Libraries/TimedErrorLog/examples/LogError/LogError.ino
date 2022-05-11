#include "TimedErrorLog.h"
#include "Sendf.h"

namespace slices {
	uint16_t onTime=2000;
};


void setup() {
	
	Serial.begin(115200);
	sendf("slices::onTime is %d\n",slices::onTime);
	randomSeed(analogRead(0));
	long r[5];
	r[0]=random(60);
	r[1]=random(24);
	r[2]=random(1,28);
	r[3]=random(1,12);
	r[4]=random(42);
	uint32_t tv = 
		long(r[0])
			+r[1]*100L
			+r[2]*10000L
			+r[3]*1000000L
			+r[4]*100000000L;
	sendf("%010lu\n",tv);
	sendf("origin of last boot\n");
	sendf("\tsettedtime\n");
	TimedErrorLog::show(TimedErrorLog::SETTEDTIME);
	sendf("\tnow\n");
	TimedErrorLog::show(TimedErrorLog::NOW);
	if (error)
		sendf("\terror: %d\n",short(error));
	else
		sendf("\tno error\n");
	
	sendf("now setting time to %010lu and delay 3 sec\n\nafter setting\n",tv);
	
	TimedErrorLog::setTime(tv);
	delay(3000);
	sendf("\tsettedtime\n");
	TimedErrorLog::show(TimedErrorLog::SETTEDTIME);
	sendf("\tnow\n");
	TimedErrorLog::show(TimedErrorLog::NOW);
	if (error)
		sendf("\terror: %d\n",short(error));
	else
		sendf("\tno error\n");
	int8_t err = random(1,126);
	sendf("now setting error to: %d\n",err);
	error = err;
	if (error)
		sendf("Error is now %d\n",short(error));
	else
		sendf("no Error\n");
	sendf("trying setting error to: %d\n",err+1);
	error = err+1;
	if (error)
		sendf("Error is now %d\n",short(error));
	else
		sendf("no Error\n");
	
	sendf("slices::onTime is %d\n",slices::onTime);
	
}
	
void loop() { }
