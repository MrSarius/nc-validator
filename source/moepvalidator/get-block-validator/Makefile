CC:=gcc
CFLAGS:=-O2 -Wall -lrt
DYNAMICLIBS:=-L/usr/local/lib -Wl,-rpath=/usr/local/lib

TARGET:=main
OBJS:=main.o

TARGET_DEBUG:=$(OUT_DEBUG)/main
OBJS_DEBUG:=$(OUT_DEBUG)/main.o $(OUT_DEBUG)/statistics.o $(OUT_DEBUG)/validator.o

all: $(OBJS)
	$(CC) $(DYNAMICLIBS) $(CFLAGS) -o $(TARGET) $(OBJS) -lmoeprlnc -lmoep -lmoepgf