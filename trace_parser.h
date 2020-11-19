/**
 * CS 537 Programming Assignment 4 (Fall 2020)
 * @author Julien de Castelnau
 * @date 11/4/2020
 * @brief Handle parsing and initial pass over trace files, to determine start and end indices.
 * @file trace_parser.h
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "linkedlist.h"

typedef struct process_data_t {
    int pid;
    int startline;
    int endline;
} PData;

LinkedList* first_pass(FILE* trace_file);