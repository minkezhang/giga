#ifndef _BLOCK_H
#define _BLOCK_H

#include <atomic>
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

			void set_prev_unsafe(const std::shared_ptr<Block>& prev);
			void set_next_unsafe(const std::shared_ptr<Block>& next);

			std::shared_ptr<Block> get_prev_unsafe();
			std::shared_ptr<Block> get_next_unsafe();

			std::shared_ptr<Block> get_prev_safe();
			std::shared_ptr<Block> get_next_safe();

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

			// cf. http://bit.ly/VTIjF0
			std::atomic<bool> prev_lock;
			std::atomic<bool> next_lock;

			std::shared_ptr<Block> prev;
			std::shared_ptr<Block> next;

			std::string data;
	};
}

#endif
