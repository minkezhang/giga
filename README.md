giga
====

Concurrent File I/O on Arbitrarily-Sized Files

Overview
----

`giga` is a concurrent file I/O library which seeks to ameliorate large-file memory problems.

The way in which files are accessed in many languages is to essentially `mmap` the contents of the file into RAM -- but doing so with large files present memory 
allocation problems that do not have an easy solution. Moreover, as of this project, there is a sparsity of file libraries in the public which implements 
`File::insert()` and `File::delete()` abstractions -- if a file were to be treated as a string, repeatedly calling file operations which alters the file length will be 
very inefficient due to the implicit `memcpy` invocations. Finally, there is also a sparsity of file libraries which deal with _concurrent_ file operations, which is 
becoming an increasingly desired feature in today's multiprocessor environment.

Thus, we need a file library which can:
* read files in its entirety without needing to allocate the size of the file of memory space to do so,
* handle file length changes gracefully and efficiently, and
* handle concurrent file changes.

Installation
----

```bash
git clone https://github.com/cripplet/giga.git
cd giga
git submodule update --init --recursive
make CONCURRENT=false PERFORMANCE=true test
```

Updating
----

```bash
git pull
git submodule foreach --recursive git checkout master
git submodule foreach --recursive git pull
```

Usage
----

An example can be found in `tutorial/` and can be executed by running:

```bash
cd tutorial/
make
./tutorial.app
```

```cpp
// create a new file ('+') in the case it does not exist, and open with read-write properties
std::shared_ptr<giga::File> f (new giga::File("new_file.txt", "rw+"));

// open a new client with write-only privileges
std::shared_ptr<giga::Client> c_1 = f->open(NULL, "w");

// open another client with the privileges of the file ("rw" in this case)
std::shared_ptr<giga::Client> c_2 = f->open();

// atomically inserts at the beginning of the file
c_2->write("prepend\n", true)

// atomically seeks to the beginning of the file, in a relative seek
c_2->seek(7, false);

// seek to 1 byte from the end of the file, in an absolute seek
c_1->seek(1, false, true);

// overwrite data here
c_1->write(" ");

// seek to the beginning of the file, in an absolute seek
c_2->seek(0, true, true)

// atomically erase "prepend"
c_2->erase(7);

// append to the end of the file
c_1->write("append");

// read from the file for at most 100 bytes
// " append"
std::cout << c_2->read(100) << std::endl;

// close files
c_1->close();
c_2->close();

// save the file
f->save();
```

More examples can be found in the `tests` directory, including usage of the performance test suite (`tests/performance.cc`).

Caveats
----

* `giga` reserves the path `/tmp/giga/` to store all intermediate files
* probably will need to be tailored a bit to compile on Windows machines

Todo
----

* tutorial
* implement a better caching backend (will NOT break interface)
* more code documentation

Contact
----

* [github](https://github.com/cripplet/giga)
* [gmail](mailto:minke.zhang@gmail.com)
* issues and feature requests should be directed to the project [issues](https://github.com/cripplet/giga/issues) page
