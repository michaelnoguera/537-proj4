#include "replace.h"
#include <stdlib.h>

static int numPages;

void Replace_initReplacementModule(int numberOfPhysicalPages) {
    numPages = numberOfPhysicalPages;
}

void* Replace_initOverhead(__attribute__((unused))VPage* vpage) { return NULL; }
void Replace_freeOverhead(__attribute__((unused)) void* o_ptr) { return; }
void Replace_notifyPageAccess(__attribute__((unused)) void* o_ptr) { return; }
void Replace_notifyPageLoad(__attribute__((unused)) void* o_ptr) { return; }
/**
 * Randomly chooses a page to evict.
 * @details Yes, this violates "MSC30-C Do not use the rand() function for
 * generating pseudorandom numbers". However, true randomness is undesirable
 * here because a) predicability of these numbers has no impact on security, and
 * b) determinism is useful for testing. This does mean running the program
 * multiple times may produce the same result. See
 * https://wiki.sei.cmu.edu/confluence/display/c/ for details.
 * @return index of page, within the interval [0, numberOfPages)
 */
unsigned long Replace_getPageToEvict() { return rand() % numPages; }

void Replace_freeReplacementModule() { return; }