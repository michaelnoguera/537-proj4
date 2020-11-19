/**
 * CS 537 Programming Assignment 4 (Fall 2020)
 * @author Julien de Castelnau
 * @date 11/4/2020
 * @brief Handle parsing and initial pass over trace files, to determine start and end indices.
 * @file trace_parser.c
 */

#include "trace_parser.h"
#include "linkedlist.h"

LinkedList* first_pass(FILE* trace_file) {
    assert(trace_file != NULL);
    
    LinkedList* trace_data = ll_initialize();
    size_t bufsize = 0;
    char *line = NULL;


    long pid = 0;
    int start_line_number = 1;

    int curr_line_number = 1;

    // read line into temp heap-alloc'd buffer
    // "If *lineptr is set to NULL and *n is set 0 before the call, 
    // then getline() will allocate a buffer for storing the line.  
    // This buffer should be freed by the user program even if getline() failed." See getline(3)
    while (getline(&line, &bufsize, trace_file) >= 0) {
        assert(line != NULL);

        long curr_pid = strtol(line, NULL, 10);
        if (curr_pid == 0) {
            fprintf(stderr, "ERROR: Invalid trace file format at line %d", curr_line_number);
        }
        if (curr_pid != pid && pid != 0) {
            PData* curr_pdata = (PData*) malloc(sizeof(PData));
            curr_pdata->pid = pid;
            curr_pdata->startline = start_line_number;
            curr_pdata->endline = curr_line_number - 1;
            ll_push(trace_data, curr_pdata);

            start_line_number = curr_line_number;
        }
        
        line = NULL;
        bufsize = 0;
        pid = curr_pid;
        curr_line_number++;
    }
    if (pid != 0) {
        PData* curr_pdata = (PData*) malloc(sizeof(PData));
        curr_pdata->pid = pid;
        curr_pdata->startline = start_line_number;
        curr_pdata->endline = curr_line_number - 1;
        ll_push(trace_data, curr_pdata); 
    }
    return trace_data;
}