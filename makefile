CC = gcc
CFLAGS = -Wall
DEPS = math_functions.c map.c fold1.c util.c
OBJ = math_functions.o map.o fold1.o util.o dataPar.o

dataPar: dataPar.o
	$(CC) -pthread $(OBJ) -o dataPar

dataPar.o: dataPar.c $(DEPS)
	$(CC) $(CFLAGS) -c -pthread $(DEPS) dataPar.c
