#ifndef _EXCEPTION_H
#define _EXCEPTION_H

#include <exception>
#include <string>

namespace giga {
	class NotImplemented : public std::exception {
		public:
			NotImplemented(std::string func_name);
			virtual const char *what() const throw();
		protected:
			std::string func_name;
			std::string buffer;
	};

	/**
	 * general error class
	 */
	class InvalidOperation : public std::exception {
		public:
			InvalidOperation(std::string func_name, std::string msg);
			virtual const char *what() const throw();
		protected:
			std::string func_name;
			std::string msg;
			std::string buffer;
	};

	class RuntimeError : public std::exception {
		public:
			RuntimeError(std::string func_name, std::string msg);
			virtual const char *what() const throw();
		protected:
			std::string func_name;
			std::string msg;
			std::string buffer;
	};

	class FileNotFound : public std::exception {
		public:
			FileNotFound(std::string filename);
			virtual const char *what() const throw();
		protected:
			std::string filename;
			std::string buffer;
	};

	class InvalidFileOperation : public std::exception {
		public:
			InvalidFileOperation(std::string filename, std::string mode, int error_type);
			virtual const char *what() const throw();
		protected:
			std::string filename;
			std::string mode;
			int error_type;
			std::string buffer;
	};
}

#endif
