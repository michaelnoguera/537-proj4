// Simulates memory.

#include "intervaltree.h"
#include "linkedlist.h"
#include <sys/queue.h>
#include <stdbool.h>

#ifndef _MEMORY_
#    define _MEMORY_

/*
- - ---------------------------------------------------------------------------------------------------------- - -
    ...|ppn,pnode,vpn,pid,vnode,overhead|ppn,pnode,vpn,pid,vnode,overhead|ppn,pnode,vpn,pid,vnode,overhead|...
- - ---------------------------------------------------------------------------------------------------------- - -
^*page
*/

typedef struct vpage_t {
    unsigned long vpn; // virtual page number
    unsigned long pid; // process id
    void* vnode; // pointer to a node in the vpn-keyed tree. there is one of
                 // these trees per process. used for forward page resolution
                 // (vpn->page)
    void* overhead;
} VPage;

// Represents a page from memory, identified both by a pid, vpn pair and a ppn
// USE THIS AS VALUE IN TREES
typedef struct ppage_t {
    unsigned long ppn; // physical page number
    SLIST_ENTRY(freelistnode_t) node;
    void* pnode; // pointer to a node in the ppn-keyed tree, used for reverse
                 // page resolution (ppn->page)
    
    VPage virtualPage;
} PPage;

typedef Page** PAS; // physical address space
// allocated once we know the amount of pages (=pmem/pgsize)


// MAINTAIN FREE LIST OF UNUSED PAGES
struct freelist_t *freelist_head;

typedef Page** VAS; // virtual address space, one per process

// Page_init()

// Page_destroy()

// replace(old page, new page)
//bool replace(Page* old, Page* new);

#endif