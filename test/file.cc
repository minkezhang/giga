#include "libs/catch/include/catch.hpp"

#include <memory>

#include "src/exception.h"
#include "src/file.h"

TEST_CASE("file|initialization") {
	REQUIRE_THROWS_AS(giga::File("test/files/nonexistent.txt", "r"), giga::FileNotFound);

	std::shared_ptr<giga::File> file (new giga::File("test/files/empty.txt", "r"));
	REQUIRE(file->get_n_clients() == 0);
	std::shared_ptr<giga::Client> c = file->open();
	REQUIRE(file->get_n_clients() == 1);
	std::shared_ptr<giga::Client> d = file->open();
	REQUIRE(file->get_n_clients() == 2);

	REQUIRE(c->get_pos() == 0);
	REQUIRE(c->get_id() == 0);
	REQUIRE(d->get_id() == 1);

	file->close(c);
	REQUIRE(file->get_n_clients() == 1);
	file->close(d);
	REQUIRE(file->get_n_clients() == 0);
}

