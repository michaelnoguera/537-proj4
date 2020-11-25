/**
 * CS 537 Programming Assignment 4 (Fall 2020)
 * @author Julien de Castelnau
 * @date 11/24/2020
 * @brief Implementation of a simple priority queue with push, pop and print operations supported.
 * @file pqueue.h
 */

#ifndef _PQUEUE_
#define _PQUEUE_

typedef struct pqueue_node_t {
    void* data;
    int priority;
    struct pqueue_node_t* next;
} PQueueNode;

typedef struct pqueue_t {
    PQueueNode* head;
} PQueue;

PQueue* pq_init();
PQueueNode* pq_initnode(void* data, int priority);
void* pq_pop(PQueue* pq);
void pq_push(PQueue* pq, void* data, int priority);
void pqueue_print(PQueue* pq);

#endif