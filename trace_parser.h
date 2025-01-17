/**
 * CS 537 Programming Assignment 4 (Fall 2020)
 * @author Julien de Castelnau
 * @date 11/22/2020
 * @brief Handle parsing and initial pass over trace files, to determine start
 * and end indices.
 * @file trace_parser.h
 */

#ifndef _TRACE_PARSER_
#define _TRACE_PARSER_

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "memory.h"
#include "process.h"

/**
 * Runs a first pass over the specified trace file, passed using a FILE*
 * pointer. Organizes a priority queue for each chunk of PID reference lines
 * found, and merges their intervals into an interval tree
 *
 * @param trace_file FILE* pointing to the trace file
 * @param The process queue to contain the synthesized process data
 */
void first_pass(FILE* trace_file);

#endif
