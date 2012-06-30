CC = gcc
CFLAGS = -std=gnu99

configtest:
	$(CC) $(CFLAGS) -lconfig -o configtest configtest.c

clean:
	rm -f configtest
