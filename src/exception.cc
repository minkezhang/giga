#include <sstream>

#include <iostream>

#include "libs/stacktrace/call_stack.hpp"
#include "libs/stacktrace/stack_exception.hpp"

#include "src/exception.h"

giga::InvalidOperation::InvalidOperation(std::string func_name, std::string msg) {
	this->func_name = func_name;
	this->msg = msg;
}
const char *giga::InvalidOperation::what() const throw() {
	std::stringstream buffer;
	buffer << "InvalidOperation(" << this->func_name << "): Invalid operation on " << this->func_name << "() -- " << this->msg;
	return(buffer.str().c_str());
}

giga::RuntimeError::RuntimeError(std::string func_name, std::string msg) {
	this->func_name = func_name;
	this->msg = msg;
}
const char *giga::RuntimeError::what() const throw() {
	std::stringstream buffer;
	buffer << "RuntimeError(" << this->func_name << "): " << this->func_name << "() -- " << this->msg;
	return(buffer.str().c_str());
}

giga::NotImplemented::NotImplemented(std::string func_name) {
	this->func_name = func_name;
}
const char *giga::NotImplemented::what() const throw() {
	std::stringstream buffer;
	buffer << "NotImplemented(" << this->func_name << "): Method not implemented " << this->func_name << "()";
	return(buffer.str().c_str());
}

giga::FileNotFound::FileNotFound(std::string filename) {
	this->filename = filename;
}
const char *giga::FileNotFound::what() const throw() {
	std::stringstream buffer;
	buffer << "FileNotFound(" << this->filename << "): Cannot find file " << this->filename;
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
