#include "libs/catch/include/catch.hpp"

#include <atomic>

TEST_CASE("general|compatibility") {
	std::string buf = "";
	REQUIRE(buf.size() == 0);
	buf = "a long test string yadda yadda yadda";
	buf = "";
	buf.reserve(0);
	REQUIRE(buf.size() == 0);
	/**
	 * if buffer doesn't clear memory on a resize, unloading won't actually save any memory
	 *	string::reserve does not guarantee freeing up memory, depending on implementation
	 *	cf. http://bit.ly/1spckXq
	 */
	REQUIRE(buf.capacity() == 0);

	/**
	 * std::atomic<bool> instances are used in the data blocks to guard Block::prev and Block::next access
	 * we need to save space here and ensure the use of this data structure won't bloat the size of the linked list
	 */
	REQUIRE(sizeof(std::atomic<bool>) == 1);
}
