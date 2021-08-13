#include <iostream>

#include <scheduler.h>

FCFS_Scheduler::FCFS_Scheduler(bool logflag){
    logFlag = logflag;
}

Process* FCFS_Scheduler::get_next_process(){
    if(logFlag){
        cout<<"SCHED ("<<runQueue.size()<<"): ";
        for(Process* p : runQueue)
            cout<<" "<<p->procId<<":"<<p->state_ts;
        cout<<endl;
    }
    Process* proc = nullptr;
    if(!runQueue.empty()){
        proc = runQueue.front();
        runQueue.pop_front();
    }
    return proc;
}

void FCFS_Scheduler::add_process(Process* process){
    runQueue.push_back(process);
}

bool FCFS_Scheduler::test_preempt(){
    return false;
}

void FCFS_Scheduler::print_name(){
    cout<<"FCFS";
}

LCFS_Scheduler::LCFS_Scheduler(bool logflag){
    logFlag = logflag;
}

Process* LCFS_Scheduler::get_next_process(){
    if(logFlag){
        cout<<"SCHED ("<<runQueue.size()<<"): ";
        for(Process* p : runQueue)
            cout<<" "<<p->procId<<":"<<p->state_ts;
        cout<<endl;
    }
    Process* proc = nullptr;
    if(!runQueue.empty()){
        proc = runQueue.front();
        runQueue.pop_front();
    }
    return proc;
}

void LCFS_Scheduler::add_process(Process* process){
    runQueue.push_front(process);
}

bool LCFS_Scheduler::test_preempt(){
    return false;
}

void LCFS_Scheduler::print_name(){
    cout<<"LCFS";
}

SRTF_Scheduler::SRTF_Scheduler(bool logflag){
    logFlag = logflag;
}

Process* SRTF_Scheduler::get_next_process(){
    if(logFlag){
        cout<<"SCHED ("<<runQueue.size()<<"): ";
        for(Process* p : runQueue)
            cout<<" "<<p->procId<<":"<<p->state_ts;
        cout<<endl;
    }
    Process* proc = nullptr;
    if(!runQueue.empty()){
        proc = runQueue.front();
        runQueue.pop_front();
    }
    return proc;
}

void SRTF_Scheduler::add_process(Process* process){
    list<Process*>::iterator runQIterator = runQueue.begin();
    while(runQIterator != runQueue.end() && (*runQIterator)->procRemainingTime <= process->procRemainingTime)
        runQIterator++;
    if(runQIterator == runQueue.end())
        runQueue.push_back(process);
    else
        runQueue.insert(runQIterator,process);
}

bool SRTF_Scheduler::test_preempt(){
    return false;
}

void SRTF_Scheduler::print_name(){
    cout<<"SRTF";
}

RR_Scheduler::RR_Scheduler(bool logflag){
    logFlag = logflag;
}

Process* RR_Scheduler::get_next_process(){
    if(logFlag){
        cout<<"SCHED ("<<runQueue.size()<<"): ";
        for(Process* p : runQueue)
            cout<<" "<<p->procId<<":"<<p->state_ts;
        cout<<endl;
    }
    Process* proc = nullptr;
    if(!runQueue.empty()){
        proc = runQueue.front();
        runQueue.pop_front();
    }
    return proc;
}

void RR_Scheduler::add_process(Process* process){
    process->procDynPriority = process->procPriority - 1;
    runQueue.push_back(process);
}

bool RR_Scheduler::test_preempt(){
    return false;
}

void RR_Scheduler::print_name(){
    cout<<"RR";
}

PRIO_Scheduler::PRIO_Scheduler(int maxprios, bool logflag){
    list<Process*> initializer;
    for(int i = 0; i < maxprios; i++){
        //push_back makes a copy while pushing.
        mlpRunActQueue.push_back(initializer);
        mlpRunExpQueue.push_back(initializer);
    }
    numActProc = numExpProc = 0;
    logFlag = logflag;
}

