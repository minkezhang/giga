#include "src/client.h"

giga::Client::Client(cachepp::identifier id, const std::shared_ptr<File>& file) : id(id), file(file), is_closed(0) {
	this->l = std::shared_ptr<std::recursive_mutex> (new std::recursive_mutex);
}

bool giga::Client::get_is_closed() {
	std::lock_guard<std::recursive_mutex> l(*this->l);
	return(this->is_closed);
}

void giga::Client::open() {
	std::lock_guard<std::recursive_mutex> l(*this->l);
	this->set_is_closed(false);
}

void giga::Client::close() {
	std::lock_guard<std::recursive_mutex> l(*this->l);
	this->set_is_closed(true);
}

void giga::Client::set_is_closed(bool is_closed) {
	std::lock_guard<std::recursive_mutex> l(*this->l);
	if(this->get_is_closed() == is_closed) { return; }
	if(is_closed) {
	} else {
	}
	this->is_closed = is_closed;
}


giga::ClientData::ClientData() {}
