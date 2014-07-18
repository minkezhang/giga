#include <memory>
#include <sstream>

#include "src/block.h"
#include "src/exception.h"

#include "src/info.h"

giga::ClientInfo::ClientInfo(const std::shared_ptr<giga::Block>& block) {
	this->block = block;
	this->block_lock = false;
	this->block_offset = 0;
}

std::shared_ptr<giga::Block> giga::ClientInfo::get_block() {
	this->lock_block();
	std::shared_ptr<giga::Block> r = this->block;
	this->unlock_block();
	return(r);
}

giga::giga_size giga::ClientInfo::get_block_offset() { return(this->block_offset); }

giga::giga_size giga::ClientInfo::get_global_position() {
	std::shared_ptr<giga::Block> block = this->block;

	if(block == NULL) { return(0); }

	while(block->get_prev_safe() != NULL) { block = block->get_prev_safe(); }

	int global_pos = 0;

	while(block != this->get_block()) {
		global_pos += block->get_size();
		block = block->get_next_safe();
	}

	return(global_pos + this->block_offset);
}

void giga::ClientInfo::set_block(const std::shared_ptr<giga::Block>& block) {
	this->lock_block();
	this->block = block;
	this->unlock_block();
}

/**
 * lock self, lock block
 *
 * setting bytes directly
 * EOF signature -- block_offset == this->block->get_size()
 */
void giga::ClientInfo::set_block_offset(giga::giga_size block_offset) {
	std::stringstream buffer;

	if((block_offset > this->get_block()->get_size()) || (block_offset < 0)) {
		buffer << "invalid offset position (block size " << this->get_block()->get_size() << ", but received offset of " << block_offset << ")";
		throw(giga::InvalidOperation("giga::ClientInfo::set_block_offset", buffer.str()));
	}

	this->block_offset = block_offset;
}

void giga::ClientInfo::lock_block() { while(this->block_lock.exchange(true)) {} }
void giga::ClientInfo::unlock_block() { this->block_lock = false; }

giga::BlockInfo::BlockInfo(const std::shared_ptr<giga::Block>& block) {
	this->block = block;
	this->n_access = 0;
}

std::shared_ptr<giga::Block> giga::BlockInfo::get_block() { return(this->block); }
giga::giga_size giga::BlockInfo::get_n_access() { return(this->n_access); }

void giga::BlockInfo::increment() { this->n_access++; }
void giga::BlockInfo::decrement() { this->n_access--; }
