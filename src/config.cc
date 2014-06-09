#include <cstddef>

#include "src/config.h"

giga::Config::Config() : giga::Config::Config(16 * 1024) {}
giga::Config::Config(size_t page_size) {
	this->page_size = page_size;
}

size_t giga::Config::get_page_size() { return(this->page_size); }
