// Simulates memory.

#include "linkedlist.h"
#include "intervaltree.h"
#include <stdbool.h>

#ifndef _MEMORY_
#define _MEMORY_

// Represents a page from memory, identified both by a pid, vpn pair and a ppn
// USE THIS AS VALUE IN TREES
typedef struct page_t {
    unsigned long ppn;    // physical page number
    void* pnode; // pointer to a node in the ppn-keyed tree, used for reverse
                 // page resolution (ppn->page)
    unsigned long vpn;    // virtual page number
    unsigned long pid;    // process id
    void* vnode; // pointer to a node in the vpn-keyed tree. there is one of
                 // these trees per process. used for forward page resolution
                 // (vpn->page)
    bool free;
    void* overhead;
} Page;

typedef Page** PAS; // physical address space
// allocated once we know the amount of pages (=pmem/pgsize)

LinkedList* /* of PHYSICALLY FREE pages */ freelist;

typedef Page** VAS; // virtual address space, one per process

enum ProcessState { RUNNING, WAITING, BLOCKED, NOTSTARTED, FINISHED };

typedef struct process_t {
    unsigned long pid; // identifies this process overall
    enum ProcessState state;
    unsigned int waiting;
    unsigned long firstline;
    unsigned long currentline;
    unsigned long lastline;
    IntervalNode* lineIntervals;
    VAS vas;
} Process;
Process* Process_init();

void ProcessQueues_init();
static LinkedList* ProcessQueues[5];
void Process_run(Process* p);
void Process_wait(Process* p);
void Process_block(Process* p);
void Process_close(Process* p);

int destroyProcess();

LinkedList runOrderQueue;
LinkedList waitingOnDisk;

PAS* PAS_init(unsigned long size);

// Process_init()

// Process_destroy()

// Page_init()

// Page_destroy()

// replace(old page, new page)
bool replace(Page* old, Page* new);

#endif