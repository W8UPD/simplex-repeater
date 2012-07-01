CC = g++
CFLAGS = -Wall -lconfig -I/usr/include/speech_tools -lFestival -leststring -lestools -lestbase -lcurl -I/usr/include/libxml2 -lxml2 -lportaudio -lhamlib -lm -lpthread
SRC = $(wildcard *.c)
.PHONY : all

all:
	$(CC) $(CFLAGS) -o master $(SRC)

clean:
	rm -f master
