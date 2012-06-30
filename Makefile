CC = gcc
CFLAGS = -std=gnu99 -lconfig -lespeak -lcurl -I/usr/include/libxml2 -lxml2

all : weather configtest
.PHONY : all

clean:
	rm -f weather configtest
