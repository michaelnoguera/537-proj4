#include "memory.h"

/**
 * A generic constructor method for overhead struct
 * @param o_ptr A void* holding the overhead struct
 */
void* Replace_initOverhead();

/**
 * A generic destructor method for overhead struct
 * @param o_ptr A void* holding the overhead struct
 */
void Replace_freeOverhead(void* o_ptr);

/**
 * A generic method to update any overhead/tracking information
 * the implementation algorithm might need, such as the last 
 * access time, or the clock reference bit.
 * Called once after every reference in simulator.c
 * @param o_ptr A void* holding the overhead struct
 */
void Replace_updateOverhead(void* o_ptr);

/**
 * The core "replacement algorithm" method. Uses whatever algorithm is 
   specified by the implementation of this header file to give back a
   PID representing a new spot in memory.
 * @param none
 */
int Replace_getNewPage();

