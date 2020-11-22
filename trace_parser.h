/**
 * CS 537 Programming Assignment 4 (Fall 2020)
 * @author Julien de Castelnau
 * @date 11/22/2020
 * @brief Handle parsing and initial pass over trace files, to determine start and end indices.
 * @file trace_parser.h
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "linkedlist.h"
#include "memory.h"

// Maps a pid to its owner process struct, for tsearch purposes
struct PidMap {
    int pid;
    Process* owner;
};

/**
 * Runs a first pass over the specified trace file, passed using a FILE* pointer.
 * Organizes a linked list for each chunk of PID reference lines found, and merges
 * their intervals into an interval tree
 * 
 * @param trace_file FILE* pointing to the trace file
 * @return The linkedlist containing the synthesized process data
 */
LinkedList* first_pass(FILE* trace_file);