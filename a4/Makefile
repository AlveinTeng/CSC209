PORT=y
FLAGS = -DPORT=$(PORT) -Wall -g -std=gnu99 

twerver : twerver.o socket.o
	gcc $(FLAGS) -o $@ $^

%.o : %.c socket.h
	gcc $(FLAGS) -c $<

tidy :
	rm *.o

clean :
	rm *.o twerver