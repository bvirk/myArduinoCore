#include "../Libraries/Sendf/Sendf.h"
#include "../Libraries/ProcListLoop/taskrunner.h"
#include "../Libraries/ProcListLoop/task.h"
#include <Arduino.h>

TasksCtl::TasksCtl(int s) : current(0), size(s), permitAgainDelay(500), lastTaskFinishedTime(0) {}
void TasksCtl::next() {
    current++;
    if (current == size)
        current=0;
}


template< int taskCount>
TaskRunner<taskCount>::TaskRunner(void (*task[])(TasksCtl &)) : tc(taskCount) {
    for (int i=0; i < tc.size; i++)
        this->task[i] = task[i];
    
}

template< int taskCount>
void TaskRunner<taskCount>::run() {
    if (tc.lastTaskFinishedTime+tc.permitAgainDelay > millis())
		return;
    (this->task[tc.current])(tc);
    tc.lastTaskFinishedTime = millis();
     
}


#define sizeofArray(x) (sizeof(x)/sizeof(x[0]))

void templateinstantiateensurer() {
    TaskRunner<sizeofArray(tasks::funcs)> taskrunner(tasks::funcs);
    taskrunner.run();
}