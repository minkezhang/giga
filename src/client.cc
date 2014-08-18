#include "src/client.h"

giga::Client::Client(cachepp::identifier id, const std::shared_ptr<File>& file) : id(id), file(file), is_closed(0) {}

bool giga::Client::get_is_closed() { return(this->is_closed); }
void giga::Client::open() { this->set_is_closed(false); }
void giga::Client::close() { this->set_is_closed(true); }

void giga::Client::set_is_closed(bool is_closed) {
	if(this->get_is_closed() == is_closed) { return; }
	if(is_closed) {
	} else {
	}
	this->is_closed = is_closed;
}


giga::ClientData::ClientData() {}
