# You may modify this file any way you like (except for the handin
# rule). You instructor will type "make" on your specific Makefile to
# build your proxy from sources.

CC = gcc
CFLAGS = -g -Wall
LDFLAGS = -lpthread

csapp.o: csapp.c csapp.h
	$(CC) $(CFLAGS) -c csapp.c

hostinfo.o: hostinfo.c csapp.h
	$(CC) $(CFLAGS) -c hostinfo.c

hostinfo: hostinfo.o csapp.o
	$(CC) $(CFLAGS) hostinfo.o csapp.o -o hostinfo $(LDFLAGS)

clean:
	rm -f *.o hostinfo

