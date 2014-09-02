CC=g++

# yadda yadda yadda beware -O3 optimizations and threading
#	-Wl,--no-as-need -- cf. http://bit.ly/ToQTcC
#	-fstack-protector-all -- cf. http://bit.ly/1vXj2nU
CFLAGS=-Wall -Werror -O3 -std=c++11 -g -Wl,--no-as-needed -ldl -rdynamic -fstack-protector-all
# turbocharge seeking on large files
CFLAGS+=-D _FILE_OFFSET_BITS=64

#custom flags
CONCURRENT?=false
PERFORMANCE?=false
ifeq ($(CONCURRENT), true)
	CFLAGS+=-D _GIGA_CONCURRENT_TESTS
endif
ifeq ($(PERFORMANCE), true)
	CFLAGS+=-D _GIGA_PERFORMANCE_TESTS
endif

INCLUDE=-Iinclude/

# include statements necessary to link all the individual libraries
INCLUDE_LIBS=-Iexternal/catch/include/ -Iexternal/exceptionpp/include/ -Iinclude/libs/stacktrace/ -Iexternal/cachepp/include/

# std::thread relies on the pthread lib
LIBS=-pthread

# remember to add all sources from subdirectories as well here
SOURCES=src/*cc tests/*cc libs/*/*cc

OBJECTS=$(SOURCES:.cc=.o)

EXECUTABLE=giga.app

.PHONY: all test clean prep memcheck

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	@$(CC) $(CFLAGS) $(INCLUDE_LIBS) $(INCLUDE) $(OBJECTS) -o $@ $(LIBS)

# prep test files
prep:
	@rm -f "tests/files/giga-file-save"
	@rm -f "tests/files/page-zero-size"
	@rm -f "tests/files/nonexistent"
	@rm -f "tests/files/giga-performance"

	@mkdir -p "tests/files/"
	@mkdir -p "/tmp/giga/"
	@echo "hello world!" > "tests/files/giga-file-read"
	@echo "foo" > "tests/files/foo"
	@echo "foobar" > "tests/files/giga-insert-concurrent"
	@touch "tests/files/page-zero-size"
	@touch "tests/files/giga-file-save"

# remember to run unit tests
test: clean $(EXECUTABLE) prep
	@# set ulimit -c unlimited to get a core dump and analyze via gdb
	@#	cf. http://bit.ly/1zlOj8u, http://bit.ly/1n2ONGD, http://bit.ly/1n2ONGD, http://bit.ly/VCQ0yM
	@ulimit -c unlimited && time ./$(EXECUTABLE) | tee results.log

memcheck: clean $(EXECUTABLE) prep
	@# cf. http://bit.ly/1t4tJIx
	@ulimit -c unlimited && valgrind --leak-check=full --show-leak-kinds=all ./$(EXECUTABLE) 2>&1 | tee memcheck.log

clean:
	@rm -f $(EXECUTABLE) *.o *.log core tests/files/*

trace: $(EXECUTABLE)
	@strace -f ./$(EXECUTABLE) > strace.log 2>&1
