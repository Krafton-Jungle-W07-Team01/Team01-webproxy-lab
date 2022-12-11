CC = gcc
CFLAGS = -g -Wall
LDFLAGS = -lpthread
SRCS = csapp.c hostinfo.c echoclient.c echoserveri.c
OBJS = $(SRCS:%.c=%.o)
TARGET = hostinfo client server

%.o : %.c
	cc $(CFLAGS) -c $< -o $@ -I. 

all: $(OBJS) hostinfo client server

hostinfo: 
	$(CC) $(CFLAGS) hostinfo.o csapp.o -o hostinfo $(LDFLAGS)

client: csapp.o echoclient.o
	$(CC) $(CFLAGS) csapp.o echoclient.o -o client $(LDFLAGS)

server: csapp.o echoserveri.o
	$(CC) $(CFLAGS) csapp.o echoserveri.o -o server $(LDFLAGS)

clean:
	rm -f *.o $(TARGET)

re: clean all
