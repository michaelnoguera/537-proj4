/**
 * CS 537 Programming Assignment 4 (Fall 2020)
 * @file simulator.c
 * @brief Core module that coordinates the simulation
 * @author Michael Noguera
 */

#include "simulator.h"
#include "intervaltree.h"
#include "replace.h"
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
    fseek(tracefile, 0, SEEK_SET); // reset ptr
    Process* p;

    while (notDone()) {
        // 0. Account for clock tick
        time += CLOCK_TICK;
        // printf("Tick! time=%i\n", time);

        // 1. Advance disk wait counter if needed
        if (Process_existsWithStatus(BLOCKED)) {
            // printf("%s\n", "Checking on blocked processes.");
            if (--(Process_peek(BLOCKED)->waitTime) == 0) {
                Process* p = Process_peek(BLOCKED);
        
                assert(p->waiting_VPN != NULL);
                int ppn;
                if (Memory_hasFreePage()) {
                    ppn = Memory_getFreePage();
                } else {
                    // ppn = Replace_getNewPage(p->waiting_VPN);
                }
                Memory_loadPage(p->waiting_VPN, ppn);
                Process_switchStatus(RUNNING, WAITING); // i/o completed
                Process_switchStatus(BLOCKED, RUNNING);
            }
        }

        // 2. If there isn't a current process, choose the next one
        if (!Process_existsWithStatus(RUNNING)) {
            // printf("%s\n", "Searching for next process to run.");
            if (Process_existsWithStatus(WAITING)) { // 1. resume interrupted?
                Process_switchStatus(WAITING, RUNNING);
            } else if (Process_existsWithStatus(NOTSTARTED)) { // 2. new proc.
                Process_switchStatus(NOTSTARTED, RUNNING);
            } else if (Process_existsWithStatus(BLOCKED)) {
                // TODO if time, implement magic jumping through time
                // Stat_nothing_happened();
                continue; // all blocked, short-circuit to next cycle
            } else {
                perror(
                  "Main loop did not terminate when it should have - check "
                  "while condition"); // TODO delete this debug statement

                break; // everything's done, exit loop
            }

            // We just got a new process, update state
            p = Process_peek(RUNNING);
            printf("switching to pid=%lu at position=%lu\n", p->pid, p->currentPos);
            fseek(tracefile, p->currentPos, SEEK_SET);
        }

        // 3. Find line to run next
        unsigned long pid;
        unsigned long vpn;

        assert(p->currInterval != NULL);
        // case 1: next line immediately follows the current one
        if (it_contains(p->currInterval->low, p->currInterval->high,
                        p->currentline)) {
            fscanf(tracefile, "%lu %lu\n", &pid, &vpn);
            printf("%lu %lu\n", pid, vpn);
            assert(pid == p->pid);
            p->currentline++;
        } else {
            // Intervals cannot overlap, so the next greater interval will
            // appear directly to the right of a given node
            if (p->currInterval->right == NULL) {
                printf("%s\n", "This process finished!.");
                Process_switchStatus(RUNNING, FINISHED);
                // FREE ALL PAGES THIS PROCESSED USED HERE
                // TODO make sure Process_free() works as intended;
                Process_free(p);
                p = NULL;
            } else {
                printf("%s\n", "Context switch!");
                Process_jumpToNextInterval(p);
                Process_switchStatus(RUNNING, WAITING);
            }
        }

        // 4. Simulate memory reference
        // get the virtual page-> look up in page table for this proc.
        //VPage* v = PageTable_get(p->pageTable, vpn, pid);

        // Create new v. page if none exists
        /*if (v == NULL) {
            // this is a new allocation
            // create a new page: v = new page
        }

        // Hit or miss?
        if (v->inMemory) {
            // tell the replacement module it was accessed
            // continue
            // else
            // Stat_hit();
        } else {
            // need to swap page from disk
            // block the process
            // continue
            // ...when block expires...
            // tell the replacement module that the page is ready
            // Stat_miss();
        }*/

        // 

            
           


        // lookup VPN in page table and either fault or perform successful
        // reference
    }

    printf("%s\n", "===DONE WITH SIMULATION===");
    return;
}