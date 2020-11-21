// Simulates memory.

#include "linkedlist.h"
#include <bool.h>

// Represents a page from memory, identified both by a pid, vpn pair and a ppn
// USE THIS AS VALUE IN TREES
typedef struct page_t {
    long ppn;    // physical page number
    void* pnode; // pointer to a node in the ppn-keyed tree, used for reverse
                 // page resolution (ppn->page)
    long vpn;    // virtual page number
    long pid;    // process id
    void* vnode; // pointer to a node in the vpn-keyed tree. there is one of
                 // these trees per process. used for forward page resolution
                 // (vpn->page)
    void* overhead;
} Page;

/*

STRUCTURES
===========
Process representation
Page representation
VAS (arr. of proc.s)
PAS (arr. of proc.s)
run order queue - processes are sorted by starting line number
disk I/O wait queue - for processes waiting on I/O



.==============================================================================.
| 1. FIRST PASS                                                                |
'------------------------------------------------------------------------------'
1 | 2  45  <-- parse=>"line"
2 | 3  50
PARSER: provide a 1d sorted (by PID) array of Processes
+++ Process state after parsing
pid = (known)
state = NOTSTARTED
firstline = (known) <set to 0 at first, then on each occurance if zero, set to line>
                    if firstline was just set, enqueue proc on run order queue
currentline = <current line in parsing file>
lastline = (known) <set to currentline's value at EOF>
vas = NULL
+++

.==============================================================================.
| 2. MAIN LOOP                                                                 |
'------------------------------------------------------------------------------'

Is there a process at the front of the disk wait queue?
Yes-> decrement its counter
      if it's ready to run:
        set curr's status to waiting
        enqueue curr on the wait queue
        set wait[0]'s status to running
        curr <- wait[0]
No -> continue

Is there a currently running process? (some variable Process curr in main)
Yes-> continue
No -> get the next process (by start line number) with status NOTSTARTED, or break

assert we have a process to run

get the next line for that process

Is the page in memory?
Yes-> continue // page table hit
No -> call replacement module

assert we have a page that has been accessed

tell stat to update statistics
tell replace that there was a hit

increment time counter

continue to start of loop

.==============================================================================.
| 3. EXIT                                                                      |
'------------------------------------------------------------------------------'
print stats and exit with EXIT_SUCCESS




================================================================================

// populate a process struct





end up with a pile of pids, each tagged with a start and end line number
How to organize into a searchable group?

*/


typedef Page** PAS; // physical address space
// allocated once we know the amount of pages (=pmem/pgsize)

static PAS pas;

typedef Page** VAS; // virtual address space, one per process

enum ProcessState { NOTSTARTED, RUNNING, WAITING, FINISHED };
typedef struct process_t {
    long pid; // identifies this process overall
    ProcessState state;
    unsigned int waiting;
    unsigned long firstline;
    unsigned long currentline;
    unsigned long lastline;
    VAS vas;
} Process;

LinkedList runOrderQueue;
LinkedList waitingOnDisk;

PAS* PAS_init(unsigned long size);

// Process_init()

// Process_destroy()

// Page_init()

// Page_destroy()

// replace(old page, new page)
bool replace(Page* old, Page* new);
