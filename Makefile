CC = gcc
CFLAGS = -g -Wall

# Target to build the test program using your memory manager
tester_mm: tester.o mem_manage.o
	$(CC) $(CFLAGS) -o tester_mm tester.o mem_manage.o

# Target to build the test program using the system malloc/free
tester_system: tester.c
	$(CC) $(CFLAGS) -o tester_system tester.c -DUSE_SYSTEM_MALLOC

# Compile mem_manage.o
mem_manage.o: mem_manage.c mem_manage.h
	$(CC) $(CFLAGS) -c mem_manage.c

# Compile tester.o
tester.o: tester.c mem_manage.h
	$(CC) $(CFLAGS) -c tester.c

# Clean up generated files
clean:
	rm -f tester_mm tester_system *.o
