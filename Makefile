CC = gcc
CFLAGS = -g -Wall -Werror
BIN = ftserver

all: ftserver

default: ftserver

ftserver: ft_func.o ftserver.o
	$(CC) $(CFLAGS) -o $(BIN) ft_func.o ftserver.o 

ft_func.o:
	$(CC) $(CFLAGS) -c ft_func.c

ftserver.o: 
	$(CC) $(CFLAGS) -c ftserver.c

clean:
	rm -f *.o $(BIN)

