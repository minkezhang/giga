giga
====

Concurrent File I/O on Arbitrarily-Sized Files

Overview
----

`giga` is a concurrent file I/O library which seeks to ameliorate large-file memory problems.

Internally, a file is represented in memory with a linked-list bit-array keeping track of dirty blocks of data. On file save, all the dirty bits are then written to the 
file; intermediate states of dirty file portions are stored on disk in `/tmp/`.

Current Features
----
* unit tests covering concurrency usage
* concurrent and sequential reads implemented

Installation
----

```
git clone --recursive https://github.com/cripplet/giga.git
```

cf. `test/concurrent.cc` for usage example
