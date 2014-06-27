#include <memory>
#include <sstream>

#include "src/block.h"
#include "src/client.h"
#include "src/exception.h"

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

	while(block != this->block) {
		global_pos += block->get_size();
		block = block->get_next();
	}

	return(global_pos + this->block_offset);
}

void giga::ClientInfo::set_block(const std::shared_ptr<giga::Block>& block) {
	this->block = block;
}

/**
 * lock self, lock block
 *
 * setting bytes directly
 * EOF signature -- block_offset == this->block->get_size()
 */
void giga::ClientInfo::set_block_offset(giga::giga_size block_offset) {
	std::stringstream buffer;
	buffer << "invalid offset position (block size " << this->block->get_size() << ", but received offset of " << block_offset << ")";

	if((block_offset > this->block->get_size()) || (block_offset < 0)) {
		throw(giga::InvalidOperation("giga::ClientInfo::set_block_offset", buffer.str()));
	}
	this->block_offset = block_offset;
}

giga::BlockInfo::BlockInfo(const std::shared_ptr<giga::Block>& block) {
	this->block = block;
	this->n_access = 0;
	this->lock = std::shared_ptr<std::mutex> (new std::mutex());
}

std::shared_ptr<giga::Block> giga::BlockInfo::get_block() { return(this->block); }
giga::giga_size giga::BlockInfo::get_n_access() { return(this->n_access); }
std::shared_ptr<std::mutex> giga::BlockInfo::get_lock() { return(this->lock); }

void giga::BlockInfo::increment() { this->n_access++; }
