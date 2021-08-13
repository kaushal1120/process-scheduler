using namespace std;
#include <list>
#include <vector>
#include <process.h>

class Scheduler {
public:
    list<Process*> runQueue;
    bool logFlag;
    virtual Process* get_next_process() = 0;
    virtual void add_process(Process* process) = 0;
    virtual bool test_preempt() = 0;
    virtual void print_name() = 0;
};

class FCFS_Scheduler : public Scheduler {
public:
    FCFS_Scheduler(bool logflag);

    Process* get_next_process();

    void add_process(Process* process);

    void print_name();

    bool test_preempt();
};

class LCFS_Scheduler : public Scheduler {
public:
    LCFS_Scheduler(bool logflag);

    Process* get_next_process();

    void add_process(Process* process);

    void print_name();

    bool test_preempt();
};

class SRTF_Scheduler : public Scheduler {
public:
    SRTF_Scheduler(bool logflag);

    Process* get_next_process();

    void add_process(Process* process);

    void print_name();

    bool test_preempt();
};

class RR_Scheduler : public Scheduler {
public:
    RR_Scheduler(bool logflag);

    Process* get_next_process();

    void add_process(Process* process);

    void print_name();

    bool test_preempt();
};

class PRIO_Scheduler : public Scheduler {
private:
    vector<list<Process*>> mlpRunActQueue;
    vector<list<Process*>> mlpRunExpQueue;
    int numActProc;
    int numExpProc;
public:
    PRIO_Scheduler(int maxprios, bool logflag);

    Process* get_next_process();

    void add_process(Process* process);

    void print_name();

    bool test_preempt();
};

class PREPRIO_Scheduler : public Scheduler {
private:
    vector<list<Process*>> mlpRunActQueue;
    vector<list<Process*>> mlpRunExpQueue;
    int numActProc;
    int numExpProc;
public:
    PREPRIO_Scheduler(int maxprios, bool logflag);

    Process* get_next_process();

    void add_process(Process* process);

    void print_name();

    bool test_preempt();
};