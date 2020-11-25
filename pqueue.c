/**
 * CS 537 Programming Assignment 4 (Fall 2020)
 * @author Julien de Castelnau
 * @date 11/24/2020
 * @brief Implementation of a simple priority queue with push, pop and print operations supported.
 * @file pqueue.c
 */

#include <stdlib.h>
#include <stdio.h>

#include "pqueue.h"

// TODO: Attribution?
// https://www.geeksforgeeks.org/priority-queue-using-linked-list/
// can throw it out if the code is too similar. couldn't find any other examples as good though

PQueueNode* pq_initnode(void* data, int priority) {
    PQueueNode* pqn;
    if ((pqn = (PQueueNode*) malloc(sizeof(PQueueNode))) == NULL) {
        perror("Error allocating memory for new PQueue node.");
        exit(EXIT_FAILURE);
    }
    pqn->data = data;
    pqn->priority = priority;
    pqn->next = NULL;

    return pqn;
}

PQueue* pq_init() {
    PQueue* pq;
    if ((pq = (PQueue*) malloc(sizeof(PQueue))) == NULL) {
        perror("Error allocating memory for new PQueue instance.");
        exit(EXIT_FAILURE);
    }
    return pq;
}

void* pq_pop(PQueue* pq) 
{ 
    PQueueNode* temp = pq->head; 
    void* return_data = pq->head->data;
    pq->head = pq->head->next; 
    free(temp); 
    return return_data;
} 

void pq_push(PQueue* pq, void* data, int priority) {
    PQueueNode* newnode_temp = pq_initnode(data, priority);

    if(pq->head == NULL) {
        pq->head = newnode_temp;
    } else if ((pq->head)->priority > priority) {
        newnode_temp->next = pq->head;
        pq->head = newnode_temp;    
    } else {
        PQueueNode* curr_head = pq->head;

        while (curr_head->next != NULL && 
            curr_head->next->priority < priority) {
            curr_head = curr_head->next;
        }

        newnode_temp->next = curr_head->next;
        curr_head->next = newnode_temp;
    }
}

// PQueue print, specific to Process structs.

#include "memory.h"

void pqueue_print(PQueue* pq) {
    if (pq == NULL) return;
    printf("\x1B[4mPriority Queue\x1B[0m\n");
    printf("\t\x1B[31maddr\x1B[0m = %p\n", (void*)pq);
 
    // print head ptr
    printf("\t\x1B[95m(0 <- head)\x1B[0m\n");

    // print all nodes
    PQueueNode* pqn = pq->head;
    int i = 0;
    while (pqn != NULL) {
        printf(
            "\t\x1B[2m->\x1B[0m\x1B[33m%3d\x1B[0m\x1B[1m pid: %ld start: "
            "%ld end: %ld \x1B[0m, INTERVALS: ",
            i,
            ((Process*)pqn->data)->pid, 
            ((Process*)pqn->data)->firstline, 
            ((Process*)pqn->data)->lastline);
        it_print(((Process*)pqn->data)->lineIntervals);
        printf("\n");
        pqn = pqn->next;
        i++;
    }
}