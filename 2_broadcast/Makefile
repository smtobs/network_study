CC = gcc
CFLAGS = -Wall

all: broadcast_send broadcast_recv

broadcast_send: broadcast_send.c broadcast.h
	$(CC) $(CFLAGS) -o broadcast_send broadcast_send.c

broadcast_recv: broadcast_recv.c broadcast.h
	$(CC) $(CFLAGS) -o broadcast_recv broadcast_recv.c

clean:
	rm -f broadcast_send broadcast_recv
