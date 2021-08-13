#include <process.h>

Process::Process(int pId, int state, int priority, int at, int totCpuBurst, int cpuBurst, int ioBurst){
    procId = pId;
    procState = state;
    procPriority = priority;
    procDynPriority = priority-1;
    procAt = at;
    state_ts = at;
    procTotCpuBurst = totCpuBurst;
    procTotIoBurst = 0;
    procCpuBurstSeed = cpuBurst;
    procIoBurstSeed = ioBurst;
    procRemainingTime = totCpuBurst;
    procCpuWaitTime = 0;
    procRemainingQuantum = 0;
    expired = false;
}