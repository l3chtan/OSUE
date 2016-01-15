# @file Makefile 
# @author Edward Toth 0725631
# @date 03.01.2016 

CC=gcc
SHM=-lrt
SEM=-pthread
DEFS_Abgabe=-D_XOPEN_SOURCE=500 -D_BSD_SOURCE
DEFS=-D_XOPEN_SOURCE=500 -D_DEFAULT_SOURCE
CFLAGS=-Wall -g -std=c99 -pedantic $(DEFS) $(SEM) $(SHM)

SERVER=hangman-server
CLIENT=hangman-client
COMMON=hangman-common

all: $(SERVER) $(CLIENT) 

$(SERVER): $(SERVER).c $(COMMON).h
	$(CC) $(CFLAGS) -o $@ $<

$(CLIENT): $(CLIENT).c $(COMMON).h
	$(CC) $(CFLAGS) -o $@ $<

clean: 
	rm -f $(SERVER) $(CLIENT) 
.PHONY: clean


