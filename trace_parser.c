/**
 * CS 537 Programming Assignment 4 (Fall 2020)
 * @author Julien de Castelnau
 * @date 11/22/2020
 * @brief Handle parsing and initial pass over trace files, to determine start
 * and end indices.
 * @file trace_parser.c
 * Use of tsearch in this module is based off of Robert William's example (@765)
 * https://github.com/increscent/c_examples/blob/main/tsearch/main.c
 */
#define _GNU_SOURCE


#include "trace_parser.h"
#include "process.h"

#include <assert.h>
#include <errno.h>
#include <search.h>
#include <stdio.h>

// Maps a pid to its owner process struct, for tsearch purposes
struct PidMap {
    int pid;
    Process* owner;
};

// Compare two PidMap structs based on their pids. Used in tsearch()
static int compare(const void* l, const void* r) {
    const struct PidMap* lpid = l;
    const struct PidMap* rpid = r;
    if (lpid->pid < rpid->pid) {
        return -1;
    } else if (lpid->pid > rpid->pid) {
        return 1;
    } else {
        return 0;
    }
}

// Pidmap constructor
static struct PidMap* make_PidMap(int pid) {
    struct PidMap* new_PidMap;
    if ((new_PidMap = (struct PidMap*)malloc(sizeof(struct PidMap))) == NULL) {
        perror("Failed to allocate memory for new PID->Proces* map node.");
        exit(EXIT_FAILURE);
    }

    new_PidMap->pid = pid;
    new_PidMap->owner = NULL;

    return new_PidMap;
}

// Free a pidmap, for in use tdestroy()
static void PidMap_free(void* PidMap) {
    struct PidMap* pdh = PidMap;
    if (!pdh) { return; }
    free(pdh);
    return;
}

/**
 * Runs a first pass over the specified trace file, passed using a FILE*
 * pointer. After running this function, all Process structs will be in the
 * RUNNABLE ProcessQueue.
 * @param trace_file file provided as input
 */
void first_pass(FILE* trace_file) {
    assert(trace_file != NULL);

    void* search_tree = 0; // search tree to store already seen PIDs in

    // track fields for current process
    unsigned long pid = 0;
    unsigned long start_line_number = 1;
    long start_fpos;
    long curr_fpos;
    if ((start_fpos = ftell(trace_file)) == -1) {
        perror("Error retrieving file position in tracefile.");
        exit(EXIT_FAILURE);
    }
    unsigned long curr_line_number = 1;

    int fscanf_result = 0;
    do {
        if ((curr_fpos = ftell(trace_file)) == -1) {
            perror("Error retrieving file position in tracefile.");
            exit(EXIT_FAILURE);
        }

        unsigned long curr_pid;
        unsigned long curr_vpn;
         // Parse current line using fscanf
        fscanf_result = fscanf(trace_file, "%lu %lu", &curr_pid, &curr_vpn);
        if (curr_pid == 0 && fscanf_result >= 0) {
            fprintf(stderr, "ERROR: Invalid trace file format at line %ld",
                    curr_line_number);
            exit(EXIT_FAILURE);
        }

        // Was the PID found on current line different than the last? enter
        // condition for creating a new Process* struct and adding to
        // list/queue.
        if (pid != 0 && (curr_pid != pid || fscanf_result <= 0)) {
            struct PidMap* new_pdm;       // interval tree query
            struct PidMap* search_result; // result of tsearch
            new_pdm = make_PidMap(pid); // create the search query based on PID
            search_result = tsearch(new_pdm, &search_tree, compare);
            if (search_result == 0) {
                perror("Error searching for pid holder node.");
                exit(EXIT_FAILURE);
            } else {
                // The tsearch query did not fail. Was it inserted or was an
                // existing entry found?
                struct PidMap* existing = *(struct PidMap**)search_result;
                if (existing != new_pdm) {
                    // An existing result was found.
                    // Merge intervals in the current occurrence and the
                    // existing occurrence, using it_insert().
                    PidMap_free(new_pdm);

                    assert(existing->owner != NULL);
                    existing->owner->lastline =
                      (existing->owner->lastline < curr_line_number - 1)
                        ? curr_line_number - 1
                        : existing->owner->lastline;

                    IntervalNode* new_IntervalNode =
                      it_initnode(start_line_number, curr_line_number - 1);
                    it_setFpos(new_IntervalNode, start_fpos);
                    it_insert(existing->owner->lineIntervals, new_IntervalNode);
                } else {
                    // no process existed, create one
                    IntervalNode* new_IntervalNode =
                      it_initnode(start_line_number, curr_line_number - 1);
                    it_setFpos(new_IntervalNode, start_fpos);
                    Process* curr_proc =
                      Process_init(pid, start_line_number, curr_line_number - 1,
                                   new_IntervalNode);

                    new_pdm->owner = curr_proc; // update the owner of the entry
                                                // in the search tree
                }
                start_fpos = curr_fpos;
                start_line_number = curr_line_number; // reset start_line_number
            }
        }

        pid = curr_pid;
        curr_line_number++;
    } while (fscanf_result > 0);

    tdestroy(search_tree, PidMap_free); // destroy search tree
}