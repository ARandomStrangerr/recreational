CC = gcc
CFLAGS = -Wall -Wextra -g
TARGET = out
SRCS = communication/socket.c orchestrator/socket_management.c main.c
OBJS = $(SRCS:.c=.o)
$(TARGET):$(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

clean:
	rm -f $(OBJS) $(TARGET)
