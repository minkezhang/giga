#include "src/block.h"

giga::Block::Block(giga::giga_size global_offset, giga::giga_size size, const std::shared_ptr<giga::Block>& prev, const std::shared_ptr<giga::Block>& next) {
	this->global_offset = global_offset;
	this->size = size;
	this->prev = prev;
	this->next = next;
}

void giga::Block::load() {
	this->is_loaded = 1;
}

void giga::Block::unload() {
	this->is_loaded = 0;
}

giga::giga_size giga::Block::read(std::string buffer, giga::giga_size n_bytes) {
	buffer.assign("...");
	return(3);
}

giga::giga_size giga::Block::write(std::string buffer) {
	this->is_dirty = 1;
	return(buffer.length());
}

giga::giga_size giga::Block::get_size() { return(this->size); }

std::shared_ptr<giga::Block> giga::Block::get_prev() { return(this->prev); }
std::shared_ptr<giga::Block> giga::Block::get_next() { return(this->next); }

void giga::Block::set_prev(const std::shared_ptr<giga::Block>& prev) { this->prev = prev; }
void giga::Block::set_next(const std::shared_ptr<giga::Block>& next) { this->next = next; }
