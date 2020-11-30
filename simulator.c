/**
 * CS 537 Programming Assignment 4 (Fall 2020)
 * @file simulator.c
 * @brief Core module that coordinates the simulation
 * @author Michael Noguera
 */

#include "simulator.h"
#include <assert.h>

enum {
    CLOCK_TICK = 1,
    DISK_PENALTY = 2000,
};

static_assert(CLOCK_TICK > 0, "Clock tick must be greater than zero.");
static_assert(DISK_PENALTY > 0, "Disk penalty must be greater than zero.");

void Simulator_runSimulation(FILE* tracefile) {
    bool finished = false;
    unsigned int time = 0; // time in nanoseconds

    // unsigned long currentline = 1;

    while (!finished) {
        time += CLOCK_TICK;
        printf("Tick! time=%i\n", time);
        // Advance disk wait counter if needed
        if (Process_existsWithStatus(BLOCKED)) {
            printf("Checking on blocked processes.\n");
            if (--(Process_peek(BLOCKED)->waitCounter) == 0) {
                Process_switchStatus(RUNNING, WAITING); // i/o completed
                Process_switchStatus(BLOCKED, RUNNING);
            }
        }
        // If there isn't a current process, choose the next one
        if (!Process_existsWithStatus(RUNNING)) {
            printf("Searching for next process to run.\n");
            if (Process_existsWithStatus(WAITING)) { // 1. resume interrupted?
                Process_switchStatus(WAITING, RUNNING);
            } else if (Process_existsWithStatus(NOTSTARTED)) { // 2. new proc.
                Process_switchStatus(NOTSTARTED, RUNNING);
            } else if (Process_existsWithStatus(BLOCKED)) {
                continue; // all blocked, short-circuit to next cycle
            } else {
                assert(!Process_existsWithStatus(RUNNING));
                assert(!Process_existsWithStatus(WAITING));
                assert(!Process_existsWithStatus(BLOCKED));
                assert(!Process_existsWithStatus(NOTSTARTED));
                return; // nothing left to do, we're done here
            }
        }

        printf("Run process with pid=%lu\n", Process_peek(RUNNING)->pid);

        // Get the next read for that process
        // How to search interval tree?
    }
}