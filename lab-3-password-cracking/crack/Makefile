CC = gcc
CFLAGS = -Wall -Wextra -pthread
LDLIBS = -lcrypt

TARGET = crack

all: $(TARGET)

$(TARGET): crack.o
	$(CC) $(CFLAGS) -o $(TARGET) crack.o $(LDLIBS)

crack.o: crack.c
	$(CC) $(CFLAGS) -c crack.c

clean:
	rm -f $(TARGET) *.o

.PHONY: all clean
