#include <memory>

#include "src/client.h"

#include "src/info.h"

giga::ClientInfo::ClientInfo(const std::shared_ptr<giga::Client>& client) {
	this->client = client;
	this->block_offset = 0;
	this->global_position = 0;
}

const std::shared_ptr<giga::Client>& giga::ClientInfo::get_client() { return(this->client); }

giga::giga_size giga::ClientInfo::get_block_offset() { return(this->block_offset); }
giga::giga_size giga::ClientInfo::get_global_position() { return(this->global_position); }
