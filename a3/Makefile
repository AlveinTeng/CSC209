FLAGS = -Wall -std=gnu99 -g
EXECS = generate_points closest

all: ${EXECS}

generate_points: generate_points.o
	gcc ${FLAGS} -o $@ $^ -lm

closest: closest.o serial_closest.o parallel_closest.o utilities_closest.o
	gcc ${FLAGS} -o $@ $^ -lm

parallel_closest.o: parallel_closest.c serial_closest.h utilities_closest.h
	gcc ${FLAGS} -c $<

serial_closest.o: serial_closest.c utilities_closest.h
	gcc ${FLAGS} -c $<

generate_points.o: generate_points.c point.h
	gcc ${FLAGS} -c $<

closest.o: closest.c parallel_closest.h serial_closest.h utilities_closest.h
	gcc ${FLAGS} -c $<

utilities_closest.o: utilities_closest.c
	gcc ${FLAGS} -c $<

clean:
	rm -rf *.o ${EXECS}
