/**
 * CS 537 Programming Assignment 4 (Fall 2020)
 * @file stat.c
 * @brief Tracks statistics from simulation
 * @author Julien de Castelnau
 */

#include "stat.h"
#include "memory.h"
#include "process.h"
#include <stdio.h>

static struct stat_t* ProgStats;

// Initialize stat structure
void Stat_init() {
    ProgStats = (struct stat_t*)malloc(sizeof(struct stat_t));
    if ((ProgStats) == NULL) {
        perror("Error allocating memory for stats data structure.");
        exit(EXIT_FAILURE);
    }
    ProgStats->tmu = 0;
    ProgStats->trp = 0;
    ProgStats->tmr = 0;
    ProgStats->tpi = 0;
}

// No matter what happens this tick, this will still be called and still applies.
void Stat_default(unsigned long numTicks) {
    ProgStats->tmu += Memory_howManyAllocPages() * numTicks;
    // ONLY ONE PROCESS CAN BE IN THE 'RUNNING' QUEUE AT A TIME!
    ProgStats->trp += (ProcessQueue_numWaitingProcs() + 
                    ((int)Process_existsWithStatus(RUNNING))) * numTicks;
}

// This tick, a hit happened
void Stat_hit() {
    ProgStats->tmr += 1;
}

// This tick, a miss happened
void Stat_miss() {
    ProgStats->tpi += 1;
}

// Print the stats out directly, given an end time for the program.
void Stat_printStats(unsigned long time) {
    printf("%ld", ProgStats->tmu);
    float amu = ProgStats->tmu / time;
    float arp = ProgStats->trp / time;

    printf("Program stats:\n");
    printf("AMU: %f (%f%% of total memory)\n", amu, (amu/Memory_getTotalSize()) * 100);
    printf("ARP: %f\n", arp);
    printf("TMR: %lu\n", ProgStats->tmr);
    printf("TPI: %lu\n", ProgStats->tpi);
}