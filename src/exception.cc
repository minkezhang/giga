#include <sstream>

#include "libs/stacktrace/call_stack.hpp"

#include "src/exception.h"

const char *giga::BaseError::what() const throw() { return(this->buffer.c_str()); }

giga::InvalidOperation::InvalidOperation(std::string func_name, std::string msg) {
	this->func_name = func_name;
	this->msg = msg;

	std::stringstream buffer;
	buffer << "InvalidOperation(" << this->func_name << "): Invalid operation on " << this->func_name << "() -- " << this->msg;
	this->buffer = buffer.str();
}

giga::RuntimeError::RuntimeError(std::string func_name, std::string msg) {
	this->func_name = func_name;
	this->msg = msg;

	stacktrace::call_stack st;

	std::stringstream buffer;
	buffer << "RuntimeError(" << this->func_name << "): " << this->func_name << "() -- " << this->msg << std::endl << st.to_string();
	this->buffer = buffer.str();
}

giga::NotImplemented::NotImplemented(std::string func_name) {
	this->func_name = func_name;

	std::stringstream buffer;
	buffer << "NotImplemented(" << this->func_name << "): Method not implemented " << this->func_name << "()";
	this->buffer = buffer.str();
}

giga::FileNotFound::FileNotFound(std::string filename) {
	this->filename = filename;

	std::stringstream buffer;
	buffer << "FileNotFound(" << this->filename << "): Cannot find file " << this->filename;
	this->buffer = buffer.str();
}

giga::InvalidFileOperation::InvalidFileOperation(std::string filename, std::string mode, int error_type) {
	this->filename = filename;
	this->mode = mode;
	this->error_type = error_type;

	std::stringstream buffer;
	buffer << "InvalidFileOperation(" << this->filename << ", " << this->mode << ", " << this->error_type << "): Invalid " << this->mode << " operation with ferror(fp) = " << this->error_type;
	this->buffer = buffer.str();
}
