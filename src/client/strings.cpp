#include "strings.h"

#include <string>
#include <sstream>
#include <algorithm> 
#include <functional> 
#include <cctype>
#include <locale>
#include <map>
#include <vector>
#include <cstdlib>

static std::map<std::string, std::vector<std::string> > locales;

std::string STRING(const char *ID) {
	try {
		auto &strs = locales.at(ID);
		size_t num = rand() % strs.size();
		return strs.at(num);
	} catch (std::out_of_range&) {
		return ID;
	}
}

std::string STRING(const char *ID, const std::initializer_list<std::string> &args) {
	std::string str = STRING(ID);
	if (str.empty()) return str;

	int i=0;
	for (auto to : args) {
		std::string from = '{' + std::to_string(i) + '}';

		size_t start_pos = 0;
		while((start_pos = str.find(from, start_pos)) != std::string::npos) {
			str.replace(start_pos, from.length(), to);
			start_pos += to.length();
		}
		++i;
	}

	return str;
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

		locales[str_id].push_back(str_locale);
	}

	return true;
}