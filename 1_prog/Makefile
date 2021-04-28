CC = gcc
CFLAGS = -g -O0 -MD
DEBUG_FLAGS = -D TEST
GCOV_FLAGS = -fprofile-arcs -ftest-coverage
DOT_FLAGS = -Tpng

test: unittest.out
	./unittest.out

memcheck: unittest.out
	valgrind --leak-check=full -v ./$<

unittest.out: unittests.o rb-tree.o
	$(CC) $(CFLAGS) $(DEBUG_FLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) $(DEBUG_FLAGS) -c -o $@ $<

cov_tree: rb-tree.gcno rb-tree.gcda
	gcov rb-tree.c

rb-tree.gcno: unittests.c rb-tree.c
	$(CC) $(GCOV_FLAGS) $(DEBUG_FLAGS) -o cov.out $^

rb-tree.gcda: cov.out
	./cov.out

cov.out: unittests.c rb-tree.c
	$(CC) $(GCOV_FLAGS) $(DEBUG_FLAGS) -o cov.out $^

%.png: %.dot
	dot $(DOT_FLAGS) -o $@ $<

%.dot: unittest.out
	./unittest.out

-include *.d

.PHONY: clean

clean:
	rm *.o *.d *.out *.gcov *.gcda *.gcno *.dot *.png
