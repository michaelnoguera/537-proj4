#include "process.h"
#include "memory.h"
#include <assert.h>
#include <search.h>
#include <stdio.h>

static STAILQ_HEAD(processQueue_t, process_t) pq[NUM_OF_PROCESS_STATUSES];
static int numProcs_Waiting = 0;

void ProcessQueues_init() {
    STAILQ_INIT(&pq[RUNNING]); // should only ever have one element
    STAILQ_INIT(&pq[WAITING]); // waiting for scheduler
    STAILQ_INIT(&pq[BLOCKED]); // waiting on disk
    STAILQ_INIT(&pq[NOTSTARTED]);
    STAILQ_INIT(&pq[FINISHED]);
}

/** Insert to a sorted priority queue*/
static void ProcessQueue_enqueuePriority(Process* p, struct processQueue_t* q) {
    Process* head = STAILQ_FIRST(q);

    do {
        if (head == NULL || (p->currentline <= head->currentline)) {
            STAILQ_INSERT_AFTER(q, p, head, procs);
        }
    } while ((head = STAILQ_NEXT(head, procs)) != NULL);
}

int ProcessQueue_numWaitingProcs() {
    return numProcs_Waiting;
}

static void ProcessQueue_enqueue(Process* p, struct processQueue_t* q) {
    if (q == &pq[WAITING]) {
        ProcessQueue_enqueuePriority(p, q);
        numProcs_Waiting++;
        STAILQ_INSERT_TAIL(q, p,
                           procs); // TODO replace with sorted insert function
    } else {
        STAILQ_INSERT_TAIL(q, p, procs);
    }
}

void ProcessQueue_printQueue(ProcessStatus q_s) {
    Process* head = STAILQ_FIRST(&pq[q_s]);

    printf("Queue: ");
    switch (q_s) {
        case RUNNING:
            printf("RUNNING\n");
            break;
        case WAITING:
            printf("WAITING\n");
            break;
        case BLOCKED:
            printf("BLOCKED\n");
            break;
        /*case NOTSTARTED:
            printf("NOTSTARTED\n");
            break;*/
        case NUM_OF_PROCESS_STATUSES:
            printf("NUM_OF_PROCESS_STATUSES (not a valid queue)");
            break;
        case FINISHED:
            printf("FINISHED\n");
            break;
    }
    if (head == NULL) printf("Empty Queue \n");
    int i = 0;
    do {
        printf(
          "\t\x1B[2m->\x1B[0m\x1B[33m%3d\x1B[0m\x1B[1m pid: %ld start: "
          "%ld end: %ld \x1B[0m, INTERVALS: ",
          i, head->pid, head->firstline, head->lastline);
        it_print(head->lineIntervals);
        printf("\n");
        i++;
    } while ((head = STAILQ_NEXT(head, procs)) != NULL);
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

    new = VPage_init(new->vpn, new->pid);

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
 */
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
}


/**
 * Peek at the status at the head of a given status queue. NULL if none present.
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
    p->waiting_VPN = NULL;
    p->lineIntervals = lineIntervals;

    p->currentPos = lineIntervals->fpos_start;
    p->currInterval = lineIntervals;

    p->pageTable = PageTable_init();
    p->status = NOTSTARTED;
    STAILQ_INSERT_TAIL(&pq[NOTSTARTED], p, procs);

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
    if (p == NULL) return NULL;
    STAILQ_REMOVE_HEAD(&pq[s1], procs);
    p->status = s2;
    ProcessQueue_enqueue(p, &pq[s2]);
    return p;
}

/**
 * Checks to see if the specified process has more lines left to run.
 * @param p process
 * @return true if there are lines left for the process to run, false if all
 * trace lines have completed
 */
inline bool Process_hasLinesRemaining(const Process* p) {
    assert(p->currInterval != NULL);
    return (p->currInterval->right == NULL);
}

/**
 * Context switch; we are done with the current process for now, but it will
 * appear again.
 */
inline void Process_jumpToNextInterval(Process* p) {
    p->currInterval = p->currInterval->right;
    p->currentline = p->currInterval->low;
    p->currentPos = p->currInterval->fpos_start;
}

VPage* Process_allocVirtualPage(Process* p, unsigned long vpn) {
    VPage* v = VPage_init(p->pid, vpn);
    PageTable_add(p->pageTable, v);
    return v;
}

VPage* Process_getVirtualPage(Process* p, unsigned long vpn) {
    return PageTable_get(p->pageTable, vpn);
}

bool Process_virtualPageInMemory(Process* p, unsigned long vpn) {
    VPage* v = PageTable_get(p->pageTable, vpn);
    if (v == NULL) return false;
    return v->inMemory;
}

/**
 * MUST have a free space in memory before calling
 */
unsigned long Process_loadVirtualPage(Process* p, unsigned long vpn) {
    VPage* v = PageTable_get(p->pageTable, vpn);
    assert(v != NULL && "Can't load null into RAM.");
    assert(v->inMemory == false && "Page already in RAM.");
    assert(Memory_hasFreePage() && "Must be free space in memory to add to.");
    unsigned long ppn = Memory_getFreePage();
    Memory_loadPage(v, ppn);
    return v->inMemory;

}

/**
 * Destructs and frees a Process.
 * @param p pointer to heap-alloc'd process
 */
void Process_free(Process* p) { free(p); }
