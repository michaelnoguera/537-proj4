#define _GNU_SOURCE 

#include "process.h"
#include "memory.h"
#include "replace.h"
#include <assert.h>
#include <search.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

typedef struct node_t
{
  /* Callers expect this to be the first element in the structure - do not
     move!  */
  const void *key;
  uintptr_t left_node; /* Includes whether the node is red in low-bit. */
  uintptr_t right_node;
} *node;

static STAILQ_HEAD(processQueue_t, process_t) pq[NUM_OF_PROCESS_STATUSES];

void ProcessQueues_init() {
    STAILQ_INIT(&pq[RUNNABLE]); // should only ever have one element
    STAILQ_INIT(&pq[BLOCKED]);  // waiting on disk
    STAILQ_INIT(&pq[FINISHED]);
}

/** Insert to a sorted priority queue*/
static void ProcessQueue_enqueuePriority(Process* p, struct processQueue_t* q) {
    assert(p != NULL && q != NULL);

    // edge case: empty queue
    if (STAILQ_EMPTY(q)) {
        STAILQ_INSERT_HEAD(q, p, procs);
        return;
    }
    
    Process* prev = NULL;
    Process* curr = NULL;

    // iterate until past spot- but prev will still be the element before
    STAILQ_FOREACH(curr, q, procs) {
        assert(curr->pid != p->pid);
        if (curr->currentPos > p->currentPos) break;
        prev = curr;
    }

    if (prev == NULL) { // first element checked was greater
        STAILQ_INSERT_HEAD(q, p, procs);
    } else { // insert using ptr
        STAILQ_INSERT_AFTER(q, prev, p, procs);
    }
}


static void ProcessQueue_enqueue(Process* p, struct processQueue_t* q) {
    if (q == &pq[RUNNABLE]) {
        ProcessQueue_enqueuePriority(p, q);
        
    } else {
        STAILQ_INSERT_TAIL(q, p, procs);
    }
}

void ProcessQueue_printQueue(ProcessStatus q_s) {
    Process* head = STAILQ_FIRST(&pq[q_s]);

    printf("Queue: ");
    switch (q_s) {
        case RUNNABLE:
            printf("RUNNABLE\n");
            break;
        case BLOCKED:
            printf("BLOCKED\n");
            break;
        case FINISHED:
            printf("FINISHED\n");
            break;
        case NUM_OF_PROCESS_STATUSES:
            printf("NUM_OF_PROCESS_STATUSES (not a valid queue)");
            break;
    }
    if (STAILQ_EMPTY(&pq[q_s])) {
        printf("Empty Queue \n");
        return;
    }
    if (head == NULL) {
        perror("WARN: Queue has null head, but is not empty?");
        return;
    }
    for (int i = 0; head != NULL; i++, head = STAILQ_NEXT(head, procs)) {
        printf(
          "\t\x1B[2m->\x1B[0m\x1B[33m%3d\x1B[0m\x1B[1m (%p) pid: %ld start: "
          "%ld current: %ld end: %ld \x1B[0m, INTERVALS: ",
          i, (void*)head, head->pid, head->firstline, head->currentline,
          head->lastline);
        it_print(head->lineIntervals);
        printf("\n");
    }
}


static PageTable* PageTable_init() {
    PageTable* pt;
    if ((pt = (PageTable*)malloc(sizeof(PageTable))) == NULL) {
        perror("Error allocating memory for page table.");
        exit(EXIT_FAILURE);
    }
    return pt;
}

static int PageTable_compare(const void* vp1, const void* vp2) {
    VPage* vp1_page = (VPage*)vp1;
    VPage* vp2_page = (VPage*)vp2;
    if (vp1_page->vpn < vp2_page->vpn) {
        return -1;
    } else if (vp1_page->vpn > vp2_page->vpn) {
        return 1;
    } else {
        return 0;
    }
}

static VPage* PageTable_get(PageTable* pt, unsigned long vpn) {
    VPage* search_query;
    VPage* search_result;

    search_query = VPage_init(0, vpn);

    if ((search_result = tfind(search_query, pt, PageTable_compare)) == NULL) {
        VPage_free(search_query);
        return NULL;
    } else {
        return *(VPage**)search_result;
    }
}

static int PageTable_add(PageTable* pt, VPage* new) {
    VPage* search_result;
    VPage* temp_result;

    if ((search_result = tsearch(new, pt, PageTable_compare)) == NULL) {
        perror("Error in page table lookup.");
        exit(EXIT_FAILURE);
    } else {
        temp_result = *(VPage**)search_result;

        if (temp_result != new) {
            return 1; // collision with existing
        } else {
            return 0; // success
        }
    }
}

/**
 * @return page if found, NULL if not
 *
static VPage* PageTable_remove(PageTable* pt, unsigned long vpn) {
    VPage* entry_to_remove;

    if ((entry_to_remove = PageTable_get(pt, vpn)) == NULL) {
        // Entry removal failed. The node was not found in the page table.
        return NULL;
    } else {
        // Entry was found. call tdelete() and remove
        tdelete(entry_to_remove, pt, PageTable_compare);
        VPage_free(entry_to_remove);
        return entry_to_remove;
    }
}*/


/**
 * Peek at the proc. at the head of a given status queue. NULL if none present.
 */
Process* Process_peek(ProcessStatus status) {
    return STAILQ_FIRST(&pq[status]);
}

/**
 * @return true if there is a process in the specified status queue, else false.
 */
bool Process_existsWithStatus(ProcessStatus status) {
    return !(STAILQ_EMPTY(&pq[status]));
}

