#include <memory>

#include "src/block.h"
#include "src/client.h"

#include "src/info.h"

giga::ClientInfo::ClientInfo(const std::shared_ptr<giga::Client>& client, const std::shared_ptr<giga::Block>& block) {
	this->client = client;
	this->block = block;
	this->block_offset = 0;
}

std::shared_ptr<giga::Block> giga::ClientInfo::get_block() { return(this->block); }

const std::shared_ptr<giga::Client>& giga::ClientInfo::get_client() { return(this->client); }

giga::giga_size giga::ClientInfo::get_block_offset() { return(this->block_offset); }
giga::giga_size giga::ClientInfo::get_global_position() {
	std::shared_ptr<giga::Block> block = this->block;

	if(block == NULL) { return(0); }
	while(block->get_prev() != NULL) { block = block->get_prev(); }
	int global_pos = 0;
	while(block != this->block->get_next()) {
		global_pos += block->get_size();
	}
	return(global_pos + this->block_offset);
}

/**
 * lock self, lock block
 * n_bytes as if we're dealing with this->block_offset = 0
 */
void giga::ClientInfo::set_block_offset(giga::giga_size n_bytes) {
	// set block if necessary
	if(n_bytes > 0) {
		/**
		while((this->block_offset + n_bytes) > this->block->get_size()) {
			n_bytes -= (this->block->get_size() - this->block_offset);
			this->block = this->block->get_next();
			this->block_offset = 0;
		}
		this->block_offset = n_bytes;
		*/
	} else {
		/**
		while(n_bytes > this->block->get_size()) {
			n_bytes -= (this->block->get_size() - this->block_offset);
			this->block = this->block->get_next();
			this->block_offset = this->block->get_size();
		// ...
		}
		*/
	}
}
