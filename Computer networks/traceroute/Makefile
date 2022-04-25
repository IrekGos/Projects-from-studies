PROGRAM_NAME=traceroute
SOURCES=traceroute.c wrappers.c
OBJECTS=$(SOURCES:.c=.o)

GCC=gcc
CFLAGS= -std=gnu17 -Wall -Wextra

all: compile link

compile:
	$(GCC) $(CFLAGS) -c $(SOURCES)
	
link: 
	$(GCC) $(OBJECTS) -o $(PROGRAM_NAME)
	
clean:
	-rm *.o
	
distclean:
	-rm *.o $(PROGRAM_NAME)
