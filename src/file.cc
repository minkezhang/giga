#include <memory>
#include <vector>

#include "src/file.h"

giga::File::File(std::string filename) : filename(filename) {
	this->n_clients = 0;
}

std::map<int, std::shared_ptr<giga::ClientInfo>> giga::File::get_client_list() { return(this->client_list); }

giga::giga_size giga::File::get_client_pos(const std::shared_ptr<giga::Client>& client) {
	return(this->client_list[client->get_id()]->get_global_position());
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
	this->client_list_lock.lock();
	this->client_list[c->get_id()] = c_info;
	this->client_list_lock.unlock();
	return(c);
}

void giga::File::close(const std::shared_ptr<giga::Client>& client) {
	this->client_list_lock.lock();
	this->client_list.erase(client->get_id());
	this->client_list_lock.unlock();
}

void giga::File::save() {}
