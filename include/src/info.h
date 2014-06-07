#ifndef _INFO_H
#define _INFO_H

#include <memory>

#include "src/block.h"
#include "src/client.h"
#include "src/global.h"

namespace giga {
	class Client;
	class ClientInfo {
		public:
			ClientInfo(const std::shared_ptr<Client>& client, const std::shared_ptr<Block>& block);

			const std::shared_ptr<Client>& get_client();

			std::shared_ptr<Block> get_block();

			giga_size get_block_offset();
			giga_size get_global_position();

			void set_block_offset(giga_size n_bytes);

		private:
			std::shared_ptr<Client> client;
			std::shared_ptr<Block> block;

			giga_size block_offset;
	};
}

#endif
