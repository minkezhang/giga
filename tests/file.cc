#include "libs/catch/catch.hpp"
#include "libs/exceptionpp/exception.h"

#include "src/file.h"

TEST_CASE("giga|config-probe") {
	giga::Config c = giga::Config(100, 200, 2);
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
	REQUIRE_THROWS_AS(giga::File("tests/files/nonexistent/nonexistent", "r+"), exceptionpp::InvalidOperation);

	std::shared_ptr<giga::File> f (new giga::File("tests/files/foo", "r", giga::Config(3, 4, 2)));
	REQUIRE(f->get_filename().compare("tests/files/foo") == 0);
	REQUIRE(f->get_mode().compare("r") == 0);

	REQUIRE_THROWS_AS(f->i(NULL, ""), exceptionpp::InvalidOperation);
	REQUIRE_THROWS_AS(f->d(NULL, 10), exceptionpp::InvalidOperation);
	REQUIRE_THROWS_AS(f->w(NULL, ""), exceptionpp::InvalidOperation);

	std::shared_ptr<giga::Client> c = f->open();
	REQUIRE_NOTHROW(c->read(10));
	REQUIRE_THROWS_AS(c->erase(10), exceptionpp::InvalidOperation);
	REQUIRE_THROWS_AS(c->write(""), exceptionpp::InvalidOperation);
	c->close();

	REQUIRE_THROWS_AS(f->open(c, "r"), exceptionpp::InvalidOperation);

	f = std::shared_ptr<giga::File> (new giga::File("tests/files/foo", "w", giga::Config(3, 4, 2)));
	REQUIRE_THROWS_AS(f->r(NULL, 10), exceptionpp::InvalidOperation);

	REQUIRE_NOTHROW(giga::File("tests/files/nonexistent", "+"));
	remove("tests/files/nonexistent");
}

TEST_CASE("giga|file-open") {
	std::shared_ptr<giga::File> f (new giga::File("tests/files/giga-file-read", "r"));
	std::shared_ptr<giga::Client> c = f->open();
	REQUIRE(c->get_pos() == 0);
	c->close();
}

TEST_CASE("giga|file-seek") {
	std::shared_ptr<giga::File> f (new giga::File("tests/files/giga-file-read", "rw", giga::Config(2, 2, 2)));
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

	// absolute seeks
	REQUIRE(c->seek(0, true, true) == 0);
	REQUIRE(c->seek(5, true, true) == 5);
	REQUIRE(c->seek(1, true, true) == 1);
	REQUIRE(c->seek(0, false, true) == 13);
	REQUIRE(c->seek(13, false, true) == 0);
	REQUIRE(c->seek(5, false, true) == 8);

	c->close();
}

TEST_CASE("giga|file-read") {
	std::shared_ptr<giga::File> f (new giga::File("tests/files/giga-file-read", "rw", giga::Config(2, 2, 2)));
	std::shared_ptr<giga::Client> c = f->open();
	REQUIRE(c->read(0).compare("") == 0);
	REQUIRE(c->read(1).compare("h") == 0);
	REQUIRE(c->read(2).compare("el") == 0);
	REQUIRE(c->read(100).compare("lo world!\n") == 0);
	c->close();
	c->open();
	REQUIRE(c->read(100).compare("hello world!\n") == 0);
	c->close();
}

