/**
 * CS 537 Programming Assignment 4 (Fall 2020)
 * @file replace-fifo.c
 * @brief Replacement module implementing the FIFO (First In, First Out) policy.
 * Overhead is tied to virtual rather than physical pages.
 * @author Michael Noguera
 * @details based on the LRU implementation, does not use a singly linked list
 * because that would make removal O(n)
 */

#include "replace.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/queue.h>

/**
 * FIFO Queue
 *  - oldest items are on the head of the queue
 *  - based on a doubly-linked tail queue
 */
static TAILQ_HEAD(fifo_queue_t,
                  fifo_item) fifo_queue = TAILQ_HEAD_INITIALIZER(fifo_queue);
struct fifo_queue_t* headp;

// FIFO queue entry type
struct fifo_item {
    VPage* parent; // reverse pointer to virtual page this overhead represents
    bool inQueue;  // track if page is in use by this module for O(1) cleanup
    TAILQ_ENTRY(fifo_item) entries; // list overhead
};

static int capacity = 0; // size of physical memory, thus max size of queue
static int pages = 0;    // current size of queue

/** Initializes replacement module overhead and FIFO queue */
void Replace_initReplacementModule(int numberOfPhysicalPages) {
    capacity = numberOfPhysicalPages;
    TAILQ_INIT(&fifo_queue);
}

/** Frees replacement module overhead and FIFO queue */
void Replace_freeReplacementModule() {
    // "faster tailq deletion" from queue(2) man page
    struct fifo_item* n1 = TAILQ_FIRST(&fifo_queue);
    while (n1 != NULL) {
        struct fifo_item* n2 = TAILQ_NEXT(n1, entries);
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

    struct fifo_item* overhead = malloc(sizeof(struct fifo_item));
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
    struct fifo_item* overhead = (struct fifo_item*)o_ptr;
    if (overhead->inQueue) {
        TAILQ_REMOVE(&fifo_queue, overhead, entries);
        pages--;
    }

<<<<<<< HEAD
void Replace_notifyPageAccess(void* o_ptr) {
    struct fifounit* overhead = (struct fifounit*)o_ptr;
    printf("FIFO: Adding %ld,%ld to queue\n", overhead->parent->pid, overhead->parent->vpn);
    TAILQ_REMOVE(&fifoq, overhead, entries);
    TAILQ_INSERT_HEAD(&fifoq, overhead, entries);
    numAllocated++;
=======
    free(o_ptr);
>>>>>>> 89c05d453264ceb75e77733de7ba9259bc143fbd
}

/**
 * Does nothing, because FIFO does not adapt based frequency of access
 */
void Replace_notifyPageAccess(__attribute__((unused)) void* o_ptr) {}


/**
 * Enqueue page to FIFO queue
 * @details O(1)
 * @param o_ptr pointer to overhead struct in virtual page
 */
void Replace_notifyPageLoad(void* o_ptr) {
<<<<<<< HEAD
    struct fifounit* overhead = (struct fifounit*)o_ptr;
    printf("FIFO: Removing %ld,%ld from queue\n", overhead->parent->pid, overhead->parent->vpn);
    TAILQ_REMOVE(&fifoq, overhead, entries);
=======
    assert(o_ptr != NULL);
    assert(pages >= 0 && pages <= capacity);

    // perform enqueue, update overhead
    struct fifo_item* overhead = (struct fifo_item*)o_ptr;
    assert(overhead->inQueue != true);
    overhead->inQueue = true;
    TAILQ_INSERT_TAIL(&fifo_queue, overhead,
                      entries); // tail holds newest items
    pages++;
>>>>>>> 89c05d453264ceb75e77733de7ba9259bc143fbd
}

/**
 * Pop oldest page from FIFO queue
 * @details O(1)
 * @return PPN of page to evict
 */
unsigned long Replace_getPageToEvict() {
<<<<<<< HEAD
    FIFO_printQueue();
    struct fifounit* last = TAILQ_LAST(&fifoq, fifoq_t);
    assert(last->parent->inMemory);
    int ret_ppn = last->parent->currentPPN;
    TAILQ_REMOVE(&fifoq, last, entries);

    printf("FIFO: Evicting PPN %d\n", ret_ppn);
    return ret_ppn;
=======
    assert(pages >= 0 && pages <= capacity);

    // head of queue holds oldest item
    struct fifo_item* n1 = TAILQ_FIRST(&fifo_queue);

    // remove page from replacement module
    TAILQ_REMOVE(&fifo_queue, n1, entries);
    n1->inQueue = false;
    pages--;

    return n1->parent->currentPPN;
>>>>>>> 89c05d453264ceb75e77733de7ba9259bc143fbd
}