CC=g++

# yadda yadda yadda beware -O3 optimizations and threading
CFLAGS=-Wall -Werror -O3 -std=c++11 -g -Wl,--no-as-needed

INCLUDE=-Iinclude/ -Iinclude/libs/catch/include/

# std::thread relies on the pthread lib          
LIBS=-lpthread

# remember to add all sources from subdirectories as well here
SOURCES=src/*cc test/*cc libs/*/*cc

OBJECTS=$(SOURCES:.cc=.o)

EXECUTABLE=giga_tests.app

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(CFLAGS) $(INCLUDE) $(OBJECTS) -o $@ $(LIBS)

# remember to run unit tests
test: clean $(EXECUTABLE)
	./$(EXECUTABLE) > results.log

clean:
	rm -f $(EXECUTABLE) *.o *.log
