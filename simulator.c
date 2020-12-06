/**
 * CS 537 Programming Assignment 4 (Fall 2020)
 * @file simulator.c
 * @brief Core module that coordinates the simulation
 * @author Michael Noguera and Julien de Castelnau
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

static inline bool Simulator_notDone() {
    return Process_existsWithStatus(RUNNABLE)
           || Process_existsWithStatus(BLOCKED);
}

static inline void Simulator_saveRunningProcessLine(FILE* tracefile) {
    assert(tracefile != NULL && Process_peek(RUNNABLE) != NULL);
    Process_peek(RUNNABLE)->currentPos = ftell(tracefile);
}

static inline void Simulator_seekSavedLine(FILE* tracefile, Process* p) {
    assert(p != NULL && p->status != FINISHED);
    if (fseek(tracefile, p->currentPos, SEEK_SET) != 0
        || ftell(tracefile) != p->currentPos) {
        fprintf(stderr, "fseek to position %lu failed, ended up at %lu\n",
                p->currentPos, ftell(tracefile));
        exit(EXIT_FAILURE);
    }
}

static inline void Simulator_finishCurrentDiskIO() {
    Process* p = Process_peek(BLOCKED);
    assert(p != NULL && p->status == BLOCKED);
    assert(p->waitTime == 0 && "Page can't load until after process waits");
    assert(p->waitingOnPage != NULL && "no page to fetch");

    unsigned long ppn;

    if (Memory_hasFreePage()) {
        ppn = Memory_getFreePage();
        Memory_loadPage(p->waitingOnPage, ppn);
        Replace_notifyPageLoad(p->waitingOnPage->overhead);
    } else {
        ppn = Replace_getPageToEvict();
        Memory_evictPage(ppn);
        Memory_loadPage(p->waitingOnPage, ppn);
        Replace_notifyPageLoad(p->waitingOnPage->overhead);
    }
    p->waitingOnPage = NULL;
}

static inline void Simulator_safelySwitchStatus(FILE* tracefile, Process* p,
                                                ProcessStatus new,
                                                long fpos_hack) {
    assert(tracefile != NULL && ferror(tracefile) == 0);
    assert(p != NULL);

    ProcessStatus old = p->status;

    assert(Process_peek(old) == p && "not at head of queue");

    if (old == RUNNABLE) {
        assert(tracefile != NULL && Process_peek(RUNNABLE) != NULL);
        Process_peek(RUNNABLE)->currentPos = fpos_hack;

    } else if (old == BLOCKED) {
        assert(p->waitTime == 0
               && "Cannot resume blocked process prematurely.");
    } else if (old == FINISHED) {
        assert(false && "ERROR: Attempted to resume already-finished process.");
    }

    if (Process_switchStatus(old, new) != p) {
        fprintf(stderr,
                "ERROR: Corruption in process queue. Do not modify data "
                "structures concurrently with execution.");
    }

    if (new == RUNNABLE) {
        Simulator_seekSavedLine(tracefile, p);
    } else if (new == BLOCKED) {
        assert(p->waitTime == DISK_PENALTY
               && "Set block timer in order to block process.");
    } else if (new == FINISHED) {
        assert(!Process_hasLinesRemainingInFile(p)
               && "Cannot mark process as finished with lines left to run.");
    }
}

unsigned long Simulator_runSimulation(FILE* tracefile) {
    unsigned long time = 0; // time in nanoseconds

    assert(tracefile != NULL && "tracefile can't be null");
    rewind(tracefile); // reset ptr
    Process* p = Process_peek(RUNNABLE);

    while (Simulator_notDone()) {
        // 0. Account for clock tick
        time += CLOCK_TICK;
        Stat_default(CLOCK_TICK);

        // 1. Advance disk wait counter if needed
        if (Process_existsWithStatus(BLOCKED)) {
            Process_peek(BLOCKED)->waitTime -= CLOCK_TICK;
            if (Process_peek(BLOCKED)->waitTime == 0) {
                Simulator_finishCurrentDiskIO(); // evicts if needed
                Simulator_safelySwitchStatus(tracefile, Process_peek(BLOCKED),
                                             RUNNABLE, 0);
                continue;
            }
        }

        // 2. If all processes are blocked, jump to the time where one finishes
        // waiting
        if (!Process_existsWithStatus(RUNNABLE)) {
            assert((Process_peek(BLOCKED))->waitTime != 0);
            unsigned long skip =
              CLOCK_TICK * ((Process_peek(BLOCKED))->waitTime - 1);
            time += skip;
            Stat_default(skip);
            Process_peek(BLOCKED)->waitTime -= skip;
            assert((Process_peek(BLOCKED))->waitTime == 1);
            continue;
        }


        Process* old_p = p;
        p = Process_peek(RUNNABLE);
        if (old_p != p) {
            if (p->currentPos != ftell(tracefile)) {
                Simulator_seekSavedLine(tracefile, p);
            }
        }

        // 3. Find line to run next
        unsigned long pid;
        unsigned long vpn;
        assert(p != NULL);
        assert(p->currInterval != NULL);

        long fpos_hack = ftell(tracefile);
        if (fscanf(tracefile, "%lu %lu\n", &pid, &vpn) != 2) {
            perror("Error reading from trace file.");
            exit(EXIT_FAILURE);
        }

        if (pid != p->pid) {
            if (p->status != RUNNABLE) {
                fprintf(stderr, "Process escaped block queue.\n");
            }
            if (p->currInterval->low < p->currentline
                && p->currInterval->high > p->currentline) {
                fprintf(stderr, "Interval tree provided invalid location.\n");
            }
            for (int i = 0; i < NUM_OF_PROCESS_STATUSES; i++) {
                // ProcessQueue_printQueue(i);
            }
            assert(pid == p->pid);
        }

        // 4. Simulate memory reference
        // get the virtual page-> look up in page table for this proc.
        VPage* v = Process_getVirtualPage(p, vpn);

        // Create new v. page if none exists
        if (v == NULL) {
            v = Process_allocVirtualPage(p, vpn);
            assert(v != NULL);
            assert(Process_getVirtualPage(p, vpn) == v);
        }

        // Is it a hit or a miss?
        if (Process_virtualPageInMemory(p, vpn)) {
            Stat_hit();
            Replace_notifyPageAccess(v->overhead);

            if (Process_onLastLineInInterval(p)
                && Process_hasIntervalsRemaining(p)) {
                
                // advance file pointer
                Process_jumpToNextInterval(p);
                Simulator_seekSavedLine(tracefile, p);

                // context switch
                Process_switchStatus(RUNNABLE, RUNNABLE); // does not check for
                                                          // safety, but fast
            } else if (Process_hasLinesRemainingInInterval(p)) {
                p->currentline++;
            } else {
                // no remaining intervals, no remaining lines -> finished
                Simulator_safelySwitchStatus(tracefile, p, FINISHED, 0);
                Process_quit(p); // clean up and free memory
            }
        } else {
            Stat_miss();
            p->waitTime = DISK_PENALTY;
            p->waitingOnPage = v;
            Simulator_safelySwitchStatus(tracefile, p, BLOCKED, fpos_hack);
        }
    }

    return time;
}