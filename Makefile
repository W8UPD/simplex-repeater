CC = gcc
CFLAGS = -std=gnu99 -Wall -lconfig -lespeak -lcurl -I/usr/include/libxml2 -lxml2 -lportaudio
SRC = $(wildcard *.c)
.PHONY : all

all:
	$(CC) $(CFLAGS) -o master $(SRC)

clean:
	rm -f master
