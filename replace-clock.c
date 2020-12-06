#include "replace.h"
#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>

struct clock_overhead {
    VPage* parent;
};

static unsigned long clock_hand;
static int numPages;
static bool* shadowMem_Reflist;

void Replace_initReplacementModule(int numberOfPhysicalPages) {
    clock_hand = 0;
    numPages = numberOfPhysicalPages;
    if ((shadowMem_Reflist = (bool*)malloc(sizeof(bool) * numPages)) == NULL) {
        perror("Cannot allocate memory for clock algorithm shadow rarray.");
    }
}

void Replace_freeReplacementModule() { return; }

void* Replace_initOverhead(__attribute__((unused)) VPage* vpage) {
    struct clock_overhead* co =
      (struct clock_overhead*)malloc(sizeof(struct clock_overhead));
    co->parent = vpage;
    return co;
}

void Replace_freeOverhead(void* o_ptr) {
    free((struct clock_overhead*)o_ptr);
}

void Replace_notifyPageAccess(void* o_ptr) {
    // We assume the Vpage is in memory because this gets called
    // after it was just referenced.
    assert(((struct clock_overhead*)o_ptr)->parent->inMemory);
    shadowMem_Reflist[((struct clock_overhead*)o_ptr)->parent->currentPPN] =
      true;
}

// unimplemented
void Replace_notifyPageLoad(void* o_ptr) { Replace_notifyPageAccess(o_ptr); }

unsigned long Replace_getPageToEvict() {
    assert(!Memory_hasFreePage());
    // unsigned long start_pos = clock_hand;

    while (shadowMem_Reflist[clock_hand] == true) {
        shadowMem_Reflist[clock_hand] = false;
        clock_hand = (clock_hand + 1) % numPages;
    }
    return (clock_hand);
}