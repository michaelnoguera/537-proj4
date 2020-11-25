#include "memory.h"

Page* Page_init_used(unsigned long ppn, void* pnode, unsigned long vpn,
                     unsigned long pid, void* vnode, void* overhead) {
    Page* p = malloc(sizeof(Page));
    if (p == NULL) {
        perror("memory allocation failed");
        exit(EXIT_FAILURE);
    }

    p->ppn = ppn;
    p->pnode = pnode;
    p->vpn = vpn;
    p->pid = pid;
    p->vnode = vnode;
    p->free = false;
    p->overhead = overhead;
}

Page* Page_init_free(unsigned long ppn, void* pnode, void* overhead) {
    Page* p = malloc(sizeof(Page));
    if (p == NULL) {
        perror("memory allocation failed");
        exit(EXIT_FAILURE);
    }

    p->ppn = ppn;
    p->pnode = pnode;
    p->vpn = 0;
    p->pid = 0;
    p->vnode = NULL;
    p->free = true;
    p->overhead = overhead;
}