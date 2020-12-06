/**
 * CS 537 Programming Assignment 4 (Fall 2020)
 * @file replace.h
 * @brief Replacement module interface, defines a common set of interface functions
 *  which all implementations can make use of.
 * @author Julien de Castelnau and Michael Noguera
 */

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
void* Replace_initOverhead(VPage* vpage);

/**
 * A generic destructor method for overhead struct
 * @param o_ptr A void* holding the overhead struct
 */
void Replace_freeOverhead(void* o_ptr);

/**
 * Cleans up module overhead
 * @details run after simulation complete
 */
void Replace_freeReplacementModule();

/**
 * A generic method to update any overhead/tracking information the
 * implementation algorithm might need, such as the last access time, or the
 * clock reference bit, when the page is accessed.
 * @details Called once after every page hit in simulator.c
 * @param o_ptr A void* holding the overhead struct
 */
void Replace_notifyPageAccess(void* o_ptr);

/**
 * Another generic method to update any overhead/tracking information the
 * implementation algorithm might need, such as the last access time, or the
 * clock reference bit, when the page is not in memory.
 * @details Called once after every page load (after disk I/O completes) in simulator.c
 * @param o_ptr A void* holding the overhead struct
 */
void Replace_notifyPageLoad(void* o_ptr);

/**
 * Core "replacement algorithm" method. Uses the corresponding module's
 * algorithm to choose a victim.
 * @return ppn of page to evict
 */
unsigned long Replace_getPageToEvict();
