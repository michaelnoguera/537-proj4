/**
 * CS 537 Programming Assignment 4 (Fall 2020)
 * @file simulator.c
 * @brief Core module that coordinates the simulation
 * @author Michael Noguera
 */

#include "simulator.h"
#include "intervaltree.h"
#include "replace.h"
#include "stat.h"
#include <assert.h>

enum {
    CLOCK_TICK = 1,
    DISK_PENALTY = 2000000,
};

static_assert(CLOCK_TICK > 0, "Clock tick must be greater than zero.");
static_assert(DISK_PENALTY > 0, "Disk penalty must be greater than zero.");
static_assert(DISK_PENALTY % CLOCK_TICK == 0,
              "Disk penalty must be a multiple of clock tick, else ticks won't "
              "ever add up to a completed I/O.");

static inline bool notDone() {
    return Process_existsWithStatus(RUNNING)
           || Process_existsWithStatus(WAITING)
           || Process_existsWithStatus(BLOCKED)
           || Process_existsWithStatus(NOTSTARTED);
}

static inline void Simulator_jumpInFile(FILE* tracefile, Process** p) {
    assert(Process_existsWithStatus(RUNNING));
    *p = Process_peek(RUNNING);
    printf("--> now running pid=%lu at position=%lu\n", (*p)->pid,
           (*p)->currentPos);
    fseek(tracefile, (*p)->currentPos, SEEK_SET);
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
            if ((Process_peek(BLOCKED)->waitTime) == 0) {
                p = Process_peek(BLOCKED);
                printf("Process %lu is done waiting!\n", p->pid);

                assert(p->waitingOnPage != NULL);
                int ppn;
                if (Memory_hasFreePage()) {
                    printf("%s\n", "Memory has an empty spot.");
                    ppn = Memory_getFreePage();
                } else {
                    printf("%s\n", "No empty spot in memory, need to evict.");
                    ppn = Replace_getPageToEvict();
                    printf("Victim = %i\n", ppn);
                    Memory_evictPage(ppn);
                }

                Memory_loadPage(p->waitingOnPage, ppn);
                printf("Loaded virtual page %lu into ppn %i\n",
                       p->waitingOnPage->vpn, ppn);
                
                p->waitingOnPage = NULL;

                Process_switchStatus(RUNNING,
                                     WAITING); // i/o completed, interrupt
                Process_switchStatus(BLOCKED, RUNNING);
                Simulator_jumpInFile(tracefile, &p);
            } else {
                Process_peek(BLOCKED)->waitTime -= CLOCK_TICK;
            }
        }

        if (Process_existsWithStatus(RUNNING)
            && Process_peek(RUNNING)->waitTime != 0) {
            for (int i = 0; i < NUM_OF_PROCESS_STATUSES; i++) {
                ProcessQueue_printQueue(i);
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
                assert((Process_peek(BLOCKED))->waitTime != 0);
                time += CLOCK_TICK * (Process_peek(BLOCKED))->waitTime - 1;
                Process_peek(BLOCKED)->waitTime = 0;
                continue; // all blocked, short-circuit to next cycle
            } else {
                perror(
                  "Main loop did not terminate when it should have - check "
                  "while condition"); // TODO delete this debug statement

                return; // everything's done, exit loop
            }

            // new running process, new place in file
            Simulator_jumpInFile(tracefile, &p);
        }

        assert(Process_existsWithStatus(RUNNING));
        if (Process_peek(RUNNING)->waitTime != 0) {
            for (int i = 0; i < NUM_OF_PROCESS_STATUSES; i++) {
                ProcessQueue_printQueue(i);
            }
            assert(false);
        }

        // 3. Find line to run next
        unsigned long pid;
        unsigned long vpn;
        assert(p != NULL);
        assert(p->currInterval != NULL);
        // case 1: next line immediately follows the current one
        if (it_contains(p->currInterval->low, p->currInterval->high,
                        p->currentline)) {
            fscanf(tracefile, "%lu %lu\n", &pid, &vpn);
            printf("%lu %lu\n", pid, vpn);
            if (pid != p->pid) { printf("oops\n"); }
            assert(pid == p->pid);
            p->currentline++;
        } else {
            // Intervals cannot overlap, so the next greater interval will
            // appear directly to the right of a given node
            if (p->currInterval->right != NULL) {
                printf("%s\n", "Jump to next interval...");
                Process_jumpToNextInterval(p);
                Process_switchStatus(RUNNING, WAITING);
                continue;
            } else {
                printf("%s\n", "This process finished!.");
                Process_switchStatus(RUNNING, FINISHED);
                // Process_free(p);
                continue;
            }
        }

        // 4. Simulate memory reference
        // get the virtual page-> look up in page table for this proc.
        VPage* v = Process_getVirtualPage(p, vpn);

        // Create new v. page if none exists
        if (v == NULL) {
            printf("\t%s\n", "new page, allocating virtual page");
            v = Process_allocVirtualPage(p, vpn);
            assert(v != NULL);
            assert(Process_getVirtualPage(p, vpn) == v);
        }

        if (Process_virtualPageInMemory(p, vpn)) {
            printf("\t%s\n", "physical memory hit");
            // tell the replacement module it was accessed
            // continue
            // else
            Stat_hit();
        } else {
            printf("\t%s\n", "physical memory miss --> block & switch!");
            p->currentPos =
              ftell(tracefile); // save location for context switch
            Process_switchStatus(RUNNING, BLOCKED);
            p->waitTime = DISK_PENALTY;
            if (p->pid == 30) {
                printf("um ok");
            }
            p->waitingOnPage = v;
            Stat_miss();
            continue;
        }

        //


        // lookup VPN in page table and either fault or perform successful
        // reference
    }

    printf("%s\n", "===DONE WITH SIMULATION===");
    return;
}