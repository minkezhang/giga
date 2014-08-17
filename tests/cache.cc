#include <memory>

#include "libs/cachepp/simpleserialcache.h"
#include "libs/cachepp/testsuite.h"
#include "libs/catch/catch.hpp"

#include "src/page.h"

TEST_CASE("giga|cache-sanity") {
	std::shared_ptr<cachepp::SimpleSerialCache<giga::Page>> c (new cachepp::SimpleSerialCache<giga::Page>(1));
	std::shared_ptr<std::vector<std::shared_ptr<giga::Page>>> l (new std::vector<std::shared_ptr<giga::Page>>());
	std::shared_ptr<giga::Page> p (new giga::Page(1, "tests/files/foo", 0, 4));
	l->push_back(p);

	auto t = cachepp::TestSuite<cachepp::SimpleSerialCache<giga::Page>, cachepp::SimpleSerialCacheData, giga::Page>(c);
	std::shared_ptr<std::vector<size_t>> access_pattern (new std::vector<size_t> { 0, 0, 0, 0, 0 });
	std::shared_ptr<std::vector<std::shared_ptr<cachepp::SimpleSerialCacheData>>> access_pattern_aux (new std::vector<std::shared_ptr<cachepp::SimpleSerialCacheData>>());
	std::shared_ptr<std::vector<size_t>> line_size (new std::vector<size_t> (1, 1024));

	t.correctness(l, 10000, false);
	t.performance("SAN", l, line_size, access_pattern, access_pattern_aux, .5, 10000, false);
	std::cout << t.get_result().to_string(false);
}

TEST_CASE("giga|cache-LRU") {
}
