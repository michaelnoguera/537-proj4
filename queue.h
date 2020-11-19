/**
 * CS 537 Programming Assignment 4 (Fall 2020)
 * @author Michael Noguera (noguera) <mnoguera(at)wisc.edu>
 * @author Julien de Castelnau (de-castelnau) <decastelnau(at)wisc.edu>
 * @date 10/13/2020
 *
 * @file queue.h
 * @brief Thread-safe queue implementation that holds void pointers
 * @version 2.0
 */

#ifndef _QUEUE_
#define _QUEUE_

#include <pthread.h>
#include <time.h>

/**
 * Thread-safe implementation of a dynamically-allocated array-based queue
 *
 * @details The queue has fixed size (set by q_initialize), and is implemented
 * as a dynamically-allocated circular buffer.
 *
 * All supported operations are O(1). Synchronization is handled through a
 * single mutex lock, so although this structure is thread-safe, it does not
 * support simultaneous enqueue and dequeue operations.
 *
 * Behavior is not guaranteed if internal fields are modified by external
 * functions. Do not add items to the buffer directly, or access members by
 * reading them directly, rather prefer q_enqueue() and q_dequeue() as provided
 * here.
 */
typedef struct Queue {
    pthread_mutex_t lock;
    pthread_cond_t empty;
    pthread_cond_t full;

    int head;     // next index to enqueue at
    int tail;     // next index to dequeue from
    size_t size;  // max items the queue can hold
    void* item[]; // array of void* contained in the queue
} Queue;

/**
 * Initializes a new empty Queue.
 *
 * @param size queue capacity
 * @return pointer to new heap-allocated Queue
 */
Queue* q_initialize(const size_t size);

/**
 * Adds a new Node to the end of a Queue.
 *
 * @param q The queue to enqueue to.
 * @param value The value of the new node, of type void*.
 */
void q_enqueue(Queue* q, void* value);

/**
 * Removes a node from the end of a queue
 *
 * @param q The queue from which to dequeue from
 * @return the pointer that was removed, which can point to NULL
 */
void* q_dequeue(Queue* q);

/**
 * Peek at next node to be dequeued
 *
 * @param q the queue, which will not be altered
 * @return the next value that would be returned by dequeue, without actually
 * removing it from the queue
 */
void* q_peek(Queue* q);

/**
 * Frees a Queue struct and all values presently in it. Only call this method
 * when you are sure the queue will not be used again.
 *
 * @param ptr to Queue struct that should be freed.
 */
void q_free(Queue* ptr);
#endif
