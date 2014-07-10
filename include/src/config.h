#ifndef _CONFIG_H
#define _CONFIG_H

#include <cstddef>
#include "src/global.h"

/**
 * Config::Config provides a way to configure the File::File instance
 *
 * these functions MAY be called by the user, and are guaranteed to be atomic
 */
namespace giga {
	class Config {
		public:
			Config();
			Config(size_t page_size, size_t n_cache_entries);
			size_t get_page_size();
			size_t get_n_cache_entries();

		private:
			size_t page_size;
			size_t n_cache_entries;
	};
}

#endif
