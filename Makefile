CC=g++

# yadda yadda yadda beware -O3 optimizations and threading
# -Wl,--no-as-need -- cf. http://bit.ly/ToQTcC
CFLAGS=-Wall -Werror -O3 -std=c++11 -g -Wl,--no-as-needed -ldl -rdynamic

INCLUDE=-Iinclude/

# include statements necessary to link all the individual libraries
INCLUDE_LIBS=-Iinclude/libs/catch/include/ -Iinclude/libs/ -Iinclude/libs/stacktrace/

# std::thread relies on the pthread lib
# the segfault lib is very useful for the backtrace generated upon SIGSEGV
#	cf. http://bit.ly/1qFQDTG
# alternatively, set ulimit -c unlimited to get a core dump in /tmp/, and analyze via gdb
#	cf. http://bit.ly/1zlOj8u, http://bit.ly/1n2ONGD, http://bit.ly/1n2ONGD
LIBS=-pthread -lSegFault

# remember to add all sources from subdirectories as well here
SOURCES=src/*cc test/*cc libs/*/*cc

OBJECTS=$(SOURCES:.cc=.o)

EXECUTABLE=giga_tests.app

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(CFLAGS) $(INCLUDE) $(INCLUDE_LIBS) $(OBJECTS) -o $@ $(LIBS)

# remember to run unit tests
test: clean $(EXECUTABLE)
	ulimit -c unlimited
	./$(EXECUTABLE) | tee results.log

clean:
	rm -f $(EXECUTABLE) *.o *.log
