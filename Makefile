CC = gcc
CFLAGS = -std=gnu99 -Wall -g -pthread
OBJS = list.o threadpool.o main.o

.PHONY: all clean test

GIT_HOOKS := .git/hooks/pre-commit

all: $(GIT_HOOKS) sort

$(GIT_HOOKS):
	@scripts/install-git-hooks
	@echo

deps := $(OBJS:%.o=.%.o.d)
%.o: %.c
	$(CC) $(CFLAGS) -o $@ -MMD -MF .$@.d -c $<

sort: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) -rdynamic

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

clean:
	rm -f $(OBJS) sort exec_time.csv
	@rm -rf $(deps)
-include $(deps)
