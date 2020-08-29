#include "processes_queue.h"

#include "dtcas3.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdio>

#include "cmdline.h" 
#include "common2.h"

void read_autostakkert_session_config_line(std::string line, std::string *filename, std::vector<cv::Point> *cm_list, int *cm_list_start, int *cm_list_end, int *cm_frame_count);

// ************************************************************
// ************** AS!3 session and WJ derot files *************
// ************************************************************

void read_autostakkert_session_file(std::string configfile, std::string *filename, std::vector<cv::Point> *cm_list, int *cm_list_start, int *cm_list_end, int *cm_frame_count) {
	(*filename) = "";
	(*cm_list_start) = -1;
	(*cm_list_end) = -1;
	(*cm_frame_count) = -1;
	
	for (std::string line : read_txt(configfile)) {
		read_autostakkert_session_config_line(line, filename, cm_list, cm_list_start, cm_list_end, cm_frame_count);
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
			(*filename) = ""; 
			std::string winjupos_derotation_filename(acquisition_file2);
			std::string WJ_derot_extension;
			WJ_derot_extension = WJ_DEROT_EXT;
			winjupos_derotation_filename = winjupos_derotation_filename.substr(0, winjupos_derotation_filename.find_last_of(".") + 1) + WJ_derot_extension;
			if (!file_exists(winjupos_derotation_filename)) {
				WJ_derot_extension = WJ_DEROT_EXT_OLD;
				winjupos_derotation_filename = winjupos_derotation_filename.substr(0, winjupos_derotation_filename.find_last_of(".") + 1) + WJ_derot_extension;
			}
//***** test if WJ derotation file exists
			if (file_exists(winjupos_derotation_filename)) {
				read_winjupos_file(winjupos_derotation_filename, filename, WJ_derot_extension);
				if (!file_exists((*filename))) {
					(*filename) = dirfilename(configfile, (*filename));
					if (!file_exists((*filename))) (*filename) = "";
				}
			} else {
//***** test if WJ derotated acquisition exists in current directory
				//winjupos_derotation_filename = configfile.substr(0, configfile.find_last_of("\\") + 1) + winjupos_derotation_filename.substr(winjupos_derotation_filename.find_last_of("\\") + 1, winjupos_derotation_filename.length());
				winjupos_derotation_filename = dirfilename(configfile, winjupos_derotation_filename);
				if (file_exists(winjupos_derotation_filename)) {
					read_winjupos_file(winjupos_derotation_filename, filename, WJ_derot_extension);
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
void read_autostakkert_session_config_line(std::string line, std::string *filename, std::vector<cv::Point> *cm_list, int *cm_list_start, int *cm_list_end, int *cm_frame_count) {
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

/**********************************************************************************************//**
* @fn	void read_winjupos_file(const std::string winjupos_derotation_filename, std::string *filename, std::string extension)
*
* @brief	Returns in filename name of acquisition used in winjupos_derotation_filename
*
* @author	Marc
* @date	2020-04-16
*
* @param [in]		winjupos_derotation_filename	WinJupos derotation filename
* @param [out]		filename  						pointer to acquisition filename
* @param [out]		extension						extension of winjupos derotation filename (eg drs or drs.xml)
* @param [in,out]	pmessage					pointer to wstring streal of message to be displayed
**************************************************************************************************/

void read_winjupos_file(const std::string winjupos_derotation_filename, std::string *filename, std::string extension)
{
	(*filename) = "";
	if (extension == WJ_DEROT_EXT_OLD) {
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
				if ((i >= strlen(separator)) && (strncmp(reinterpret_cast<char*> (&buffer[i - strlen(separator) + 1]), separator, separator_len) == 0)) get_string = true;
			}
			else {
				if (strncmp(reinterpret_cast<char*> (&buffer[i]), "*", 1) != 0) {
					//				if (!strncmp(reinterpret_cast<char*> (&buffer[i]), "\0", 1) == 0) filename_char[index++] = buffer[i];
					if (buffer[i] >= ' ') {
						filename_char[index++] = buffer[i];
						filename_char[index] = '\0';
					}
				}
				else break;
			}
		}
		(*filename) = std::string(filename_char);
		if ((filename->length() > 2) && (filename->find_first_of(":") == std::string::npos)) (*filename) = winjupos_derotation_filename.substr(0, winjupos_derotation_filename.find_first_of(":") + 1) + (*filename);
		input.close();
	}
	else if (extension == WJ_DEROT_EXT) {
		// <SourceVideoFile>G:\work\Impact\tests\winjupos\Jup_L_03_06_2010_203058b.avi< / SourceVideoFile>
		std::ifstream input(winjupos_derotation_filename);
		std::string line;
		std::string sourcevideo_tag("<SourceVideoFile>");
		std::string sourcevideo_tag_end("</SourceVideoFile>");
		while (std::getline(input, line) && ((*filename)=="")) {
			if (line.find(sourcevideo_tag) != std::string::npos) {
				(*filename) = line.substr(line.find(sourcevideo_tag) + sourcevideo_tag.length(), line.find(sourcevideo_tag_end)-(line.find(sourcevideo_tag) + sourcevideo_tag.length()));
			}
		}
		input.close();
	}
}


// ************************************************************
// ****************** AS! processes ***************************
// ************************************************************

/**********************************************************************************************
*
* @fn	AutostakkertInstancesNumber()
*
* @brief	Get number of Autostakkert processes running
*
* @author	Marc
* @date		2020-04-15
*
* @return	number of Autostakkert processes running
**************************************************************************************************/

int AutostakkertInstancesNumber()
{
	return ProcessRunningInstancesNumber(AUTOSTAKKERTFILENAME);
}

/**********************************************************************************************
*
* @fn	IsParentAutostakkert()
*
* @brief	returns if parent of current DeTeCt process is AutoStakkert, and its PID
*
* @author	Marc
* @date		2020-04-15
*
* @param	[out]	pASpid	PID of Autostakkert parent
*
* @return	TRUE if parent of current DeTeCt process is AutoStakkert, PID of Autostakkert parent
**************************************************************************************************/

BOOL IsParentAutostakkert(DWORD *pASpid)
{
	DWORD pid, ppid;
	int e;
	wchar_t wfname[MAX_PATH] = { 0 };

	pid = GetCurrentProcessId();
	ppid = getParentPID(pid);
	e = getProcessName(ppid, wfname, MAX_PATH);
	if (wcsstr(wfname, LAUTOSTAKKERTFILENAME) != NULL) {
		(*pASpid) = ppid;
		return TRUE;
	}
	(*pASpid) = 0;
	return FALSE;
}

BOOL IsParentDeTeCt(DWORD *pASpid)
{
	DWORD pid, ppid;
	int e;
	wchar_t wfname[MAX_PATH] = { 0 };
	char DeTeCtNameChar[MAX_PATH];

	pid = GetCurrentProcessId();
	ppid = getParentPID(pid);
	e = getProcessName(ppid, wfname, MAX_PATH);
	if (wcsstr(wfname, (CString) DeTeCtFileName(DeTeCtNameChar)) != NULL) {
		(*pASpid) = ppid;
		return TRUE;
	}
	(*pASpid) = 0;
	return FALSE;
}

BOOL IsParentAutostakkertRunning(const DWORD ASpid)
{
	return IsProcessRunning(ASpid);
}


