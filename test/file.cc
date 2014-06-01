#include "catch/include/catch.hpp"

#include <memory>

#include "src/file.h"

TEST_CASE("file|initialization") {
	std::shared_ptr<giga::File> file (new giga::File("file_open", "r"));
	REQUIRE(file->get_client_list().size() == 0);
	std::shared_ptr<giga::Client> c = file->open();
	REQUIRE(file->get_client_list().size() == 1);
	std::shared_ptr<giga::Client> d = file->open();
	REQUIRE(file->get_client_list().size() == 2);

	REQUIRE(c->get_pos() == 0);
	REQUIRE(c->get_id() == 0);
	REQUIRE(d->get_id() == 1);
}
