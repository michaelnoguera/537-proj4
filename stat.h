// tracks statistics from simulation

struct stat_t {
    long amu; // average memory utilization
    long arp; // average runable processes
    long tmr; // total memory references
    long tpi; // total page ins: number of page faults
};

// This tick, a hit happened
void Stat_hit();

// This tick, a miss happened
void Stat_miss();

// This tick, everything was blocked, so nothing happened
void Stat_nothing_happened();