#include <iostream>
#include <unistd.h>
using namespace std;
#include <list>
#include <fstream>
#include <vector>
#include <iomanip>
#include <cstring>

#include <deslayer.h>
#include <scheduler.h>

#define CREATED 0
#define READY 1
#define RUNNING 2
#define BLOCKED 3
#define PREEMPT 4

#define TRANS_TO_READY 0
#define TRANS_TO_RUN 1
#define TRANS_TO_BLOCK 2
#define TRANS_TO_PREEMPT 3

#define DEFAULT_QUANTUM 10000
#define DEFAULT_MAXPRIOS 4

class Sched{
private:
    DESLayer* desLayer;
    Scheduler* scheduler;
    vector<Process*> processes;
    bool vFlag, eFlag;
    int ofs;
    int quantum;
    int CURRENT_TIME;
    double ioUtilTime;
    int lastIoEndTime;
    vector<int> randvals;
    string STATE_MAP[5] = {"CREATED","READY", "RUNNG", "BLOCK", "PREEMPT"};
    string TRANS_MAP[4] = {"READY","RUNNG", "BLOCK", "PREEMPT"};
public:
    Sched(char* filename, char* randfile, bool vflag, bool eflag, bool tflag, char sflag, int timequantum, int maxprios){
        vFlag = vflag;
        eFlag = eflag;
        quantum = timequantum;
        switch(sflag){
            case 'F':
                scheduler = new FCFS_Scheduler(tflag);
                break;
            case 'L':
                scheduler = new LCFS_Scheduler(tflag);
                break;
            case 'S':
                scheduler = new SRTF_Scheduler(tflag);
                break;
            case 'R':
                scheduler = new RR_Scheduler(tflag);
                break;
            case 'P':
                scheduler = new PRIO_Scheduler(maxprios,tflag);
                break;
            case 'E':
                scheduler = new PREPRIO_Scheduler(maxprios,tflag);
                break;
            default:
                scheduler = new FCFS_Scheduler(tflag);
                break;
        }

        ifstream rand_stream;
        rand_stream.open(randfile);
        if(!rand_stream){
            cerr<<"Not a valid random file <" << (randfile ? randfile : "null") << ">" <<endl;
            exit(1);
        }
        int number;
        rand_stream >> number;
        while (rand_stream >> number) randvals.push_back(number);
        rand_stream.close();
        ofs=0;

        CURRENT_TIME = 0;
        ioUtilTime = 0;
        lastIoEndTime = 0;

        ifstream f_stream;
        f_stream.open(filename);
        if(!f_stream){
            cerr<<"Not a valid inputfile <" << (filename ? filename : "null") << ">" <<endl;
            exit(1);
        }

        desLayer = new DESLayer();

        int at, tc, cb, io;
        f_stream >> at;
        int pId = 0;
        while(!f_stream.eof()){
            f_stream >> tc >> cb >> io;
            //For each input line create a process object, a process-create event and enter the event into the event queue.
            Process* process = new Process(pId++,CREATED,getRandom(maxprios),at,tc,cb,io);
            processes.push_back(process);
            desLayer->put_event(new Event(process, at, CREATED, TRANS_TO_READY));
            f_stream >> at;
        }

        if(eFlag){
            cout<<"ShowEventQ:";
            desLayer->show_event_queue(0);
            cout<<endl;
        }
        f_stream.close();
    }

    int getRandom(int burst) {
        int rand = 1 + (randvals[ofs++] % burst);
        //Wrapping around
        if(ofs == randvals.size()) ofs=0;
        return rand;
    }

