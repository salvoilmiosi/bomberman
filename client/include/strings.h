#ifndef __STRINGS_H__
#define __STRINGS_H__

#include <string>
#include <sstream>
#include <initializer_list>
#include <utility>

std::string STRING(const char *ID);

std::string STRING(const char *ID, std::initializer_list<std::string> args);

std::string STRING_NO(const char *ID, int num);

std::string STRING_NO(const char *ID, int num, std::initializer_list<std::string> args);

int STRING_COUNT(const char *ID);

template<typename T> std::string toString(T obj) {
	std::ostringstream oss;
	oss << obj;
	return oss.str();
}

template<typename ... T> std::string STRING_NO(const char *ID, int num, T ... args) {
	return STRING_NO(ID, num, {toString(args) ...});
}

template<typename ... T> std::string STRING(const char *ID, T ... args) {
	return STRING_NO(ID, 0, args ...);
}

bool load_locale(const std::string &locale);

#endif // __STRINGS_H__
