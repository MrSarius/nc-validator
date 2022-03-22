CC:=gcc
CFLAGS:=-O2 -Wall -lrt
DYNAMICLIBS:=-L/usr/local/lib -Wl,-rpath=/usr/local/lib
OUT:=build
OUT_DEBUG:=$(OUT)/debug

TARGET:=$(OUT)/main
OBJS:=$(OUT)/main.o

TARGET_DEBUG:=$(OUT_DEBUG)/main
OBJS_DEBUG:=$(OUT_DEBUG)/main.o

all: $(OBJS)
	$(CC) $(DYNAMICLIBS) $(CFLAGS) -o $(TARGET) $(OBJS) -lmoeprlnc -lmoep -lmoepgf

$(OUT)/%.o: %.c
	@mkdir -p $(OUT)
	$(CC) -c -o $@ $< $(CFLAGS)

# -g flag is for debugging
# -O0 removes compiler optimization -> fixes <optimized out> for debugger variables
# However, this might lead to different code execution for debug and production code!
debug: $(OBJS_DEBUG)
	$(CC) -g $(DYNAMICLIBS) $(CFLAGS) -o $(TARGET_DEBUG) $(OBJS_DEBUG) -lmoeprlnc -lmoep -lmoepgf

$(OUT_DEBUG)/%.o: %.c
	@mkdir -p $(OUT_DEBUG)
	$(CC) -g -c -o $@ $< $(CFLAGS) -O0

.PHONY: clean
clean:
	rm -fv $(OBJS)

clean_debug:
	rm -fv $(OBJS_DEBUG)