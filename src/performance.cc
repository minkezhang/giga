#include <algorithm>
#include <iomanip>
#include <ostream>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

#include <iostream>

#include "libs/exceptionpp/exception.h"

#include "src/performance.h"

bool giga::Result::is_dup(char l, char r) { return(((l == ' ') || (l == '\n')) && (l == r)); }

giga::Result::Result() : size(0) {}

void giga::Result::push_back(std::string tag, size_t n_transactions, double total_runtime, size_t total_data, size_t file_size, double data_size, size_t cache_size, size_t page_size, size_t n_clients) {
	this->tag.push_back(tag);
	this->n_transactions.push_back(n_transactions);
	this->total_runtime.push_back(total_runtime);
	this->total_data.push_back(total_data);
	this->file_size.push_back(file_size);
	this->data_size.push_back(data_size);
	this->cache_size.push_back(cache_size);
	this->page_size.push_back(page_size);
	this->n_clients.push_back(n_clients);
	this->size++;
}

size_t giga::Result::get_size() { return(this->size); }

std::string giga::Result::get_tag(size_t index) { return(this->tag.at(index)); }
double giga::Result::get_latency(size_t index) { return(this->total_runtime.at(index) / this->n_transactions.at(index)); }
double giga::Result::get_throughput(size_t index) { return(this->total_data.at(index) / this->total_runtime.at(index)); }
size_t giga::Result::get_file_size(size_t index) { return(this->file_size.at(index)); }
double giga::Result::get_data_size(size_t index) { return(this->data_size.at(index)); }
size_t giga::Result::get_cache_size(size_t index) { return(this->cache_size.at(index)); }
size_t giga::Result::get_page_size(size_t index) { return(this->page_size.at(index)); }
size_t giga::Result::get_n_clients(size_t index) { return(this->n_clients.at(index)); }

std::string giga::Result::to_string(bool is_tsv) {
	size_t pad = 11;
	std::string sep = " | ";

	if(this->get_size() == 0) {
		throw(exceptionpp::InvalidOperation("giga::Result::to_string", "reporting zero trials"));
	}

	std::stringstream buffer;
	buffer << std::setw(pad) << "trial" << sep << std::setw(3) << "tag" << sep << std::setw(pad) << "tput (B/us)" << sep << std::setw(pad) << "lat (B)" << sep << std::setw(pad) << "file (B)" << sep << std::setw(pad) << "data (B)" << sep << std::setw(pad) << "cache" << sep << std::setw(pad) << "page (B)" << sep << std::setw(pad) << "n_clients" << std::endl;
	buffer << std::string(buffer.str().length(), '=') << std::endl;
	for(size_t index = 0; index < this->get_size(); ++index) {
		buffer << std::setprecision(2) << std::fixed;
		buffer << std::setw(pad) << index + 1 << sep << std::setw(3) << this->get_tag(index) << sep << std::setw(pad) << this->get_throughput(index) << sep << std::setw(pad) << this->get_latency(index) << sep << std::setw(pad) << this->get_file_size(index) << sep << std::setw(pad) << this->get_data_size(index) << sep << std::setw(pad) << this->get_cache_size(index) << sep << std::setw(pad) << this->get_page_size(index) << sep << std::setw(pad) << this->get_n_clients(index) << std::endl;
	}

	std::string ret = buffer.str();
	// format to tabs
	if(is_tsv) {
		std::replace(ret.begin(), ret.end(), '|', '\t');
		// cf. http://bit.ly/1p6ATc7
		ret.erase(std::remove(ret.begin(), ret.end(), '='), ret.end());

		// cf. http://bit.ly/1ywrQSZ
		std::string::iterator end = std::unique(ret.begin(), ret.end(), giga::Result::is_dup);
		ret.erase(end, ret.end());

		// cf. http://bit.ly/1oN9U5R
		std::stringstream s;
		s << std::endl << "\t";
		size_t pos = 0;
		while((pos = ret.find(s.str())) != std::string::npos) {
			ret.erase(pos + 1, 1);
		}
		pos = 0;
		while((pos = ret.find(" \t ")) != std::string::npos) {
			ret.erase(pos + 2, 1);
			ret.erase(pos, 1);
		}

		ret = ret.substr(1);
	}
	return(ret);
}

std::ostream& operator<< (std::ostream& os, giga::Result& obj) {
	os << obj.to_string(false);
	return(os);
}

giga::Performance::Performance() : result(giga::Result()) {}
void giga::Performance::set_file(std::shared_ptr<giga::File> file) { this->file = file; }
giga::Result giga::Performance::get_result() { return(this->result); }

void giga::Performance::run(std::vector<size_t> access_pattern, std::vector<uint8_t> type, std::vector<size_t> data_size, size_t n_clients) {
	auto f = this->file.lock();
	if(f == NULL) {
		throw(exceptionpp::InvalidOperation("giga::Performance::run", "file not set"));
	}
	if(n_clients == 0) {
		throw(exceptionpp::InvalidOperation("giga::Performance::run", "n_clients must be non-zero"));
	}
	if((access_pattern.size() != type.size()) || (type.size() != data_size.size())) {
		throw(exceptionpp::InvalidOperation("giga::Performance::run", "invalid vector sizes"));
	}

	std::vector<std::thread> threads;
	std::shared_ptr<std::atomic<double>> runtime (new std::atomic<double> (0));
	for(size_t i = 0; i < n_clients; ++i) {
		threads.push_back(std::thread(&giga::Performance::aux_run, this, runtime, f->open(), access_pattern, type, data_size));
	}
	for(size_t i = 0; i < n_clients; ++i) {
		threads.at(i).join();
	}
	// this->result.push_back(...);
}

void giga::Performance::aux_run(const std::shared_ptr<std::atomic<double>>& runtime, const std::shared_ptr<giga::Client>& client, std::vector<size_t> access_pattern, std::vector<uint8_t> type, std::vector<size_t> data_size) {
	for(size_t i = 0; i < access_pattern.size(); ++i) {
		std::cout << "  giga::Performance::aux_run -- index " << i << std::endl;
		switch(type.at(i)) {
			case giga::Performance::R:
				break;
			case giga::Performance::W:
				break;
			case giga::Performance::I:
				break;
			case giga::Performance::E:
				break;
			default:
				throw(exceptionpp::InvalidOperation("giga::Performance::aux_run", "invalid type parameter"));
		}
	}
}
