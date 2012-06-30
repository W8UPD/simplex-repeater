CC = gcc
CFLAGS = -std=gnu99

configtest:
	$(CC) $(CFLAGS) -lconfig -lespeak -o configtest configtest.c

clean:
	rm -f configtest
