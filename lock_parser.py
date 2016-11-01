#!/usr/bin/python3

import os
import sys

is_threads = lambda value: len(value) == 2 \
	and value[0] == '#threads_num'

is_mutex = lambda value: len(value) == 9 \
	and value[0] == 'Mutex'				\
	and value[1] == '#'					\
	and value[2] == 'Locked'			\
	and value[3] == 'Changed'			\
	and value[4] == 'Cont.'				\
	and value[5] == 'tot.Time[ms]'

is_time = lambda value: len(value) > 0 \
	and value[0] == '#mergesort-concurrency-time'

is_divider = lambda value: len(value) > 0 \
	and value[0] == "==divider=="

if __name__ == '__main__':

	if len(sys.argv) < 2:
		os.exit("./lock_parser [output.txt]")

	INPUT_FILE = 'tmpt_lock.txt'
	OUTPUT_FILE = sys.argv[1]

	fin = open(INPUT_FILE, 'r'); #using default encoding
	fout = open(OUTPUT_FILE, 'w'); #using default encoding
	tokens = list()
	thread_line_num = int(-1)
	mutex_line_num = int(-1)
	time_line_num = int(-1)
	line_num = list()

	for line in fin:
		tokens.append(line.split())

	#print(tokens)

	for key, value in enumerate(tokens):
		if is_threads(value):
			thread_line_num = key
		if is_mutex(value):
			mutex_line_num = key
		if is_time(value):
			time_line_num = key
		if is_divider(value):
			line_num.append([thread_line_num, mutex_line_num, time_line_num])
			thread_line_num = -1
			mutex_line_num = -1
			time_line_num = -1

	#for value in line_num:
	#	print(value)

	fout.write("#Id, Threads, Locked, Changed, Cont., Exec_time(ms)\n")
	for index, value in enumerate(line_num):
		thread = tokens[value[0]][1]
		locked = 0
		changed = 0
		contention = 0
		times = tokens[value[2]][1]
		if value[1] != -1:
			locked = tokens[value[1]+1][1]
			changed = tokens[value[1]+1][2]
			contention = tokens[value[1]+1][3]
		fout.write("{}, {}, {}, {}, {}, {}\n".format(index+1,thread,locked,changed,contention,times))
	
	fin.close()
	fout.close()
