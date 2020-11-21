typedef struct pqueue_node_t {
    void* data;
    int priority;
    struct pqueue_node_t* next;
} PQueueNode;

typedef struct pqueue_t {
    PQueueNode* head;
} PQueue;

PQueueNode* pq_initnode(void* data, int priority);