#include "process.h"
#include "memory.h"

static STAILQ_HEAD(processQueue_t, process_t) pq[NUM_OF_PROCESS_STATUSES];

void ProcessQueues_init() {
    STAILQ_INIT(&pq[RUNNING]); // should only ever have one element
    STAILQ_INIT(&pq[WAITING]); // waiting for scheduler
    STAILQ_INIT(&pq[BLOCKED]); // waiting on disk
    STAILQ_INIT(&pq[NOTSTARTED]);
    STAILQ_INIT(&pq[FINISHED]);
}

/**
 * Constructs a new Process and initializes internal fields.
 * @param pid pid of new process
 * @param status
 */
Process* Process_init(unsigned long pid, unsigned long firstline,
                      unsigned long lastline, IntervalNode* lineIntervals) {
    Process* p = malloc(sizeof(Process));
    p->pid = pid;
    p->firstline = firstline;
    p->currentline = firstline;
    p->lastline = lastline;
    p->lineIntervals = NULL;
    p->lineIntervals = lineIntervals;
    p->vas = NULL;

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
    STAILQ_INSERT_TAIL(&pq[status], p, procs);
}

/**
 * Pops the Process at the head of the s1 queue and enqueues it on the s2 queue.
 * Faster than setStatus because it doesn't need to search.
 * @param s1 source status
 * @param s2 destination status
 */
void Process_switchStatus(ProcessStatus s1, ProcessStatus s2) {
    Process* p = STAILQ_FIRST(&pq[s1]);
    STAILQ_REMOVE_HEAD(&pq[s1], procs);
    p->status = s2;
    STAILQ_INSERT_TAIL(&pq[s2], p, procs);
}

inline static void ProcessQueue_enqueue(Process* p, struct processQueue_t* q) {
    if (q == &pq[WAITING]) {
        STAILQ_INSERT_TAIL(q, p,
                           procs); // TODO replace with sorted insert function
    } else {
        STAILQ_INSERT_TAIL(q, p, procs);
    }
}

/**
 * Peek at the status at the head of a given status queue. NULL if none present.
 */
Process* Process_peek(ProcessStatus status) {
    return STAILQ_FIRST(&pq[status]);
}

/**
 * Destructs and frees a Process.
 * @param p pointer to heap-alloc'd process
 */
void Process_free(Process* p) {
    free(p);
}

// TODO ProcessQueue_addPriority()
// iterate with STAILQ_NEXT
// use STAILQ_INSERT_AFTER