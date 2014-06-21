#include "libs/md5/md5.h"

#include "src/crypto.h"

std::string giga::Crypto::hash(std::string s) { return(md5(s)); }

