TARGET   = autoaway.so
OBJS     = autoaway.o

my_CPPFLAGS =
my_CFLAGS   = -Wall -O2 -fPIC
my_LDFLAGS  =

PC_LIBS     = x11 xscrnsaver
CFLAGS     := $(CFLAGS) $(shell pkg-config $(PC_LIBS) --cflags)
LIBS       := $(shell pkg-config $(PC_LIBS) --libs)

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) -shared -o $@ $^ $(my_CFLAGS) $(CFLAGS) $(my_LDFLAGS) $(LDFLAGS) $(LIBS)

%.o: %.c
	$(CC) -c -o $@ $< $(my_CPPFLAGS) $(CPPFLAGS) $(my_CFLAGS) $(CFLAGS)

clean:
	$(RM) $(OBJS) $(TARGET)
