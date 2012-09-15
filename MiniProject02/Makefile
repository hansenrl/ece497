

CC:=/usr/bin/gcc
CFLAGS := -g -std=c99

.PHONY: all
all: accel

accel.o: accel.c
	gcc $(CFLAGS) -c $^ -o $@

accel: accel.o
	gcc $(CFLAGS) $^ -o $@
