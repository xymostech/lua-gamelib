.PHONY: all clean
all: lua-game

OBJECTS = \
	main.o \
	draw.o \
	lua.o

-include $(OBJECTS:.o=.d)

%.o: %.c
	clang -g -c -o $@ $<
	gcc -MM $*.c -MF $*.d

lua-game: $(OBJECTS)
	clang -g -o $@ $^ -llua -lSDL2 -pthread

clean:
	rm -f lua-game lua-thread-test *.o *.d
