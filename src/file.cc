#include <memory>
#include <vector>

#include "src/file.h"

giga::File::File(std::string filename) : filename(filename) {
	this->n_clients = 0;
}

giga::giga_size giga::File::get_client_pos(const std::shared_ptr<giga::Client>& client) {
	return(this->client_list.at(0)->get_global_position());
}

giga::giga_size giga::File::seek(const std::shared_ptr<giga::Client>& client, giga_size global_pos) { return(0); }

giga::giga_size giga::File::read(const std::shared_ptr<giga::Client>& client, std::string buffer, giga::giga_size n_bytes) {
	buffer.assign("test string");
	return(buffer.length());
}

giga::giga_size giga::File::write(const std::shared_ptr<giga::Client>& client, std::string buffer) {
	return(buffer.length());
}

std::shared_ptr<giga::Client> giga::File::open() {
	std::shared_ptr<giga::Client> c (new giga::Client(this->shared_from_this(), this->n_clients++));
	std::shared_ptr<giga::ClientInfo> c_info (new giga::ClientInfo(c));
	this->client_list.push_back(c_info);
	return(c);
}

void giga::File::close(const std::shared_ptr<giga::Client>& client) {}

void giga::File::save() {}
