#include "src/client.h"

giga::Client::Client(cachepp::identifier id, const std::shared_ptr<File>& file) : id(id), file(file), is_closed(0) {
	this->l = std::shared_ptr<std::recursive_mutex> (new std::recursive_mutex);
}

cachepp::identifier giga::Client::get_identifier() {
	std::lock_guard<std::recursive_mutex> l(*this->l);
	return(this->id);
}

size_t giga::Client::get_pos() {
	std::lock_guard<std::recursive_mutex> l(*this->l);
	if(this->get_is_closed()) { throw(exceptionpp::InvalidOperation("giga::Client::get_pos", "invalid operation on a closed client")); }
	return(this->file->s(this->shared_from_this(), 0, false));
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

size_t giga::Client::seek(size_t len, bool is_forward) {
	std::lock_guard<std::recursive_mutex> l(*this->l);
	if(this->file) {
		return(this->file->s(this->shared_from_this(), len, is_forward));
	}
	return(0);
}

void giga::Client::set_is_closed(bool is_closed) {
	std::lock_guard<std::recursive_mutex> l(*this->l);
	if(this->get_is_closed() == is_closed) { return; }
	if(this->file != NULL) {
		if(is_closed) {
			this->file->open(this->shared_from_this());
		} else {
			this->file->close(this->shared_from_this());
		}
	}
	this->is_closed = is_closed;
}

giga::ClientData::ClientData(cachepp::identifier id) : id(id) {}

void giga::ClientData::set_file_offset(size_t file_offset) { this->file_offset = file_offset; }
void giga::ClientData::set_page(std::list<std::shared_ptr<giga::Page>>::iterator page) { this->page = page; }
void giga::ClientData::set_page_offset(size_t page_offset) { this->page_offset = page_offset; }

size_t giga::ClientData::get_file_offset() { return(this->file_offset); }
std::list<std::shared_ptr<giga::Page>>::iterator giga::ClientData::get_page() { return(this->page); }
size_t giga::ClientData::get_page_offset() { return(this->page_offset); }
