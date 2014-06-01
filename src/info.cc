#include <memory>

#include "src/block.h"
#include "src/client.h"

#include "src/info.h"

giga::ClientInfo::ClientInfo(const std::shared_ptr<giga::Client>& client, const std::shared_ptr<giga::Block>& block) {
	this->client = client;
	this->block = block;
	this->block_offset = 0;
	this->global_position = 0;
}

std::shared_ptr<giga::Block> giga::ClientInfo::get_block() { return(this->block); }

const std::shared_ptr<giga::Client>& giga::ClientInfo::get_client() { return(this->client); }

giga::giga_size giga::ClientInfo::get_block_offset() { return(this->block_offset); }
giga::giga_size giga::ClientInfo::get_global_position() { return(this->global_position); }

void giga::ClientInfo::set_block_offset(giga_size n_bytes) {
	// set block if necessary
	if(n_bytes > 0) {
		// ...
	} else {
		// ...
	}
}

void giga::ClientInfo::set_global_position(giga_size n_bytes) { this->global_position = n_bytes; }

