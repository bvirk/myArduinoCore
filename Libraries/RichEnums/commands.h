#include <Arduino.h>

namespace cmdFuncs {
uint8_t pinstat(uint8_t argc, char *argv[]);
uint8_t setAllOut(uint8_t argc, char *argv[]);
uint8_t setOut(uint8_t argc, char *argv[]);
uint8_t getIn(uint8_t argc, char *argv[]);
uint8_t setPinModes(uint8_t argc, char *argv[]);
uint8_t setPinMode(uint8_t argc, char *argv[]);
uint8_t play(uint8_t argc, char *argv[]);
const char * bedname(uint8_t bedid);
uint8_t bedtid(uint8_t argc, char *argv[]);
uint8_t setTime(uint8_t argc, char *argv[]);
uint8_t setError(uint8_t argc, char *argv[]);
uint8_t fan(uint8_t argc, char *argv[]);
uint8_t tone(uint8_t argc, char *argv[]);
uint8_t verbose(uint8_t argc, char *argv[]);
};