#include <event.h>
using namespace std;
#include <list>

class DESLayer {
private:
    list<Event*> eventQueue;
    string TRANS_MAP[4] = {"READY","RUNNG", "BLOCK", "PREEMPT"};
public:
    Event* get_event();

    void put_event(Event* event);

    void rm_event(Process* p);

    void show_event_queue(bool showTransition);

    /**
    Gets time of the impending event in the event queue.
    **/
    int get_next_event_time();

    /**
    Gets next event time for the process passed as argument.
    **/
    int get_next_event_time(Process* p);
};