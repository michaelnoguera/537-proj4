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
LinkedList* first_pass(FILE* trace_file) {
    assert(trace_file != NULL);

    void *search_tree = 0; // search tree to store already seen PIDs in
    struct PidMap *new_pdm; // temp. variable to store the search PidMap query in
    struct PidMap *search_result; // temp. variable to store search result of tsearch()
    
    LinkedList* trace_data = ll_initialize(); // linkedlist to store process data list in
    size_t bufsize = 0; // used for getline
    char *line = NULL; // current line


    long pid = 0; // The current pid
    unsigned long start_line_number = 1; // The starting line number of the current block of PIDs
    unsigned long curr_line_number = 1; // The current line number in the file.

    // Are we on the last line? Used to jump back to main condition for the last PID block
    // See TODO on goto statement below.
    bool on_lastline = false;

    // endline is updated everytime you see the pid in the trace file

    // read line into temp heap-alloc'd buffer
    // "If *lineptr is set to NULL and *n is set 0 before the call, 
    // then getline() will allocate a buffer for storing the line.  
    // This buffer should be freed by the user program even if getline() failed." See getline(3)
    while (getline(&line, &bufsize, trace_file) >= 0) {
        assert(line != NULL);

        long curr_pid = strtol(line, NULL, 10); // Parse current line
        if (curr_pid == 0) {
            fprintf(stderr, "ERROR: Invalid trace file format at line %ld", curr_line_number);
        }
        // Was the PID found on current line different than the last? enter condition for
        // creating a new Process* struct and adding to list/queue.
        if (curr_pid != pid && pid != 0) {
            re_enter: // label for re-entry on last PID
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
                    
                    ll_push(trace_data, curr_proc);
                    new_pdm->owner=curr_proc; // update the owner of the entry in the search tree
                    
                    start_line_number = curr_line_number; // reset start_line_number
                }
                // exit loop completely if this was entered through the last PID block condition
                if (on_lastline) goto exit_here;
            }
        }
        
        // reset vars for getline()
        line = NULL;
        bufsize = 0;
        pid = curr_pid;
        curr_line_number++;
    }

    // TODO: Fix and replace with a more canonically "C" way of doing things.
    // The reason why goto is used here is because I need some kind of way 
    // of running the large code block in the if statement above for the last line;
    // but it only triggers when the PID is changed. This will obviously never happen for 
    // the last block of PIDs, so instead we jump back to that block, with an extra boolean condition
    // that says not to jump back here unless it's the last line.

    // Functionally, this works great. Just such a bad thing to do that I think we
    // could actually get style points docked. It does make the control flow a little
    // less intuitive but this is one of the situations where goto is hugely useful.

    // Here a couple potential alternative solutions however:
    // 1) Add an "or" clause to the if statement that can some how check if the current
    //    line is equal to the last line in the file, but I'm not aware of a way to do that.
    // 2) Just copy and paste the block. This makes the code really ugly and you would have to read
    //    through the entire thing only to find that it's literally the exact same code.
    // 3) Use a different trigger for creating the process struct, rather than on a changing PID.
    // There is problably some way to do this, but again. I'm not aware of one.
    // 4) Add a dummy line to the end of the file, so that it will get discarded, but the actual
    //    last line before it is kept. I think this is the cleanest and least work intensive of all of these,
    //    but you would have to figure out how to write to the end of only the file representation, without
    //    touching the file itself. No idea how to do that.
    
    // pid is 0 iff the file is empty
    if (pid != 0) {
        on_lastline = true;
        goto re_enter; // re-enter loop
    }

    exit_here: 
    
    tdestroy(search_tree, PidMap_free); // destroy search tree

    return trace_data;
}