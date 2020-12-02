# See README for notes about project organization

CFLAGS=-Wall -Wextra -pedantic -std=gnu11
SCAN_BUILD_DIR=scan-build-out

.PHONY:clean test all scan-build scan-view

all: pfsim-random

# build executable
# TODO: Make separate binary targets for each scheduling algorithm. Those haven't been created yet though, so this is a placeholder
pfsim-random: main.o simulator.o trace_parser.o linkedlist.o intervaltree.o process.o memory.o replace_random.o stat.o
	gcc -o pfsim-random main.o simulator.o trace_parser.o linkedlist.o intervaltree.o process.o memory.o stat.o

replace_random.o: replace.h memory.h process.h
ifeq ($(DEBUG),true)
	gcc -g -c -o $@ $< $(CFLAGS)
else
	gcc -c -o $@ $< $(CFLAGS)
endif

main.o: main.c simulator.h trace_parser.h intervaltree.h process.h memory.h
ifeq ($(DEBUG),true)
	gcc -g -c -o $@ $< $(CFLAGS)
else
	gcc -c -o $@ $< $(CFLAGS)
endif

simulator.o: simulator.c simulator.h memory.h process.h
ifeq ($(DEBUG),true)
	gcc -g -c -o $@ $< $(CFLAGS)
else
	gcc -c -o $@ $< $(CFLAGS)
endif

trace_parser.o: trace_parser.c trace_parser.h linkedlist.h intervaltree.h process.h
ifeq ($(DEBUG),true)
	gcc -g -c -o $@ $< $(CFLAGS)
else
	gcc -c -o $@ $< $(CFLAGS)
endif

linkedlist.o: linkedlist.c linkedlist.h
ifeq ($(DEBUG),true)
	gcc -g -c -o $@ $< $(CFLAGS)
else
	gcc -c -o $@ $< $(CFLAGS)
endif

intervaltree.o: intervaltree.c intervaltree.h
ifeq ($(DEBUG),true)
	gcc -g -c -o $@ $< $(CFLAGS)
else
	gcc -c -o $@ $< $(CFLAGS)
endif

process.o: process.c process.h memory.h
ifeq ($(DEBUG),true)
	gcc -g -c -o $@ $< $(CFLAGS)
else
	gcc -c -o $@ $< $(CFLAGS)
endif

memory.o: memory.c memory.h process.h
ifeq ($(DEBUG),true)
	gcc -g -c -o $@ $< $(CFLAGS)
else
	gcc -c -o $@ $< $(CFLAGS)
endif

stat.o: stat.c memory.h process.h
ifeq ($(DEBUG),true)
	gcc -g -c -o $@ $< $(CFLAGS)
else
	gcc -c -o $@ $< $(CFLAGS)
endif


# Run test framework
test: all
	@bash test.sh

# Clean files
clean:
	rm -f *.o
	rm -f pfsim-random

# Run the Clang Static Analyzer
scan-build: clean
	scan-build -o $(SCAN_BUILD_DIR) make

# View the one scan avaialble
scan-view: scan-build
	xdg-open $(SCAN_BUILD_DIR)/*/index.html 