    void simulation() {
        Event* evt;
        int ioBurst, cpuBurst;
        bool CALL_SCHEDULER = false;
        Process* CURRENT_RUNNING_PROCESS = nullptr;
        int timeInPrevState = 0;
        bool preemptFlag;
        int nextEventTime;
        while(evt = desLayer->get_event()) {
            Process *proc = evt->evtProcess; // this is the process the event works on
            CURRENT_TIME = evt->evtTimeStamp; // time jumps discretely
            timeInPrevState = CURRENT_TIME - proc->state_ts; // good for accounting
            switch(evt->evtTransition) { // which state to transition to?
                case TRANS_TO_READY:
                    // must come from BLOCKED or from PREEMPTION
                    // must add to run queue
                    if(vFlag){
                        cout<<CURRENT_TIME<<" "<<proc->procId<<" "<<timeInPrevState<<": "<<STATE_MAP[proc->procState]<<" -> "<<STATE_MAP[READY];
                        if(proc->procState == RUNNING)
                            cout<<"  cb="<<proc->procRemainingQuantum<<" rem="<<proc->procRemainingTime<<" prio="<<proc->procDynPriority;
                        cout<<endl;
                    }
                    //Adding to run queue
                    scheduler->add_process(proc);
                    proc->state_ts = CURRENT_TIME;
                    proc->procState = READY;

                    //If a preemptive scheduler is being used and there is a process to preempt
                    if(CURRENT_RUNNING_PROCESS != nullptr && scheduler->test_preempt()){
                        nextEventTime = desLayer->get_next_event_time(CURRENT_RUNNING_PROCESS); //get time of currently running process being blocked
                        preemptFlag = (CURRENT_RUNNING_PROCESS->procDynPriority < proc->procDynPriority) & (nextEventTime != CURRENT_TIME);
                        if(vFlag){
                            cout<<"---> PRIO preemption "<<CURRENT_RUNNING_PROCESS->procId<<" by "<< proc->procId << " ? " << proc->procPriority<<proc->procDynPriority<<CURRENT_RUNNING_PROCESS->procPriority<<CURRENT_RUNNING_PROCESS->procDynPriority;
                            cout<<" TS="<<nextEventTime<<" now="<<CURRENT_TIME<<") --> "<<(preemptFlag ? "YES" : "NO")<<endl;
                        }
                        if(preemptFlag){
                            if(eFlag){
                                cout<<"RemoveEvent("<<CURRENT_RUNNING_PROCESS->procId<<"):";
                                desLayer->show_event_queue(0);
                                cout<<" ==> ";
                            }
                            //Remove next block event for currently running process as it is being preempted now
                            desLayer->rm_event(CURRENT_RUNNING_PROCESS);
                            if(eFlag){
                                desLayer->show_event_queue(1);
                                cout<<endl;
                                cout<<"  AddEvent("<<CURRENT_TIME<<":"<<CURRENT_RUNNING_PROCESS->procId<<":"<<STATE_MAP[PREEMPT]<<"):";
                                desLayer->show_event_queue(1);
                                cout<<" ==> ";
                            }
                            //Add a preempt event for the currently running process at current time
                            desLayer->put_event(new Event(CURRENT_RUNNING_PROCESS,CURRENT_TIME,RUNNING,TRANS_TO_PREEMPT));
                            if(eFlag){
                                desLayer->show_event_queue(1);
                                cout<<endl;
                            }
                            //Update time spent doing cpu
                            CURRENT_RUNNING_PROCESS->procRemainingQuantum += nextEventTime - CURRENT_TIME;
                            CURRENT_RUNNING_PROCESS->procRemainingTime += nextEventTime - CURRENT_TIME;
                        }
                    }
                    CALL_SCHEDULER = true; // conditional on whether something is run
                    break;
                case TRANS_TO_RUN:
                    // create event for either preemption or blocking
                    if(proc->procRemainingQuantum > 0)
                        cpuBurst = proc->procRemainingQuantum;
                    else
                        cpuBurst = getRandom(proc->procCpuBurstSeed);
                    cpuBurst = proc->procRemainingTime > cpuBurst ? cpuBurst : proc->procRemainingTime;

                    if(vFlag) cout<<CURRENT_TIME<<" "<<proc->procId<<" "<<timeInPrevState<<": "<<STATE_MAP[proc->procState]<<" -> "<<STATE_MAP[RUNNING]<<" cb="<<cpuBurst<<" rem="<<proc->procRemainingTime<<" prio="<<proc->procDynPriority<<endl;

                    proc->procCpuWaitTime += timeInPrevState;
                    proc->state_ts = CURRENT_TIME;
                    proc->procState = RUNNING;
                    if(cpuBurst <= quantum){ //Allotted cpu burst over. Go for IO
                        if(eFlag){
                            cout<<"  AddEvent("<<CURRENT_TIME + cpuBurst<<":"<<CURRENT_RUNNING_PROCESS->procId<<":"<<STATE_MAP[BLOCKED]<<"):";
                            desLayer->show_event_queue(1);
                            cout<<" ==> ";
                        }
                        desLayer->put_event(new Event(proc,CURRENT_TIME + cpuBurst,RUNNING,TRANS_TO_BLOCK));
                        proc->procRemainingQuantum = 0;
                        proc->procRemainingTime = proc->procRemainingTime - cpuBurst;
                    }
                    else{ //Quantum is complete. Prempt and wait for next turn for cpu
                        if(eFlag){
                            cout<<"  AddEvent("<<CURRENT_TIME + quantum<<":"<<CURRENT_RUNNING_PROCESS->procId<<":"<<STATE_MAP[PREEMPT]<<"):";
                            desLayer->show_event_queue(1);
                            cout<<" ==> ";
                        }
                        desLayer->put_event(new Event(proc,CURRENT_TIME + quantum,RUNNING,TRANS_TO_PREEMPT));
                        proc->procRemainingQuantum = cpuBurst - quantum;
                        proc->procRemainingTime = proc->procRemainingTime > quantum ? proc->procRemainingTime - quantum : 0;
                    }
                    if(eFlag){
                        desLayer->show_event_queue(1);
                        cout<<endl;
                    }
                    break;
                case TRANS_TO_BLOCK:                    
                    //create an event for when process becomes READY again
                    proc->state_ts = CURRENT_TIME;
                    proc->procDynPriority = proc->procPriority-1;
                    if(proc->procRemainingTime > 0){
                        ioBurst = getRandom(proc->procIoBurstSeed);
                        if (vFlag) cout<<CURRENT_TIME<<" "<<proc->procId<<" "<<timeInPrevState<<": "<<STATE_MAP[proc->procState]<<" -> "<<STATE_MAP[BLOCKED]<<"  ib="<<ioBurst<<" rem="<<proc->procRemainingTime<<endl;
                        proc->procTotIoBurst += ioBurst;
                        proc->procState = BLOCKED;
                        //Track total time spent doing IO skipping overlapping IO
                        if(CURRENT_TIME > lastIoEndTime)
                            ioUtilTime += ioBurst;
                        else
                            if(CURRENT_TIME + ioBurst > lastIoEndTime)
                                ioUtilTime += CURRENT_TIME + ioBurst - lastIoEndTime;
                        lastIoEndTime = lastIoEndTime > CURRENT_TIME + ioBurst ? lastIoEndTime : CURRENT_TIME + ioBurst;
                        if(eFlag){
                            cout<<"  AddEvent("<<CURRENT_TIME + ioBurst<<":"<<CURRENT_RUNNING_PROCESS->procId<<":"<<STATE_MAP[READY]<<"):";
                            desLayer->show_event_queue(1);
                            cout<<" ==> ";
                        }
                        desLayer->put_event(new Event(proc,CURRENT_TIME + ioBurst,BLOCKED,TRANS_TO_READY));
                        if(eFlag){
                            desLayer->show_event_queue(1);
                            cout<<endl;
                        }
                    }
                    else
                        if(vFlag) cout<<CURRENT_TIME<<" "<<proc->procId<<" "<<timeInPrevState<<": "<<"Done"<<endl;
                    CURRENT_RUNNING_PROCESS = nullptr;
                    CALL_SCHEDULER = true;
                    break;
                case TRANS_TO_PREEMPT:
                    // add to runqueue (no event is generated)
                    if(vFlag){
                        cout<<CURRENT_TIME<<" "<<proc->procId<<" "<<timeInPrevState<<": "<<STATE_MAP[RUNNING]<<" -> "<<STATE_MAP[READY];
                        cout<<"  cb="<<proc->procRemainingQuantum<<" rem="<<proc->procRemainingTime<<" prio="<<proc->procDynPriority;
                        cout<<endl;
                    }
                    if(--proc->procDynPriority == -1){
                        proc->procDynPriority = proc->procPriority-1;
                        proc->expired=true;
                    }
                    scheduler->add_process(proc);
                    proc->state_ts = CURRENT_TIME;
                    proc->procState = READY;
                    CURRENT_RUNNING_PROCESS = nullptr;
                    CALL_SCHEDULER = true;
                    break;
            }
            // remove current event object from Memory
            delete evt;
            evt = nullptr;

            if(CALL_SCHEDULER) {
                if (desLayer->get_next_event_time() == CURRENT_TIME)
                    continue; //process next event from Event queue
                CALL_SCHEDULER = false; // reset global flag
                if (CURRENT_RUNNING_PROCESS == nullptr) { //Ignore if a process is already running
                    CURRENT_RUNNING_PROCESS = scheduler->get_next_process();
                    if (CURRENT_RUNNING_PROCESS == nullptr) //No processes to run
                        continue;
                    //create event to make processes runnable for some time.
                    if(eFlag){
                        cout<<"  AddEvent("<<CURRENT_TIME<<":"<<CURRENT_RUNNING_PROCESS->procId<<":"<<STATE_MAP[RUNNING]<<"):";
                        desLayer->show_event_queue(1);
                        cout<<" ==> ";
                    }
                    desLayer->put_event(new Event(CURRENT_RUNNING_PROCESS,CURRENT_TIME,CURRENT_RUNNING_PROCESS->procState,TRANS_TO_RUN));
                    if(eFlag){
                        desLayer->show_event_queue(1);
                        cout<<endl;
                    }
                }
            }
        }
    }

