#ifndef _BLOCK_H
#define _BLOCK_H

#include <cstdlib>
#include <memory>
#include <mutex>

#include "src/file.h"
#include "src/global.h"

namespace giga {
	class File;
	class Block : public std::enable_shared_from_this<Block> {
		public:
			Block(giga_size global_offset, size_t size, const std::shared_ptr<Block>& prev, const std::shared_ptr<Block>& next);

			giga_size get_id();
			giga_size get_size();

			void set_prev(const std::shared_ptr<Block>& prev);
			void set_next(const std::shared_ptr<Block>& next);

			std::shared_ptr<Block> get_prev();
			std::shared_ptr<Block> get_next();

			void load(std::string filename, std::string mode);
			void unload(std::string filename);

			int get_is_loaded();
			int get_is_dirty();

			giga_size read(giga_size start, const std::shared_ptr<std::string>& buffer, giga_size n_bytes);
			giga_size write(giga_size start, const std::shared_ptr<std::string>& buffer);

		private:
			giga_size id;
			giga_size global_offset;
			size_t size;

			int is_dirty;
			int is_loaded;

			std::shared_ptr<Block> prev;
			std::shared_ptr<Block> next;

			std::string data;

			std::mutex block_lock;
	};
}

#endif
