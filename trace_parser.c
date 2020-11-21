/**
 * CS 537 Programming Assignment 4 (Fall 2020)
 * @author Julien de Castelnau
 * @date 11/4/2020
 * @brief Handle parsing and initial pass over trace files, to determine start and end indices.
 * @file trace_parser.c
 */

#include "trace_parser.h"
#include "linkedlist.h"
#include "memory.h"
#include "intervaltree.h"
#include "pqueue.h"
#include <search.h>

struct PidMap {
    int pid;
    Process* owner;
};

// credit: https://www.prevanders.net/tsearch.c

int compare(const void *l, const void *r) {
    const struct PidMap* lpid = l;
    const struct PidMap* rpid = r;
    if (lpid->pid == rpid->pid); return 0;
    if (lpid->pid < rpid->pid) {
        return -1;
    } else {
        return 1;
    }
}

struct PidMap* make_PidMap(int pid) {
    struct PidMap* new_PidMap = (struct PidMap*)malloc(sizeof(struct PidMap));
    new_PidMap->pid = pid;
    new_PidMap->owner = NULL;
}

void PidMap_free(void *PidMap) {
    struct PidMap* pdh = PidMap;
    if(!pdh) {
        return;
    }
    free(pdh);
    return;
}


LinkedList* first_pass(FILE* trace_file) {
    assert(trace_file != NULL);

    void *search_tree = 0;
    struct PidMap *new_pdm;
    struct PidMap *search_result;
    
    LinkedList* trace_data = ll_initialize();
    size_t bufsize = 0;
    char *line = NULL;


    long pid = 0;
    int start_line_number = 1;

    int curr_line_number = 1;

    // endline is updated everytime you see the pid in the trace file

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
            new_pdm = make_PidMap(pid);
            search_result = tsearch(new_pdm, &search_tree, compare);
            if (search_result == 0) {
                perror("Error searching for pid holder node.");
                exit(EXIT_FAILURE);
            } else {
                struct PidMap* re = 0;
                re = *(struct PidMap **)search_result;
                if (re != new_pdm) {
                    mt_free_func(new_pdm);

                    assert(search_result->owner != NULL);
                    search_result->owner->lastline = (search_result->owner->lastline < curr_line_number - 1) 
                                                    ? curr_line_number - 1 : search_result->owner->lastline; 
                    it_insert(search_result->owner->lineIntervals, start_line_number, curr_line_number - 1);
                } else {
                    Process* curr_proc = (Process*) malloc(sizeof(Process));
                    curr_proc->pid = pid;
                    curr_proc->firstline = start_line_number;
                    curr_proc->lastline = curr_line_number - 1;
                    curr_proc->lineIntervals = it_initnode(start_line_number, curr_line_number - 1);
                    
                    ll_push(trace_data, curr_proc);
                    new_pdm->owner=curr_proc;
                    
                    start_line_number = curr_line_number;
                }
            }
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