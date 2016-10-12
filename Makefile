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

gencsv: all
	for i in 1 2 4 8 16 32 64 128 256; do \
		for j in {1..20}; do \ 
			echo $RANDOM | ./sort $i 20; \
		done  \
	done

check:
	diff result.txt ./dictionary/words.txt

clean:
	rm -f $(OBJS) sort
	@rm -rf $(deps)

-include $(deps)
