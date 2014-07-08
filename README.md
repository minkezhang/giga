giga
====

Concurrent File I/O on Arbitrarily-Sized Files

Overview
----

`giga` is a concurrent file I/O library which seeks to ameliorate large-file memory problems.

The way in which files are accessed in many languages is to essentially `mmap` the contents of the file into RAM -- but doing so with large files present memory 
allocation problems that do not have an easy solution. Moreover, as of this project, there is a sparsity of file libraries in the public which implements `File::insert()` 
and `File::delete()` abstractions -- if a file were to be treated as a string, repeatedly calling file operations which alters the file length will be very inefficient due 
to the implicit `memcpy` invocations. Finally, there is also a sparsity of file libraries which deal with _concurrent_ file operations, which is becoming an increasingly 
desired feature in today's multiprocessor environment.

Thus, we need a file library which can
* read files in its entirety without needing to allocate `File::size()` of memory space to do so
* handle file length changes gracefully and efficiently
* handle concurrent file changes

Current Features
----
* object-oriented file framework
* unit tests covering concurrency usage
* concurrent and sequential reads implemented

Planned Features
----
* `overwrite(buf)` (cf. `fwrite(..., buf, ...)`), `insert(buf)`, and `delete(n)` write modes
* `seek(n)`, `save()`
* possible network integration
* performance testing
* change internal file representation to be more scalable

Known Bugs
----
* memory leak when calling file destructor, due to linked list

Installation and Testing
----

```
git clone --recursive https://github.com/cripplet/giga.git
make test
```

cf. `test/concurrent.cc` for usage example

Internals
----

The most desired feature of this library for the author are the file operations which alters the file length -- if these are to be implemented successfully, this would 
make the work of file editor development to be a much less work-intensive process, as the problems of dealing with varing file lengths are abstracted away into the 
library; we therefore chose to represent the file as a _doubly-linked list_.

### Concurrent Doubly-Linked List

#### Modifying Links

A difficulty of doubly-linked lists exists, in that an implementation of an efficient _concurrent_ list cannot be easily found. We write down our implementation notes 
here so as to be abstracted later, either by the author or by some enterprising programmer.

Given a doubly-linked node `n`, we wish to have the ability to append some data to the end of `n`: `n.insert(head_node, tail_node)`. In order to do this, we need two 
locks per node: a lock guarding access to `n.prev` and a lock guarding access to `n.next`:

```
// we assume all nodes from h to t are newly created, and therefore cannot be accessed by any other thread
public void Node::insert(Node h, Node t) {
	Node::lock(this->next_l);
	Node::lock(this->get_next_unsafe()->prev_l);

	// configuring the list segment to be appended
	h->set_prev_unsafe(this);
	t->set_next_unsafe(this->next);

	// adding to list -- the goal here is to set these nodes directly WITHOUT locking anything else
	this->get_next_unsafe()->set_prev_unsafe(t);
	this->set_next_unsafe(h);

	Node::unlock(this->get_next_unsafe()->prev_l);
	Node::unlock(this->next_l);
}

/**
 * guarded access -- same logic for get_prev_safe()
 *	these should be called by external functions and therefore must WAIT for pending inserts()
 */
public Node Node::get_next_safe() {
	Node::lock(this->next_l);
	...
}

/**
 * NO EXTERNAL FUNCTION can set next and prev nodes by acquiring the locks
 */
public void Node::set_next_safe(Node n) = delete;
```

The order of acquiring the locks is important in `Node::insert` -- we are extending the singly-linked link list hand-over-hand locking protocol to a doubly-linked list. 
The way in which we lock is strictly ordered and symmetrical across all nodes, thereby eliminating deadlock.

Because `get` and `set` operations are trivial, we can use spin-locks here (implemented in `Node::lock` and `Node::unlock`), which can be implemented using 
`std::atomic<bool>` of size `1 byte`, _reducing the memory footprint_ of the linked list. This is preferable to using more complex mutexes, such as `std::mutex`, which 
is of size `40 bytes`.

#### Modifying Content

Note that in the previous notes, we have not mentioned modifying the actual _content_ of the nodes.

Ideally, we would also like to add a lock per node to deal with contention on a per-node basis; unfortunately, we would like these locks to provide some more features, 
such as implementing the conditional variable API to notify waiting threads and such -- however, the memory footprint for such locks are prohibitively large for use on a 
per-node basis.

The use-case of these doubly-linked lists is that of large-file I/O, which is to say, we do not expect to be accessing all parts of the file at once -- we will be 
implementing a _cache_ to deal with accessing the content, and lock the associated _cache line_ when editing a block -- given `n_cache_lines` locks, we can call a simple 
hashing function `lock_index = n.id % n_cache_lines` to acquire an appropriate lock. Here, we are sacrificing a degree of concurrency (multiple nodes will map to the 
same lock), but at the benefit of a low memory footprint.

#### Alternatives

Of course, doubly linked lists are not the only way in which a file can be represented efficiently in memory -- at the cost of a higher overhead, we could theoretically 
implement a concurrent, generalized search tree, similar to the way in which the kernel represents files -- for very, very large files, we would just add another layer 
between the root node and the leaves. This approach also has the benefit of being slightly more documented (Bronson et al. 2010, Lehman and Yao, 1981), though in terms 
of large file I/O, we would still expect a caching technique similar to the one implemented in this library. We felt justified in using a doubly linked list as of now 
due to the use-case -- that is, multiple clients editing localized portions of the file, with little to no random seeks by any given client.

### File

#### Structure

```
typedef long long int giga_size;
class Block {
	private:
		size_t size;
		string data;			// actual data to be saved onto disk and wiped, or loaded into memory
		bool is_loaded, is_dirty;
		giga_size global_offset;

		// doubly-linked list implementation
		giga_size id;
		Block *next, *prev;
		std::atomic<bool> prev_lock, next_lock;
};
```

Internally, a file is represented in memory with a linked list bit-array keeping track of dirty blocks of data. `Block::global_offset` is a file offset variable -- we 
can use this to read directly from a file at the specified position, for `Block::size` bytes. If `Block::write` is called, `Block::is_dirty` is set to `true` and 
`Block::global_offset` is set to `0` -- upon a subsequent call to `Block::unload`, `Block::data` is written to a file in `/tmp/`. On calling `Block::load`, a file path 
is generated based on if `Block::is_dirty` is set, to read either from the original file, or read the changes stored in `/tmp/`.

On calling `File::save`, we walk through the linked list and write all changes to the file.

#### Cache Behavior

Each call to `Block::read` or `Block::write` will increment an `n_access` variable stored in the cache line -- if the cache is full, the file library walks through the 
cache and selects the cache line with the lowest `n_access` to be evicted (this is a simplified and generalized second-chance algorithm).

Contact
----

https://github.com/cripplet/giga/issues

