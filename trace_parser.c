/**
 * CS 537 Programming Assignment 4 (Fall 2020)
 * @author Julien de Castelnau
 * @date 11/22/2020
 * @brief Handle parsing and initial pass over trace files, to determine start and end indices.
 * @file trace_parser.c
 */

#define _GNU_SOURCE

#include "trace_parser.h"
#include "pqueue.h"
#include "process.h"
#include <search.h>

// credit: https://www.prevanders.net/tsearch.c
// TODO: Email bart for attribution?

struct PageTableEntry {
    int vpn;
    int pid;
    int ppn;
};

// Compare two PidMap structs based on their pids. Used in tsearch()
int compare(const void *l, const void *r) {
    const struct PidMap* lpid = l;
    const struct PidMap* rpid = r;
    if (lpid->pid == rpid->pid) return 0;
    if (lpid->pid < rpid->pid) {
        return -1;
    } else {
        return 1;
    }
}

// Pidmap constructor
struct PidMap* make_PidMap(int pid) {
    struct PidMap* new_PidMap = (struct PidMap*)malloc(sizeof(struct PidMap));
    new_PidMap->pid = pid;
    new_PidMap->owner = NULL;

    return new_PidMap;
}

// Free a pidmap, for in use tdestroy()
void PidMap_free(void *PidMap) {
    struct PidMap* pdh = PidMap;
    if(!pdh) {
        return;
    }
    free(pdh);
    return;
}

// Runs a first pass over the specified trace file, passed using a FILE* pointer.
PQueue* first_pass(FILE* trace_file) {
    assert(trace_file != NULL);

    void *search_tree = 0; // search tree to store already seen PIDs in
    struct PidMap *new_pdm; // temp. variable to store the search PidMap query in
    struct PidMap *search_result; // temp. variable to store search result of tsearch()
    
    //LinkedList* trace_data = ll_initialize(); // linkedlist to store process data list in
    PQueue* trace_data = pq_init();
    size_t bufsize = 0; // used for getline
    char *line = NULL; // current line


    long pid = 0; // The current pid
    unsigned long start_line_number = 1; // The starting line number of the current block of PIDs
    unsigned long curr_line_number = 1; // The current line number in the file.

    int getline_result; // to store getline result, checked when on the last line

    // read line into temp heap-alloc'd buffer
    // "If *lineptr is set to NULL and *n is set 0 before the call, 
    // then getline() will allocate a buffer for storing the line.  
    // This buffer should be freed by the user program even if getline() failed." See getline(3)
    do {
        getline_result = getline(&line, &bufsize, trace_file);

        long curr_pid = strtol(line, NULL, 10); // Parse current line
        if (curr_pid == 0 && getline_result >= 0) {
            fprintf(stderr, "ERROR: Invalid trace file format at line %ld", curr_line_number);
            exit(EXIT_FAILURE);
        }
        // Was the PID found on current line different than the last? enter condition for
        // creating a new Process* struct and adding to list/queue.
        if (pid != 0 && (curr_pid != pid || getline_result < 0)) {
           // re_enter: // label for re-entry on last PID
            new_pdm = make_PidMap(pid); // create the search query based on PID
            search_result = tsearch(new_pdm, &search_tree, compare);
            if (search_result == 0) {
                perror("Error searching for pid holder node.");
                exit(EXIT_FAILURE);
            } else {
                // The tsearch query did not fail. Was it inserted or was an existing entry found?
                struct PidMap* re = 0;
                re = *(struct PidMap **)search_result;
                if (re != new_pdm) {
                    // An existing result was found. 
                    // Merge intervals in the current occurence and the existing occcurence,
                    // using it_insert().
                    PidMap_free(new_pdm);

                    assert(re->owner != NULL);
                    re->owner->lastline = (re->owner->lastline < curr_line_number - 1) 
                                                    ? curr_line_number - 1 : re->owner->lastline; 
                    it_insert(re->owner->lineIntervals, start_line_number, curr_line_number - 1);
                   
                } else {
                    // No existing entry existed, so it was simply added.
                    // We create the Process struct holding the line number data and its tree.
                    Process* curr_proc = (Process*) malloc(sizeof(Process));
                    curr_proc->pid = pid;
                    curr_proc->firstline = start_line_number;
                    curr_proc->lastline = curr_line_number - 1;
                    curr_proc->lineIntervals = it_initnode(start_line_number, curr_line_number - 1);
                    
                    pq_push(trace_data, curr_proc, curr_proc->firstline);
                    new_pdm->owner=curr_proc; // update the owner of the entry in the search tree
                    
                    start_line_number = curr_line_number; // reset start_line_number
                }
            }
        }
        
        // reset vars for getline()
        line = NULL;
        bufsize = 0;
        pid = curr_pid;
        curr_line_number++;
    } while (getline_result >= 0);
    
    tdestroy(search_tree, PidMap_free); // destroy search tree

    return trace_data;
}