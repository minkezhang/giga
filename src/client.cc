#include <memory>

#include "src/client.h"

giga::Client::Client(const std::shared_ptr<giga::File>& file, int id) {
	this->file = file;
	this->id = id;
}

int giga::Client::get_id() { return(this->id); }

giga::giga_size giga::Client::get_pos() {
	return(this->file->get_client_pos(this->shared_from_this())); }

giga::giga_size giga::Client::seek(giga::giga_size global_pos) {
	return(0);
}
