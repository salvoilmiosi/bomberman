#include "string.h"

#include <string>
#include <sstream>
#include <algorithm> 
#include <functional> 
#include <cctype>
#include <locale>
#include <map>

static std::map<std::string, std::string> locales;

const char *STRING(const char *ID) {
	try {
		return locales.at(ID).c_str();
	} catch (std::out_of_range) {
		return ID;
	}
}

static void trim(std::string &s) {
	s.erase(s.begin(), std::find_if(s.begin(), s.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
	s.erase(std::find_if(s.rbegin(), s.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
}

bool load_locale(const std::string &locale) {
	std::istringstream iss(locale);

	std::string line;

	while (std::getline(iss, line)) {
		trim(line);
		
		if (line.empty()) continue;
		if (line.at(0) == '#') continue;

		std::istringstream line_ss(line);

		std::string str_id;
		std::string str_locale;

		line_ss >> str_id;

		std::getline(line_ss, str_locale);
		trim(str_locale);

		if (str_locale.empty()) continue;

		locales[str_id] = str_locale;
	}

	return true;
}