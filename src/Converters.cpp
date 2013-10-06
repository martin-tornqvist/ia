#include "Converters.h"

#include <string>
#include <sstream>
#include <iostream>

std::string toString(int in) {
	std::ostringstream buffer;
	buffer << in;
	return buffer.str();
}

int toInt(std::string in) {
	int _nr;
	std::istringstream buffer(in);
	buffer >> _nr;
	return _nr;
}

