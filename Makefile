CC = g++
CFLAGS = -std=c++11 -Wall -pedantic -g -Isrc -Itests
LIBS = -pthread

SRCDIR    := src src/linked_list src/binary_tree src/concurrent_map src/server src/client
TESTDIR   := tests tests/linked_list tests/binary_tree tests/concurrent_map
SRC       := $(foreach dr, $(SRCDIR), $(wildcard $(dr)/*.[ch]pp))
TEST      := $(foreach dr, $(TESTDIR), $(wildcard $(dr)/*.[ch]pp))
OBJS      := $(addsuffix .o,$(basename $(SRC)))
TESTOBJS  := $(addsuffix .o,$(basename $(TEST)))
TESTBASE  := tests
TESTBINS  := $(TESTBASE)/linked_list/ll_test $(TESTBASE)/binary_tree/binary_tree_test $(TESTBASE)/concurrent_map/cmap_tests

all: $(OBJS) $(TESTBINS)

%.o: %.cpp 
	@ echo "Compiling object file: $@"
	@ $(CC) -c $(CFLAGS) $< -o $@ $(LIBS)

$(TESTBINS): % : %.o $(OBJS) $(TESTOBJS)
	@ echo "Compiling test binary: $@"
	$(CC) $(CFLAGS) $< -o $@ $(LIBS)

run_tests: $(TESTBINS)
	@ $(foreach bin, $(TESTBINS), echo "Running binary: $(bin)"; ./$(bin);)

clean:
	rm -f $(OBJS) $(TESTOBJS) $(TESTBINS)