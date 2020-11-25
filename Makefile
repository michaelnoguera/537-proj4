# See README for notes about project organization

CFLAGS=-Wall -Wextra -pedantic
SCAN_BUILD_DIR=scan-build-out

.PHONY:clean test all scan-build scan-view

all: pfsim-placeholder

# build executable
# TODO: Make separate binary targets for each scheduling algorithm. Those haven't been created yet though, so this is a placeholder
main.o: main.c trace_parser.h linkedlist.h process.h
pfsim-placeholder: main.o trace_parser.o linkedlist.o intervaltree.o pqueue.o process.o
	gcc -o pfsim-placeholder main.o trace_parser.o linkedlist.o intervaltree.o pqueue.o process.o

main.o: main.c trace_parser.h linkedlist.h pqueue.h intervaltree.h
ifeq ($(DEBUG),true)
	gcc -g -c -o $@ $< $(CFLAGS)
else
	gcc -c -o $@ $< $(CFLAGS)
endif

trace_parser.o: trace_parser.c trace_parser.h linkedlist.h pqueue.h intervaltree.h process.h
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

pqueue.o: pqueue.c pqueue.h
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
	rm -f pfsim-placeholder

# Run the Clang Static Analyzer
scan-build: clean
	scan-build -o $(SCAN_BUILD_DIR) make

# View the one scan avaialble
scan-view: scan-build
	xdg-open $(SCAN_BUILD_DIR)/*/index.html 