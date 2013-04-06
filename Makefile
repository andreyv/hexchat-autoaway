TARGET   = autoaway.so
OBJS     = autoaway.o

PC_LIBS  = x11 xscrnsaver

CPPFLAGS := $(CPPFLAGS)
CFLAGS   := -Wall -O2 -fPIC $(shell pkg-config $(PC_LIBS) --cflags) $(CFLAGS)
LDFLAGS  := $(LDFLAGS)

LIBS     := $(shell pkg-config $(PC_LIBS) --libs)

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) -shared -o $@ $^ $(CFLAGS) $(LDFLAGS) $(LIBS)

%.o: %.c
	$(CC) -c -o $@ $< $(CPPFLAGS) $(CFLAGS)

clean:
	$(RM) $(OBJS) $(TARGET)
