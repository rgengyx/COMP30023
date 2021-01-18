CC=gcc
CFLAGS=-c -Wall
OBJ = scheduler
all: $(OBJ)
$(OBJ): scheduler.o linkedlist.o page.o
	$(CC) scheduler.o linkedlist.o page.o -o $(OBJ)
scheduler.o: scheduler.c linkedlist.h page.h
	$(CC) $(CFLAGS) scheduler.c
linkedlist.o: linkedlist.h linkedlist.c
	$(CC) $(CFLAGS) linkedlist.c
page.o: page.h page.c
	$(CC) $(CFLAGS) page.c
clean:
	rm -rf $(OBJ)
	rm -rf *.o