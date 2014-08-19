CC=g++

# yadda yadda yadda beware -O3 optimizations and threading
#	-Wl,--no-as-need -- cf. http://bit.ly/ToQTcC
#	-fstack-protector-all -- cf. http://bit.ly/1vXj2nU
CFLAGS=-Wall -Werror -O3 -std=c++11 -g -Wl,--no-as-needed -ldl -rdynamic -fstack-protector-all

INCLUDE=-Iinclude/

# include statements necessary to link all the individual libraries
INCLUDE_LIBS=-Iexternal/catch/include/ -Iexternal/exceptionpp/include/ -Iinclude/libs/stacktrace/ -Iexternal/cachepp/include/

# std::thread relies on the pthread lib
LIBS=-pthread

# remember to add all sources from subdirectories as well here
SOURCES=src/*cc tests/*cc libs/*/*cc

OBJECTS=$(SOURCES:.cc=.o)

EXECUTABLE=giga.app

.PHONY: all test clean prep

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(CFLAGS) $(INCLUDE_LIBS) $(INCLUDE) $(OBJECTS) -o $@ $(LIBS)

# prep test files
prep:
	@echo "hello world!" > "tests/files/giga-file-read"
	@echo "foo" > "tests/files/foo"

# remember to run unit tests
test: clean $(EXECUTABLE) prep
	@# set ulimit -c unlimited to get a core dump and analyze via gdb
	@#	cf. http://bit.ly/1zlOj8u, http://bit.ly/1n2ONGD, http://bit.ly/1n2ONGD, http://bit.ly/VCQ0yM
	ulimit -c unlimited && ./$(EXECUTABLE) | tee results.log

clean:
	@rm -f $(EXECUTABLE) *.o *.log core
