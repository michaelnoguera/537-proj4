#include "memory.h"
#include "replace.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/queue.h>

#pragma message "replace-lru is still incomplete"

static TAILQ_HEAD(lrq_t, lrunit) lrq = TAILQ_HEAD_INITIALIZER(lrq);

struct lrq_t* headp;
struct lrunit {
    VPage* parent;
    TAILQ_ENTRY(lrunit) entries;
};

static inline bool Replace_isValid(struct lrunit* l) {
    if (l == NULL) return false;
    if (l->parent == NULL) return false;
    if (l->parent->inMemory == false) return false;
    if (l->parent->overhead != l) return false;
    if (l->parent->pid == 0) return false;
    return true;
}

static inline unsigned long Replace_ppnFromUnit(struct lrunit* l) {
    return l->parent->currentPPN;
}

static int capacity = 0;
static int pages = 0;

void Replace_initReplacementModule(int numberOfPhysicalPages) {
    capacity = numberOfPhysicalPages;
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
    
    return overhead;
}

void Replace_freeOverhead(void* o_ptr) {
    assert(pages >= 0);
    assert(pages <= capacity);
    struct lrunit* overhead = (struct lrunit*)o_ptr;
    if (overhead->entries.tqe_next || overhead->entries.tqe_prev) {
        TAILQ_REMOVE(&lrq, overhead, entries);
        pages--;
    }
    free(o_ptr);
}

void Replace_notifyPageAccess(void* o_ptr) {
    assert(pages >= 0);
    assert(pages <= capacity);
    struct lrunit* overhead = (struct lrunit*)o_ptr;
    assert(overhead->entries.tqe_next || overhead->entries.tqe_prev);
    TAILQ_REMOVE(&lrq, overhead, entries);
    TAILQ_INSERT_TAIL(&lrq, overhead, entries);
}

void Replace_notifyPageLoad(void* o_ptr) {
    assert(pages >= 0);
    assert(pages <= capacity);
    assert(o_ptr != NULL);
    struct lrunit* overhead = (struct lrunit*)o_ptr;
    TAILQ_INSERT_TAIL(&lrq, overhead, entries);
    pages++;
}

unsigned long Replace_getPageToEvict() {
    assert(!Memory_hasFreePage());
    assert(pages >= 0);
    assert(pages <= capacity);
    printf("first in queue = %lu, inMemory = %i\n",
           TAILQ_FIRST(&lrq)->parent->vpn, TAILQ_FIRST(&lrq)->parent->inMemory);
    // assert(TAILQ_FIRST(&lrq)->parent->inMemory);
    struct lrunit* n1 = TAILQ_FIRST(&lrq);
    if (!Replace_isValid(n1)) {
        perror("Invalid entry in replacement list");
        exit(EXIT_FAILURE);
    }
    TAILQ_REMOVE(&lrq, n1, entries);
    pages--;
    return n1->parent->currentPPN;
}