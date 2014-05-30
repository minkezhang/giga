CC=g++

# yadda yadda yadda beware -O3 optimizations and threading
CFLAGS=-Wall -Werror -O3 -std=c++11 -g

INCLUDE=-Iinclude/

# std::thread relies on the pthread lib          
LIBS=-lpthread -lncurses

# remember to add all sources from subdirectories as well here
SOURCES=src/*cc

OBJECTS=$(SOURCES:.cc=.o)

EXECUTABLE=giga.app

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(CFLAGS) $(INCLUDE) $(OBJECTS) -o $@ $(LIBS)

# remember to run unit tests
test: clean $(EXECUTABLE)
	./$(EXECUTABLE) --test

clean:
	rm -f $(EXECUTABLE) *.o

