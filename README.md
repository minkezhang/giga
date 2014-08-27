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
make CONCURRENT=false PERFORMANCe=true test
```

Updating
----

```bash
git pull
git submodule foreach --recursive git checkout master
git submodule foreach --recursive git pull
```

Contact
----

* [github](https://github.com/cripplet/giga)
* [gmail](mailto:minke.zhang@gmail.com)
* issues and feature requests should be directed to the project [issues](https://github.com/cripplet/giga/issues) page
