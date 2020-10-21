SHELL = /bin/bash
FLAGS = -Wall -std=gnu99 -g

.PHONY: check_benford clean

all: benford

benford: benford.o benford_helpers.o
	gcc ${FLAGS} -o $@ $^

%.o: %.c benford_helpers.h
	gcc ${FLAGS} -c $<

check_benford: benford
	@check_benford_output=`echo -e "12176\n5476\n543\n3490\n24892\n28619\n2595\n1878" | ./benford 2 | cmp sample-benford-output 2>&1`; \
	if [ -z "$$check_benford_output" ]; then \
		echo Compiled and benford check passed; \
	else \
		echo Failed benford check; \
	fi

clean:
	rm -f *.o benford
