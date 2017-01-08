CC=g++
CFLAGS = -Wall
EXEC=consumer
OBJECTS=consumer.o functions.o

consumer: $(OBJECTS)
	$(CC) $(CFLAGS) -o $(EXEC) $(OBJECTS)

consumer.o: consumer.cpp
	$(CC) $(CFLAGS) -c consumer.cpp

functions.o: functions.cpp
	$(CC) $(CFLAGS) -c functions.cpp

.PHONY: clean

clean:
	rm -f $(EXEC) $(OBJECTS)
