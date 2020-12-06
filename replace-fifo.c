#include "replace.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/queue.h>

#pragma message "replace-fifo has not been tested"


static TAILQ_HEAD(fifoq_t, fifounit) fifoq = TAILQ_HEAD_INITIALIZER(fifoq);
struct fifoq *headp;
struct fifounit {
    VPage* parent;
    TAILQ_ENTRY(fifounit) entries;
};

static int numPages;
static int numAllocated;

void Replace_initReplacementModule(int numberOfPhysicalPages) {
    numPages = numberOfPhysicalPages;
    TAILQ_INIT(&fifoq);
}

void Replace_freeReplacementModule() {
    // "faster tailq deletion" from queue(2) man page
    struct fifounit* n1 = TAILQ_FIRST(&fifoq);
    while (n1 != NULL) {
        struct fifounit* n2 = TAILQ_NEXT(n1, entries);
        free(n1);
        n1 = n2;
    }
}

void* Replace_initOverhead(VPage* vpage) {
    struct fifounit* overhead = malloc(sizeof(struct fifounit));
    if (overhead == NULL) {
        perror("Memory allocation error in replacement module.");
        exit(EXIT_FAILURE);
    }
    overhead->parent = vpage;
    TAILQ_INSERT_HEAD(&fifoq, overhead, entries);

    return (void*)overhead;
}

void Replace_freeOverhead(void* o_ptr) {
    struct fifounit* overhead = (struct fifounit*)o_ptr;
    TAILQ_REMOVE(&fifoq, overhead, entries);
    free(overhead);
}

void Replace_notifyPageAccess(void* o_ptr) {
    struct fifounit* overhead = (struct fifounit*)o_ptr;
    TAILQ_REMOVE(&fifoq, overhead, entries);
    TAILQ_INSERT_HEAD(&fifoq, overhead, entries);
    numAllocated++;
}

void Replace_notifyPageLoad(void* o_ptr) {
    struct fifounit* overhead = (struct fifounit*)o_ptr;
    TAILQ_REMOVE(&fifoq, overhead, entries);
}

unsigned long Replace_getPageToEvict() {
    int ret_ppn = 0;
    if(numAllocated == numPages) {
        struct fifounit* last = TAILQ_LAST(&fifoq, fifoq_t);
        assert(last->parent->inMemory);
        int ret_ppn = last->parent->currentPPN;
        TAILQ_REMOVE(&fifoq, last, entries);
    }
    return ret_ppn;
}