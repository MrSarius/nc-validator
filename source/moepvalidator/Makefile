CC:=gcc
CFLAGS:=-O2 -Wall -lrt

TARGET:=main
OBJS:=main.o statistics.o validator.o

all: $(OBJS)
	$(CC) -o $(TARGET) $(OBJS) -lmoep $(CFLAGS)

%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)

.PHONY: clean
clean:
	rm -fv $(OBJS)
