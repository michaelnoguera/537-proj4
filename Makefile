# See README for notes about project organization

CFLAGS=-Wall -Wextra -pedantic -std=gnu11
SCAN_BUILD_DIR=scan-build-out
COMMON_MODULES=main.o simulator.o trace_parser.o intervaltree.o process.o memory.o stat.o

.PHONY:clean test all scan-build scan-view

all: pfsim-random pfsim-clock

# build executable
pfsim-clock: $(COMMON_MODULES) replace-clock.o
	gcc -o pfsim-clock $(COMMON_MODULES) replace-clock.o

pfsim-random: $(COMMON_MODULES) replace-random.o
	gcc -o pfsim-random $(COMMON_MODULES) replace-random.o

replace-clock.o: replace-clock.c replace.h memory.h process.h
ifeq ($(DEBUG),true)
	gcc -g -c -o $@ $< $(CFLAGS)
else
	gcc -c -o $@ $< $(CFLAGS)
endif

replace-random.o: replace-random.c replace.h memory.h process.h
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

trace_parser.o: trace_parser.c trace_parser.h intervaltree.h process.h
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

memory.o: memory.c memory.h process.h replace.h
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
