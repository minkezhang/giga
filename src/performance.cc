#include <algorithm>
#include <deque>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <iomanip>
#include <ostream>
#include <sstream>
#include <string>
#include <sys/types.h>
#include <sys/sysinfo.h>
#include <thread>
#include <vector>

#include <iostream>

#include "libs/exceptionpp/exception.h"

#include "src/performance.h"

#define USEC 1000000

bool giga::Result::is_dup(char l, char r) { return(((l == ' ') || (l == '\n')) && (l == r)); }

giga::Result::Result() : size(0), i_count(0) {}

void giga::Result::push_back(std::string tag, size_t n_transactions, double total_runtime, size_t total_data, size_t read, size_t write, size_t insert, size_t erase, size_t file_size, size_t cache_size, size_t init_size, size_t max_size, double miss_rate, size_t n_clients) {
	this->index.push_back(i_count++);
	this->tag.push_back(tag);
	this->n_transactions.push_back(n_transactions);
	this->total_runtime.push_back(total_runtime);
	this->total_data.push_back(total_data);
	this->read.push_back(read);
	this->write.push_back(write);
	this->insert.push_back(insert);
	this->erase.push_back(erase);
	this->file_size.push_back(file_size);
	this->cache_size.push_back(cache_size);
	this->init_size.push_back(init_size);
	this->max_size.push_back(max_size);
	this->miss_rate.push_back(miss_rate);
	this->n_clients.push_back(n_clients);
	this->size++;
}

size_t giga::Result::get_size() { return(this->size); }

std::string giga::Result::get_tag(size_t index) { return(this->tag.at(index)); }
double giga::Result::get_latency(size_t index) { return(this->total_runtime.at(index) / this->n_transactions.at(index)); }
double giga::Result::get_throughput(size_t index) { return(this->total_data.at(index) / this->total_runtime.at(index)); }
size_t giga::Result::get_file_size(size_t index) { return(this->file_size.at(index)); }
double giga::Result::get_data_size(size_t index) { return((double) this->total_data.at(index) / this->n_transactions.at(index)); }
size_t giga::Result::get_cache_size(size_t index) { return(this->cache_size.at(index)); }
size_t giga::Result::get_init_size(size_t index) { return(this->init_size.at(index)); }
size_t giga::Result::get_max_size(size_t index) { return(this->max_size.at(index)); }
size_t giga::Result::get_n_clients(size_t index) { return(this->n_clients.at(index)); }
double giga::Result::get_read(size_t index) { return((this->read.at(index) / this->n_transactions.at(index)) * 100); }
double giga::Result::get_write(size_t index) { return((this->write.at(index) / this->n_transactions.at(index)) * 100); }
double giga::Result::get_insert(size_t index) { return((this->insert.at(index) / this->n_transactions.at(index)) * 100); }
double giga::Result::get_miss_rate(size_t index) { return(this->miss_rate.at(index) * 100); }
double giga::Result::get_erase(size_t index) { return((this->erase.at(index) / this->n_transactions.at(index)) * 100); }

