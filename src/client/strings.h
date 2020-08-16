#ifndef __STRINGS_H__
#define __STRINGS_H__

#include <string>
#include <sstream>
#include <initializer_list>
#include <utility>

inline std::string toString(const std::string &str) {
	return str;
}

inline std::string toString(const char *str) {
	return str;
}

template<typename T> std::string toString(T obj) {
	std::ostringstream oss;
	oss << obj;
	return oss.str();
}

std::string STRING(const char *ID);

std::string STRING(const char *ID, const std::initializer_list<std::string> &args);

template<typename ... T> std::string STRING(const char *ID, T ... args) {
	return STRING(ID, {toString(args) ...});
}

bool load_locale(const std::string &locale);

#endif // __STRINGS_H__
