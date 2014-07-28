#ifndef _BLOCK_H
#define _BLOCK_H

#include <atomic>
#include <cstdlib>
#include <memory>
#include <mutex>

#include "src/file.h"
#include "src/global.h"
#include "src/info.h"

/**
 * Block::Block is the memory management module of the library
 *
 * these functions MUST NOT be called by the user
 */
namespace giga {
	class File;
	class ClientInfo;
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

			bool get_is_loaded();
			bool get_is_dirty();

			giga_size read(giga_size start, const std::shared_ptr<std::string>& buffer, giga_size n_bytes);
			giga_size append(const std::shared_ptr<std::string>& buffer);
			giga_size write(giga_size start, const std::shared_ptr<std::string>& buffer, bool is_insert);
			giga_size erase(giga_size start, size_t len);

			void insert(const std::shared_ptr<Block>& head, const std::shared_ptr<Block>& tail);

			void lock_data();
			void unlock_data();

			void enqueue(int client_id, const std::shared_ptr<ClientInfo>& client_info);
			void dequeue(int client_id, const std::shared_ptr<ClientInfo>& client_info);
			bool at(int client_id);

		private:
			giga_size id;
			giga_size global_offset;
			size_t size;

			bool is_dirty;
			bool is_loaded;

			// cf. http://bit.ly/VTIjF0
			std::atomic<bool> prev_lock;
			std::atomic<bool> next_lock;
			std::atomic<bool> data_lock;

			std::shared_ptr<Block> prev;
			std::shared_ptr<Block> next;

			std::string data;
			std::string checksum;

			std::atomic<bool> queue_lock;
			std::map<int, std::shared_ptr<ClientInfo>> queue;

			void lock_prev();
			void lock_next();
			void lock_queue();

			void unlock_prev();
			void unlock_next();
			void unlock_queue();
	};
}

#endif
