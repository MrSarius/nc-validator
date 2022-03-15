CC:=gcc
CFLAGS:=-O2 -Wall -lrt
DYNAMICLIBS:=-L/usr/local/lib -Wl,-rpath=/usr/local/lib

TARGET:=main
OBJS:=main.o statistics.o validator.o

# TODO: remove debug flag -g
all: $(OBJS)
	 $(CC) -g $(DYNAMICLIBS) $(CFLAGS) -o $(TARGET) $(OBJS) -lmoeprlnc -lmoep -lmoepgf

%.o: %.c
	$(CC) -g -c -o $@ $< $(CFLAGS)

.PHONY: clean
clean:
	rm -fv $(OBJS)