std::string giga::Result::pop_front(bool is_tsv, bool include_header) {
	size_t large_pad = 11;
	size_t med_pad = 6;
	size_t small_pad = 3;
	std::string sep = " | ";

	if(this->get_size() == 0) {
		throw(exceptionpp::InvalidOperation("giga::Result::to_string", "reporting zero trials"));
	}

	std::stringstream buffer;

	if(include_header) {
		buffer << std::setw(large_pad) << "trial" << sep << std::setw(small_pad) << "tag" << sep << std::setw(med_pad) << "R (%)" << sep << std::setw(med_pad) << "W (%)" << sep << std::setw(med_pad) << "I (%)" << sep << std::setw(med_pad) << "E (%)" << sep << std::setw(large_pad) << "tput (B/us)" << sep << std::setw(large_pad) << "lat (us)" << sep << std::setw(large_pad) << "file (B)" << sep << std::setw(large_pad) << "data (B)" << sep << std::setw(large_pad) << "cache" << sep << std::setw(large_pad) << "init (B)" << sep << std::setw(large_pad) << "max (B)" << sep << std::setw(small_pad) << "N" << sep << std::setw(large_pad) << "miss (%)" << std::endl;
		buffer << std::string(buffer.str().length(), '=') << std::endl;
	}

	buffer << std::setprecision(2) << std::fixed;
	buffer << std::setw(large_pad) << this->index.at(0) + 1 << sep << std::setw(small_pad) << this->get_tag(0) << sep << std::setw(med_pad) << this->get_read(0) << sep << std::setw(med_pad) << this->get_write(0) << sep << std::setw(med_pad) << this->get_insert(0) << sep << std::setw(med_pad) << this->get_erase(0) << sep << std::setw(large_pad) << this->get_throughput(0) << sep << std::setw(large_pad) << this->get_latency(0) << sep << std::setw(large_pad) << this->get_file_size(0) << sep << std::setw(large_pad) << this->get_data_size(0) << sep << std::setw(large_pad) << this->get_cache_size(0) << sep << std::setw(large_pad) << this->get_init_size(0) << sep << std::setw(large_pad) << this->get_max_size(0) << sep << std::setw(small_pad) << this->get_n_clients(0) << sep << std::setw(large_pad) << this->get_miss_rate(0) << std::endl;

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

	this->index.pop_front();
	this->tag.pop_front();
	this->n_transactions.pop_front();
	this->total_runtime.pop_front();
	this->total_data.pop_front();
	this->read.pop_front();
	this->write.pop_front();
	this->insert.pop_front();
	this->erase.pop_front();
	this->file_size.pop_front();
	this->cache_size.pop_front();
	this->init_size.pop_front();
	this->max_size.pop_front();
	this->miss_rate.pop_front();
	this->n_clients.pop_front();
	this->size--;

	return(ret);
}

std::string giga::Result::to_string(bool is_tsv) {
	size_t size = this->get_size();

	if(this->get_size() == 0) {
		throw(exceptionpp::InvalidOperation("giga::Result::to_string", "reporting zero trials"));
	}

	std::stringstream buf;
	for(size_t i = 0; i < size; ++i) {
		buf << this->pop_front(is_tsv, i == 0);
	}
	return(buf.str());
}

std::ostream& operator<< (std::ostream& os, giga::Result& obj) {
	os << obj.to_string(false);
	return(os);
}

giga::Performance::Performance() : result(std::shared_ptr<giga::Result> (new giga::Result())) {}
void giga::Performance::set_file(std::shared_ptr<giga::File> file) { this->file = file; }
std::shared_ptr<giga::Result> giga::Performance::get_result() { return(this->result); }

size_t giga::Performance::parse_line(char *line) {
	int i = strlen(line);
	while(*line < '0' || *line > '9') { line++; }
	line[i - 3] = '\0';
	i = (size_t) atoi(line);
	return(i);
}

size_t giga::Performance::get_usage() {
	FILE *fp = fopen("/proc/self/status", "r");
	size_t result = 0;
	char line[128];

	while(fgets(line, 128, fp) != NULL) {
		if(strncmp(line, "VmRSS:", 6) == 0) {
			result = parse_line(line);
			break;
		}
	}

	fclose(fp);
	fp = NULL;

	return(result * 1024);
}

