CC:=gcc
CFLAGS:=-O2 -Wall -lrt
DYNAMICLIBS:=-L/usr/local/lib -Wl,-rpath=/usr/local/lib

TARGET:=main
OBJS:=main.o statistics.o validator.o

all: $(OBJS)
	 $(CC) $(DYNAMICLIBS) $(CFLAGS) -o $(TARGET) $(OBJS) -lmoeprlnc -lmoep 

%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)

.PHONY: clean
clean:
	rm -fv $(OBJS)