CC = g++
CFLAGS = -Wall -pthread -fsanitize=address

TARGET = Server
OBJS = server.o TCPRequestChannel.o
DIRS = DATABASE received

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.cpp
	$(CC) $(CFLAGS) -c -o $@ $^

test: $(TARGET)
	sudo ./tests.sh

clean:
	rm -rf $(DIRS)
	rm -rf $(TARGET) $(OBJS)
