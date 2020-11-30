/**
 * CS 537 Programming Assignment 4 (Fall 2020)
 * @file simulator.c
 * @brief Core module that coordinates the simulation
 * @author Michael Noguera
 */

#include "simulator.h"
#include "intervaltree.h"

static inline int Simulator_parseVPN(Process* p, FILE* tracefile) {
    int vpn;
    fscanf(tracefile, "%*d %d", &vpn);
    p->currentline++;
    return vpn;
}

void Simulator_runSimulation(FILE* tracefile) {
    bool finished = false;
    unsigned long clock = 0; // time in nanoseconds
    //unsigned long currentline = 1;
    Process* curr = NULL; // currently running process

    while (!finished) {
        Process* first_wait = Process_peek(BLOCKED); 
        if (first_wait != NULL) {
            assert(first_wait->waitTime > 0);
            first_wait->waitTime--;

            if(first_wait->waitTime == 0) {
                // fill in details for loading pages into memory
            }
        }
        // Identify the process to run this "tick"
        if (curr == NULL) {
            curr = Process_peek(WAITING);
            if (curr == NULL) {
                if (first_wait == NULL) {
                    // There are no processes waiting on I/O,
                    // and there are no proceses ready to be run,
                    // so we quit.
                    finished = true;
                    break;
                } else {
                    // advance time forward by the amount of time left remaining on the first element in the queue
                    clock += first_wait->waitTime;
                    first_wait->waitTime=0;
                    Process_switchStatus(BLOCKED, RUNNING);
                    curr = Process_peek(RUNNING);
                    fseek(tracefile, curr->currentPos, SEEK_SET);
                }
            }
        }
        
        assert(curr->currInterval != NULL);
        if (contains(curr->currInterval->low,
                     curr->currInterval->high,
                     curr->currentline)) {
            Simulator_parseVPN(curr, tracefile);
        } else {
            // Intervals cannot overlap, so the next greater interval will appear directly to the right of a given node
            if (curr->currInterval->right == NULL) {
                Process_switchStatus(RUNNING, FINISHED);
                // FREE ALL PAGES THIS PROCESSED USED HERE
            } else {
                // Context switch; we are done with the current process for now,
                // but it will appear again.
                curr->currInterval = curr->currInterval->right;
                curr->currentline = curr->currInterval->low;
                curr->currentPos = curr->currInterval->fpos_start;

                Process_switchStatus(WAITING, WAITING);
            }
        }

        // lookup VPN in page table and either fault or perform successful reference
    }
}