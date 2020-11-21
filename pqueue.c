#include <stdlib.h>

#include "pqueue.h"

// TODO: Attribution?
// https://www.geeksforgeeks.org/priority-queue-using-linked-list/
// can throw it out if the code is too similar. couldn't find any other examples as good though

PQueueNode* pq_initnode(void* data, int priority) {
    PQueueNode* pqn;
    if ((pqn == (PQueueNode*) malloc(sizeof(PQueueNode))) == NULL) {
        perror("Error allocating memory for new PQueue node.");
    }
    pqn->data = data;
    pqn->priority = priority;
    pqn->next = NULL;

    return pqn;
}

void pq_pop(PQueue* pq) 
{ 
    PQueueNode* temp = pq->head; 
    pq->head = pq->head->next; 
    free(temp); 
} 

void pq_push(PQueue* pq, void* data, int priority) {
    PQueueNode* newnode_temp = pq_initnode(data, priority);
    int merge_result;

    if ((pq->head)->priority > priority) {
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