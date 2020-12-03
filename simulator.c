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
    return Process_existsWithStatus(RUNNABLE)
           || Process_existsWithStatus(RUNNABLE)
           || Process_existsWithStatus(BLOCKED)
           || Process_existsWithStatus(RUNNABLE);
}

static inline void Simulator_seekSavedLine(FILE* tracefile, Process* p) {
    printf("--> now running pid=%lu at position=%lu\n", p->pid, p->currentPos);
    fseek(tracefile, p->currentPos, SEEK_SET);
}

static inline void Simulator_saveProcessState(FILE* tracefile, Process* p) {
    assert(tracefile != NULL && p != NULL);
    // assert((*p)->currInterval != NULL);
    // if (Process_hasLinesRemainingInInterval(*p)) {
    p->currentPos = ftell(tracefile);
    //} else if (Process_hasIntervalsRemaining(*p)) {
    //    printf("%s\n", "Saving state: jumping to next interval");
    //    Process_jumpToNextInterval(*p);
    //    (*p)->currentPos = ftell(tracefile);
    //} else if (Process_hasLinesRemainingInFile(*p)) {
    //    perror("WARNING: something's wrong... lines not in interval");
    //} else {
    //    printf("%s\n", "Saving state: This process has finished!.");
    //}
}

static inline void Simulator_loadSavedProcessState(FILE* tracefile,
                                                   Process* p) {
    assert(p != NULL && p->status != FINISHED);
    Simulator_seekSavedLine(tracefile, p);
}

static inline void Simulator_loadPendingPage(Process* p) {
    assert(p != NULL);
    assert(p->waitTime == 0 && "Page can't load until after process waits");

    unsigned long ppn;

    if (Memory_hasFreePage()) {
        printf("%s\n", "Memory has an empty spot.");
        ppn = Memory_getFreePage();
        Memory_loadPage(p->waitingOnPage, ppn);
    } else {
        printf("%s\n", "No empty spot in memory, need to evict.");
        ppn = Replace_getPageToEvict();
        Memory_evictPage(ppn);
        Memory_loadPage(p->waitingOnPage, ppn);
        printf("Victim = %lu\n", ppn);
    }
    printf("Loaded virtual page %lu into ppn %lu\n", p->waitingOnPage->vpn,
           ppn);
    p->waitingOnPage = NULL;
}

unsigned long Simulator_runSimulation(FILE* tracefile) {
    unsigned long time = 0; // time in nanoseconds

    // unsigned long currentline = 1;
    assert(tracefile != NULL && "tracefile can't be null");
    fseek(tracefile, 0, SEEK_SET); // reset ptr
    Process* p;

    while (notDone()) {
        // 0. Account for clock tick
        time += CLOCK_TICK;
        Stat_default(CLOCK_TICK);

        // 1. Advance disk wait counter if needed
        if (Process_existsWithStatus(BLOCKED)) {
            Process_peek(BLOCKED)->waitTime -= CLOCK_TICK;
            if (Process_peek(BLOCKED)->waitTime == 0) {
                // resume process that is returning from disk i/o
                Process* restarting = Process_peek(BLOCKED);
                assert(restarting->waitingOnPage != NULL);
                assert(restarting->status == BLOCKED);

                printf("Process %lu is done waiting!\n", restarting->pid);

                // load context
                Simulator_loadPendingPage(restarting);
                Simulator_loadSavedProcessState(tracefile, restarting);

                // resume blocked process
                if (Process_hasLinesRemainingInFile(p)) {
                    Process_switchStatus(BLOCKED, RUNNABLE);
                }
            }
        }

        // 2. Determine process to run
        if (!Process_existsWithStatus(RUNNABLE)) {
            if (Process_existsWithStatus(RUNNABLE)) { // 1. resume interrupted?
                Simulator_loadSavedProcessState(tracefile,
                                                Process_peek(RUNNABLE));
                Process_switchStatus(RUNNABLE, RUNNABLE);
            } else if (Process_existsWithStatus(RUNNABLE)) { // 2. new proc.
                Simulator_loadSavedProcessState(tracefile,
                                                Process_peek(RUNNABLE));
                Process_switchStatus(RUNNABLE, RUNNABLE);
            } else if (Process_existsWithStatus(BLOCKED)) {
                continue; // nothing runnable, wait for disk/io to finish
            } else {
                perror(
                  "Main loop did not terminate when it should have - check "
                  "while condition"); // TODO delete this debug statement
                return time;          // everything's done, exit loop
            }
        }

        assert(Process_existsWithStatus(RUNNABLE));
        if (Process_peek(RUNNABLE)->waitTime != 0) {
            perror("BLOCKED PROC ESCAPED BLOCK QUEUE!");
            for (int i = 0; i < NUM_OF_PROCESS_STATUSES; i++) {
                ProcessQueue_printQueue(i);
            }
            assert(Process_peek(RUNNABLE)->waitTime == 0);
        }

        if (p != Process_peek(RUNNABLE)) {
            unsigned long oldpid = p->pid;
            p = Process_peek(RUNNABLE);
            printf("Running process has changed (%lu->%lu)\n", oldpid, p->pid);
        }

        Simulator_loadSavedProcessState(tracefile, p);

        // 3. Find line to run next
        unsigned long pid;
        unsigned long vpn;
        assert(p != NULL);
        assert(p->currInterval != NULL);
        // case 1: next line immediately follows the current one
        if (Process_hasLinesRemainingInInterval(p)) {
            fscanf(tracefile, "%lu %lu\n", &pid, &vpn);

            printf("%lu %lu\n", pid, vpn);
            if (pid != p->pid) {
                for (int i = 0; i < NUM_OF_PROCESS_STATUSES; i++) {
                    ProcessQueue_printQueue(i);
                }
                assert(pid == p->pid);
            }

            p->currentline++;
        } else {
            if (!Process_hasLinesRemainingInFile(p)) {
                printf("%s\n", "This process finished!.");
                Simulator_saveProcessState(tracefile, p);
                Process_switchStatus(RUNNABLE, FINISHED);
                // Process_free(p);
                continue;
            }
            // Intervals cannot overlap, so the next greater interval will
            // appear directly to the right of a given node
            if (it_contains(p->currInterval->low, p->currInterval->high,
                            p->currentline)) {
                printf("%s\n", "WARN: THIS LINE WAS IN THE INTERVAL");
                assert(false);
                Simulator_saveProcessState(tracefile, p);
                Process_switchStatus(RUNNABLE, RUNNABLE);
                continue;
            } else if (Process_hasIntervalsRemaining(p)) {
                printf("%s\n", "Jump to next interval...");
                Process_jumpToNextInterval(p);
                Simulator_saveProcessState(tracefile, p);
                Process_switchStatus(RUNNABLE, RUNNABLE);
                continue;
            }
            perror("Invalid state.");
            assert(false);
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
            printf("\t%s\n", "hit");
            // Replace_notifyPageAccess(v->currentPPN);
            Stat_hit();
        } else {
            printf("\t%s\n", "miss --> block & switch!");
            Process_switchStatus(RUNNABLE, BLOCKED);
            p->waitTime = DISK_PENALTY;
            p->waitingOnPage = v;
            Stat_miss();
        }
        Simulator_saveProcessState(tracefile, p);

    }

    printf("%s\n", "===DONE WITH SIMULATION===");
    return time;
}