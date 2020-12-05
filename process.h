#include "memory.h"
#include <stdio.h>
#include <sys/queue.h>

#ifndef _PROCESS_
#    define _PROCESS_

typedef enum ProcessStatus {
    RUNNABLE = 0,
    BLOCKED = 1,
    FINISHED = 2,
    NUM_OF_PROCESS_STATUSES = 3,
} ProcessStatus;

typedef void* PageTable;

// Represents a process
typedef struct process_t {
    unsigned long pid; // identifies this process overall

    // Queue overhead
    STAILQ_ENTRY(process_t) procs;
    ProcessStatus status;

    // Process's location in the file
    size_t firstline;
    size_t currentline;
    size_t lastline;
    long currentPos;
    IntervalNode* currInterval;
    IntervalNode* lineIntervals;

    // Wait info
    unsigned long waitTime; // timer for a disk operation in ticks
    VPage* waitingOnPage;

    // Map of VPN->PPN
    PageTable pageTable;
} Process;

void ProcessQueues_init(); // global static variable manages process states

void ProcessQueue_printQueue(ProcessStatus q_s);

int ProcessQueue_numWaitingProcs();

// Use of BSD tail queue based on queue(3) manpage
// and "Minimal example of TAILQ usage out of <sys/queue.h> library"
// (https://stackoverflow.com/q/22315213/11639533)

Process* Process_init();

Process* Process_init(unsigned long pid, unsigned long firstline,
                      unsigned long lastline, IntervalNode* lineIntervals);

Process* Process_peek(ProcessStatus status);

void Process_quit(Process* p);

bool Process_existsWithStatus(ProcessStatus status);

void Process_setStatus(Process* p, ProcessStatus status);
Process* Process_switchStatus(ProcessStatus s1, ProcessStatus s2);

bool Process_hasLinesRemainingInFile(const Process* p);
bool Process_hasLinesRemainingInInterval(const Process* p);
size_t Process_linesRemainingInInterval(const Process* p);
bool Process_onLastLineInInterval(const Process* p);
bool Process_hasIntervalsRemaining(const Process* p);
void Process_jumpToNextInterval(Process* p);

VPage* Process_allocVirtualPage(Process* p, unsigned long vpn);
VPage* Process_getVirtualPage(Process* p, unsigned long vpn);
bool Process_virtualPageInMemory(Process* p, unsigned long vpn);
unsigned long Process_loadVirtualPage(Process* p, unsigned long vpn);

void Process_free(Process* p);

#endif