    void printSummary(){
        scheduler->print_name();
        if(quantum != DEFAULT_QUANTUM) cout<<" "<<quantum;
        cout<<endl;
        double cpuUtilTime=0;
        double totTurnarndTime=0;
        double totWaitTime=0;
        for(Process* p : processes){
            cpuUtilTime += p->procTotCpuBurst;
            totTurnarndTime += p->state_ts - p->procAt;
            totWaitTime += p->procCpuWaitTime;
            cout<<setfill('0')<<setw(4)<<p->procId<<": ";
            cout<<setfill(' ')<<setw(4)<<p->procAt<<" "<<setw(4)<<p->procTotCpuBurst<<" "<<setw(4)<<p->procCpuBurstSeed<<" "<<setw(4)<<p->procIoBurstSeed<<" "<<setw(1)<<p->procPriority;
            cout<<" | "<<setw(5)<<p->state_ts<<" "<<setw(5)<<p->state_ts-p->procAt<<" "<<setw(5)<<p->procTotIoBurst<<" "<<setw(5)<<p->procCpuWaitTime<<endl;
        }
        cout<<"SUM: "<<fixed<<setprecision(2)<<CURRENT_TIME<<" "<<100*(cpuUtilTime/(double)CURRENT_TIME)<<" "<<100*(ioUtilTime/(double)CURRENT_TIME)<<" "<<totTurnarndTime/(double)processes.size()<<" "<<totWaitTime/(double)processes.size()<<" ";
        cout<<fixed<<setprecision(3)<<100*((double)processes.size()/(double)CURRENT_TIME)<<endl;
    }
};

