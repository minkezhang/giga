#ifndef _INFO_H
#define _INFO_H

#include <memory>

#include "src/client.h"

#include "src/global.h"

namespace giga {
	class Client;
	class ClientInfo {
		public:
			ClientInfo(const std::shared_ptr<Client>& client);

			const std::shared_ptr<Client>& get_client();

			giga_size get_block_offset();
			giga_size get_global_position();

		private:
			std::shared_ptr<Client> client;

			// + <BLOCK>

			giga_size block_offset;
			giga_size global_position;
	};
}

#endif
