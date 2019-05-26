#include "dtcas3.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdio>

#include "cmdline.h" 

std::string autostakkert_extension = "as3";

bool starts_with(const std::string& s1, const std::string& s2) {
	return s2.size() <= s1.size() && s1.compare(0, s2.size(), s2) == 0;
}

bool replace(std::string& str, const std::string& from, const std::string& to) {
	size_t start_pos = str.find(from);
	if (start_pos == std::string::npos)
		return false;
	str.replace(start_pos, from.length(), to);
	return true;
}

std::vector<std::string> read_txt(std::string path) {
	std::ifstream file(path);
	std::string line;
	std::vector<std::string> lines;
	while (std::getline(file, line)) {
		lines.push_back(line);
	}
	return lines;
}


void read_autostakkert_file(std::string configfile, std::string *filename, std::vector<cv::Point> *cm_list, int *cm_list_start) {
	for (std::string line : read_txt(configfile)) {
		read_autostakkert_config_line(line, filename, cm_list, cm_list_start);
	}
	
//***** test if filename exists with full name or at the same directory as configfile
	std::ifstream filetest(filename->c_str());
	if (!filetest) {
		std::string acquisition_file2(filename->c_str());
		(*filename) = configfile.substr(0, configfile.find_last_of("\\") + 1) + acquisition_file2.substr(acquisition_file2.find_last_of("\\") + 1, acquisition_file2.length());
		std::ifstream filetest2(filename->c_str());
		if (filetest2) filetest2._close();
		else (*filename) = "";
	} else filetest._close();
}

void read_autostakkert_config_line(std::string line, std::string *filename, std::vector<cv::Point> *cm_list, int *cm_list_start) {
//	std::ifstream file(path, std::ios::in);
//	std::string line;
	std::vector<std::string> lines;
	float x, y;

//	while (std::getline(file, line)) {
	while (line.find(' ') == 0) line.erase(line.find(' '), 1);
	if (starts_with(line, "file")) {
		line = line.substr(strlen("file"), line.length());
		while (line.find(' ') != std::string::npos) line.erase(line.find(' '), 1);
		*filename = line;
	} else if (starts_with(line, "_limit_min")) {
		line = line.substr(strlen("_limit_min"), line.length());
		while (line.find(' ') != std::string::npos) line.erase(line.find(' '), 1);
		std::istringstream ss(line);
		ss >> x;
		(*cm_list_start) = (int) x;
	}	else if (starts_with(line, "_limit_active")) {
		line = line.substr(strlen("_limit_active"), line.length());
		while (line.find(' ') != std::string::npos) line.erase(line.find(' '), 1);
		if (line == "False") (*cm_list_start) = 0;
	} else if (starts_with(line, "f ")) {
		lines.push_back(line);
		line = line.substr(strlen("f "), line.length());
		// replace comma by point
		while (line.find(',') != std::string::npos) replace(line, ",", ".");
		std::istringstream ss(line);
		ss >> x >> y;
		cm_list->push_back(cv::Point(round(x), round(y)));
	}
//	}
}