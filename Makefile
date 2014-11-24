.PHONY: all clean
all: lua-game

CFLAGS = -Wall -Wextra -g
LDFLAGS = -llua -lSDL2 -pthread -lGL

OBJECTS = \
	main.o \
	draw.o \
	lua.o

-include $(OBJECTS:.o=.d)

%.o: %.c
	clang -c $(CFLAGS) -o $@ $<
	gcc -MM $*.c -MF $*.d

lua-game: $(OBJECTS)
	clang $(CFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	rm -f lua-game lua-thread-test *.o *.d
