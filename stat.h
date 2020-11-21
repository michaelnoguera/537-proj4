// tracks statistics from simulation

struct stat_t {
    long amu; // average memory utilization
    long arp; // average runable processes
    long tmr; // total memory references
    long tpi; // total page ins: number of page faults
};

void stat_update(); // TODO decide on interface