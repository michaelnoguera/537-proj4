/**
 * CS 537 Programming Assignment 4 (Fall 2020)
 * @file replace-lru.c
 * @brief Replacement module implementing the LRU (Least-Recently-Used) policy.
 * Overhead is tied to virtual rather than physical pages.
 * @author Michael Noguera
 */

#include "replace.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/queue.h>

/**
 * LRU Queue
 *  - oldest items are on the head of the queue
 *  - based on a doubly-linked tail queue
 */
static TAILQ_HEAD(lrq_t, lrunit) lrq = TAILQ_HEAD_INITIALIZER(lrq);
struct lrq_t* headp;

// LRU queue entry type
struct lrunit {
    VPage* parent; // reverse pointer to virtual page this overhead represents
    bool inQueue;  // track if page is in use by this module for O(1) cleanup
    TAILQ_ENTRY(lrunit) entries; // list overhead
};

static int capacity = 0; // size of physical memory, thus max size of queue
static int pages = 0; // current size of queue

/** Initializes replacement module overhead and LRU queue */
void Replace_initReplacementModule(int numberOfPhysicalPages) {
    capacity = numberOfPhysicalPages;
    TAILQ_INIT(&lrq);
}

/** Frees replacement module overhead and LRU queue */
void Replace_freeReplacementModule() {
    // "faster tailq deletion" from queue(2) man page
    struct lrunit* n1 = TAILQ_FIRST(&lrq);
    while (n1 != NULL) {
        struct lrunit* n2 = TAILQ_NEXT(n1, entries);
        free(n1);
        n1 = n2;
    }
}

/**
 * Constructs a new overhead struct for use with this replacement module
 * @param vpage pointer to virtual page this is overhead for
 * @return overhead struct
 */
void* Replace_initOverhead(VPage* vpage) {
    assert(vpage != NULL && vpage->overhead == NULL);

    struct lrunit* overhead = malloc(sizeof(struct lrunit));
    if (overhead == NULL) {
        perror("Memory allocation error in replacement module.");
        exit(EXIT_FAILURE);
    }
    overhead->parent = vpage;
    overhead->inQueue = false;

    return overhead;
}
/**
 * Free overhead given a pointer
 * @details if still in replacement queue, removes in O(1)
 * @param o_ptr overhead struct to free
 */
void Replace_freeOverhead(void* o_ptr) {
    assert(o_ptr != NULL);
    assert(pages >= 0 && pages <= capacity);

    // remove if necessary
    struct lrunit* overhead = (struct lrunit*)o_ptr;
    if (overhead->inQueue) {
        TAILQ_REMOVE(&lrq, overhead, entries);
        pages--;
    }

    free(o_ptr);
}

/**
 * Return page to tail of LRU queue
 * @details O(1)
 * @param o_ptr pointer to overhead struct in virtual page
 */
void Replace_notifyPageAccess(void* o_ptr) {
    assert(o_ptr != NULL);
    assert(pages >= 0 && pages <= capacity);

    // grab the page and stick it back on the tail of the queue
    struct lrunit* overhead = (struct lrunit*)o_ptr;
    assert(overhead->inQueue);
    TAILQ_REMOVE(&lrq, overhead, entries);
    TAILQ_INSERT_TAIL(&lrq, overhead, entries);
}

/**
 * Enqueue page to LRU queue
 * @details O(1)
 * @param o_ptr pointer to overhead struct in virtual page
 */
void Replace_notifyPageLoad(void* o_ptr) {
    assert(o_ptr != NULL);
    assert(pages >= 0 && pages <= capacity);

    // perform enqueue, update overhead
    struct lrunit* overhead = (struct lrunit*)o_ptr;
    assert(overhead->inQueue != true);
    overhead->inQueue = true;
    TAILQ_INSERT_TAIL(&lrq, overhead, entries); // tail holds newest items
    pages++;
}

/**
 * Pop oldest page from LRU queue
 * @details O(1)
 * @return PPN of page to evict
 */
unsigned long Replace_getPageToEvict() {
    assert(pages >= 0 && pages <= capacity);

    // head of queue holds oldest item
    struct lrunit* n1 = TAILQ_FIRST(&lrq);

    // remove page from replacement module
    TAILQ_REMOVE(&lrq, n1, entries);
    n1->inQueue = false;
    pages--;

    return n1->parent->currentPPN;
}