SHELL = /bin/bash
FLAGS = -Wall -std=gnu99 -g

.PHONY: check_life2D clean

all: life2D

life2D: life2D.o life2D_helpers.o
	gcc ${FLAGS} -o $@ $^

%.o: %.c life2D_helpers.h
	gcc ${FLAGS} -c $<

check_life2D: life2D
	@check_life2D_output=`./life2D 5 4 10 < sample-life2D-input | cmp sample-life2D-output 2>&1`; \
	if [ -z "$$check_life2D_output" ]; then \
		echo Compiled and life2D check passed; \
	else \
		echo Failed life2D check; \
	fi

clean:
	rm -f *.o life2D
