/**
 * CS 537 Programming Assignment 4 (Fall 2020)
 * @file replace-clock.c
 * @brief Replacement module implementing the Clock algorithm.
 * Overhead is tied to physical pages in the form of a shadow array on memory, for
 * reference bits.
 * @author Julien de Castelnau
 * @details Note that overhead is still needed to update said reference bits 
 * upon successful hits, for which the only information we have is the VPN which 
 * just got a hit.
 */

#include "replace.h"
#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>

// Clock doesn't need any overhead information, other than a reference
// back to its parent VPN so that PPNs can be resolved and used to 
// make changes to the freelist shadow array.
struct clock_overhead {
    VPage* parent;
};

// index into memory (freelist), a PPN which indicates where the clock hand currently is
static unsigned long clock_hand; 
// size of memory in pages
static int numPages;
// a shadow array of memory, which stores reference bits for each page frame.
static bool* shadowMem_Reflist;

// Creates shadow array, initializes clock_hand to 0
void Replace_initReplacementModule(int numberOfPhysicalPages) {
    clock_hand = 0;
    numPages = numberOfPhysicalPages;
    if ((shadowMem_Reflist = (bool*)malloc(sizeof(bool) * numPages)) == NULL) {
        perror("Cannot allocate memory for clock algorithm shadow rarray.");
    }
}

void Replace_freeReplacementModule() { 
    free(shadowMem_Reflist);    
}

/**
 * Need to set the parent pointer in the overhead.
 * @param VPage* caller/parent VPage
 * @return a pointer to the newly allocated overhead struct
 */
void* Replace_initOverhead(VPage* vpage) {
    struct clock_overhead* co =
      (struct clock_overhead*)malloc(sizeof(struct clock_overhead));
    co->parent = vpage;
    return co;
}

void Replace_freeOverhead(void* o_ptr) {
    free((struct clock_overhead*)o_ptr);
}

/**
 * A page was referenced, so we turn the reference bit on.
 * @param o_ptr void* to overhead struct
 */
void Replace_notifyPageAccess(void* o_ptr) {
    // We assume the Vpage is in memory because this gets called
    // after it was just referenced.
    assert(((struct clock_overhead*)o_ptr)->parent->inMemory);
    shadowMem_Reflist[((struct clock_overhead*)o_ptr)->parent->currentPPN] =
      true;
}

// unimplemented
void Replace_notifyPageLoad(void* o_ptr) { Replace_notifyPageAccess(o_ptr); }

/**
 * Core clock algorithm. Sweeps through the reference bits till it finds a 0,
 * setting the 1s to 0s on its way.
 * @return PPN of page frame to evict
 */
unsigned long Replace_getPageToEvict() {
    assert(!Memory_hasFreePage());

    // loop through until a 0 is found, setting every 1 to 0
    while (shadowMem_Reflist[clock_hand] == true) {
        shadowMem_Reflist[clock_hand] = false;
        clock_hand = (clock_hand + 1) % numPages;
    }
    return (clock_hand);
}