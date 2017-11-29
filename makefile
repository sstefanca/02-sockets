CC=gcc
CFLAGS=-Wall -Werror -ggdb
LDFLAGS=-pthread
FILES=
OBJECTS=$(addsuffix .o , $(FILES))

all: server client

server: server.o $(OBJECTS)
	gcc $(LDFLAGS) -o $@ $^

client: client.o $(OBJECTS)
	gcc $(LDFLAGS) -o $@ $^

clean:
	rm -fv server client server.o client.o $(OBJECTS) *~

test_variables:
	echo $(FILES)
	echo $(OBJECTS)

.PHONY: all clean test_variables
