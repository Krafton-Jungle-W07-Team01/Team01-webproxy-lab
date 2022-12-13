#You may modify this file any way you like (except for the handin
# rule). You instructor will type "make" on your specific Makefile to
# build your proxy from sources.

CC = gcc
CFLAGS = -g -Wall
LDFLAGS = -lpthread
OBJS = csapp.o hostinfo.o echoclient.o echoserveri.o
TARGET = hostinfo client server

csapp.o: csapp.c csapp.h
hostinfo.o: hostinfo.c csapp.h
echoclient.o: echoclient.c csapp.h
echo.o: echo.c csapp.h
echoserveri.o: echoserveri.c echo.c csapp.h

all: $(TARGET)

hostinfo: hostinfo.o csapp.o
	$(CC) $(CFLAGS) hostinfo.o csapp.o -o hostinfo $(LDFLAGS)

client: csapp.o echoclient.o
	$(CC) $(CFLAGS) csapp.o echoclient.o -o client $(LDFLAGS)

server: csapp.o echo.o echoserveri.o
	$(CC) $(CFLAGS) csapp.o echo.o echoserveri.o -o server $(LDFLAGS)

clean:
	rm -f *.o $(TARGET)

