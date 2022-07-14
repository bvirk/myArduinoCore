#ifndef taskrunner_h
#define taskrunner_h
#include <stdint.h>

struct TasksCtl {
    int current;
    int size;
    uint32_t permitAgainDelay;
    uint32_t lastTaskFinishedTime;
    TasksCtl(int size);
    void next();
};


template< int taskCount>
struct TaskRunner {
    TasksCtl tc;
    void (*task[taskCount])(TasksCtl &);
    
    //TaskRunner(void (*slicefuncs[])(TasksCtl &),TasksCtl &d);
    TaskRunner(void (*task[])(TasksCtl &));
    void run();
};



#endif