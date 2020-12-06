/**
 * CS 537 Programming Assignment 4 (Fall 2020)
 * @file stat.c
 * @brief Tracks statistics from simulation
 * @author Julien de Castelnau and Michael Noguera
 */

// The tmu and trp fields get converted into the correct amu and arp fields at the
// point of the program exit, where they are divided by the total time.
struct stat_t {
    unsigned long tmu; // total memory utilization (compounding)
    unsigned long trp; // total runable processes (compounding)
    unsigned long tmr; // total memory references
    unsigned long tpi; // total page ins: number of page faults
};

// Initialize stat structure
void Stat_init();

// No matter what happens this tick, this will still be called and still applies.
void Stat_default(unsigned long numTicks);

// This tick, a hit happened
void Stat_hit();

// This tick, a miss happened
void Stat_miss();

// Print the stats out directly, given an end time for the program.
void Stat_printStats(unsigned long time);