/**
 * Constructs a new Process and initializes internal fields.
 * @param pid pid of new process
 * @param firstline first line number in tracefile corresponding to this proc.
 * @param lastline last line number in tracefile corresponding to this proc.
 * @param lineIntervals interval tree overhead
 */
Process* Process_init(unsigned long pid, unsigned long firstline,
                      unsigned long lastline, IntervalNode* lineIntervals) {
    Process* p = malloc(sizeof(Process));
    p->pid = pid;
    p->firstline = firstline;
    p->currentline = firstline;
    p->lastline = lastline;
    p->waitTime = 0;
    p->waitingOnPage = NULL;
    p->lineIntervals = lineIntervals;

    p->currentPos = lineIntervals->fpos_start;
    p->currInterval = lineIntervals;

    p->pageTable = PageTable_init();
    p->status = RUNNABLE;
    STAILQ_INSERT_TAIL(&pq[RUNNABLE], p, procs);

    return p;
}

/**
 * Changes the status of a Process to status.
 * @param p pointer to target process
 * @param status new status to assign
 */
void Process_setStatus(Process* p, ProcessStatus status) {
    STAILQ_REMOVE(&pq[p->status], p, process_t, procs);
    p->status = status;
    ProcessQueue_enqueue(p, &pq[status]);
}

/**
 * Pops the Process at the head of the s1 queue and enqueues it on the s2 queue.
 * Faster than setStatus because it doesn't need to search.
 * @param s1 source status
 * @param s2 destination status
 * @return the process that was moved, or null if none found in s1 queue
 */
Process* Process_switchStatus(ProcessStatus s1, ProcessStatus s2) {
    Process* p = STAILQ_FIRST(&pq[s1]);
    if (p == NULL) {
        perror("WARN: Impossible switch, no process on source queue.");
        return NULL;
    }
    STAILQ_REMOVE_HEAD(&pq[s1], procs);
    p->status = s2;
    ProcessQueue_enqueue(p, &pq[s2]);
    return p;
}

/**
 * Checks to see if the specified process has more lines left to run in the file
 * @param p process
 * @return true if there are lines left for the process to run, false if all
 * trace lines have completed
 */
inline bool Process_hasLinesRemainingInFile(const Process* p) {
    return (p->currentline < p->lastline);
}

/**
 * Checks to see if the specified process has more lines left to run in the
 * current interval
 * @param p process
 * @return true if there are lines left for the process to run, false if all
 * trace lines within the current interval have completed
 */
inline bool Process_hasLinesRemainingInInterval(const Process* p) {
    return (p->currentline < p->currInterval->high);
}

/**
 * @return number of lines remaining in current interval
 */
inline size_t Process_linesRemainingInInterval(const Process* p) {
    return p->currInterval->high - p->currentline;
}

/**
 * @return true if currentline is the last line in this interval
 */
inline bool Process_onLastLineInInterval(const Process* p) {
    return p->currInterval->high == p->currentline;
}

/**
 * @return true if there is another interval of trace lines to run for this
 * process or false if the process is in/finished with its last one
 */
inline bool Process_hasIntervalsRemaining(const Process* p) {
    return ((void*)p->currInterval->right != NULL);
}

/**
 * Context switch; we are done with the current process for now, but it will
 * appear again.
 */
inline void Process_jumpToNextInterval(Process* p) {
    assert(Process_hasIntervalsRemaining(p));
    assert(Process_onLastLineInInterval(p));
    if (p != NULL && p->currInterval != NULL) {
        p->currInterval = p->currInterval->right;
        p->currentline = p->currInterval->low;
        p->currentPos = p->currInterval->fpos_start;
    } else {
        perror("Tried to jump to next interval when none existed");
        if (p != NULL) fprintf(stderr, "pid=%lu\n", p->pid);
    }
}

VPage* Process_allocVirtualPage(Process* p, unsigned long vpn) {
    VPage* v = VPage_init(p->pid, vpn);
    PageTable_add(p->pageTable, v);
    return v;
}

VPage* Process_getVirtualPage(Process* p, unsigned long vpn) {
    assert(p != NULL);
    return PageTable_get(p->pageTable, vpn);
}

bool Process_virtualPageInMemory(Process* p, unsigned long vpn) {
    VPage* v = PageTable_get(p->pageTable, vpn);
    if (v == NULL) return false;
    return v->inMemory;
}

void PageTable_free_destroyVPage(void* vp_node) {
    VPage* vp = vp_node;
    VPage_free(vp);
}

void Process_quit(Process* p) {
    assert(p->pageTable != NULL);
    PageTable* pagetable_tmp = p->pageTable;

    tdestroy(*pagetable_tmp, PageTable_free_destroyVPage);
    Process_free(p);

    return;
}

/**
 * MUST have a free space in memory before calling
 * @param p process that the page belongs to
 * @param vpn virtual page identifier
 * @return ppn where the page was inserted
 *
unsigned long Process_loadVirtualPage(Process* p, unsigned long vpn) {
    assert(p != NULL);
    VPage* v = PageTable_get(p->pageTable, vpn);
    assert(v != NULL && "Can't load null page into RAM.");
    assert(v->inMemory == false && "Page already in RAM.");
    assert(Memory_hasFreePage() && "Must be free space in memory to add to.");
    unsigned long ppn = Memory_getFreePage();
    Memory_loadPage(v, ppn);
    v->inMemory = true;
    v->currentPPN = ppn;
    return ppn;
}*/

/**
 * Destructs and frees a Process.
 * @param p pointer to heap-alloc'd process
 */
void Process_free(Process* p) { free(p); }
