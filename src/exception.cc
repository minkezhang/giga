#include <sstream>

#include "src/exception.h"

giga::FileNotFound::FileNotFound(std::string filename) {
	this->filename = filename;
}
const char *giga::FileNotFound::what() const throw() {
	std::stringstream buffer;
	buffer << "FileNotFound(" << this->filename << "): Cannot find file '" << this->filename << "'";
	return(buffer.str().c_str());
}

giga::InvalidFileOperation::InvalidFileOperation(std::string filename, std::string mode, int error_type) {
	this->filename = filename;
	this->mode = mode;
	this->error_type = error_type;
}
const char *giga::InvalidFileOperation::what() const throw() {
	std::stringstream buffer;
	buffer << "InvalidFileOperation(" << this->filename << ", " << this->mode << ", " << this->error_type << "): Invalid " << this->mode << " operation with ferror(fp) = " << this->error_type;
	return(buffer.str().c_str());
}
