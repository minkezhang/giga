CC=g++

# yadda yadda yadda beware -O3 optimizations and threading
# -Wl,--no-as-need -- cf. http://bit.ly/ToQTcC
CFLAGS=-Wall -Werror -O3 -std=c++11 -g -Wl,--no-as-needed -ldl -rdynamic

INCLUDE=-Iinclude/

# include statements necessary to link all the individual libraries
INCLUDE_LIBS=-Iinclude/libs/catch/include/ -Iinclude/libs/ -Iinclude/libs/stacktrace/

# std::thread relies on the pthread lib          
LIBS=-lpthread

# remember to add all sources from subdirectories as well here
SOURCES=src/*cc test/*cc libs/*/*cc

OBJECTS=$(SOURCES:.cc=.o)

EXECUTABLE=giga_tests.app

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(CFLAGS) $(INCLUDE) $(INCLUDE_LIBS) $(OBJECTS) -o $@ $(LIBS)

# remember to run unit tests
test: clean $(EXECUTABLE)
	./$(EXECUTABLE) | tee results.log

concurrent: clean $(EXECUTABLE)
	while true; do ./$(EXECUTABLE); sleep 1; done;

clean:
	rm -f $(EXECUTABLE) *.o *.log
