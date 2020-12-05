#include "replace.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/queue.h>

#pragma message "replace-lru is still incomplete"


static TAILQ_HEAD(lrq_t, lrunit) lrq = TAILQ_HEAD_INITIALIZER(lrq);
struct lrq *headp;
struct lrunit {
    VPage* parent;
    TAILQ_ENTRY(lrunit) entries;
};

static int numPages;

void Replace_initReplacementModule(int numberOfPhysicalPages) {
    numPages = numberOfPhysicalPages;
    TAILQ_INIT(&lrq);
}

void Replace_freeReplacementModule() {
    // "faster tailq deletion" from queue(2) man page
    struct lrunit* n1 = TAILQ_FIRST(&lrq);
    while (n1 != NULL) {
        struct lrunit* n2 = TAILQ_NEXT(n1, entries);
        free(n1);
        n1 = n2;
    }
}

void* Replace_initOverhead(VPage* vpage) {
    struct lrunit* overhead = malloc(sizeof(struct lrunit));
    if (overhead == NULL) {
        perror("Memory allocation error in replacement module.");
        exit(EXIT_FAILURE);
    }
    overhead->parent = vpage;
    TAILQ_INSERT_HEAD(&lrq, overhead, entries);
    #pragma message "replace-lru does not properly initialize overhead yet"

    return overhead;
}

void Replace_freeOverhead(void* o_ptr) {
    struct lrunit* overhead = (struct lrunit*)o_ptr;
    TAILQ_REMOVE(&lrq, overhead, entries);
    free(overhead);
}

void Replace_updateOverhead(void* o_ptr) {
    struct lrunit* overhead = (struct lrunit*)o_ptr;
    TAILQ_REMOVE(&lrq, overhead, entries);
    TAILQ_INSERT_HEAD(&lrq, overhead, entries);
}

void Replace_notifyPageAccess(int ppn) {
    struct lrunit* overhead = (struct lrunit*)Memory_getVPage(ppn)->overhead;
    TAILQ_REMOVE(&lrq, overhead, entries);
    TAILQ_INSERT_HEAD(&lrq, overhead, entries);
}

unsigned long Replace_getPageToEvict() {
    #pragma message "replace-lru has no eviction policy yet"
    return 0;
}