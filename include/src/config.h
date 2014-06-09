#ifndef _CONFIG_H
#define _CONFIG_H

#include <cstddef>

namespace giga {
	class Config {
		public:
			Config();
			Config(size_t page_size);

			size_t get_page_size();

		private:
			size_t page_size;
	};
}

#endif
