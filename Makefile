#https://stackoverflow.com/questions/2214575/passing-arguments-to-make-run
ifeq (run,$(firstword $(MAKECMDGOALS)))
RUN_ARGS := $(wordlist 2,$(words $(MAKECMDGOALS)),$(MAKECMDGOALS))
$(eval $(RUN_ARGS):;@:)
endif

CC=mpicc
CFLAGS=-g -ggdb3 -Wall -Wextra
LD=mpicc
LDFLAGS=-lm

SOURCES=main.c
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=main.bin

MPI_ARGS=-np 4

.PHONY: all run debug test clean

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(LD) $(LDFLAGS) $< -o $@

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

run: $(EXECUTABLE)
	mpirun $(MPI_ARGS) $(EXECUTABLE) $(RUN) $(RUN_ARGS)

#debug: $(EXECUTABLE)
#  gdb ./$(EXECUTABLE)

test: $(EXECUTABLE) $(TESTINPUT)
	./$(EXECUTABLE) < $(TESTINPUT)

clean:
	rm -f $(OBJECTS) $(EXECUTABLE)
