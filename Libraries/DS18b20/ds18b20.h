#define EARTH_SERIAL 0x8102162CBF22EE28
#define AIR_SERIAL   0x36021616B726EE28

void initTemperatureSensing();
float tempC(uint64_t addr);