#include <deslayer.h>
#include <iostream>

Event* DESLayer::get_event(){
    Event* event = eventQueue.front();
    eventQueue.pop_front();
    return event;
}

void DESLayer::put_event(Event* event){
    list<Event*>::iterator evtIterator = eventQueue.begin();
    while(evtIterator != eventQueue.end() && (*evtIterator)->evtTimeStamp <= event->evtTimeStamp) evtIterator++;
    if(evtIterator==eventQueue.end())
        eventQueue.push_back(event);
    else
        eventQueue.insert(evtIterator,event);
}

void DESLayer::rm_event(Process* p){
    list<Event*>::iterator evtIterator = eventQueue.begin();
    while(evtIterator != eventQueue.end() && (*evtIterator)->evtProcess != p) evtIterator++;
    if(evtIterator != eventQueue.end())
        eventQueue.erase(evtIterator);
}

void DESLayer::show_event_queue(bool showTransition){
    for(auto v : eventQueue){
        cout<<"  "<<v->evtTimeStamp<<":"<<v->evtProcess->procId;
        if(showTransition) cout<<":"<<TRANS_MAP[v->evtTransition];
    }
}

/**
Gets time of the impending event in the event queue.
**/
int DESLayer::get_next_event_time(){
    if(eventQueue.size() > 0)
        return eventQueue.front()->evtTimeStamp;
    return -1;
}

/**
Gets next event time for the process passed as argument.
**/
int DESLayer::get_next_event_time(Process* p){
    list<Event*>::iterator evtIterator = eventQueue.begin();
    while(evtIterator != eventQueue.end() && (*evtIterator)->evtProcess != p)
        evtIterator++;
    if(evtIterator == eventQueue.end())
        return -1;
    return (*evtIterator)->evtTimeStamp;
}