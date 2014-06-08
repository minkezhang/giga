#include <memory>

#include "src/client.h"

giga::Client::Client(const std::shared_ptr<giga::File>& file, int id) {
	this->file = file;
	this->id = id;
}

int giga::Client::get_id() { return(this->id); }

giga::giga_size giga::Client::read(const std::shared_ptr<std::string>& buffer, giga::giga_size n_bytes) {
	return(this->file->read(this->shared_from_this(), buffer, n_bytes));
}

giga::giga_size giga::Client::get_pos() {
	return(this->file->get_client_pos(this->shared_from_this())); }

giga::giga_size giga::Client::seek(giga::giga_size global_pos) {
	return(this->file->seek(this->shared_from_this(), global_pos));
}

void giga::Client::lock_client() { this->client_lock.lock(); }
void giga::Client::unlock_client() { this->client_lock.unlock(); }