int main (int argc, char* argv[]) {
    int vFlag = 0;
    int tFlag = 0;
    int eFlag = 0;
    char* sArg = nullptr;
    char schedAlgo = 'F';
    char* quantumStr = nullptr;
    int quantum;
    char* prioStr = nullptr;
    int maxPrios = DEFAULT_MAXPRIOS;
    char c;

    opterr = 0;

    while ((c = getopt(argc, argv, "vtes:")) != -1){
        switch (c) {
            case 'v':
                vFlag = 1;
                break;
            case 't':
                tFlag = 1;
                break;
            case 'e':
                eFlag = 1;
                break;
            case 's':
                sArg = (char*) malloc(strlen(optarg) + 1);
                strcpy(sArg, optarg);
                sArg = strtok(sArg,":");
                schedAlgo = sArg[0];
                if(schedAlgo != 'F' && schedAlgo != 'L' && schedAlgo != 'S' && schedAlgo != 'R' && schedAlgo != 'P' && schedAlgo != 'E'){
                    cerr << "Unknown Scheduler spec: -v {FLSRPE}" << endl;
                    return 1;
                }
                else if(schedAlgo == 'F' || schedAlgo == 'L' || schedAlgo == 'S')
                    quantum = DEFAULT_QUANTUM;
                else{
                    quantumStr = sArg+1;
                    quantum = atoi(quantumStr);
                    if(!quantumStr || (quantum == 0 && strcmp(quantumStr,"0") != 0)){
                        cerr << "Invalid scheduler param <" <<optarg<< ">"<< endl;
                        return 1;
                    }
                    prioStr = strtok(NULL, ":");
                    maxPrios = prioStr ? atoi(prioStr) : DEFAULT_MAXPRIOS;
                    if((schedAlgo == 'P' || schedAlgo == 'E') && (optarg[strlen(optarg)-1] == ':' || (maxPrios == 0 && strcmp(prioStr,"0") != 0))){
                        cerr << "Invalid scheduler param <" <<optarg<< ">"<< endl;
                        return 1;
                    }
                }
                break;
            case '?':
                if (optopt == 's')
                    cerr << "Unknown Scheduler spec: -v {FLSRPE}" << endl;
                else if (isprint (optopt)){
                    cerr << argv[0] << ": invalid option -- '" << (char) optopt << "'" << endl;
                    cerr << "Usage: " << argv[0] << " [-v] inputfile randomfile" << endl;
                }
                else{
                    cerr << argv[0] << ": invalid option -- '" << std::hex << optopt << "'" << endl;
                    cerr << "Usage: " << argv[0] << " [-v] inputfile randomfile" << endl;
                }
                return 1;
            default:
                abort();
        }
    }
    Sched* sched = new Sched(optind < argc ? argv[optind++] : 0, optind < argc ? argv[optind] : 0, vFlag, eFlag & vFlag, tFlag & vFlag, schedAlgo, quantum, maxPrios);
    sched->simulation();
    sched->printSummary();
    return 0;
}