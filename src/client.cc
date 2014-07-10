#include <memory>

#include "src/info.h"

#include "src/client.h"

giga::Client::Client(const std::shared_ptr<giga::File>& file, const std::shared_ptr<giga::ClientInfo>& client_info, const::std::shared_ptr<giga::Client>& next, int id) {
	this->file = file;
	this->client_info = client_info;
	this->next = next;
	this->id = id;
	this->is_closed = false;
}

std::shared_ptr<giga::ClientInfo> giga::Client::get_client_info() { return(this->client_info); }
bool giga::Client::get_is_closed() { return(this->is_closed); }
void giga::Client::set_is_closed() { this->is_closed = true; }

int giga::Client::get_id() { return(this->id); }

giga::giga_size giga::Client::read(const std::shared_ptr<std::string>& buffer, giga::giga_size n_bytes) {
	return(this->file->read(this->shared_from_this(), buffer, n_bytes));
}

giga::giga_size giga::Client::get_pos() {
	return(this->file->get_client_pos(this->shared_from_this()));
}

giga::giga_size giga::Client::seek(giga::giga_size global_pos) {
	return(this->file->seek(this->shared_from_this(), global_pos));
}

void giga::Client::lock_client() { this->client_lock.lock(); }
void giga::Client::unlock_client() { this->client_lock.unlock(); }

std::shared_ptr<giga::Client> giga::Client::get_next_unsafe() { return(this->next); }
void giga::Client::set_next_unsafe(const std::shared_ptr<giga::Client>& next) { this->next = next; }

/**
 * insert the list defined by head into the current list
 *	we assume the new list is fresh and do not need to be locked
 */
void giga::Client::insert(const std::shared_ptr<giga::Client>& head) {
	this->lock_client();
	std::shared_ptr<giga::Client> tail = head;
	while(tail->get_next_unsafe() != NULL) {
		tail = tail->get_next_unsafe();
	}
	tail->set_next_unsafe(this->next);
	this->set_next_unsafe(head);
	tail.reset();
	this->unlock_client();
}

/**
 * cf. Herlihy and Shavit, "Fine-Grained Synchronization" for reference implementation
 */
void giga::Client::erase(const std::shared_ptr<giga::Client>& target) {
	if(this->get_id() == target->get_id()) {
		this->lock_client();
		this->set_is_closed();
		this->unlock_client();
		return;
	}

	bool success = false;

	std::shared_ptr<giga::Client> pred = this->shared_from_this();
	std::shared_ptr<giga::Client> curr = NULL;
	while(!success) {
		pred->lock_client();
		curr = pred->get_next_unsafe();
		if(curr == NULL) {
			break;
		}
		curr->lock_client();
		if(curr->get_id() == target->get_id()) {
			pred->set_next_unsafe(target->get_next_unsafe());
			target->set_is_closed();
			success = true;
		}
		pred->unlock_client();
		pred = curr;
		curr->unlock_client();
	}
	if(!success) {
		throw(giga::InvalidOperation("giga::Client::erase", "cannot find client in client list"));
	}
}
