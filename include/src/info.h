#ifndef _INFO_H
#define _INFO_H

#include <atomic>
#include <memory>
#include <mutex>

#include "src/block.h"
#include "src/global.h"

/**
 * ClientInfo::ClientInfo and BlockInfo::BlockInfo are internal meta-data designed to ensure the smooth workings of the file instance
 *
 * these functions MUST NOT be called by the user
 */
namespace giga {
	class Block;

	/**
	 * wrapper around client and associated housekeeping info
	 */
	class ClientInfo : public std::enable_shared_from_this<ClientInfo> {
		public:
			ClientInfo();

			std::shared_ptr<Block> get_block();

			giga_size get_block_offset();
			giga_size get_global_position();

			void set_block(int client_id, const std::shared_ptr<Block>& block);
			void set_block_offset(giga_size n_bytes);

		private:
			std::atomic<giga_size> block_offset;

			std::atomic<bool> block_lock;
			std::shared_ptr<Block> block;
			void lock_block();
			void unlock_block();
	};

	/**
	 * wrapper around a block loaded in memory and associated housekeeping info
	 */
	class BlockInfo {
		public:
			BlockInfo(const std::shared_ptr<Block>& block);
			std::shared_ptr<Block> get_block();
			giga_size get_n_access();

			void increment();
			void decrement();

		private:
			std::shared_ptr<Block> block;
			giga_size n_access;
	};
}

#endif
