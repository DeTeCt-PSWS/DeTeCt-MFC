#include "dtcas3.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdio>

#include "cmdline.h" 


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


void read_autostakkert_file(std::string configfile, std::string *filename, std::vector<cv::Point> *cm_list, int *cm_list_start, int *cm_list_end, int *cm_frame_count) {
	(*filename) = "";
	(*cm_list_start) = -1;
	(*cm_list_end) = -1;
	(*cm_frame_count) = -1;
	
	for (std::string line : read_txt(configfile)) {
		read_autostakkert_config_line(line, filename, cm_list, cm_list_start, cm_list_end, cm_frame_count);
		//do not read all file if cm list not needed and other values already found
		if ((cm_list == NULL) && ((*filename) != "") && ((*cm_list_start) != -1) && ((*cm_list_end) != -1) && ((*cm_frame_count) != -1)) break;
	}
	
//***** test if filename exists with full name or at the same directory as configfile
	if (!file_exists(filename->c_str())) {
		std::string acquisition_file2(filename->c_str());
		//(*filename) = configfile.substr(0, configfile.find_last_of("\\") + 1) + acquisition_file2.substr(acquisition_file2.find_last_of("\\") + 1, acquisition_file2.length());
		(*filename) = dirfilename(configfile, acquisition_file2);
//***** test if acquisition file is WJ derotated file
		if ((!file_exists(filename->c_str())) && (acquisition_file2.find_last_of(WJ_DEROT_STRING) > 0)) {
			std::string winjupos_derotation_filename(acquisition_file2);
			winjupos_derotation_filename = winjupos_derotation_filename.substr(0, winjupos_derotation_filename.find_last_of(".") + 1) + WJ_DEROT_EXT;
//***** test if WJ derotation file exists
			if (file_exists(winjupos_derotation_filename)) {
				read_winjupos_file(winjupos_derotation_filename, filename);
				if (!file_exists((*filename))) {
					(*filename) = dirfilename(configfile, (*filename));
					if (!file_exists((*filename))) (*filename) = "";
				}
			} else {
//***** test if WJ derotated acquisition exists in current directory
				//winjupos_derotation_filename = configfile.substr(0, configfile.find_last_of("\\") + 1) + winjupos_derotation_filename.substr(winjupos_derotation_filename.find_last_of("\\") + 1, winjupos_derotation_filename.length());
				winjupos_derotation_filename = dirfilename(configfile, winjupos_derotation_filename);
				if (file_exists(winjupos_derotation_filename)) {
					read_winjupos_file(winjupos_derotation_filename, filename);
//					if (!file_exists((*filename))) {
//						(*filename) = dirfilename(configfile, (*filename));
					if (!file_exists((*filename))) (*filename) = "";
//					}
				} else (*filename) = "";
			}
		} else (*filename) = "";
	}
}

/**** read autostakkert session file to return relevant information, including frames' alignment ***/
void read_autostakkert_config_line(std::string line, std::string *filename, std::vector<cv::Point> *cm_list, int *cm_list_start, int *cm_list_end, int *cm_frame_count) {
//	std::ifstream file(path, std::ios::in);
	std::vector<std::string> lines;
	float x, y;

	while (line.find(' ') == 0) line.erase(line.find(' '), 1);
	if (starts_with(line, "file")) {
		line = line.substr(strlen("file"), line.length());
		while (line.find(' ') != std::string::npos) line.erase(line.find(' '), 1);
		*filename = line;
	} else if (starts_with(line, "_frames_count")) {
		line = line.substr(strlen("_frames_count"), line.length());
		while (line.find(' ') != std::string::npos) line.erase(line.find(' '), 1);
		std::istringstream ss(line);
		ss >> x;
		(*cm_frame_count) = (int)x;
	} else if (starts_with(line, "_limit_min")) {
		line = line.substr(strlen("_limit_min"), line.length());
		while (line.find(' ') != std::string::npos) line.erase(line.find(' '), 1);
		std::istringstream ss(line);
		ss >> x;
		(*cm_list_start) = (int) x;
	} else if (starts_with(line, "_limit_max")) {
		line = line.substr(strlen("_limit_max"), line.length());
		while (line.find(' ') != std::string::npos) line.erase(line.find(' '), 1);
		std::istringstream ss(line);
		ss >> x;
		(*cm_list_end) = (int)x;
	}	else if (starts_with(line, "_limit_active")) {
		line = line.substr(strlen("_limit_active"), line.length());
		while (line.find(' ') != std::string::npos) line.erase(line.find(' '), 1);
		if (line == "False") {
			(*cm_list_start) = 0;
			(*cm_list_end) = 999999;
		}
	} else if ((cm_list != NULL) && (starts_with(line, "f "))) {
		lines.push_back(line);
		line = line.substr(strlen("f "), line.length());
		// replace comma by point
		while (line.find(',') != std::string::npos) replace(line, ",", ".");
		std::istringstream ss(line);
		ss >> x >> y;
		cm_list->push_back(cv::Point((int)round(x), (int)round(y)));
	}
}

void read_winjupos_file(std::string winjupos_derotation_filename, std::string *filename)
{
	std::ifstream input(winjupos_derotation_filename, std::ios::binary);
	char separator[] = "*WS*12345678901234";
	int separator_len = 4;
	char filename_char[MAX_STRING];
	filename_char[0] = '\0';

	// copies all data into buffer
	std::vector<unsigned char> buffer(std::istreambuf_iterator<char>(input), {});
	bool get_string = false;
	int index = 0;

	for (std::vector<unsigned char>::size_type i = 0; i != buffer.size(); i++) {
		if (!get_string) {
			if ((i >= strlen(separator)) && (strncmp(reinterpret_cast<char*> (&buffer[i-strlen(separator)+1]), separator, separator_len) == 0)) get_string = true;
		} else {
			if (strncmp(reinterpret_cast<char*> (&buffer[i]), "*", 1) != 0) {
//				if (!strncmp(reinterpret_cast<char*> (&buffer[i]), "\0", 1) == 0) filename_char[index++] = buffer[i];
				if (buffer[i] >= ' ') {
					filename_char[index++] = buffer[i];
					filename_char[index] = '\0';
				}
			} else break;
		}
	}
	(*filename) = std::string(filename_char);
	if ((filename->length() > 2) && (filename->find_first_of(":") == std::string::npos)) (*filename) = winjupos_derotation_filename.substr(0, winjupos_derotation_filename.find_first_of(":")+1) + (*filename);
}

/**** return if file with filename exists ***/
bool file_exists(std::string filename) {
	std::ifstream filetest(filename);
	if (!filetest) return false;
	filetest._close();
	return true;
}

/**** return full filename constructed from first file directory and second file short filename ***/
std::string dirfilename(std::string directoryname_from_path, std::string filename_from_path) {
	return directoryname_from_path.substr(0, directoryname_from_path.find_last_of("\\") + 1) + filename_from_path.substr(filename_from_path.find_last_of("\\") + 1, filename_from_path.length());
}