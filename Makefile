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
	for i in `seq 1 1 256`; do ./sort $$i input.txt; done

check:
	diff result.txt ./dictionary/words.txt

plot: gencsv
	gnuplot myplot.gp

clean:
	rm -f $(OBJS) sort exec_time.csv
	@rm -rf $(deps)
-include $(deps)