void giga::Performance::run(std::string tag, std::vector<size_t> access_pattern, std::vector<uint8_t> type, std::vector<size_t> data_size, size_t n_clients, size_t n_attempts) {
	auto f = this->file.lock();
	if(f == NULL) {
		throw(exceptionpp::InvalidOperation("giga::Performance::run", "file not set"));
	}
	if(n_clients == 0) {
		throw(exceptionpp::InvalidOperation("giga::Performance::run", "n_clients must be non-zero"));
	}
	if(n_attempts == 0) {
		throw(exceptionpp::InvalidOperation("giga::Performance::run", "n_attempts must be non-zero"));
	}
	if((access_pattern.size() != type.size()) || (type.size() != data_size.size())) {
		throw(exceptionpp::InvalidOperation("giga::Performance::run", "invalid vector sizes"));
	}
	if(tag.length() > 3) {
		throw(exceptionpp::InvalidOperation("giga::Performance::run", "tag must be at most three characters"));
	}

	std::vector<size_t> type_tracker = std::vector<size_t> (4, 0);
	for(size_t i = 0; i < type.size(); ++i) {
		if(type.at(i) > giga::Performance::E) {
			throw(exceptionpp::InvalidOperation("giga::Performance::aux", "invalid type parameter"));
		}
		type_tracker.at(type.at(i)) += n_attempts * n_clients;
	}

	std::vector<std::thread> threads;
	std::shared_ptr<std::atomic<double>> runtime (new std::atomic<double> (0));
	std::shared_ptr<std::atomic<size_t>> data (new std::atomic<size_t> (0));
	size_t f_size;

	for(size_t attempt = 0; attempt < n_attempts; ++attempt) {
		threads.clear();
		f->load();
		f_size = f->get_size();
		for(size_t i = 0; i < n_clients; ++i) {
			threads.push_back(std::thread(&giga::Performance::aux_run, this, runtime, data, f->open(), access_pattern, type, data_size));
		}
		for(size_t i = 0; i < n_clients; ++i) {
			threads.at(i).join();
		}
	}

	this->result->push_back(tag, access_pattern.size() * n_attempts * n_clients, *runtime, *data, type_tracker.at(0), type_tracker.at(1), type_tracker.at(2), type_tracker.at(3), f_size, f->get_config().get_cache_size(), f->get_config().get_i_page_size(), f->get_config().get_m_page_size(), f->get_miss_rate(), n_clients);
}

void giga::Performance::aux_run(const std::shared_ptr<std::atomic<double>>& runtime, const std::shared_ptr<std::atomic<size_t>>& data, const std::shared_ptr<giga::Client>& client, std::vector<size_t> access_pattern, std::vector<uint8_t> type, std::vector<size_t> data_size) {
	size_t local_data = 0;
	double local_runtime = 0;
	std::clock_t start;
	std::vector<uint8_t> buf;
	std::string str;
	size_t mem = 0;
	for(size_t i = 0; i < access_pattern.size(); ++i) {
		start = std::clock();
		client->seek(access_pattern.at(i), true, true);
		local_runtime += (std::clock() - start) / (double) (CLOCKS_PER_SEC / USEC);
		switch(type.at(i)) {
			case giga::Performance::R:
				start = std::clock();
				client->read(data_size.at(i));
				local_runtime += (std::clock() - start) / (double) (CLOCKS_PER_SEC / USEC);
				mem = giga::Performance::get_usage();
				break;
			case giga::Performance::W:
				buf = std::vector<uint8_t> (data_size.at(i), 0xff);
				start = std::clock();
				client->write(std::string(buf.begin(), buf.end()));
				local_runtime += (std::clock() - start) / (double) (CLOCKS_PER_SEC / USEC);
				break;
			case giga::Performance::I:
				buf = std::vector<uint8_t> (data_size.at(i), 0xff);
				start = std::clock();
				client->write(std::string(buf.begin(), buf.end()), true);
				local_runtime += (std::clock() - start) / (double) (CLOCKS_PER_SEC / USEC);
				break;
			case giga::Performance::E:
				start = std::clock();
				client->erase(data_size.at(i));
				local_runtime += (std::clock() - start) / (double) (CLOCKS_PER_SEC / USEC);
				break;
		}
		if(mem != giga::Performance::get_usage()) {
			std::cout << "changed mem on loop i: " << i << ", old: " << mem / 1024 << " vs. new: " << giga::Performance::get_usage() / 1024 << " kB" << std::endl;
		}
		local_data += data_size.at(i);
	}
	client->close();

	*data += local_data;

	double expected = runtime->load();
	double desired;
	do {
		desired = expected + local_runtime;
	} while(!runtime->compare_exchange_weak(expected, desired));
}
