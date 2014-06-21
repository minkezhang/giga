#ifndef _CONFIG_H
#define _CONFIG_H

#include <cstddef>
#include "src/global.h"

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
