#ifndef task_h
#define task_h
#include "taskrunner.h"
// 
//
namespace tasks  { // KEEP ALIGNED AT SOURCE LINE 7
extern float fanSwitchTemp;
extern bool toneOn;
extern bool verboseOn;
extern bool waterOn;








// and keeps emty comments lines4
void onBlinks(TasksCtl & tc);
void fanCtl(TasksCtl & tc);
void waterctl(TasksCtl & tc);
void breakLoop(TasksCtl & tc);
void offBlinks(TasksCtl & tc);
//
extern void (*funcs[5])(TasksCtl &); // line number x ->  x-20
};

#endif