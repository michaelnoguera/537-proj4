/**
 * CS 537 Programming Assignment 4 (Fall 2020)
 * @authors Michael Noguera and Julien de Castelnau
 * @date 11/22/2020
 * @file main.c
 * @brief parses command line arguments & coordinates main simulation loop
 * @version FINAL
 */

#define _GNU_SOURCE

#include "intervaltree.h"
#include "memory.h"
#include "process.h"
#include "replace.h"
#include "simulator.h"
#include "stat.h"
#include "trace_parser.h"

#include <errno.h>
#include <getopt.h>
#include <string.h>


/**
 * Parses args, validates memory size and page size
 * @param[in] argc from main
 * @param[in] argv from main
 * @param[out] memsize total size of memory in bytes
 * @param[out] pagesize size of one page in bytes
 * @param[out] filename provided trace file name
 * @returns values via the parameter fields labeled "out", or exits with an error if invalid input provided.
 * */
inline static void parseArgs(int argc, char** argv, int* memsize, int* pagesize,
                             char** filename) {
    // use getopt to handle input
    int opt = 0;
    while ((opt = getopt(argc, argv, "-p:m:h")) != -1) {
        switch ((char)opt) {
            case 'm':
                assert(optarg != NULL);
                errno = 0;
                *memsize = (int)strtol(optarg, NULL, 10);
                if (errno != 0) {
                    fprintf(stderr,
                            "Error parsing -m, must be a valid integer.\n");
                    exit(EXIT_FAILURE);
                }
                break;
            case 'p':
                assert(optarg != NULL);
                errno = 0;
                *pagesize = (int)strtol(optarg, NULL, 10);
                if (errno != 0) {
                    fprintf(stderr,
                            "Error parsing -p, must be a valid integer.\n");
                    exit(EXIT_FAILURE);
                }
                break;
            case 'h':
                // help message printed by '-h'
                printf("Usage:\n");
                printf(
                  "  ./pfsim-lru [-m real memory size] [-p page size] "
                  "<tracefile>\n");
                printf(
                  "  ./pfsim-fifo [-m real memory size] [-p page size] "
                  "<tracefile>\n");
                printf(
                  "  ./pfsim-clock [-m real memory size] [-p page size] "
                  "<tracefile>\n");
                printf(
                  "  ./pfsim-random [-m real memory size] [-p page size] "
                  "<tracefile>\n");
                printf("\nOptions:\n");
                printf("  -h\t");
                printf("Prints this message.\n");
                printf("  -m\t");
                printf(
                  "Amount of physical memory avaliable, in megabytes. "
                  "Defaults to 1 MB.\n");
                printf("  -p\t");
                printf(
                  "Page size as a number of bytes, must be a power of two. "
                  "Defaults to 4096 bytes.\n");

                exit(EXIT_FAILURE);
                break;
            case '?':
                // error message directs user to call '-h'
                if (argv != NULL && argv[0] != NULL) {
                    printf("Try '%s -h' for more information.\n",
                           basename(argv[0]));
                } else {
                    fprintf(stderr, "Could not recognize executable name.");
                    printf("Specify '-h' for more information.\n");
                }

                exit(EXIT_FAILURE);
                break;
            default:
                *filename = optarg;
                break;
        }
    }

    // validate page size
    if (*pagesize && *pagesize % 2 != 0) {
        fprintf(stderr, "ERROR: page size must be a power of two\n");
        exit(EXIT_FAILURE);
    } else if (*pagesize < 0) {
        fprintf(stderr, "ERROR: page size must be positive\n");
        exit(EXIT_FAILURE);
    } else if (*pagesize == 0) {
        fprintf(
          stderr,
          "\x1B[2mWARN: page size (-p) not specified, defaulting to 4096 bytes\x1B[0m\n");
        *pagesize = 4096;
    }

    // validate and calculate memory size
    if (*memsize < 0) {
        fprintf(stderr, "ERROR: memory size must be positive\n");
        exit(EXIT_FAILURE);
    } else if (*memsize == 0) {
        fprintf(stderr,
                "\x1B[2mWARN: memory size (-m) not specified, defaulting to 1 MB\x1B[0m\n");
        *memsize = 1;
    }
    *memsize *= 0x100000; // MB -> bytes

    // make sure that pages fit in memory
    if (*pagesize > *memsize) {
        fprintf(stderr,
                "ERROR: specified page size is larger than memory size\n");
        exit(EXIT_FAILURE);
    }

    // filename is required
    if (*filename == NULL) {
        fprintf(stderr,
                "ERROR: must specify valid file name on command line\n");
        exit(EXIT_FAILURE);
    }
}

/**
 * Main method, coordinates simulation
 * @param argc # of cmdline args
 * @param argv cmdline args
 * @see simulator.c for main loop of simulation
 * @return EXIT_SUCCESS on success and EXIT_FAILURE on failure
 */
int main(int argc, char** argv) {
    // 1. Parse command line arguments
    int memsize = 0;
    int pagesize = 0;
    char* filename = NULL;
    parseArgs(argc, argv, &memsize, &pagesize, &filename);
    assert(memsize > 0);
    assert(pagesize > 0);
    assert(filename != NULL);
    
    int numberOfPhysicalPages = memsize / pagesize;
    assert(numberOfPhysicalPages > 0);

    // 2. Open tracefile
    FILE* tracefile = NULL;
    tracefile = fopen(filename, "r");
    if (tracefile == NULL) {
        fprintf(stderr, "ERROR: error opening specified trace file\n");
        exit(EXIT_FAILURE);
    }
    
    printf("\x1B[1m\x1B[7m%s\x1B[0m\n"," PARAMETERS ");
    printf("  \x1B[1m%s\x1B[0m\n", filename);
    printf("  page size: %i B\n", pagesize);
    printf("  memory size: %i MB\n", memsize/0x100000);
    printf("  = %i pages\n", numberOfPhysicalPages);

    // 3. Initialize helper modules
    Memory_init(numberOfPhysicalPages);
    Replace_initReplacementModule(numberOfPhysicalPages);
    Stat_init();
    ProcessQueues_init();

    // 4. Read "first pass", ennumerating pids and building interval tree
    first_pass(tracefile);

    // 5. Run the simulation
    unsigned long exit_time = Simulator_runSimulation(tracefile);

    // 6. Output results
    Stat_printStats(exit_time);
    return EXIT_SUCCESS;
}
