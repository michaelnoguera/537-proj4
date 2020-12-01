// Simulates memory.

#include "intervaltree.h"
#include "linkedlist.h"
#include <stdbool.h>
#include <sys/queue.h>

#ifndef _MEMORY_
#define _MEMORY_

/*
- -
----------------------------------------------------------------------------------------------------------
- -
    ...|ppn,pnode,vpn,pid,vnode,overhead|ppn,pnode,vpn,pid,vnode,overhead|ppn,pnode,vpn,pid,vnode,overhead|...
- -
----------------------------------------------------------------------------------------------------------
- -
^*page
*/

typedef unsigned int* freelist_t;
// use a better name? unsigned long was appearing in too 
// many places, it was getting quite long to type out
typedef unsigned long ul64; 

typedef struct vpage_t {
    // VIRTUAL page identified by <VPN, PID>
    ul64 vpn; // virtual page number
    ul64 pid; // process id

    void* overhead; // for replacement policy

    // page has a PHYSICAL location as well
    bool inMemory;
    ul64 currentPPN;
} VPage;

// Represents a page from memory, identified both by a pid, vpn pair and a ppn
// USE THIS AS VALUE IN TREES
typedef struct ppage_t {
    ul64 ppn; // physical page number
    SLIST_ENTRY(freelistnode_t) node;

    VPage* virtualPage;
} PPage;

// allocated once we know the amount of pages (=pmem/pgsize)

/**
 * Initialize Memory module.
 * @param numberOfPhysicalPages amount of physical memory/page size
 */
void Memory_init(size_t numberOfPhysicalPages);

/**
 * Accesses the physical page with a given ppn
 * @param ppn index into memory
 * @return PPage if present, NULL if not
 */
PPage* Memory_getPPage(ul64 ppn);

/**
 * Accesses the virtual page with a given ppn
 */
VPage* Memory_getVPage(ul64 ppn);

/**
 * Frees the page at a given ppn by sending the virtual page to backing store
 */
void Memory_evictPage(ul64 ppn);

/**
 * Load a page in to memory
 * @return none
 */
void Memory_loadPage(VPage* virtualPage, ul64 ppn);

/**
 * @return the ppn of the next free page
 */
ul64 Memory_getFreePage();

/**
 * @return true if there is a free page in memory
 */
bool Memory_hasFreePage();

/**
 * Constructs a new Virtual Page given it's virtual identifier
 * @param pid process id
 * @param vpn virtual page number
 * @param initOverhead function that returns pointer to overhead struct to store
 * in vpage.
 * Interface for initOverhead must match
 * ```C
 * Overhead* initOverhead(ul64 pid, ul64 vpn);
 * ```
 * @return pointer to new VPage struct.
 */
VPage* VPage_init(ul64 pid, ul64 vpn);

void VPage_free(VPage* vp);

// Page_destroy()

// replace(old page, new page)
// bool replace(PPage* old, PPage* new);

#endif