#include "memory.h"
#include <stdio.h>
#include <sys/queue.h>

#ifndef _PROCESS_
#    define _PROCESS_

typedef enum ProcessStatus {
    RUNNING,
    WAITING,
    BLOCKED,
    NOTSTARTED,
    FINISHED,
    NUM_OF_PROCESS_STATUSES
} ProcessStatus;

typedef void* PageTable;

// Represents a process
typedef struct process_t {
    unsigned long pid; // identifies this process overall

    // Queue overhead
    STAILQ_ENTRY(process_t) procs;
    ProcessStatus status;

    int waitCounter;

    // Process's location in the file
    unsigned long firstline;
    unsigned long currentline;
    unsigned long lastline;
    long currentPos;
    IntervalNode* currInterval;

    IntervalNode* lineIntervals;

    // Wait time
    unsigned long waitTime; // timer for a disk operation in ticks

    // Map of VPN->PPN
    PageTable pageTable;
} Process;

void ProcessQueues_init(); // global static variable manages process states

void ProcessQueue_printQueue(ProcessStatus q_s);

// Use of BSD tail queue based on queue(3) manpage
// and "Minimal example of TAILQ usage out of <sys/queue.h> library"
// (https://stackoverflow.com/q/22315213/11639533)

Process* Process_init();

Process* Process_init(unsigned long pid, unsigned long firstline,
                      unsigned long lastline, IntervalNode* lineIntervals);

Process* Process_peek(ProcessStatus status);

bool Process_existsWithStatus(ProcessStatus status);

Process* Process_switchStatus(ProcessStatus s1, ProcessStatus s2);

bool Process_hasLinesRemaining(const Process* p);

PageTable* PageTable_init();
VPage* PageTable_get(PageTable* pt, int vpn, int pid);
int PageTable_add(PageTable* pt, int vpn, int pid, int ppn);
int PageTable_remove(PageTable* pt, int vpn, int pid);

#endif