TEST_CASE("giga|file-erase") {
	std::shared_ptr<giga::File> f (new giga::File("tests/files/giga-file-read", "rw", giga::Config(2, 5, 2)));
	std::shared_ptr<giga::Client> c_1 = f->open();
	std::shared_ptr<giga::Client> c_2 = f->open();

	REQUIRE(c_2->seek(1, true) == 1);

	REQUIRE(c_2->erase(1) == 1);
	REQUIRE(f->get_size() == 12);
	REQUIRE(c_1->get_pos() == 0);
	REQUIRE(c_2->get_pos() == 1);
	REQUIRE(c_1->read(100).compare("hllo world!\n") == 0);
	REQUIRE(c_2->read(100).compare("llo world!\n") == 0);

	REQUIRE(c_1->seek(100, false) == 0);
	REQUIRE(c_2->seek(100, false) == 0);
	REQUIRE(c_1->seek(1, true) == 1);
	REQUIRE(c_2->seek(2, true) == 2);
	REQUIRE(c_1->get_pos() == 1);
	REQUIRE(c_2->get_pos() == 2);

	REQUIRE(c_1->erase(1) == 1);
	REQUIRE(f->get_size() == 11);
	REQUIRE(c_1->get_pos() == 1);
	REQUIRE(c_2->get_pos() == 1);
	REQUIRE(c_1->read(100).compare("lo world!\n") == 0);
	REQUIRE(c_2->read(100).compare("lo world!\n") == 0);
	REQUIRE(c_1->seek(100, false) == 0);
	REQUIRE(c_2->seek(100, false) == 0);
	REQUIRE(c_2->read(100).compare("hlo world!\n") == 0);
	REQUIRE(c_1->read(100).compare("hlo world!\n") == 0);

	c_1->close();
	c_2->close();

	f.reset();
	f = std::shared_ptr<giga::File> (new giga::File("tests/files/giga-file-read", "rw", giga::Config(2, 5, 2)));
	c_1 = f->open();
	c_2 = f->open();

	REQUIRE(c_1->seek(100, false) == 0);
	REQUIRE(c_2->seek(100, false) == 0);
	REQUIRE(c_2->seek(2, true) == 2);

	REQUIRE(c_1->erase(2) == 2);
	REQUIRE(c_1->get_pos() == 0);
	REQUIRE(c_2->get_pos() == 0);
	REQUIRE(c_1->read(100).compare("llo world!\n") == 0);
	REQUIRE(c_2->seek(100, false) == 0);
	REQUIRE(c_2->read(100).compare("llo world!\n") == 0);

	REQUIRE(c_1->seek(100, false) == 0);
	REQUIRE(c_2->seek(100, false) == 0);
	REQUIRE(c_2->seek(3, true) == 3);

	REQUIRE(c_1->erase(2) == 2);
	REQUIRE(c_1->get_pos() == 0);
	REQUIRE(c_2->get_pos() == 1);

	REQUIRE(c_1->read(100).compare("o world!\n") == 0);
	REQUIRE(c_2->read(100).compare(" world!\n") == 0);
	REQUIRE(c_1->erase(100) == 0);

	REQUIRE(c_1->seek(100, false) == 0);

	REQUIRE(c_1->erase(100) == 9);
	REQUIRE(c_1->read(100).compare("") == 0);
	REQUIRE(c_2->read(100).compare("") == 0);
	REQUIRE(f->get_size() == 0);
	REQUIRE(c_1->get_pos() == 0);
	REQUIRE(c_2->get_pos() == 0);

	c_1->close();
	c_2->close();

	f.reset();
	f = std::shared_ptr<giga::File> (new giga::File("tests/files/giga-file-read", "rw", giga::Config(2, 5, 2)));
	c_1 = f->open();
	c_2 = f->open();

	REQUIRE(c_1->seek(2, true) == 2);
	REQUIRE(c_2->seek(2, true) == 2);

	REQUIRE(c_1->erase(4) == 4);
	REQUIRE(c_1->read(100).compare("world!\n") == 0);
	REQUIRE(c_2->read(100).compare("world!\n") == 0);

	c_1->close();
	c_2->close();
}

