#include <cstddef>

#include "src/exception.h"

#include "src/config.h"

giga::Config::Config() : giga::Config::Config(16 * 1024, 24 * 1024, 100) {}
giga::Config::Config(size_t page_size, size_t max_block_size, size_t n_cache_entries) {
	if(page_size == 0) {
		throw(giga::InvalidOperation("giga::Config::Config", "page size must be strictly positive"));
	}
	if(max_block_size < page_size) {
		throw(giga::InvalidOperation("giga::Config::Config", "max page size must be greater than or equal to the page size"));
	}
	if(n_cache_entries == 0) {
		throw(giga::InvalidOperation("giga::Config::Config", "cache size must be strictly positive"));
	}
	this->page_size = page_size;
	this->max_block_size = max_block_size;
	this->n_cache_entries = n_cache_entries;
}

size_t giga::Config::get_max_block_size() { return(this->max_block_size); }
size_t giga::Config::get_page_size() { return(this->page_size); }
size_t giga::Config::get_n_cache_entries() { return(this->n_cache_entries); }
