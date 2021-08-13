#include <event.h>

Event::Event(Process* process, int timeStamp, int oldState, int transition){
    evtProcess = process;
    evtTimeStamp = timeStamp;
    evtOldState = oldState;
    evtTransition = transition;
}