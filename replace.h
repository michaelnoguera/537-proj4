#include "memory.h"


/**
 * Initializes general replacement module overhead, if any.
 * @details run before page allocation starts
 */
void Replace_initReplacementModule(int numberOfPhysicalPages);

/**
 * A generic constructor method for overhead struct
 * @return A void pointer to a new overhead struct
 */
void* Replace_initOverhead();

/**
 * A generic destructor method for overhead struct
 * @param o_ptr A void* holding the overhead struct
 */
void Replace_freeOverhead(__attribute__((unused))void* o_ptr);

/**
 * Cleans up module overhead
 * @details run after simulation complete
 */
void Replace_freeReplacementModule();

/**
 * A generic method to update any overhead/tracking information the
 * implementation algorithm might need, such as the last access time, or the
 * clock reference bit.
 * @details Called once after every reference in simulator.c
 * @param o_ptr A void* holding the overhead struct
 */
void Replace_updateOverhead(void* o_ptr);

/**
 * Core "replacement algorithm" method. Uses the corresponding module's
 * algorithm to choose a victim.
 * @return ppn of page to evict
 */
unsigned long Replace_getPageToEvict();

void Replace_notifyPageAccess(
  int ppn); // because ppn lookup is constant and vpn is log
            // build an internal representation of page
            // Fifo: enqueue, clock: create bit, etc.