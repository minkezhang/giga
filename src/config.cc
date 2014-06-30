#include <cstddef>

#include "src/exception.h"

#include "src/config.h"

giga::Config::Config() : giga::Config::Config(16 * 1024, 100) {}
giga::Config::Config(size_t page_size, size_t n_cache_entries) {
	this->page_size = page_size;
	this->n_cache_entries = n_cache_entries;
	if(this->n_cache_entries == 0) {
		throw(giga::InvalidOperation("giga::Config::Config", "cache size must be strictly positive"));
	}
}

size_t giga::Config::get_page_size() { return(this->page_size); }
size_t giga::Config::get_n_cache_entries() { return(this->n_cache_entries); }