Process* PRIO_Scheduler::get_next_process(){
    if(logFlag){
        cout<<"{ ";
        for(int i = 0; i < mlpRunActQueue.size(); i++){
            cout<<"[";
            for(Process* p : mlpRunActQueue[i])
                cout<<p->procId<<",";
            cout<<"]";
        }
        cout<<"} : ";
        cout<<"{ ";
        for(int i = 0; i < mlpRunExpQueue.size(); i++){
            cout<<"[";
            for(Process* p : mlpRunExpQueue[i])
                cout<<p->procId<<",";
            cout<<"]";
        }
        cout<<"} : "<<endl;
    }
    Process* proc = nullptr;

    //Switching queues if active queue becomes empty.
    if(numActProc == 0){
        mlpRunActQueue.swap(mlpRunExpQueue);
        for(int i = 0; i < mlpRunActQueue.size(); i++)
            for(Process* p : mlpRunActQueue[i])
                p->expired = false;
        int temp = numActProc;
        numActProc = numExpProc;
        numExpProc = temp;
        if(logFlag) cout<<"switched queues"<<endl;
    }
    for(int i = 0; i < mlpRunActQueue.size(); i++){
        if(mlpRunActQueue[i].size() > 0){
            proc = mlpRunActQueue[i].front();
            mlpRunActQueue[i].pop_front();
            numActProc--;
            break;
        }
    }
    return proc;
}

void PRIO_Scheduler::add_process(Process* process){
    if(process->expired){
        mlpRunExpQueue[mlpRunExpQueue.size() - process->procDynPriority - 1].push_back(process);
        numExpProc++;
    }
    else{
        mlpRunActQueue[mlpRunActQueue.size() - process->procDynPriority - 1].push_back(process);
        numActProc++;
    }
}

bool PRIO_Scheduler::test_preempt(){
    return false;
}

void PRIO_Scheduler::print_name(){
    cout<<"PRIO";
}

PREPRIO_Scheduler::PREPRIO_Scheduler(int maxprios, bool logflag){
    list<Process*> initializer;
    for(int i = 0; i < maxprios; i++){
        mlpRunActQueue.push_back(initializer);
        mlpRunExpQueue.push_back(initializer);
    }
    numActProc = numExpProc = 0;
    logFlag = logflag;
}

Process* PREPRIO_Scheduler::get_next_process(){
    if(logFlag){
        cout<<"{ ";
        for(int i = 0; i < mlpRunActQueue.size(); i++){
            cout<<"[";
            for(Process* p : mlpRunActQueue[i])
                cout<<p->procId<<",";
            cout<<"]";
        }
        cout<<"} : ";
        cout<<"{ ";
        for(int i = 0; i < mlpRunExpQueue.size(); i++){
            cout<<"[";
            for(Process* p : mlpRunExpQueue[i])
                cout<<p->procId<<",";
            cout<<"]";
        }
        cout<<"} : "<<endl;
    }
    Process* proc = nullptr;

    //Switching queues if active queue becomes empty.
    if(numActProc == 0){
        mlpRunActQueue.swap(mlpRunExpQueue);
        for(int i = 0; i < mlpRunActQueue.size(); i++)
            for(Process* p : mlpRunActQueue[i])
                p->expired = false;
        int temp = numActProc;
        numActProc = numExpProc;
        numExpProc = temp;
        if (logFlag) cout<<"switched queues"<<endl;
    }
    for(int i = 0; i < mlpRunActQueue.size(); i++){
        if(mlpRunActQueue[i].size() > 0){
            proc = mlpRunActQueue[i].front();
            mlpRunActQueue[i].pop_front();
            numActProc--;
            break;
        }
    }
    return proc;
}

void PREPRIO_Scheduler::add_process(Process* process){
    if(process->expired){
        mlpRunExpQueue[mlpRunExpQueue.size() - process->procDynPriority - 1].push_back(process);
        numExpProc++;
    }
    else{
        mlpRunActQueue[mlpRunActQueue.size() - process->procDynPriority - 1].push_back(process);
        numActProc++;
    }
}

bool PREPRIO_Scheduler::test_preempt(){
    return true;
}

void PREPRIO_Scheduler::print_name(){
    cout<<"PREPRIO";
}