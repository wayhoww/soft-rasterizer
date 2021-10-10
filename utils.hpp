#pragma once
#include <string>
#include <vector>

std::vector<std::string> split(const std::string& line, char c) {
	std::vector<std::string> components;
	size_t pos = 0;
	do {
		size_t npos = line.find(c, pos);
		if(npos != std::string::npos) {
			auto s = line.substr(pos, npos - pos);
			if (!s.empty()) {
				components.push_back(s);
			}
		} else {
			auto s = line.substr(pos);
			if (!s.empty()) {
				components.push_back(s);
			}
			break;
		}
		pos = npos + 1;
	} while (true);
	return components;
}

std::string trimmed(const std::string& str) {
	std::string out;
	bool first_non_empty_occurred = false;
	for(int i = 0; i < str.size(); i++) {
		if(str[i] != ' ') first_non_empty_occurred = true;
		if(first_non_empty_occurred) {
			out.push_back(str[i]);
		}
	}
	first_non_empty_occurred = false;
	int i;
	for(i = out.size() - 1; i >= 0; i--) {
		if(out[i] != ' ') {
			break;
		}
	}
	out.reserve(i + 1);
	return out;
}