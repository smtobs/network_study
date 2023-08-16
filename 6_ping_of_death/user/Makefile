CC=gcc
CFLAGS=-Wall
LDFLAGS=-lpcap -lpthread -lrt
TARGET=monitor_icmp
OBJS=main.o event_handler.o packet_handler.o filter_list.o

all: $(TARGET)
	
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) $(OBJS)

