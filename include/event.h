#include <process.h>

class Event {
public:
    Process* evtProcess;
    int evtTimeStamp;
    int evtOldState;
    int evtTransition;

    Event(Process* process, int timeStamp, int oldState, int transition);
};