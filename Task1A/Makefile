CC=gcc
CFLAGS=-std=c99 -pedantic -Wall -D_XOPEN_SOURCE=500 -D_DEFAULT_SOURCE -g 

all: mycompress

mycompress: mycompress.c
	$(CC) $(CFLAGS) -o mycompress mycompress.c

clean:
	rm -rf mycompress