TEST_CASE("giga|file-insert") {
	std::shared_ptr<giga::File> f (new giga::File("tests/files/giga-file-read", "rw", giga::Config(2, 3, 2)));
	std::shared_ptr<giga::Client> c_1 = f->open();
	std::shared_ptr<giga::Client> c_2 = f->open();

	REQUIRE(c_2->seek(1, true) == 1);

	REQUIRE(f->get_size() == 13);
	REQUIRE(c_1->write("foo", true) == 3);
	REQUIRE(f->get_size() == 16);
	REQUIRE(c_1->get_pos() == 3);
	REQUIRE(c_2->get_pos() == 4);
	REQUIRE(c_2->read(100).compare("ello world!\n") == 0);
	REQUIRE(c_1->read(100).compare("hello world!\n") == 0);

	REQUIRE(c_1->seek(100, false) == 0);
	REQUIRE(c_2->seek(100, false) == 0);
	REQUIRE(c_1->seek(1, true) == 1);

	REQUIRE(c_1->write("zee", true) == 3);
	REQUIRE(f->get_size() == 19);
	REQUIRE(c_1->get_pos() == 4);
	REQUIRE(c_2->get_pos() == 0);
	REQUIRE(c_1->read(100).compare("oohello world!\n") == 0);
	REQUIRE(c_2->read(100).compare("fzeeoohello world!\n") == 0);

	REQUIRE(c_2->write("addendum") == 8);
	REQUIRE(f->get_size() == 27);
	REQUIRE(c_1->seek(100, false) == 0);
	REQUIRE(c_1->seek(4, true) == 4);
	REQUIRE(c_1->read(100).compare("oohello world!\naddendum") == 0);

	c_1->close();
	c_2->close();

	f->load();

	c_1->open();
	c_2->open();

	REQUIRE(c_2->seek(2, true) == 2);
	REQUIRE(c_1->write("ab", true) == 2);
	REQUIRE(c_1->read(100).compare("hello world!\n") == 0);
	REQUIRE(c_2->read(100).compare("llo world!\n") == 0);
	REQUIRE(c_2->seek(15, false) == 0);
	REQUIRE(c_2->read(100).compare("abhello world!\n") == 0);

	c_1->close();
	c_2->close();
}

TEST_CASE("giga|file-write") {
	std::shared_ptr<giga::File> f (new giga::File("tests/files/giga-file-read", "rw", giga::Config(2, 3, 2)));
	std::shared_ptr<giga::Client> c = f->open();

	REQUIRE(c->write("") == 0);
	REQUIRE(c->get_pos() == 0);
	REQUIRE(c->write("abcde") == 5);
	REQUIRE(c->get_pos() == 5);

	REQUIRE(c->write("|world!\nEXTRAEXTRA") == 18);
	REQUIRE(f->get_size() == 23);
	REQUIRE(c->get_pos() == 23);

	REQUIRE(c->seek(100, false) == 0);
	REQUIRE(c->read(100).compare("abcde|world!\nEXTRAEXTRA") == 0);

	c->close();

	f = std::shared_ptr<giga::File> (new giga::File("tests/files/nonexistent", "rw+"));
	c = f->open();
	REQUIRE(c->write("foo") == 3);
	REQUIRE(c->seek(0, true, true) == 0);
	REQUIRE(c->erase(3) == 3);
	REQUIRE(c->get_pos() == 0);
	c->close();
}

TEST_CASE("giga|file-save") {
	std::shared_ptr<giga::File> f (new giga::File("tests/files/giga-file-save", "rw", giga::Config(2, 3, 2)));
	std::shared_ptr<giga::Client> c_1 = f->open();
	std::shared_ptr<giga::Client> c_2 = f->open();
	std::shared_ptr<giga::Client> c_3 = f->open();

	REQUIRE(f->get_size() == 0);

	c_1->write("abcde");
	c_1->write("foobarbaz\n");
	c_1->seek(100, false);
	c_1->write("prependprepend", true);
	REQUIRE(c_2->seek(9, true) == 9);
	REQUIRE(c_3->seek(9, true) == 9);
	c_1->save();

	REQUIRE(f->get_size() == 29);
	REQUIRE(c_1->seek(0, true, true) == 0);
	REQUIRE(c_1->read(100).compare("prependprependabcdefoobarbaz\n") == 0);
	REQUIRE(c_2->read(100).compare("ependabcdefoobarbaz\n") == 0);
	REQUIRE(c_3->read(100).compare("ependabcdefoobarbaz\n") == 0);
	c_1->save();
	REQUIRE(c_1->seek(10, false) == 19);
	REQUIRE(c_1->read(10).compare("foobarbaz\n") == 0);
	c_1->close();
	c_2->close();
}
