#ifndef _EXCEPTION_H
#define _EXCEPTION_H

#include <exception>
#include <string>

/**
 * the exceptions listed here MAY be called by the user in try / catch blocks
 *
 * the author does not see a reason for the user to throw these errors manually
 */
namespace giga {
	class BaseError : public std::exception {
		public:
			virtual const char *what() const throw();
		protected:
			std::string buffer;
	};

	class NotImplemented : public BaseError {
		public:
			NotImplemented(std::string func_name);
		protected:
			std::string func_name;
	};

	/**
	 * general error class
	 */
	class InvalidOperation : public BaseError {
		public:
			InvalidOperation(std::string func_name, std::string msg);
		protected:
			std::string func_name;
			std::string msg;
	};

	class RuntimeError : public BaseError {
		public:
			RuntimeError(std::string func_name, std::string msg);
		protected:
			std::string func_name;
			std::string msg;
	};

	class FileNotFound : public BaseError {
		public:
			FileNotFound(std::string filename);
		protected:
			std::string filename;
	};

	class InvalidFileOperation : public BaseError {
		public:
			InvalidFileOperation(std::string filename, std::string mode, int error_type);
		protected:
			std::string filename;
			std::string mode;
			int error_type;
	};
}

#endif
