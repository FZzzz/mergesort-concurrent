CC = gcc
CFLAGS = -std=gnu99 -Wall -g -pthread
OBJS = list.o threadpool.o

.PHONY: all clean test

GIT_HOOKS := .git/hooks/pre-commit

all: $(GIT_HOOKS) sort sort_uniform

$(GIT_HOOKS):
	@scripts/install-git-hooks
	@echo

deps := $(OBJS:%.o=.%.o.d)

csvs = sort.csv sort_uniform.csv

%.o: %.c
	$(CC) $(CFLAGS) -o $@ -MMD -MF .$@.d -c $<

sort.o: main.c
	$(CC) $(CFLAGS) -o $@ -MMD -MF .$@.d -c $<

sort_uniform.o: main.c
	$(CC) $(CFLAGS) -DUNIFORM -o $@ -MMD -MF .$@.d -c $<

sort: $(OBJS) sort.o
	$(CC) $(CFLAGS) -o $@ $^ -rdynamic

sort_uniform: $(OBJS) sort_uniform.o
	$(CC) $(CFLAGS) -o $@ $^ -rdynamic

gencsv: clean all
	for i in `seq 4 4 128`; do ./sort $$i input.txt; echo $$i; diff result.txt ./dictionary/words.txt; done

check:
	diff result.txt ./dictionary/words.txt

plot: gencsv
	gnuplot myplot.gp

cache:
	for i in `seq 4 4 128`; do \
	perf stat --repeat 5 -e L1-dcache-load-misses,cache-references ./sort $$i input.txt; echo $$i;\
	 done

variance:
	python stat.py
	gnuplot scala_plot.gp

thread = 128

runtime_cmp: sort sort_uniform
	for j in sort sort_uniform; do \
		echo > tmpt_lock.txt; \
		for i in `seq 1 1 100`; do \
			echo $$j $$i ${thread}; \
			mutrace ./$$j $(thread) input.txt 2>> tmpt_lock.txt 1>> tmpt_lock.txt; \
			echo "#threads_num $(thread)" 1>> tmpt_lock.txt; \
			echo "==divider==" 1>> tmpt_lock.txt; \
		done; \
		./lock_parser.py $$j.csv; \
		rm tmpt_lock.txt; \
	done;
	gnuplot runtime.gp

clean:
	rm -f $(OBJS) sort*.o sort sort_uniform exec_time.csv $(csvs) runtime_compare.png
	@rm -rf $(deps) .sort.o.d .sort_uniform.o.d
-include $(deps)
