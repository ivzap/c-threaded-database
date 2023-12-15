CC = g++
CFLAGS = -Wall -pthread -fsanitize=address

TARGET = Server
OBJS = server.o TCPRequestChannel.o

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.cpp
	$(CC) $(CFLAGS) -c -o $@ $^

clean:
	rm -rf $(TARGET) $(OBJS)
