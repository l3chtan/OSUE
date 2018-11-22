# @file Makefile 
# @date 22.11.2015

CC=gcc
DEFS=-D_XOPEN_SOURCE=500 -D_BSD_SOURCE
CFLAGS=-Wall -g -std=c99 -pedantic $(DEFS)

PRGR=rventgas
EMR=rshutdown
SCHED=schedule

all: $(SCHED) $(PRGR) $(EMR)

$(SCHED): $(SCHED).c
	$(CC) $(CFLAGS) -o $@ $^

$(PRGR): $(PRGR).c
	$(CC) $(CFLAGS) -o $@ $^

$(EMR): $(EMR).c
	$(CC) $(CFLAGS) -o $@ $^

clean: 
	rm -f $(SCHED) $(PRGR) $(EMR)
.PHONY: clean

