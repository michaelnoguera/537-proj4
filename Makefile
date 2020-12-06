# See README for notes about project organization

DEBUG_FLAGS= -g -O0 -Wall -Wextra -pedantic -std=gnu11
PROD_FLAGS=-Wall -Wextra -pedantic -std=gnu11 -DNDEBUG

SCAN_BUILD_DIR=scan-build-out
COMMON_MODULES=main.o simulator.o trace_parser.o intervaltree.o process.o memory.o stat.o

.PHONY:clean test all scan-build scan-view

all: pfsim-random pfsim-clock pfsim-lru pfsim-fifo

# build executable
pfsim-clock: $(COMMON_MODULES) replace-clock.o
	gcc -o pfsim-clock $(COMMON_MODULES) replace-clock.o

pfsim-random: $(COMMON_MODULES) replace-random.o
	gcc -o pfsim-random $(COMMON_MODULES) replace-random.o

pfsim-lru: $(COMMON_MODULES) replace-lru.o
	gcc -o pfsim-lru $(COMMON_MODULES) replace-lru.o

pfsim-fifo: $(COMMON_MODULES) replace-fifo.o
	gcc -o pfsim-fifo $(COMMON_MODULES) replace-fifo.o

replace-fifo.o: replace-fifo.c replace.h memory.h process.h
ifeq ($(DEBUG),true)
	gcc -c -o $@ $< $(DEBUG_FLAGS)
else
	gcc -c -o $@ $< $(PROD_FLAGS)
endif

replace-clock.o: replace-clock.c replace.h memory.h process.h
ifeq ($(DEBUG),true)
	gcc -c -o $@ $< $(DEBUG_FLAGS)
else
	gcc -c -o $@ $< $(PROD_FLAGS)
endif

replace-random.o: replace-random.c replace.h memory.h process.h
ifeq ($(DEBUG),true)
	gcc -c -o $@ $< $(DEBUG_FLAGS)
else
	gcc -c -o $@ $< $(PROD_FLAGS)
endif

replace-lru.o: replace-lru.c replace.h memory.h process.h
ifeq ($(DEBUG),true)
	gcc -c -o $@ $< $(DEBUG_FLAGS)
else
	gcc -c -o $@ $< $(PROD_FLAGS)
endif

main.o: main.c simulator.h trace_parser.h intervaltree.h process.h memory.h
ifeq ($(DEBUG),true)
	gcc -c -o $@ $< $(DEBUG_FLAGS)
else
	gcc -c -o $@ $< $(PROD_FLAGS)
endif

simulator.o: simulator.c simulator.h memory.h process.h
ifeq ($(DEBUG),true)
	gcc -c -o $@ $< $(DEBUG_FLAGS)
else
	gcc -c -o $@ $< $(PROD_FLAGS)
endif

trace_parser.o: trace_parser.c trace_parser.h intervaltree.h process.h
ifeq ($(DEBUG),true)
	gcc -c -o $@ $< $(DEBUG_FLAGS)
else
	gcc -c -o $@ $< $(PROD_FLAGS)
endif

intervaltree.o: intervaltree.c intervaltree.h
ifeq ($(DEBUG),true)
	gcc -c -o $@ $< $(DEBUG_FLAGS)
else
	gcc -c -o $@ $< $(PROD_FLAGS)
endif

process.o: process.c process.h memory.h
ifeq ($(DEBUG),true)
	gcc -c -o $@ $< $(DEBUG_FLAGS)
else
	gcc -c -o $@ $< $(PROD_FLAGS)
endif

memory.o: memory.c memory.h process.h replace.h
ifeq ($(DEBUG),true)
	gcc -c -o $@ $< $(DEBUG_FLAGS)
else
	gcc -c -o $@ $< $(PROD_FLAGS)
endif

stat.o: stat.c memory.h process.h
ifeq ($(DEBUG),true)
	gcc -c -o $@ $< $(DEBUG_FLAGS)
else
	gcc -c -o $@ $< $(PROD_FLAGS)
endif


# Run test framework
test: all
	@bash test.sh

# Clean files
clean:
	rm -f *.o
	rm -f pfsim-random
	rm -f pfsim-clock
	rm -f pfsim-lru
	rm -f pfsim-fifo
	rm -rf scan-build-out

# Run the Clang Static Analyzer
scan-build: clean
	scan-build -o $(SCAN_BUILD_DIR) make

# View the one scan avaialble
scan-view: scan-build
	xdg-open $(SCAN_BUILD_DIR)/*/index.html 
