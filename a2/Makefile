FLAGS = -Wall -std=gnu99 -g

all: print_ftree

print_ftree: print_ftree.o ftree.o 
	gcc ${FLAGS} -o $@ $^

%.o: %.c ftree.h
	gcc ${FLAGS} -c $<

clean: 
	rm -f *.o print_ftree
