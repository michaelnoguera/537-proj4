#include "replace.h"
#include <assert.h>
#include <stdlib.h>

struct clock_overhead {
    unsigned int ref : 1; // refrence bit
};

static unsigned long clock_hand;
static int numPages;

// 
void Replace_initReplacementModule(int numberOfPhysicalPages) {
    clock_hand = 0;
    numPages = numberOfPhysicalPages;
}

void Replace_freeReplacementModule() { return; }

void* Replace_initOverhead() {
    struct clock_overhead* co = 
            (struct clock_overhead*) malloc(sizeof(struct clock_overhead));
    co->ref = 0;
    return co;
}

void Replace_freeOverhead(void* o_ptr) { 
    free((struct clock_overhead*)o_ptr);
}

void Replace_updateOverhead(void* o_ptr) {
    ((struct clock_overhead*)o_ptr)->ref = 1;
}

unsigned long Replace_getPageToEvict() {
    assert(!Memory_hasFreePage());
    unsigned long start_pos = clock_hand;
    do {
        VPage* vp = Memory_getVPage(clock_hand);
        assert(vp != NULL);
        if (((struct clock_overhead*)vp->overhead)->ref) {
            ((struct clock_overhead*)vp->overhead)->ref = 0;
        } else {
            return clock_hand;
        }
        clock_hand = (clock_hand + 1) % numPages;
    } while (clock_hand != start_pos);
    // todo: what to do when the clock can't find any page to evict? 
    perror("Clock algorithm could not find a page to evict.");
    return start_pos;
}