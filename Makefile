TARGET   = autoaway.so
OBJS     = autoaway.o

SHARED_CC  ?= $(CC) -shared

PKG_CONFIG ?= pkg-config
PACKAGES    = hexchat-plugin x11 xscrnsaver

PC_CFLAGS  := $(shell $(PKG_CONFIG) $(PACKAGES) --cflags)
PC_LIBS    := $(shell $(PKG_CONFIG) $(PACKAGES) --libs)

CFLAGS     := -Wall -O2 -fPIC $(CFLAGS)

.PHONY: all clean

.DELETE_ON_ERROR:

all: $(TARGET)

$(TARGET): $(OBJS)
	$(SHARED_CC) -o $@ $^ $(CFLAGS) $(PC_CFLAGS) $(LDFLAGS) $(PC_LIBS)

%.o: %.c
	$(CC) -c -o $@ $< $(CPPFLAGS) $(CFLAGS) $(PC_CFLAGS)

clean:
	$(RM) $(OBJS) $(TARGET)
