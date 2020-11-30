/**
 * CS 537 Programming Assignment 4 (Fall 2020)
 * @file simulator.c
 * @brief Core module that coordinates the simulation
 * @author Michael Noguera
 */

#include "simulator.h"
#include "intervaltree.h"
#include <assert.h>

enum {
    CLOCK_TICK = 1,
    DISK_PENALTY = 2000,
};

static_assert(CLOCK_TICK > 0, "Clock tick must be greater than zero.");
static_assert(DISK_PENALTY > 0, "Disk penalty must be greater than zero.");

static inline bool notDone() {
    return Process_existsWithStatus(RUNNING)
           || Process_existsWithStatus(WAITING)
           || Process_existsWithStatus(BLOCKED)
           || Process_existsWithStatus(NOTSTARTED);
}

void Simulator_runSimulation(FILE* tracefile) {
    unsigned int time = 0; // time in nanoseconds

    // unsigned long currentline = 1;
    assert(tracefile != NULL && "tracefile can't be null");

    while (notDone()) {
        // 0. Account for clock tick
        time += CLOCK_TICK;
        printf("Tick! time=%i\n", time);

        // 1. Advance disk wait counter if needed
        if (Process_existsWithStatus(BLOCKED)) {
            printf("%s\n", "Checking on blocked processes.");
            if (--(Process_peek(BLOCKED)->waitCounter) == 0) {
                Process_switchStatus(RUNNING, WAITING); // i/o completed
                Process_switchStatus(BLOCKED, RUNNING);
            }
        }

        // 2. If there isn't a current process, choose the next one
        if (!Process_existsWithStatus(RUNNING)) {
            printf("%s\n", "Searching for next process to run.");
            if (Process_existsWithStatus(WAITING)) { // 1. resume interrupted?
                Process_switchStatus(WAITING, RUNNING);
            } else if (Process_existsWithStatus(NOTSTARTED)) { // 2. new proc.
                Process_switchStatus(NOTSTARTED, RUNNING);
            } else if (Process_existsWithStatus(BLOCKED)) {
                continue; // all blocked, short-circuit to next cycle
            } else {
                perror(
                  "Main loop did not terminate when it should have - check "
                  "while condition"); // TODO delete this debug statement

                break; // everything's done, exit loop
            }
        }

        // 3. Find line to run next
        Process* p = Process_peek(RUNNING);
        printf("Running process with pid=%lu\n", p->pid);

        unsigned long pid;
        unsigned long vpn;

        assert(p->currInterval != NULL);
        if (contains(p->currInterval->low, p->currInterval->high,
                     p->currentline)) {
            fscanf(tracefile, "%lu %lu", &pid, &vpn);
            assert(pid == p->pid);
            p->currentline++;
        } else {
            // Intervals cannot overlap, so the next greater interval will
            // appear directly to the right of a given node
            if (p->currInterval->right == NULL) {
                Process_switchStatus(RUNNING, FINISHED);
                // FREE ALL PAGES THIS PROCESSED USED HERE
            } else {
                // Context switch; we are done with the current process for
                // now, but it will appear again.
                p->currInterval = p->currInterval->right;
                p->currentline = p->currInterval->low;
                p->currentPos = p->currInterval->fpos_start;

                Process_switchStatus(WAITING, WAITING);
            }
        }


        // lookup VPN in page table and either fault or perform successful
        // reference
    }

    printf("%s\n", "===DONE WITH SIMULATION===");
    return;
}