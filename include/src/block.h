#ifndef _BLOCK_H
#define _BLOCK_H

#include <memory>

#include "src/global.h"

namespace giga {
	class Block {
		public:
			Block(giga_size global_offset, giga_size size, const std::shared_ptr<Block>& prev, const std::shared_ptr<Block>& next);

			giga_size get_size();

			void set_prev(const std::shared_ptr<Block>& prev);
			void set_next(const std::shared_ptr<Block>& next);

			std::shared_ptr<Block> get_prev();
			std::shared_ptr<Block> get_next();

			void load();
			void unload();

			giga_size read(std::string buffer, giga_size n_bytes);
			giga_size write(std::string buffer);

		private:
			std::string id;
			giga_size global_offset;
			giga_size size;

			int is_dirty;
			int is_loaded;

			std::shared_ptr<Block> prev;
			std::shared_ptr<Block> next;

			std::string data;
	};
}

#endif
