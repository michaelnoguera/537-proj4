#include "process.h"
#include "memory.h"
#include <search.h>
#include <stdio.h>

static STAILQ_HEAD(processQueue_t, process_t) pq[NUM_OF_PROCESS_STATUSES];

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

static void ProcessQueue_enqueue(Process* p, struct processQueue_t* q) {
    if (q == &pq[WAITING]) {
        ProcessQueue_enqueuePriority(p, q);
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
        case NOTSTARTED:
            printf("NOTSTARTED\n");
            break;
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
    p->lineIntervals = lineIntervals;

    p->currentPos = lineIntervals->fpos_start;
    p->currInterval = lineIntervals;

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
 * Destructs and frees a Process.
 * @param p pointer to heap-alloc'd process
 */
void Process_free(Process* p) { free(p); }


PageTable* PageTable_init() {
    PageTable* pt;
    if ((pt = (PageTable*)malloc(sizeof(PageTable))) == NULL) {
        perror("Error allocating memory for page table.");
        exit(EXIT_FAILURE);
    }
    return pt;
}

int PageTable_compare(const void* vp1, const void* vp2) {
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

VPage* PageTable_get(PageTable* pt, int vpn, int pid) {
    VPage* search_query;
    VPage* search_result;
    VPage* temp_result;

    search_query = VPage_init(vpn, pid);

    if ((search_result = tfind(search_query, pt, PageTable_compare)) == NULL) {
        VPage_free(search_query);
        return NULL;
    } else {
        temp_result = *(VPage**)search_result;
        if (temp_result->inMemory) {
            return temp_result;
        } else {
            return NULL;
        }
    }
}

int PageTable_add(PageTable* pt, int vpn, int pid, int ppn) {
    VPage* new_page;
    VPage* search_result;
    VPage* temp_result;

    new_page = VPage_init(vpn, pid);

    if ((search_result = tsearch(new_page, pt, PageTable_compare)) == NULL) {
        perror("Error in page table lookup.");
        exit(EXIT_FAILURE);
    } else {
        temp_result = *(VPage**)search_result;

        if (temp_result != new_page) {
            // existing node found
            VPage_free(new_page);
            return 1;
        } else {
            new_page->currentPPN = ppn;
            return 0;
        }
    }
}

int PageTable_remove(PageTable* pt, int vpn, int pid) {
    VPage* entry_to_remove;

    if ((entry_to_remove = PageTable_get(pt, vpn, pid)) == NULL) {
        // Entry removal failed. The node was not found in the page table.
        return 1;
    } else {
        // Entry was found. call tdelete() and remove
        tdelete(entry_to_remove, pt, PageTable_compare);
        free(entry_to_remove);
        return 0;
    }
}