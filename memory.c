#include "memory.h"
#include <assert.h>
#include <stdio.h>
#include <sys/queue.h>

// a representation of physical memory
static PPage** memory;
// holds memory size in pages
static size_t mem_size;
// holds the number of allocated pages
static size_t allocated;

// BitMap/Bitvector containing a 0 or 1 at a position given by the PPN,
// depending on whether the page is allocated or not, respectively.
static freelist_t freelist;

// returns the index in the bitmap array which corresponds to the
// integer chunk given by n
static inline ul64 bv_ind(ul64 n) { return n / 32; }

// returns the offset in a chunk of a bitmap array given by n
static inline unsigned int bv_ofs(ul64 n) { return n % 32; }

// finds the index of the first 0 (unallocated) in a freelist chunk
// MSB indexed starting at 0, -1 if there are no 0s
static inline int bv_ffz(unsigned int n) {
    return (n == 0) ? -1 : __builtin_clz(~n);
}

/**
 * Initialize a new free page at the ppn specified.
 * @ppn ppn of page to create
 */
static PPage* Page_init(ul64 ppn) {
    PPage* p = malloc(sizeof(PPage));
    if (p == NULL) {
        perror("memory allocation failed");
        exit(EXIT_FAILURE);
    }

    p->ppn = ppn;
    p->virtualPage = NULL;

    return p;
}

/**
 * Initialize Memory module.
 * @param numberOfPhysicalPages amount of physical memory/page size
 */
void Memory_init(size_t numberOfPhysicalPages) {
    // allocate contiguous chunk of memory to be "Memory", O(1) ppn resolution
    // via indexing
    mem_size = numberOfPhysicalPages;
    allocated = 0;
    PPage** memory = calloc(mem_size, sizeof(PPage));
    if (memory == NULL) {
        perror("memory allocation failed");
        exit(EXIT_FAILURE);
    }

    freelist = (freelist_t)malloc(sizeof(unsigned int) * (mem_size / 32));

    // they're meant to be left uninitialized, not like this
    // for (size_t p = 0; p < mem_size; p++) { Page_init(p); }
}

/**
 * Accesses the physical page with a given ppn
 * @param ppn index into memory
 * @return PPage if present, NULL if not
 */
PPage* Memory_getPPage(ul64 ppn) { return memory[ppn]; }

/**
 * Accesses the virtual page with a given ppn
 */
VPage* Memory_getVPage(ul64 ppn) {
    assert(memory[ppn] != NULL);
    return memory[ppn]->virtualPage;
}

/**
 * Frees the page at a given ppn by sending the virtual page to backing store
 */
void Memory_evictPage(ul64 ppn) {
    PPage* page = memory[ppn];
    assert(page != NULL);
    free(page); // free page

    // remove page from free list (mark as clear)
    // XOR with an integer containing 1 at the offset given by PPN
    // has effect of flipping at the offset position
    freelist[bv_ind(ppn)] ^= 0x1 << bv_ofs(ppn);
    allocated--;        // tick allocated counter
    memory[ppn] = NULL; // finally nullify at memory array
}

/**
 * Loads a page at the given PPN, replacing the old page if applicable
 * @details O(1), simply changes a few pointers
 * @param virtualPage page to load as replacement
 * @param PPN location to load in memory
 */
void Memory_loadPage(VPage* virtualPage, ul64 ppn) {
    PPage* page = Page_init(ppn);
    page->virtualPage = virtualPage;
    memory[ppn] = page;

    // add page to free list (mark as taken)
    // OR with an integer containing 1 at the offset given by PPN
    // has effect of setting to 1 at the offset position
    freelist[bv_ind(ppn)] |= 0x1 << bv_ofs(ppn);
    allocated++;        // tick allocated counter
    memory[ppn] = NULL; // finally nullify at memory array
}

/**
 * @return the ppn of the next free page, or an out of bounds index if none
 */
ul64 Memory_getFreePage() {
    ul64 fl_ind = 0;
    for (fl_ind = 0; fl_ind < mem_size; fl_ind++) {
        int fz_ind = bv_ffz(freelist[fl_ind]);
        if (fz_ind >= 0) { return fl_ind * 32 + fz_ind; }
    }
    perror(
      "WARN: Tried to get free page when none are avaliable. Use "
      "Memory_hasFreePage to check first.");
    return mem_size + 1; // out-of-bounds value as sentinel
}

/**
 * @return true if there is a free page in memory
 */
bool Memory_hasFreePage() { return mem_size == allocated; }

/**
 * Constructs a new Virtual Page given it's virtual identifier.
 * Used by Page Table to get virtual pages.
 * @param pid process id
 * @param vpn virtual page number
 * @return pointer to new VPage struct.
 */
VPage* VPage_init(ul64 pid, ul64 vpn) {
    VPage* v = malloc(sizeof(VPage));
    if (v == NULL) {
        perror("memory allocation failed");
        exit(EXIT_FAILURE);
    }

    v->pid = pid;
    v->vpn = vpn;
    // v->overhead = Replace_initOverhead();

    v->inMemory = false;

    return v;
}

void VPage_free(VPage* vp) {
    // Replace_freeOverhead(vp->overhead);
    free(vp);
}
