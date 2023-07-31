CC=gcc
CFLAGS=-I. -I/usr/include/libnl3
DEPS =
OBJ = main.o  callback.o  frequency.o ie.o station_info.o
LIBS=-lnl-3 -lnl-genl-3

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

wifi_monitor: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean

clean:
	rm -f *.o *~ wifi_monitor

