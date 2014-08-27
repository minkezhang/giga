#include "libs/giga/file.h"
#include "libs/giga/client.h"

#include <cstdio>
#include <iostream>
#include <memory>

int main() {
	remove("example.txt");

	std::shared_ptr<giga::File> f (new giga::File("example.txt", "rw+"));
	std::shared_ptr<giga::Client> c = f->open();

	c->write("Hello world!");

	c->seek(0, true, true);
	std::cout << c->read(100) << std::endl;

	c->seek(0, true, true);
	std::cout << c->get_pos() << ", " << c->erase(5) << std::endl;

	c->seek(0, true, true);
	c->write("Goodbye, cruel", true);

	c->seek(0, true, true);
	std::cout << c->read(100) << std::endl;

	c->close();
	f->save();

	c->open();

	c->seek(0, false, true);
	c->write(" But first, one last refrain");

	c->seek(0, true, true);
	std::cout << c->read(100) << std::endl;

	c->close();
	f->save();
}
