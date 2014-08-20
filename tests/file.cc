#include "libs/catch/catch.hpp"
#include "libs/exceptionpp/exception.h"

#include "src/file.h"

TEST_CASE("giga|config-probe") {
	giga::Config c = giga::Config(100, 200);
	std::shared_ptr<giga::Page> p (new giga::Page(1, "", 0, 100, false));
	REQUIRE(c.probe(p, 0, 1000) == 200);
	REQUIRE(c.probe(p, 0, 10) == 10);
	REQUIRE(c.probe(p, 10, 10) == 10);
	REQUIRE(c.probe(p, 10, 90) == 90);
	REQUIRE(c.probe(p, 10, 1000) == 190);
	REQUIRE(c.probe(p, 100, 1000) == 100);
	REQUIRE(c.probe(p, 100, 10) == 10);
	REQUIRE(c.probe(p, 100, 0) == 0);
}

TEST_CASE("giga|file") {
	REQUIRE_THROWS_AS(giga::File("tests/files/nonexistent", "r"), exceptionpp::InvalidOperation);

	giga::File f = giga::File("tests/files/foo", "r", giga::Config(3, 4));
	REQUIRE(f.get_filename().compare("tests/files/foo") == 0);
	REQUIRE(f.get_mode().compare("r") == 0);
}

TEST_CASE("giga|file-open") {
	std::shared_ptr<giga::File> f (new giga::File("tests/files/giga-file-read", "r"));
	std::shared_ptr<giga::Client> c = f->open();
	REQUIRE(c->get_pos() == 0);
	c->close();
}

TEST_CASE("giga|file-seek") {
	std::shared_ptr<giga::File> f (new giga::File("tests/files/giga-file-read", "r", giga::Config(2, 2)));
	std::shared_ptr<giga::Client> c = f->open();
	REQUIRE(f->get_size() == 13);
	REQUIRE(c->seek(2, true) == 2);
	REQUIRE(c->seek(1, false) == 1);
	REQUIRE(c->seek(1, false) == 0);
	REQUIRE(c->seek(9, true) == 9);
	REQUIRE(c->seek(3, false) == 6);
	REQUIRE(c->seek(100, true) == 13);
	REQUIRE(c->seek(100, false) == 0);
	REQUIRE(c->seek(100, true) == 13);
	c->close();
}

TEST_CASE("giga|file-read") {
	std::shared_ptr<giga::File> f (new giga::File("tests/files/giga-file-read", "r", giga::Config(2, 2)));
	std::shared_ptr<giga::Client> c = f->open();
	REQUIRE(c->read(0).compare("") == 0);
	REQUIRE(c->read(1).compare("h") == 0);
	REQUIRE(c->read(2).compare("el") == 0);
	REQUIRE(c->read(100).compare("lo world!\n") == 0);
	c->close();
}

TEST_CASE("giga|file-insert") {
	std::shared_ptr<giga::File> f (new giga::File("tests/files/giga-file-read", "r", giga::Config(2, 3)));
	std::shared_ptr<giga::Client> c_1 = f->open();
	std::shared_ptr<giga::Client> c_2 = f->open();

	REQUIRE(c_1->seek(1, true) == 1);
	REQUIRE(c_2->write("foo", true) == 3);
	REQUIRE(c_2->get_pos() == 3);
	REQUIRE(c_1->get_pos() == 4);
	REQUIRE(c_1->read(100).compare("ello world!\n") == 0);

	REQUIRE(c_1->seek(100, false) == 0);
	REQUIRE(c_2->seek(100, false) == 0);
	REQUIRE(c_1->seek(1, true) == 1);
	REQUIRE(c_1->write("foo", true) == 3);
	REQUIRE(c_2->get_pos() == 0);
	REQUIRE(c_2->read(100).compare("ffoooohello world!\n") == 0);

	REQUIRE(c_2->write("addendum") == 8);
	REQUIRE(c_1->read(100).compare("oohello world!\naddendum") == 0);

	c_1->close();
	c_2->close();
}

TEST_CASE("giga|file-write") {
	std::shared_ptr<giga::File> f (new giga::File("tests/files/giga-file-read", "r", giga::Config(2, 3)));
	std::shared_ptr<giga::Client> c = f->open();
	REQUIRE(c->write("") == 0);
	REQUIRE(c->get_pos() == 0);
	REQUIRE(c->write("abcde") == 5);
	REQUIRE(c->get_pos() == 5);
	REQUIRE(c->write("|world!\nEXTRAEXTRA") == 18);
	REQUIRE(c->get_pos() == 23);
	REQUIRE(c->seek(100, false) == 0);
	REQUIRE(c->read(100).compare("abcde|world!\nEXTRAEXTRA") == 0);
	c->close();
}
