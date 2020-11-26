#include "memory.h"
#include <stdio.h>
#include <sys/queue.h>

static Page** memory;
static SLIST_HEAD(freelist_t, freelist_head) freelist =
  SLIST_HEAD_INITIALIZER(freelist_head);



Page* Page_init(unsigned long ppn, void* pnode, void* overhead) {
    Page* p = malloc(sizeof(Page));
    if (p == NULL) {
        perror("memory allocation failed");
        exit(EXIT_FAILURE);
    }

    p->ppn = ppn;
    p->pnode = pnode;
    p->vpn = 0; // we don't actually know how its used yet
    p->pid = 0;
    p->vnode = NULL;
    p->overhead = overhead;

    // place physical page in memory
    memory[ppn] = &p;

    // mark page as unused/free
}


void Memory_init(size_t numberOfPhysicalPages) {
    Page** memory = calloc(sizeof(Page), numberOfPhysicalPages);
    if (memory == NULL) {
        perror("memory allocation failed");
        exit(EXIT_FAILURE);
    }

    freelist = SLIST_INIT(&freelist);

    // request overhead struct from replacement strategy module

    for (size_t i = 0; i < numberOfPhysicalPages; i++) {
        Page_init(i, NULL, Replace_initOverhead());
    }
}

// Page_assign(Virtual Address...)