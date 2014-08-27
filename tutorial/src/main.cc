#include "libs/giga/file.h"
#include "libs/giga/client.h"

#include <cstdio>
#include <iostream>
#include <memory>

int main() {
	remove("example.txt");

	std::shared_ptr<giga::File> f (new giga::File("example.txt", "rw+"));
	std::shared_ptr<giga::Client> c = f->open();

	// write to the end of the file
	c->write("Hello world!");

	c->seek(0, true, true);
	std::cout << c->read(100) << std::endl;

	// overwrite portions of a file
	c->seek(6, true, true);
	c->write("earth");

	c->seek(0, true, true);
	std::cout << c->read(100) << std::endl;

	// erase "Hello"
	c->seek(0, true, true);
	c->erase(5);

	c->seek(0, true, true);
	std::cout << c->read(100) << std::endl;

	c->close();
	f->save();

	// reopen the file at the beginning of the stream
	c->open();

	// insert into the file
	c->write("I love the", true);

	c->seek(0, true, true);
	std::cout << c->read(100) << std::endl;

	c->close();
	f->save();
}
