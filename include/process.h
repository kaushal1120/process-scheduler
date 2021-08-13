using namespace std;
#include <string>

#ifndef PROCESS_H
#define PROCESS_H

class Process{
public:
    int procId;
    int procState;
    int procPriority;
    int procDynPriority;
    int procAt;
    int state_ts;
    int procTotCpuBurst;
    int procTotIoBurst;
    int procCpuBurstSeed;
    int procIoBurstSeed;
    int procRemainingTime;
    int procCpuWaitTime;
    int procRemainingQuantum;
    bool expired;

    Process(int state, int at, int totCpuBurst, int cpuBurstSeed, int ioBurstSeed, int pId, int priority);
};

#endif