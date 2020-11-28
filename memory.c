#include "memory.h"
#include <assert.h>
#include <stdio.h>
#include <sys/queue.h>

static PPage** memory;
static struct freelist_t* freelist_head;
static SLIST_HEAD(freelist_t,
                  freelist_head) freelist = SLIST_HEAD_INITIALIZER(freelist);

/**
 * Initialize a new free page at the ppn specified.
 * @ppn ppn of page to create
 */
static PPage* Page_init(unsigned long ppn) {
    PPage* p = malloc(sizeof(PPage));
    if (p == NULL) {
        perror("memory allocation failed");
        exit(EXIT_FAILURE);
    }

    p->ppn = ppn;
    p->virtualPage = NULL;

    // place physical page in memory
    memory[ppn] = p;

    // add page to free list
    SLIST_INSERT_HEAD(&freelist, p, node);

    return p;
}

/**
 * Initialize Memory module.
 * @param numberOfPhysicalPages amount of physical memory/page size
 */
void Memory_init(size_t numberOfPhysicalPages) {
    // allocate contiguous chunk of memory to be "Memory", O(1) ppn resolution
    // via indexing
    PPage** memory = malloc(sizeof(PPage) * numberOfPhysicalPages);
    if (memory == NULL) {
        perror("memory allocation failed");
        exit(EXIT_FAILURE);
    }

    SLIST_INIT(&freelist);

    // request overhead struct from replacement strategy module

    for (size_t p = 0; p < numberOfPhysicalPages; p++) { Page_init(p); }
}

/**
 * Accesses the physical page with a given ppn
 * @param ppn index into memory
 * @return PPage if present, NULL if not
 */
PPage* Memory_getPPage(unsigned long ppn) {
    return memory[ppn];
}

/**
 * Accesses the virtual page with a given ppn
 */
VPage* Memory_getVPage(unsigned long ppn) {
    assert(memory[ppn] != NULL);
    return memory[ppn]->virtualPage;
}

/**
 * Frees the page at a given ppn by sending the virtual page to backing store
 */
void Memory_evictPage(unsigned long ppn);

/**
 * @return the ppn of the next free page
 */
unsigned long Memory_getFreePage();

/**
 * @return true if there is a free page in memory
 */
bool Memory_hasFreePage();

/**
 * Load a page in to memory by calling replacement module to find its spot
 * @return 0 upon success, 1 upon failure
 */
int Memory_load(VPage* virtualPage);

/**
 * Evicts the page at a given PPN, replacing it with the specified new page.
 * Significantly faster than evicting, searching for free, getting free, and storing.
 * @details O(1), simply changes a few pointers
 * @param virtualPage page to load as replacement
 * @param PPN location to load in memory
 */
void Page_replace(VPage* virtualPage, unsigned long ppn);

/**
 * Constructs a new Virtual Page given it's virtual identifier.
 * Used by Page Table to get virtual pages.
 * @param pid process id
 * @param vpn virtual page number
 * @return pointer to new VPage struct.
 */
VPage* VPage_init(unsigned long pid, unsigned long vpn) {
    VPage* v = malloc(sizeof(VPage));
    if (v == NULL) {
        perror("memory allocation failed");
        exit(EXIT_FAILURE);
    }

    v->vpn = vpn;
    //v->overhead = Replace_initOverhead();

    v->inMemory = false;

    return v;
}

void VPage_free(VPage* vp) {
    //Replace_freeOverhead(vp->overhead);
    free(vp);
}
