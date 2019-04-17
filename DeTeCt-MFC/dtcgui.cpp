/********************************************************************************/
/*                                                                              */
/*			DTC	(c) Luis Calderon, Marc Delcroix, Jon Juaristi 2012-			*/
/*                                                                              */
/********************************************************************************/


#include <string>
#include <vector>
#include <queue>
#include <iostream>
#include <ctime>
#include <numeric>
#include <algorithm>
#include <regex>

#include "DeTeCt-MFCDlg.h"
#include "dtcgui.hpp"
#include "afxwin.h"

#include <strsafe.h>

/** @brief	Options for the algorithm */
OPTS opts;
extern CDeTeCtMFCDlg dlg;

void StreamDeTeCtOSversions(std::wstringstream *ss)
{
	std::string os_version = "";

	(*ss) << full_version.c_str();
	(*ss) << " running on ";
	GetOSversion(&os_version);
	(*ss) << os_version.c_str() << " OS";
}

void GetOSversion(std::string *pos_version)
{
	OSVERSIONINFOEX os_info;
	ZeroMemory(&os_info, sizeof(OSVERSIONINFOEX));
	os_info.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
	if (GetVersionEx((OSVERSIONINFO*)&os_info)) {
		if (os_info.dwMajorVersion == 5 && os_info.dwMinorVersion == 0)
			pos_version->append("Win2000");
		if (os_info.dwMajorVersion == 5 && os_info.dwMinorVersion == 1)
			pos_version->append("WinXP");
		if (os_info.dwMajorVersion == 5 && os_info.dwMinorVersion == 2 && GetSystemMetrics(SM_SERVERR2) == 0)
			pos_version->append("WinServer2003");
		if (os_info.dwMajorVersion == 5 && os_info.dwMinorVersion == 2 && GetSystemMetrics(SM_SERVERR2) != 0)
			pos_version->append("WinServer2003R3");
		if (os_info.dwMajorVersion == 6 && os_info.dwMinorVersion == 0 && os_info.wProductType == VER_NT_WORKSTATION)
			pos_version->append("WinVista");
		if (os_info.dwMajorVersion == 6 && os_info.dwMinorVersion == 0 && os_info.wProductType != VER_NT_WORKSTATION)
			pos_version->append("WinServer2008");
		if (os_info.dwMajorVersion == 6 && os_info.dwMinorVersion == 1 && os_info.wProductType == VER_NT_WORKSTATION)
			pos_version->append("Win7");
		if (os_info.dwMajorVersion == 6 && os_info.dwMinorVersion == 1 && os_info.wProductType != VER_NT_WORKSTATION)
			pos_version->append("WinServer2008R2");
		if (os_info.dwMajorVersion == 6 && os_info.dwMinorVersion == 2 && os_info.wProductType == VER_NT_WORKSTATION)
			pos_version->append("Win8(or_above)");
		if (os_info.dwMajorVersion == 6 && os_info.dwMinorVersion == 2 && os_info.wProductType != VER_NT_WORKSTATION)
			pos_version->append("WinServer2012(or_above)");
		// Following works only for applications that have been manifested for Windows 8.1 or Windows 10
		if (os_info.dwMajorVersion == 6 && os_info.dwMinorVersion == 3 && os_info.wProductType == VER_NT_WORKSTATION)
			pos_version->append("Win8.1");
		if (os_info.dwMajorVersion == 6 && os_info.dwMinorVersion == 3 && os_info.wProductType != VER_NT_WORKSTATION)
			pos_version->append("WinServer2012R2");
		if (os_info.dwMajorVersion == 10 && os_info.dwMinorVersion == 0 && os_info.wProductType == VER_NT_WORKSTATION)
			pos_version->append("Win10");
		if (os_info.dwMajorVersion == 10 && os_info.dwMinorVersion == 0 && os_info.wProductType != VER_NT_WORKSTATION)
			pos_version->append("WinServer2016");
	}
	else {
		pos_version->append("Unknown");
	}
#if _WIN64
	pos_version->append("_64b");
#elif _WIN32
	BOOL isWow64 = FALSE;
	typedef BOOL(WINAPI *LPFN_ISWOW64PROCESS) (HANDLE, PBOOL);
	LPFN_ISWOW64PROCESS fnIsWow64Process = (LPFN_ISWOW64PROCESS)
		GetProcAddress(GetModuleHandle(TEXT("kernel32")), "IsWow64Process");
	if ((fnIsWow64Process) && (fnIsWow64Process(GetCurrentProcess(), &isWow64)))
		if (isWow64)
			pos_version->append("_64b");
		else
			pos_version->append("_32b");
#endif
}

/**********************************************************************************************//**
 * @fn	void read_files(std::string folder, std::vector<std::string> *file_list)
 *
 * @brief	Adds the supported files traversing a folder recursively.
 *
 * @author	Jon
 * @date	2017-05-12
 *
 * @param 		  	folder   	Pathname of the folder.
 * @param [in,out]	file_list	If non-null, list of files.
 **************************************************************************************************/

void read_files(std::string folder, std::vector<std::string> *file_list, std::vector<std::string> *acquisition_file_list) {
	DIR *directory;
	struct dirent *entry;
	std::string file;
	std::string acquisition_file;

	//        std::vector<std::string> non_supported_ext = { "bat", "exe", "log", "txt", "Jpg", "jpg", "Png", "png", "Tif", "tif",
	// "Bmp", "bmp", "Fit", "fit"};
	std::vector<std::string> supported_videoext = { "m4v", "avi", "ser", "wmv" };
	std::vector<std::string> supported_fileext = { "bmp", "jpg", "jpeg", "jp2", "dib", "png", "p?m", "sr", "ras", "tif",
		"tiff", "fit", "fits" };
	std::vector<std::string> supported_otherext = { "as3" };
	// Syntax files:
	// F0.* *0000_*.* *_000000.*  *_000001.* *_00000.* *_00001.* *_0000.* *_0001.* *_0.tif nb1.*
	// supported 0/1 number inside filename
	std::vector<std::string> supported_filename_number = { "0000_", "0001_", "_000000.", "_000001.", "_00000.", 
		"_00001.", "_0000.", "_0001.", "_0.tif" };
	// supported 0/1 number syntax for full filename
	std::vector<std::string> supported_fullfilename_number = { "0000.", "0001.", "00000.", "00001.", "000000.", 
		"000001.", "F0.", "nb1." };

	// ignored dtc own files
	std::vector<std::string> not_supported_suffix = { DTC_MAX_MEAN1_SUFFIX, DTC_MAX_MEAN2_SUFFIX, DTC_MEAN_SUFFIX, 
		DTC_DIFF_MEAN_SUFFIX, VIDEOTEST_SUFFIX, DTC_MAX_SUFFIX, MEAN_SUFFIX, DTC_SUFFIX };
	if (!(directory = opendir(folder.c_str()))) {
		return;
	}
	if (!(entry = readdir(directory))) {
		return;
	}
	do {
		if (entry->d_type == DT_DIR) {
			if (!strcmp(entry->d_name, ".") == 0 && !strcmp(entry->d_name, "..") == 0)
				read_files(folder + "\\" + entry->d_name, file_list, acquisition_file_list);
		}
		else {
			std::string file(entry->d_name);
			std::string extension = file.substr(file.find_last_of(".") + 1, file.length());
			std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
			if (std::find(supported_videoext.begin(), supported_videoext.end(), extension) != supported_videoext.end()) {
				file_list->push_back(folder + "\\" + entry->d_name);
				acquisition_file_list->push_back(folder + "\\" + entry->d_name);
			} else if (std::find(supported_fileext.begin(), supported_fileext.end(), extension) != supported_fileext.end()) {
				int found = false;
				for (std::string filename_number : supported_filename_number) {
					if (file.find(filename_number) != std::string::npos) {
						found = true;
						for (std::string suffix : not_supported_suffix) {
							if (file.find(suffix) != std::string::npos) found = false;
						}
					}
				}
				if (!found) {
					for (std::string filename_number : supported_fullfilename_number) {
						if (file.find(filename_number) == 0) found = true;
					}
				}
				if (found) {
					file_list->push_back(folder + "\\" + entry->d_name);
					acquisition_file_list->push_back(folder + "\\" + entry->d_name);
				}
			}
			else if (std::find(supported_otherext.begin(), supported_otherext.end(), extension) != supported_otherext.end()) {
				file_list->push_back(folder + "\\" + entry->d_name);
				if (extension.compare(autostakkert_extension) == 0) {
					std::vector<cv::Point> cm_list;

					read_autostakkert_file(folder + "\\" + file, &acquisition_file, &cm_list);
					acquisition_file_list->push_back(acquisition_file);
				}
				else {
					acquisition_file_list->push_back(folder + "\\" + entry->d_name);
				}
			}
		}
	} while (entry = readdir(directory));
	closedir(directory);
//Remove duplicates from as3 (keeping as3)
	file_list->begin();
	acquisition_file_list->begin();
	for (int i = 0; i < file_list->size(); i++) {
		file = file_list->at(i);
		std::string extension = file.substr(file.find_last_of(".") + 1, file.length());
		if (extension.compare(autostakkert_extension) == 0) {
			int j = 0;
			if (j == i) j++;
			while ((j < acquisition_file_list->size()) && (acquisition_file_list->at(j) != acquisition_file_list->at(i))) {
				j++;
				if (j == i) j++;
			}
			if (j < acquisition_file_list->size()) {
				file_list->erase(file_list->begin() + j);
				acquisition_file_list->erase(acquisition_file_list->begin() + j);
				i--;
DBOUT("Erasing " << acquisition_file_list->at(j).c_str());
			}
		}
	}
}

/**********************************************************************************************//**
 * @fn	int itemcmp(const void *a, const void *b)
 *
 * @brief	Compares items by brightness values -- unused.
 *
 * @author	Jon
 * @date	2017-05-12
 *
 * @param	a	A void to process.
 * @param	b	A void to process.
 *
 * @return	An int.
 **************************************************************************************************/

int itemcmp(const void *a, const void *b)
{
	if ((*((ITEM **)a))->point->val < (*((ITEM **)b))->point->val) return 1;
	else if ((*((ITEM **)a))->point->val > (*((ITEM **)b))->point->val) return -1;

	else return 0;
}

/**********************************************************************************************//**
* @fn	int itemcmp(const void *a, const void *b)
*
* @brief	Compares items by frame number
*
* @author	Jon
* @date		2017-01-26
*
* @param	a	A void to process.
* @param	b	A void to process.
*
* @return	An int.
**************************************************************************************************/
int framecmp(const void *a, const void *b)
{
	if ((*((ITEM **)a))->point->frame > (*((ITEM **)b))->point->frame) return 1;
	else if ((*((ITEM **)a))->point->frame < (*((ITEM **)b))->point->frame) return -1;

	else return 0;
}

/**********************************************************************************************//**
 * @fn	int impact_detect(DTCIMPACT *dtc, DTCIMPACT *dtcout, double meanValue, LIST *list, ITEM** dtcMax, int fps, double radius,
 double incrLum, int incrFrame)
 *
 * @brief	Impact detection algorithm -- Needs to be revised.
 *
 * @author	Jon
 * @date	2017-05-12
 *
 * @param [in,out]	dtc		 	If non-null, the detection item .
 * @param [in,out]	dtcout   	If non-null, the dtcout.
 * @param 		  	meanValue	The mean value of the maximum values.
 * @param [in,out]	list	 	If non-null, the list of maximum values.
 * @param [in,out]	dtcMax   	If non-null, the dtc maximum of the detection image.
 * @param 		  	fps		 	The framerate of the video.
 * @param 		  	radius   	The radius of the impact.
 * @param 		  	incrLum  	The lum in brightness of the impact.
 * @param 		  	incrFrame	The number of frames of impact.
 *
 * @return	An int.
 **************************************************************************************************/

int detect_impact(DTCIMPACT *dtc, DTCIMPACT *dtcout, double meanValue, LIST *list, ITEM** dtcMax, int fps, double radius,
	double incrLum, int incrFrame)
{
	int c;
	int x0, y0;
	int lastivalFrame;
	double maxMeanValue;
	double d;
/*	long frame_distance; */
	ITEM **ord, **tmp;
	ITEM *tmpSrc;
	int nb_impact;
	TCHAR buffer[1000];
	nb_impact = 0;
	std::vector<ITEM*> items;

	struct FrameOrder frameOrder;
	struct BrightnessOrder brightnessOrder;

	//if (fps < 0) throw std::logic_error("Negative fps value, can't operate with impact detection");
	if (list->size <= 0) return 0;
	if (!(ord = (ITEM **)calloc(list->size, sizeof(ITEM *)))) {
		throw std::bad_alloc();
	}

	for (tmpSrc = list->head, tmp = ord, c = 0; tmpSrc && c < list->size; tmpSrc = tmpSrc->next, tmp++, c++)
		*tmp = tmpSrc;

	qsort(ord, list->size, sizeof(ITEM *), itemcmp);
	maxMeanValue = get_item_list_mean_value(list);

	if (ord[0]->point->val <= meanValue*(1 + incrLum)) {
		return nb_impact;
	}

	x0 = ord[0]->point->x;
	y0 = ord[0]->point->y;
	dtc->MaxFrame = ord[0]->point->frame;

	/*
	ITEM* max = ord[0];
	std::vector<ITEM*> potential_impact;
	potential_impact.push_back(max);
	size_t length = list->size;

	for (int i = 1; i < length; i++) {
		ITEM* current = ord[i];
		// Spatial coherence
		d = sqrt(pow(current->point->x - x0, 2) + pow(current->point->y - y0, 2));
		// Temporal coherence
		frame_distance = abs(current->point->frame - max->point->frame);

		 // * Only those frames where the maximum brightness is spatially and temporally coherent with the maximum
		 // * brightness value of the whole video will be considered part of the impact.
		 // * Main problem: doesn't work well with the longer videos

		if ((d <= radius) && (frame_distance <= 20)) {
			potential_impact.push_back(current);
		}
	}

	dtc->MaxFrame = max->point->frame;
	std::sort(potential_impact.begin(), potential_impact.end(), frameOrder);
	dtc->nMinFrame = potential_impact.front()->point->frame;
	dtc->nMaxFrame = potential_impact.back()->point->frame;*/

	//int impact_frame_num = (int)std::ceil(fps * opts.timeImpact);
	int impact_frame_num = incrFrame;
	std::deque<ITEM*> potential_impact;
	ITEM* impactBrightest = nullptr;
	ITEM* brightest = nullptr;
	double maxMean = 0.0;
	
	/*double minStdDev = DBL_MAX;
	double stdDev = 0.0; */
	qsort(ord, list->size, sizeof(ITEM *), framecmp);
	for (int i = 0; i < list->size; i++) {
		if (i >= impact_frame_num) {
			double acc = 0.0;
			std::for_each(potential_impact.begin(), potential_impact.end(), [&](const ITEM* it) {
				acc += it->point->val;
			});
			double mean = double(acc / potential_impact.size());
			/*acc = 0.0;
			std::for_each(potential_impact.begin(), potential_impact.end(), [&](const ITEM* it) {
				acc += pow(it->point->val - mean, 2);
			});
			stdDev = acc / (potential_impact.size() - 1);
			if (mean < (meanValue + (1 + stdDev))) { */
			if (mean < (meanValue * (1 + incrLum))) {
				potential_impact.pop_front();
				potential_impact.push_back(ord[i]);
				continue;
			}
			std::sort(potential_impact.begin(), potential_impact.end(), brightnessOrder);
			brightest = potential_impact.front();
			// Starts from the second element, since the first is the brightest point of the queue
			/*
			bool candidate = std::all_of(potential_impact.begin() + 1, potential_impact.end(), [&](const ITEM* it) {
				d = sqrt(pow(it->point->x - brightest->point->x, 2) + pow(it->point->y - brightest->point->y, 2));
				return d <= radius;
			});
			*/
			/* make that only the 70% of the frames have to be in the place of impact */
			int count;
			std::for_each(potential_impact.begin() + 1, potential_impact.end(), [&](const ITEM* it) {
				d = sqrt(pow(it->point->x - brightest->point->x, 2) + pow(it->point->y - brightest->point->y, 2));
				if (d <= radius) count++;
			});
			bool candidate = double(count / potential_impact.size()) >= 0.7;
			/*make that only the 70% of the frames have to be in the place of impact*/
			std::sort(potential_impact.begin(), potential_impact.end(), frameOrder);
			//if (candidate && stdDev < minStdDev) {
			if (candidate && mean > maxMean) {
				impactBrightest = brightest;
				dtc->MaxFrame = brightest->point->frame;
				dtc->nMinFrame = potential_impact.front()->point->frame;
				dtc->nMaxFrame = potential_impact.back()->point->frame;
				lastivalFrame = potential_impact.size();
				potential_impact.push_back(ord[i]);
				maxMean = mean;
				continue;
			} else {
				while (potential_impact.size() > impact_frame_num)
					potential_impact.pop_front();
				potential_impact.push_back(ord[i]);
				continue;
			}
		}
		potential_impact.push_back(ord[i]);
	}

	lastivalFrame = dtc->nMaxFrame - dtc->nMinFrame + 1;
	dtcout->MaxFrame = dtc->MaxFrame;
	dtcout->nMinFrame = dtc->nMinFrame;
	dtcout->nMaxFrame = dtc->nMaxFrame;
	if (lastivalFrame >= incrFrame) {
		StringCchPrintf(buffer, sizeof(buffer) / sizeof(TCHAR), TEXT("Max lum %d at frame %ld, point (%ld, %ld).\n"),
			(int)impactBrightest->point->val, (int)impactBrightest->point->frame, (int)impactBrightest->point->x, 
			(int)impactBrightest->point->y);
		OutputDebugString(buffer);
		fflush(stdout);
		*dtcMax = create_item(create_point(impactBrightest->point->frame, impactBrightest->point->val, impactBrightest->point->x, 
			impactBrightest->point->y));
		nb_impact++;
		delete_list(list);
	}
	free(ord);
	potential_impact.clear();
	return nb_impact;
}

/**********************************************************************************************//**
 * @fn	int impact_detection(DTCIMPACT *dtc, LIST *impact, LIST *candidates, std::vector<ITEM*> candidateFrames, int fps,
 double radius, double timeImpact)
 *
 * @brief	Impact detection - UNUSED
 *
 * @author	Jon
 * @date	2017-05-12
 *
 * @param [in,out]	dtc			   	If non-null, the dtc.
 * @param [in,out]	impact		   	If non-null, the impact.
 * @param [in,out]	candidates	   	If non-null, the candidates.
 * @param [in,out]	candidateFrames	If non-null, the candidate frames.
 * @param 		  	fps			   	The FPS.
 * @param 		  	radius		   	The radius.
 * @param 		  	timeImpact	   	The time impact.
 *
 * @return	An int.
 **************************************************************************************************/

/*
int impact_detection(DTCIMPACT *dtc, LIST *impact, LIST *candidates, std::vector<ITEM*> candidateFrames, int fps, double radius,
	double timeImpact)
{
	ITEM *current, *first;
	std::vector<ITEM*> impactVec, candidateOriginal;
	double d;

	int brightness_delta, frame_difference;


	if (candidates->size <= 1)
		return 0;

	int frame_delta = std::ceil(fps * timeImpact) + 10;


	for (ITEM* candidate : candidateFrames) {
		candidateOriginal.push_back(candidate);
	}
	BrightnessOrder brightnessOrder;
	std::sort(candidateFrames.begin(), candidateFrames.end(), brightnessOrder);

	first = candidateFrames[0];
	impactVec.push_back(first);
	for (int i = 1; i < candidateOriginal.size(); i++) {
		current = candidateOriginal[i];
		d = sqrt(pow(current->point->x - first->point->x, 2) + pow(current->point->y - first->point->y, 2));
		brightness_delta = abs(current->point->val - first->point->val);
		frame_difference = abs(current->point->frame - first->point->frame);
		if (d <= radius && brightness_delta <= opts.incrLumImpact && frame_difference <= frame_delta)
			impactVec.push_back(current);
	}

	if (impactVec.size() >= std::ceil(fps * timeImpact)) {
		dtc->MaxFrame = first->point->frame;
		struct FrameOrder frameOrder;
		std::sort(impactVec.begin(), impactVec.end(), frameOrder);
		dtc->nMinFrame = impactVec.front()->point->frame;
		dtc->nMaxFrame = impactVec.back()->point->frame;
		return 1;
	}
	else {
		return 0;
	}
}
*/

/**********************************************************************************************//**
 * @fn	int detect(std::vector<std::string> file_list, OPTS opts)
 *
 * @brief	Main algorithm
 *
 * @author	Jon
 * @date	2017-05-12
 *
 * @param	file_list	List of files.
 * @param	opts	 	The options for the algorithm execution
 *
 * @return	An integer which is unused.
 **************************************************************************************************/

int detect(std::vector<std::string> file_list, OPTS opts, std::string scan_folder_path) {
	clock_t begin, end;
	cv::setUseOptimized(true);
	std::string logcation(scan_folder_path);
	std::string logcation2(scan_folder_path);
	std::string start_time = getRunTime().str().c_str();
	std::wstring wstart_time = std::wstring(start_time.begin(), start_time.end());
	logcation2.append("\\Impact_detection_run@").append(start_time);
	std::vector<int> img_save_params = { CV_IMWRITE_JPEG_QUALITY, 100 };
	
	std::wstring detection_folder_path = {};
	std::wstring details_folder_path = {};
	char max_mean_folder_path_filename[MAX_STRING];
	char max_mean2_folder_path_filename[MAX_STRING];
	char mean_folder_path_filename[MAX_STRING];
	char max_folder_path_filename[MAX_STRING];
	char diff_folder_path_filename[MAX_STRING];
	char diff_mean_folder_path_filename[MAX_STRING];
	char tmpstring[MAX_STRING];

	detection_folder_path = std::wstring(scan_folder_path.begin(), scan_folder_path.end());
	detection_folder_path = detection_folder_path.append(L"\\Impact_detection_run@").append(wstart_time);
	if (GetFileAttributes(detection_folder_path.c_str()) == INVALID_FILE_ATTRIBUTES)
		CreateDirectory(detection_folder_path.c_str(), 0);

	if (opts.detail || opts.allframes) {
		details_folder_path = std::wstring(detection_folder_path.begin(), detection_folder_path.end());
		details_folder_path = details_folder_path.append(L"\\details");
		if (GetFileAttributes(details_folder_path.c_str()) == INVALID_FILE_ATTRIBUTES)
			CreateDirectory(details_folder_path.c_str(), 0);
	}

	dtcWriteLogHeader(logcation);
	dtcWriteLogHeader(logcation2);

	std::wstring output_log_file(scan_folder_path.begin(), scan_folder_path.end());
	output_log_file = output_log_file.append(L"\\Impact_detection_run@").append(wstart_time).append(L"\\output.log");
	std::wofstream output_log(output_log_file.c_str(), std::ios_base::app);

	std::vector<LogInfo> logs;
	std::vector<LPCTSTR> logMessages;
	std::vector<std::string> log_messages;

	std::string logmessage;
	std::wstring wlogmessage;
	CString Clogmessage;

	for (std::string filename : file_list) {
//gets acquisition file from autostakkert session file
		std::string extension = filename.substr(filename.find_last_of(".")+1, filename.size()-filename.find_last_of(".")-1);
		if (extension.compare(autostakkert_extension) == 0) {
			std::string filename_acquisition;
			std::vector<cv::Point> cm_list;

			read_autostakkert_file(filename, &filename_acquisition, &cm_list);
			filename = filename_acquisition;
		}
//TODO: usage of cmlist and quality information from as3
		opts.filename = strdup(filename.c_str());
		std::string outputFolder = filename.substr(0, filename.find_last_of("\\") + 1);
		outputFolder = outputFolder.replace(0, scan_folder_path.length() + 1, "");
		std::replace(outputFolder.begin(), outputFolder.end(), '\\', '_');
		std::replace(outputFolder.begin(), outputFolder.end(), ' ', '_');
		std::string folderPath = filename.substr(0, filename.find_last_of("\\") + 1);
		std::string filePath = filename.substr(filename.find_last_of("\\") + 1, filename.find_last_of("."));
		filePath = filePath.substr(0, filePath.find_last_of("."));
		std::string outputfilename = folderPath.append(outputFolder).append(filePath).append(".jpg");
		opts.ofilename = strdup(outputfilename.c_str());
		std::wstring fname(filename.begin(), filename.end());
		std::string short_filename = filename.substr(filename.find_last_of("\\") + 1, filename.length());

		std::string message = "-------------- " + short_filename + " start --------------";
		std::string message2 = "-------------- " + filename + " start --------------";
		CDeTeCtMFCDlg::getLog()->AddString((CString)getDateTime().str().c_str() + (CString)message.c_str());
		output_log << getDateTime().str().c_str() << message2.c_str() << "\n";
		output_log.flush();

		init_string(max_mean_folder_path_filename);
		init_string(max_mean2_folder_path_filename);
		init_string(mean_folder_path_filename);
		init_string(diff_mean_folder_path_filename);
		init_string(tmpstring);

		std::string detection_folder_path_string(detection_folder_path.begin(), detection_folder_path.end());
		std::strcpy(max_mean_folder_path_filename, detection_folder_path_string.c_str());
		std::strcpy(mean_folder_path_filename, detection_folder_path_string.c_str());

		std::string detail_folder_path_string(details_folder_path.begin(), details_folder_path.end());
		std::strcpy(max_mean2_folder_path_filename, detail_folder_path_string.c_str());
		std::strcpy(diff_mean_folder_path_filename, detail_folder_path_string.c_str());

		int fps_int = 0;
		double fps_real = 0;
		DtcCapture *pCapture;
		LIST ptlist = { 0,0,NULL,NULL };
		DTCIMPACT dtc;
		DTCIMPACT outdtc;

		LIST candidates = { 0, 0, NULL, NULL };
		LIST impact = { 0, 0, NULL, NULL };

		cv::Mat pFrame; // Input frame
		cv::Mat pGryMat; // Grey frame
		cv::Mat pRefMat; // Reference frame
		cv::Mat pDifMat; // Difference frame
		cv::Mat pMskMat; // Mask frame
		cv::Mat pHisMat; // Histogram frame
		cv::Mat pThrMat; // Threshold frame
		cv::Mat pSmoMat; // Smooth frame
		cv::Mat pTrkMat; // Tracking frame
		cv::Mat pOVdMat; // Output video frame
		cv::Mat pADUavgMat; // ADU average frame
		cv::Mat pADUmaxMat; // ADU max frame
		cv::Mat pADUdtcMat; // ADU detect frame
		cv::Mat pSmoADUdtcMat; // ADU detect frame (smoothed)
		cv::Mat pADUavgDiffMat; // ADU average difference frame
		cv::Mat pADUavgMatFrame; // ADU average frame2
		cv::Mat pADUdarkMat; // ADU dark frame
		cv::Mat pFirstFrameROIMat; // Region of interest, obtained from the first frame
		cv::Rect pFirstFrameROI; // Aforementioned region of interest as a delimited rectangle
		cv::Mat pROIMat; // Region of interest, obtained for the rest of the frames
		cv::Rect pROI; // Aforementioned region of interest as a delimited rectangle
		cv::Rect pFrameROI; // ROI of current frame
		cv::Mat pAvgMat;

		cv::Mat tempROIMat, tempGryMat; // For matrices in which the ROI covers non-existing data

		cv::Mat previousGrayMat;
		std::queue<cv::Mat> refFrameQueue; // Queue to make a moving reference frame
					/* Images to be shown and/or saved */
		cv::Mat pGryImg;
		cv::Mat pRefImg;
		cv::Mat pDifImg;
		cv::Mat pMskImg;
		cv::Mat pThrImg;
		cv::Mat pSmoImg;
		cv::Mat pHisImg;
		cv::Mat pTrkImg;
		cv::Mat pOVdImg;
		cv::Mat pADUdtcImg;
		cv::Mat pADUdtcImg2;
		cv::Mat pADUavgImg;
		cv::Mat pADUdarkImg;

		int y_shift = 10;
		int x_shift = 10;

		std::vector<double> xList;
		std::vector<double> maxList;

		int N = 0;
		double totalMean =0;

		cv::Point brightestPointOfImpact;

		cv::VideoWriter *pWriter = cv::makePtr<cv::VideoWriter>();

		cv::Rect croi = { 0, 0, 0, 0 };

		cv::Point minPoint = { 0, 0 };
		cv::Point maxPoint = { 0, 0 };
		cv::Point firstFrameCm;

		cv::Scalar lum;

		double minLum = 0;
		double maxLum = 0;

		int nframe = 0;
		int framecount = 0;

		int pGryImg_height = 0;
		int pGryImg_width = 0;
		char ofilenamenormframe[MAX_STRING];
		char ofilenamemean[MAX_STRING];
		char ofilenamenorm[MAX_STRING];
		char ofilenamediffmean[MAX_STRING];
		char ofilenamemax[MAX_STRING];
		char ofilenamediff[MAX_STRING];

		init_string(ofilenamenormframe);
		init_string(ofilenamemean);
		init_string(ofilenamediffmean);
		init_string(ofilenamenorm);

		char comment[MAX_STRING];
		double duration;

		double start_time;
		double end_time;
		TIME_TYPE timetype;

		int nb_impact = -1;
		int frame_error = 0;
		int frame_errors = 0;
		int darkfile_ok = 0;
		lum.val[0] = 0.0;
		
		int frame_number;

		std::vector<ITEM*> candidateFrames;
		std::vector<ITEM*> brightestPoints;
		std::vector<cv::Mat> frameList;
		std::vector<cv::Point> cmShifts;
		std::vector<DiffImage> diffImages;
		std::vector<uint16_t> maxPtX;
		std::vector<uint16_t> maxPtY;
		std::vector<uint8_t> maxPtB;
		std::vector<uint16_t> frameErrors;
		std::vector<uint16_t> frameNumbers;
		cv::Mat xMat;
		cv::Mat yMat;
		cv::Mat bMat;
		cv::Mat impactFrame;
		cv::Mat pOrigGryMat;

		double firstFrameMean;
		double currentFrameMean;

		int tempCols = 0;
		int tempRows = 0;

		try {
			/*********************************INITIALIZATION******************************************/

			double video_duration = 0.0;
			begin = clock();

			std::vector<long> frames;

			CDeTeCtMFCDlg::getProgress()->SetStep(1);

			if (!(pCapture = dtcCaptureFromFile2(opts.filename, &framecount))) {
				CDeTeCtMFCDlg::getLog()->AddString((CString)getDateTime().str().c_str() + L"Cannot open file " +
					(CString)opts.filename);
				CDeTeCtMFCDlg::getLog()->SetTopIndex(CDeTeCtMFCDlg::getLog()->GetCount() - 1);
				CDeTeCtMFCDlg::getLog()->RedrawWindow();
				output_log << getDateTime().str().c_str() << "Cannot open file " << opts.filename << "\n";
				output_log.flush();
				continue;
			}
			switch (pCapture->type) {
			case CAPTURE_SER:
				nframe = pCapture->u.sercapture->header.FrameCount;
				break;
			case CAPTURE_FITS:
			case CAPTURE_FILES:
				nframe = pCapture->u.filecapture->FrameCount;
				break;
			default: // CAPTURE_CV
				nframe = (int)(dtcGetCaptureProperty(pCapture, CV_CAP_PROP_FRAME_COUNT));
			}
			frame_number = nframe;
			CDeTeCtMFCDlg::getProgress()->SetRange(0, nframe);
			CDeTeCtMFCDlg::getProgress()->SetPos(0);
			if ((nframe > 0) && (nframe < opts.minframes)) {
				CDeTeCtMFCDlg::getLog()->AddString((CString)getDateTime().str().c_str() + L"INFO: only " +
					(CString)std::to_string(nframe).c_str() + L" frames (minimum is " +
					(CString)std::to_string(opts.minframes).c_str() + L"), stopping processing");
				CDeTeCtMFCDlg::getLog()->SetTopIndex(CDeTeCtMFCDlg::getLog()->GetCount() - 1);
				CDeTeCtMFCDlg::getLog()->RedrawWindow();
				output_log << getDateTime().str().c_str() << "INFO: only " << nframe << " frames (minimum is " << opts.minframes
					<< "), stopping processing" << "\n";
				output_log.flush();
				message = "-------------- " + short_filename + " end --------------";
				CDeTeCtMFCDlg::getLog()->AddString((CString)getDateTime().str().c_str() + (CString)message.c_str());
				output_log << getDateTime().str().c_str() << message.c_str() << "\n";
				output_log.flush();
				CDeTeCtMFCDlg::getLog()->SetTopIndex(CDeTeCtMFCDlg::getLog()->GetCount() - 1);
				CDeTeCtMFCDlg::getLog()->RedrawWindow();
				dtcReleaseCapture(pCapture);
				pCapture = NULL;
				logmessage = "INFO: only " + std::to_string(nframe) + " frames (minimum is " + std::to_string(opts.minframes) + 
					"), stopping processing\n";
				log_messages.push_back(short_filename + ":");
				log_messages.push_back("    " + logmessage);
				continue;
			}
			dtcGetDatation(pCapture, opts.filename, nframe, &start_time, &end_time, &duration, &fps_real, &timetype, comment);
			if (fps_real < 0.02)
				fps_int = (int)dtcGetCaptureProperty(pCapture, CV_CAP_PROP_FPS);
			else
				fps_int = (int)std::floor(0.5 + fps_real);
			CDeTeCtMFCDlg::getLog()->AddString((CString)getDateTime().str().c_str() + L"Initializing capture: " +
				(CString)std::to_string(nframe).c_str() + L" frames @ " + (CString)std::to_string(fps_int).c_str() + L" fps");
			CDeTeCtMFCDlg::getLog()->SetTopIndex(CDeTeCtMFCDlg::getLog()->GetCount() - 1);
			CDeTeCtMFCDlg::getLog()->RedrawWindow();
			output_log << getDateTime().str().c_str() << "Initializing capture:  " << nframe << " frames @ " << fps_int
				<< " fps" << "\n";
			output_log.flush();

			if (opts.dateonly) {
				if (nframe > 0)
					dtcWriteLog(opts.filename, start_time, end_time, duration, fps_real, timetype, opts.filename, comment, -1, 1);
				dtcReleaseCapture(pCapture);
				pCapture = NULL;
				continue;
			}
			if (opts.wROI && opts.hROI) {
				croi = cv::Rect(0, 0, opts.wROI, opts.hROI);
			} else {
				croi = dtcGetFileROIcCM(pCapture, opts.ignore, 0);
				dtcReinitCaptureRead2(&pCapture, opts.filename);
			}
	
			if (opts.viewDif) cv::namedWindow("Initial differential photometry");
			if (opts.viewRef) cv::namedWindow("Reference frame");
			if (opts.viewROI) cv::namedWindow("ROI");
			if (opts.viewTrk) cv::namedWindow("Tracking");
			if (opts.viewMsk) cv::namedWindow("Mask");
			if (opts.viewThr) cv::namedWindow("Thresholded differential photometry");
			if (opts.viewSmo) cv::namedWindow("Smoothed differential photometry");
			if (opts.viewRes) cv::namedWindow("Resulting differential photometry");
			if (opts.viewHis) cv::namedWindow("Histogram");

			nframe = 0;
			if (opts.darkfilename) {
				if (!(pADUdarkMat = cv::imread(opts.darkfilename, CV_LOAD_IMAGE_GRAYSCALE)).data) {
					CDeTeCtMFCDlg::getLog()->AddString((CString)getDateTime().str().c_str() + L"Warning: cannot read dark frame" +
						(CString)std::string(opts.darkfilename).c_str());
					CDeTeCtMFCDlg::getLog()->SetTopIndex(CDeTeCtMFCDlg::getLog()->GetCount() - 1);
					CDeTeCtMFCDlg::getLog()->RedrawWindow();
					output_log << getDateTime().str().c_str() << "Warning: cannot read dark frame:  " << opts.darkfilename << "\n";
					output_log.flush();
					darkfile_ok = 0;
				} else {
					darkfile_ok = 1;
				}
			}

			/*********************************CAPTURE READING******************************************/
			while ((pFrame = dtcQueryFrame2(pCapture, opts.ignore, &frame_error)).data) {
				cv::medianBlur(pFrame, pFrame, 3);
				video_duration += (int)dtcGetCaptureProperty(pCapture, CV_CAP_PROP_POS_MSEC);
				nframe++;
				if (!(frame_error) == 0) {
					frame_errors += 1;
				} else {
					init_string(ofilenamemax);
					init_string(ofilenamediff);
					//init_string(comment);
					init_string(max_folder_path_filename);
					init_string(diff_folder_path_filename);
					std::strcpy(max_folder_path_filename, detail_folder_path_string.c_str());
					std::strcpy(diff_folder_path_filename, detail_folder_path_string.c_str());
					cv::Point cm;
					cv::Rect roi;
					DiffImage diffImage;
					pGryMat = dtcGetGrayMat(&pFrame, pCapture);
					cv::medianBlur(pGryMat, pGryMat, 1);
					//dtcApplyMaskToFrame(pGryMat);
					//cv::GaussianBlur(pGryMat, pGryMat, cv::Size(1, 1), 1);
					if (darkfile_ok == 1) {
						if ((pADUdarkMat.rows != pGryMat.rows) || (pADUdarkMat.cols != pGryMat.cols)) {
							CDeTeCtMFCDlg::getLog()->AddString((CString)getDateTime().str().c_str() + L"Warning: dark frame " +
								(CString)std::string(opts.darkfilename).c_str() + L" differs from the frame properties " +
								(CString)std::to_string(pADUdarkMat.rows).c_str() + L" vs " +
								(CString)std::to_string(pGryMat.rows).c_str() + L" rows, " +
								(CString)std::to_string(pADUdarkMat.cols).c_str() + L" vs " +
								(CString)std::to_string(pGryMat.cols).c_str() + L" cols");
							CDeTeCtMFCDlg::getLog()->SetTopIndex(CDeTeCtMFCDlg::getLog()->GetCount() - 1);
							CDeTeCtMFCDlg::getLog()->RedrawWindow();
							output_log << getDateTime().str().c_str() << "Warning: dark frame  " << opts.darkfilename <<
								" differs from the frame properties " << pADUdarkMat.rows << " vs " << pGryMat.rows << "rows, "
								<< pADUdarkMat.cols << " vs " << pGryMat.cols << " cols " << "\n";
							output_log.flush();
							darkfile_ok = 0;
						} else {
							cv::Mat pGryDarkMat;
							pGryDarkMat = cv::Mat(pGryMat.size(), pGryMat.type());
							cv::subtract(pGryMat, pADUdarkMat, pGryDarkMat);
							cv::threshold(pGryDarkMat, pGryMat, 0, 0, CV_THRESH_TOZERO);
							pGryDarkMat.release();
							pGryDarkMat = NULL;
						}
					}

					/*******************FIRST FRAME PROCESSING*******************/
					if (nframe == 1) {
						pGryMat.copyTo(pFirstFrameROIMat);
						pGryMat.convertTo(pGryMat, CV_8U);

						pFirstFrameROIMat = dtcApplyMask(pFirstFrameROIMat);
						//dtcApplyMaskToFrame(pFirstFrameROIMat);
						//pFirstFrameROIMat.copyTo(pFirstFrameROIMat, dtcGetMask(pFirstFrameROIMat));
						if (croi.x < 0) croi.x = 0;
						if (croi.y < 0) croi.y = 0;
						if (croi.x + croi.width > pFirstFrameROIMat.cols) croi.width = pFirstFrameROIMat.cols - croi.x;
						if (croi.y + croi.height > pFirstFrameROIMat.rows) croi.height = pFirstFrameROIMat.rows - croi.y;

						firstFrameCm.x = croi.x + croi.width / 2;
						firstFrameCm.y = croi.y + croi.height / 2;

						pFirstFrameROI = cv::Rect(croi);
						pFirstFrameROIMat = dtcReduceMatToROI(pGryMat, pFirstFrameROI);

						if (!opts.wait && (opts.viewROI || opts.viewTrk || opts.viewDif || opts.viewRef ||
							opts.viewThr || opts.viewSmo || opts.viewRes || opts.viewHis)) {
							if (fps_int > 0) {
								opts.wait = (int)(1000 / std::ceil(fps_int));
							} else {
								opts.wait = (int)(1000 / 25);
							}
						}

						nb_impact = 0;
						//init_list(&ptlist, (fps_int * opts.timeImpact));
						init_list(&ptlist, frame_number);
						init_dtc_struct(&dtc);
						init_dtc_struct(&outdtc);
						init_list(&impact, frame_number);
						init_list(&candidates, frame_number);

						pDifMat = cv::Mat(pFirstFrameROIMat.size(), CV_32F);
						pRefMat = cv::Mat(pFirstFrameROIMat.size(), CV_32F);

						pADUavgMat = cv::Mat::zeros(pFirstFrameROIMat.size(), CV_32F);
						pADUavgDiffMat = cv::Mat::zeros(pFirstFrameROIMat.size(), CV_32F);
						pADUmaxMat = cv::Mat::zeros(pFirstFrameROIMat.size(), CV_32F);
						if ((opts.ofilename) && (opts.allframes)) {
							pADUdtcMat = cv::Mat(pFirstFrameROIMat.size(), CV_32F);
							pADUavgMatFrame = cv::Mat(pFirstFrameROIMat.size(), CV_32F);
						}
						if (opts.thrWithMask || opts.viewMsk || (opts.ovfname && (opts.ovtype == OTYPE_MSK))) {
							pMskMat = cv::Mat(pFirstFrameROIMat.size(), CV_32F);
						}
						if (opts.viewThr) {
							pThrMat = cv::Mat(pFirstFrameROIMat.size(), CV_32F);
						}
						if (opts.filter.type >= 0 || opts.viewSmo) {
							pSmoMat = cv::Mat(pFirstFrameROIMat.size(), CV_32F);
						}
						if (opts.viewTrk || (opts.ovtype == OTYPE_TRK && opts.ovfname)) {
							pTrkMat = cv::Mat(pFrame.size(), CV_32F);
						}
						pAvgMat = cv::Mat(pFirstFrameROIMat.size(), CV_64F);

						firstFrameMean = cv::mean(pFirstFrameROIMat)[0];
						pFirstFrameROIMat.convertTo(pRefMat, CV_32F);
						cv::Rect bigROI = pFirstFrameROI + cv::Size(x_shift, y_shift);
						pROIMat = cv::Mat::zeros(bigROI.size(), pFirstFrameROIMat.type());
						tempGryMat = cv::Mat::zeros(pFirstFrameROI.size(), pFirstFrameROIMat.type());
					}
					/*******************EVERY FRAME PROCESSING*******************/
					pGryMat.convertTo(pGryMat, CV_8U);

					cv::Mat maskedGryMat = dtcApplyMask(pGryMat.clone());

					//pGryMat.copyTo(pGryMat, dtcGetMask(pGryMat.clone()));
					
					cm = dtcGetGrayMatCM(maskedGryMat);

					currentFrameMean = cv::mean(pGryMat)[0];
					
					//if ((cm.x <= 0) || (cm.y <= 0) || ((firstFrameMean / 10) > currentFrameMean)) {		
					//if ((cm.x <= 0) || (cm.y <= 0) || (currentFrameMean <= (0.2 * firstFrameMean))) {
					//if ((cm.x <= 0) || (cm.y <= 0) || (currentFrameMean == 0.0)) {
					//if ((cm.x <= 0) || (cm.y <= 0) || (currentFrameMean <= (0.1 * firstFrameMean))) {
					
					if ((cm.x <= 0) || (cm.y <= 0) || (currentFrameMean < 5.0)) {
						frame_errors++;
					} else {
						pFrameROI = dtcGetGrayImageROIcCM(maskedGryMat, cm, opts.medSize, opts.facSize, opts.secSize);

						pROI.x = cm.x - (pFirstFrameROI.width / 2);
						pROI.y = cm.y - (pFirstFrameROI.height / 2);
						pROI.width = pFirstFrameROI.width;
						pROI.height = pFirstFrameROI.height;

						int tlDeltaX = -x_shift * 3;
						int tlDeltaY = -y_shift * 3;

						int brDeltaX = pGryMat.cols + (x_shift * 3);
						int brDeltaY = pGryMat.rows + (y_shift * 3);

						if ((pROI.tl().x < tlDeltaX) || (pROI.tl().y < tlDeltaY) ||
							(pROI.br().x > brDeltaX) || (pROI.br().y > brDeltaY)) {
							frame_errors++;
						} else {
							cv::Point inflationPoint(-x_shift, -y_shift);
							cv::Size inflationSize(x_shift, y_shift);
							pFrameROI += inflationPoint;
							pFrameROI += inflationSize;
							pROI += inflationPoint;
							pROI += inflationSize;

							//pROIMat = cv::Mat::zeros(pROI.size(), pFirstFrameROIMat.type());
							pROIMat.setTo(cv::Scalar::all(0));

							if (pROI.x < 0) pROI.x = 0;
							if (pROI.y < 0) pROI.y = 0;
							if (pROI.width + pROI.x > pGryMat.cols) pROI.width = pGryMat.cols - pROI.x;
							if (pROI.height + pROI.y > pGryMat.rows) pROI.height = pGryMat.rows - pROI.y;

							tempROIMat = dtcReduceMatToROI(pGryMat, pROI);

							//DBOUT("tempROIMat size: " << tempROIMat.cols << "x" << tempROIMat.rows << "\n");

							tempCols = pROI.br().x > pGryMat.cols ? 0 : pROIMat.cols - tempROIMat.cols;
							tempRows = pROI.br().y > pGryMat.rows ? 0 : pROIMat.rows - tempROIMat.rows;
								
							tempROIMat.copyTo(pROIMat(cv::Rect(tempCols, tempRows,
								tempROIMat.cols, tempROIMat.rows)));
					
							//DBOUT("pROIMat size: " << pROIMat.cols << "x" << pROIMat.rows << "\n");

							if (pGryMat.type() != CV_32F)
								pGryMat.convertTo(pGryMat, CV_32F);
							if (pFirstFrameROIMat.type() != CV_32F)
								pFirstFrameROIMat.convertTo(pFirstFrameROIMat, CV_32F);
							if (pROIMat.type() != CV_32F)
								pROIMat.convertTo(pROIMat, CV_32F);

							//x and y are top left (tl) coordinates of the ROI
							roi = dtcCorrelateROI(pROIMat, pFirstFrameROIMat, pROI.tl(), pFirstFrameROI.size());
							pROI = roi;
							if (pROI.x < 0) pROI.x = 0;
							if (pROI.y < 0) pROI.y = 0;
							//if ((roi.x + roi.width > pFirstFrameROI.cols) )
							//tempGryMat = cv::Mat::zeros(roi.size(), pGryMat.type());
							tempGryMat.setTo(cv::Scalar::all(0));
							if (pROI.width + pROI.x > pGryMat.cols) pROI.width = pGryMat.cols - pROI.x;
							if (pROI.height + pROI.y > pGryMat.rows) pROI.height = pGryMat.rows - pROI.y;
							pGryMat = dtcReduceMatToROI(pGryMat, pROI);
							tempCols = pROI.br().x > pGryMat.cols ? 0 : pROIMat.cols - tempROIMat.cols;
							tempRows = pROI.br().y > pGryMat.rows ? 0 : pROIMat.rows - tempROIMat.rows;
//DBOUT("Frame          : " << nframe << "\n");
//DBOUT("x y            : " << tempCols << " " << tempRows << "\n");
//DBOUT("tempGryMat size: " << tempGryMat.cols << "x" << tempGryMat.rows << "\n");
//DBOUT("pGryMat size   : " << pGryMat.cols << "x" << pGryMat.rows << "\n");
//DBOUT("----------------\n");

/*** Following added to avoid writing outside of matrix size - algorithm correctness to be checked ? ***/
							tempCols = MIN(tempCols, tempGryMat.cols - pGryMat.cols);
							tempRows = MIN(tempRows, tempGryMat.rows - pGryMat.rows);

							pGryMat.copyTo(tempGryMat(cv::Rect(tempCols, tempRows, pGryMat.cols, pGryMat.rows)));
							
							tempGryMat.copyTo(pGryMat);

//DBOUT("pGryMat final size: " << pGryMat.cols << "x" << pGryMat.rows << "\n");
//DBOUT("pFirstFrameROIMat size: " << pFirstFrameROIMat.cols << "x" << pFirstFrameROIMat.rows << "\n");
						
							double similarity = dtcGetSimilarity(pFirstFrameROIMat, pGryMat)[0];			

							if (similarity <= 0.5) {
								DBOUT("Slightly wrong frame " << nframe << "\n");
							}

							/* Normalise image */
							pGryMat *= (firstFrameMean / cv::mean(pGryMat)[0]);
							pGryMat.convertTo(pGryMat, CV_32F);
							refFrameQueue.push(pGryMat);

							pDifMat = pGryMat - pRefMat;

							cv::Mat ifDif; // intelligent median
							cv::medianBlur(pDifMat, ifDif, 1);
							
							double maxDifVal = 0;

							cv::minMaxLoc(pDifMat, NULL, &maxDifVal, NULL, NULL);
							//DBOUT("Max of Dif mat: " << maxDifVal << "\n");

							//cv::imshow("Median blurred dif mask", ifDif);
							//cv::waitKey(1);

							//cv::threshold(ifDif, ifDif, 40.0, 0.0, cv::THRESH_TOZERO);

							cv::Mat ifMask = ifDif - pDifMat > 5;
							
							cv::Mat pDifMat2 = pDifMat.clone();
							//pDifMat2.copyTo(pDifMat, ifMask);

							ifMask.release();
							ifDif.release();
							pDifMat2.release();

							if (pDifMat.data) {
								if (opts.viewDif) {
									cv::minMaxLoc(pDifMat, &minLum, &maxLum, &minPoint, &maxPoint);
									pDifMat.convertTo(pDifImg, -1, 255.0 / maxLum, 0);
									pDifImg.convertTo(pDifImg, CV_8U);
									cv::imshow("Initial differential photometry", pDifImg);
									cv::waitKey(1);
									pDifImg.release();
									pDifImg = NULL;
								}
								if (nframe == opts.nsaveframe && opts.ofilename && opts.ostype == OTYPE_DIF) {
									cv::imwrite(opts.ofilename, pDifMat, img_save_params);
								}
							}


							//cv::threshold(pDifMat, pThrMat, opts.threshold, 0.0, CV_THRESH_TOZERO);
							//cv::threshold(pDifMat, pDifMat, opts.threshold, 0.0, CV_THRESH_TOZERO);

							if (opts.filter.type > 0) {
								switch (opts.filter.type) {
								case FILTER_BLUR:
									//Size 5x5
									cv::blur(pDifMat, pDifMat, cv::Size(opts.filter.param[0], opts.filter.param[0]));
									break;
								case FILTER_MEDIAN:
									//Size 5
									cv::medianBlur(pDifMat, pDifMat, opts.filter.param[0]);
									break;
								case FILTER_GAUSSIAN:
									//Size 5x5 Sigma 0
									cv::GaussianBlur(pDifMat, pDifMat, cv::Size(opts.filter.param[0],
										opts.filter.param[1]), opts.filter.param[2]);
									break;
								}
							}

							pDifMat.copyTo(pSmoMat);

							if (opts.viewSmo && pSmoMat.data) {
								//pSmoMat.convertTo(pSmoImg, CV_8U);
								cv::minMaxLoc(pSmoMat, &minLum, &maxLum, &minPoint, &maxPoint);
								pSmoMat.convertTo(pSmoImg, -1, 255.0 / maxLum, 0);
								pSmoImg.convertTo(pSmoImg, CV_8U);
								cv::imshow("Smoothed differential photometry", pSmoImg);
								cv::waitKey(1);
							}

							if (pMskMat.data) {
								cv::threshold(pDifMat, pMskMat, 0.0, 255.0, CV_THRESH_BINARY_INV);
							}

							if (opts.viewMsk && pMskMat.data) {
								//pMskMat.convertTo(pMskImg, CV_8U);
								cv::minMaxLoc(pMskMat, &minLum, &maxLum, &minPoint, &maxPoint);
								pMskMat.convertTo(pMskImg, -1, 255.0 / maxLum, 0);
								pMskImg.convertTo(pMskImg, CV_8U);
								cv::imshow("Mask", pMskImg);
								cv::waitKey(1);
							}
	
							//cv::blur(pDifMat, pDifMat, cv::Size(3,3));
							//cv::threshold(pDifMat, pDifMat, opts.threshold, 0.0, CV_THRESH_TOZERO);
	
							double mean = cv::mean(pDifMat)[0];
							totalMean += mean;
							double x = std::abs(double(maxLum) / double(mean)) - 1;
							if (x >= 0.7) {
								N++;
								xList.push_back(x);
							}
	
							/*ADUdtc algorithm******************************************/
							cv::add(pADUavgMat, pGryMat, pADUavgMat);
							cv::add(pADUavgDiffMat, pDifMat, pADUavgDiffMat);
							cv::max(pADUmaxMat, pGryMat, pADUmaxMat);
							if (opts.ofilename && opts.allframes) {
								pADUavgMat.convertTo(pADUavgMatFrame, -1, 1.0 / (nframe - frame_errors), 0);
								pADUavgDiffMat.convertTo(pADUavgDiffMat, -1, 1.0 / (nframe - frame_errors), 0);
								cv::subtract(pADUmaxMat, pADUavgMatFrame, pADUdtcMat);
								cv::minMaxLoc(pADUdtcMat, &minLum, &maxLum, &minPoint, &maxPoint);
								pADUdtcMat.convertTo(pADUdtcMat, -1, 255.0 / maxLum, 0);
								strncpy(ofilenamemax, opts.ofilename, strlen(opts.ofilename) - 4);
								ofilenamemax[std::strlen(opts.ofilename) - 4] = '\0';
								sprintf(ofilenamemax, "%s_dtc_max_frame%05d.jpg", ofilenamemax, nframe);
								std::strcat(max_folder_path_filename, right(ofilenamemax, strlen(ofilenamemax) - InRstr(ofilenamemax,
									"\\"), tmpstring));
								cv::imwrite(max_folder_path_filename, pADUdtcMat, img_save_params);
								cv::minMaxLoc(pADUdtcMat, &minLum, &maxLum, &minPoint, &maxPoint);
								pADUdtcMat.convertTo(pADUdtcMat, -1, 255.0 / maxLum, 0);
								strncpy(ofilenamediff, opts.ofilename, strlen(opts.ofilename) - 4);
								ofilenamediff[std::strlen(opts.ofilename) - 4] = '\0';
								sprintf(ofilenamediff, "%s_dtc_diff_frame%05d.jpg", ofilenamediff, nframe);
								std::strcat(diff_folder_path_filename, right(ofilenamediff, strlen(ofilenamediff) - 
									InRstr(ofilenamediff, "\\"), tmpstring));
								cv::minMaxLoc(pDifMat, &minLum, &maxLum, &minPoint, &maxPoint);
								pDifMat.convertTo(pDifImg, -1, 255.0 / maxLum, 0);
								pDifImg.convertTo(pDifImg, CV_8U);
								cv::imwrite(diff_folder_path_filename, pDifImg, img_save_params);
								pDifImg.release();
								pDifImg = NULL;
							}

							cv::threshold(pDifMat, pThrMat, opts.threshold, 0.0, CV_THRESH_TOZERO);
							cv::threshold(pDifMat, pDifMat, opts.threshold, 0.0, CV_THRESH_TOZERO);
	
							cv::minMaxLoc(pDifMat, &minLum, &maxLum, &minPoint, &maxPoint);
	
							if (opts.viewThr && pThrMat.data) {
								//pThrMat.convertTo(pThrImg, CV_8U);
								cv::minMaxLoc(pThrMat, &minLum, &maxLum, &minPoint, &maxPoint);
								pThrMat.convertTo(pThrImg, -1, 255.0 / maxLum, 0);
								pThrImg.convertTo(pThrImg, CV_8U);
								cv::imshow("Thresholded differential photometry", pThrImg);
								cv::waitKey(1);
							}

							if (opts.viewRef && pRefMat.data) {
								//pRefMat.convertTo(pRefImg, CV_8U);
								cv::minMaxLoc(pRefMat, &minLum, &maxLum, &minPoint, &maxPoint);
								pRefMat.convertTo(pRefImg, -1, 255.0 / maxLum, 0);
								pRefImg.convertTo(pRefImg, CV_8U);
								cv::imshow("Reference frame", pRefImg);
								cv::waitKey(1);
							}

							pMskMat.convertTo(pMskMat, CV_8U);
							if (nframe > 1) {
								if (nframe <= opts.nframesRef) {
									cv::accumulateWeighted(pGryMat, pRefMat, 1 / nframe, opts.thrWithMask ? pMskMat : cv::noArray());
								} else {
									cv::add(pRefMat, pGryMat / opts.nframesRef, pRefMat, opts.thrWithMask ? pMskMat : cv::noArray());
									cv::Mat frontMat = refFrameQueue.front();
									cv::subtract(pRefMat, frontMat / opts.nframesRef, pRefMat,
										opts.thrWithMask ? pMskMat : cv::noArray());
									frontMat.release();
									refFrameQueue.pop();
								}
							}
							if (pDifMat.data && opts.viewRes) {
								cv::minMaxLoc(pDifMat, &minLum, &maxLum, &minPoint, &maxPoint);
								pDifMat.convertTo(pDifImg, -1, 255.0 / maxLum, 0);
								pDifImg.convertTo(pDifImg, CV_8U);
								cv::imshow("Resulting differential photometry", pDifImg);
								cv::waitKey(1);
							}
							if (opts.viewHis || (opts.ovfname && (opts.ovtype == OTYPE_HIS))) {
								pHisImg = dtcGetHistogramImage(pDifMat, opts.histScale, opts.threshold);
								if (opts.viewHis) {
									cv::imshow("Histogram", pHisImg);
									cv::waitKey(1);
								}
							}

							cv::minMaxLoc(pDifMat, &minLum, &maxLum, &minPoint, &maxPoint);
							brightestPoints.push_back(create_item(create_point(nframe - frame_errors, maxLum, maxPoint.x, maxPoint.y)));
							maxPtB.push_back(maxLum);
							maxPtX.push_back(maxPoint.x);
							maxPtY.push_back(maxPoint.y);
							frameErrors.push_back(nframe - frame_errors);
							frameNumbers.push_back(nframe);
	
							if (opts.viewROI && pGryMat.data) {
								pGryMat.convertTo(pGryImg, CV_8U);
								cv::imshow("ROI", pGryImg);
								cv::waitKey(1);
							}
							if (pTrkMat.data && opts.viewTrk) {
								pFrame.copyTo(pTrkMat);
								pTrkMat.convertTo(pTrkMat, CV_8UC3);
								if (pFrame.channels() == 1)
									cv::cvtColor(pTrkMat, pTrkMat, CV_GRAY2BGR);
								Image img;
								img.frame = pTrkMat;
								img.roi = roi;
								dtcDrawCM(img, cm);
								cv::imshow("Tracking", img.frame);
								cv::waitKey(1);
							}
							if (opts.ovfname && opts.ovtype) {
								switch (opts.ovtype) {
								case OTYPE_DIF: pOVdMat = pDifMat; break;
								case OTYPE_TRK: pOVdMat = pTrkMat; break;
								case OTYPE_ROI: pOVdMat = pGryMat; break;
								case OTYPE_HIS: pOVdMat = pHisImg; break;
								case OTYPE_MSK: pOVdMat = pMskMat; break;
								}
								pWriter = dtcWriteVideo(opts.ovfname, *pWriter, pCapture, pOVdMat);
							}
							if (opts.wait && (cvWaitKey(opts.wait) == 27)) {
								break;
							}
							pGryImg_height = pGryMat.rows;
							pGryImg_width = pGryMat.cols;
						}
					}

				}

				CDeTeCtMFCDlg::getProgress()->StepIt();
				CDeTeCtMFCDlg::getProgress()->UpdateWindow();
			}

			/*********************************FINAL PROCESSING******************************************/
			refFrameQueue = std::queue<cv::Mat>();
			totalMean /= (nframe - frame_errors);

			CDeTeCtMFCDlg::getLog()->AddString((CString)getDateTime().str().c_str() +
				L"Differential photometry done, running impact detection...");
			CDeTeCtMFCDlg::getLog()->RedrawWindow();
			CDeTeCtMFCDlg::getLog()->SetTopIndex(CDeTeCtMFCDlg::getLog()->GetCount() - 1);
			output_log << getDateTime().str().c_str() << "Differential photometry done, running impact detection..." << "\n";
			output_log.flush();

			if (opts.darkfilename) {
				if (darkfile_ok == 1) {
					pADUdarkMat.release();
					pADUdarkMat = NULL;
				}
			}

			int radius = std::min(std::min(20.0, pGryMat.rows / 10.0), pGryMat.cols / 10.0);
			radius = radius > 5 ? radius : 5; //std::max gives error

//			CDeTeCtMFCDlg::getLog()->AddString((CString)getDateTime().str().c_str() + L"Running impact detection... ");
//			CDeTeCtMFCDlg::getLog()->SetTopIndex(CDeTeCtMFCDlg::getLog()->GetCount() - 1);
//			CDeTeCtMFCDlg::getLog()->RedrawWindow();
			output_log << getDateTime().str().c_str() << "Running impact detection..." << "\n";
			output_log.flush();

			/*if (fps_real == 0) {
				fps_real = double(nframe / duration);
				fps_int = (int)fps_real;
			}*/

			/*if (duration == 0) {
				duration = double(video_duration / 1000.0);
				if (fps_real == 0) {
					fps_real = double(nframe / duration);
					fps_int = (int)fps_real;
				}
			}*/

			ITEM* maxDtcImg = NULL; // Detection image
			ITEM* maxDtcImp = create_item(create_point(0, 0, 0, 0)); // Algorithm
			
			bMat = cv::Mat(cv::Size(1, nframe), CV_8UC1, maxPtB.data());
			//cv::medianBlur(bMat, bMat, 3);
			//maxPtB = bMat.data;

			/*for (int i = 0; i < nframe; i++) {
				add_tail_item(&ptlist, create_item(create_point(i + 1 - frameErrors[i], maxPtB[i], maxPtX[i], maxPtY[i])));
				maxList.push_back(maxPtB[i]);
			}*/

			std::ofstream output_csv(filename.append(".csv"));
			output_csv << "x,y,B\n";
			for (int i = 0; i < (nframe - frame_errors); i++) {
				add_tail_item(&ptlist, create_item(create_point(i + 1, maxPtB[i], maxPtX[i], maxPtY[i])));
				output_csv << (int) maxPtX[i] << "," << (int) maxPtY[i] << "," << (int) maxPtB[i] << "\n";
				//add_tail_item(&ptlist, create_item(create_point(frameErrors[i], maxPtB[i], maxPtX[i], maxPtY[i])));
				maxList.push_back(maxPtB[i]);
			}
			output_csv.close();

			double accum = 0, maxAccum = 0, maxBright = 0, maxMean = 0, brightness_factor = 0, brightnessFactor = 0,
				stdDevAccum = 0, stdev = 0;
			
			if (maxList.size() > 0) {

				accum = std::accumulate(xList.begin(), xList.end(), 0.0);
				maxAccum = std::accumulate(maxList.begin(), maxList.end(), 0.0);
				maxBright = (double) *(std::max_element(maxList.begin(), maxList.end()));
				maxMean = maxAccum / maxList.size();
				brightness_factor = (maxBright / (maxMean + opts.threshold)) - 1;
				//double brightness_factor = (maxBright / (maxMean + opts.threshold));
				//DBOUT("Brightness factor 1: " << brightness_factor << "\n");
				//brightness_factor = (maxBright / (maxMean + opts.threshold)) - (1 / opts.threshold);
				//DBOUT("Brightness factor 2: " << brightness_factor << "\n");
				brightnessFactor = maxMean / totalMean;
				stdDevAccum = 0.0;
				std::for_each(maxList.begin(), maxList.end(), [&](const double d) {
					stdDevAccum += (d - maxMean) * (d - maxMean);
				});

				stdev = sqrt(stdDevAccum / (maxList.size() - 1));
			}

			bMat.release();

			if (ptlist.size <= ptlist.maxsize && ptlist.size > opts.incrFrameImpact)
				nb_impact += detect_impact(&dtc, &outdtc, maxMean, &ptlist, &maxDtcImp, fps_int, radius, opts.incrLumImpact,
					opts.incrFrameImpact);

			delete_list(&ptlist);

			//(&outdtc)->nMaxFrame = frameNumbers[(&outdtc)->nMaxFrame];
			//(&outdtc)->nMinFrame = frameNumbers[(&outdtc)->nMinFrame];
			//(&outdtc)->MaxFrame = frameNumbers[(&outdtc)->MaxFrame];

			double impact_frames = (&outdtc)->nMaxFrame - (&outdtc)->nMinFrame;
			double log10_value = impact_frames != 0 ? std::log10((impact_frames / (opts.incrFrameImpact)) * 10) : 0;
			double confidence = (brightness_factor / opts.incrLumImpact) * log10_value;
			//double confidence = (stdev / opts.incrLumImpact) * log10_value;

			if (nframe > 0) {
				if (opts.ofilename && opts.allframes) {
					pADUdtcMat.release();
					pADUdtcMat = NULL;
					pADUavgMatFrame.release();
					pADUavgMatFrame = NULL;
					pADUavgDiffMat.release();
					pADUavgDiffMat = NULL;
				}
				if (opts.ovfname && opts.ovtype) {
					if (pWriter) {
						pWriter->release();
						pWriter = nullptr;
					}
				}

				/*ADUdtc algorithm******************************************/
				/*Mean image*/
				pADUavgMat.convertTo(pADUavgMat, CV_32F);
				pADUavgDiffMat.convertTo(pADUavgDiffMat, CV_32F);
				pADUmaxMat.convertTo(pADUmaxMat, CV_32F);
				//pGryMat.convertTo(pGryMat, CV_32FC1);
				pADUavgMat.convertTo(pADUavgMat, -1, 1.0 / (nframe - frame_errors), 0);
				cv::minMaxLoc(pADUavgMat, &minLum, &maxLum, &minPoint, &maxPoint); /*Max-mean image*/
				pADUdtcMat = cv::Mat(pGryImg_height, pGryImg_width, CV_32F);
				cv::subtract(pADUmaxMat, pADUavgMat, pADUdtcMat); /*Max-mean image*/
				//dtcApplyMaskToFrame(pADUdtcMat);
				pADUavgMat.convertTo(pADUavgMat, -1, 255.0 / maxLum, 0);
				pADUavgDiffMat.convertTo(pADUavgDiffMat, -1, 1.0 / (nframe - frame_errors), 0);
				cv::minMaxLoc(pADUavgDiffMat, &minLum, &maxLum, &minPoint, &maxPoint); /*Max-mean image*/
				pADUavgDiffMat.convertTo(pADUavgDiffMat, -1, 255.0 / maxLum, 0);

				snprintf(ofilenamemean, strlen(opts.ofilename) - 4, "%s", opts.ofilename);
				ofilenamemean[std::strlen(ofilenamemean)] = '\0';
				strcat(ofilenamemean, "_dtc_mean.jpg");
				/*Max-mean normalized image*/
				pADUavgMat.convertTo(pADUavgMat, CV_8U);
				//cv::imwrite(ofilenamemean, pADUavgMat);
				std::strcat(mean_folder_path_filename, right(ofilenamemean, strlen(ofilenamemean) - InRstr(ofilenamemean,
					"\\"), tmpstring));
				cv::imwrite(mean_folder_path_filename, pADUavgMat, img_save_params);
				if (opts.detail) {
					snprintf(ofilenamemean, strlen(opts.ofilename) - 4, "%s", opts.ofilename);
					ofilenamemean[std::strlen(ofilenamemean)] = '\0';
					strcat(ofilenamemean, "_dtc_mean.jpg");
					/*Max-mean normalized image*/
					//pADUavgMat.convertTo(pADUavgMat, CV_8U);
					cv::imwrite(ofilenamemean, pADUavgMat);
					std::strcat(mean_folder_path_filename, right(ofilenamemean, strlen(ofilenamemean) - InRstr(ofilenamemean,
						"\\"), tmpstring));
					cv::imwrite(mean_folder_path_filename, pADUavgMat);
					snprintf(ofilenamediffmean, strlen(opts.ofilename) - 4, "%s", opts.ofilename);
					ofilenamediffmean[std::strlen(ofilenamediffmean)] = '\0';
					strcat(ofilenamediffmean, "_dtc_diff_mean.jpg");
					/*Max-mean normalized image*/
					pADUavgDiffMat.convertTo(pADUavgDiffMat, CV_8U);
					//cv::imwrite(ofilenamemean, pADUavgDiffMat);
					std::strcat(diff_mean_folder_path_filename, right(ofilenamediffmean, strlen(ofilenamediffmean) -
						InRstr(ofilenamediffmean, "\\"), tmpstring));
					cv::imwrite(diff_mean_folder_path_filename, pADUavgDiffMat, img_save_params);
				}
				/*Max-mean image*/
				cv::medianBlur(pADUdtcMat, pSmoADUdtcMat, 3);
				cv::minMaxLoc(pSmoADUdtcMat, &minLum, &maxLum, &minPoint, &maxPoint);
				pSmoADUdtcMat.release();
				(ITEM*) (maxDtcImg) = create_item(create_point(0, 0, maxPoint.x, maxPoint.y));
				cv::minMaxLoc(pADUdtcMat, &minLum, &maxLum, &minPoint, &maxPoint);
				/*Max-mean normalized image*/
				pADUdtcMat.convertTo(pADUdtcMat, -1, 255.0 / maxLum, 0);
				double distance = sqrt(pow(maxDtcImg->point->x - maxDtcImp->point->x, 2) +
					pow(maxDtcImg->point->y - maxDtcImp->point->y, 2));
				snprintf(ofilenamenorm, strlen(opts.ofilename) - 4, "%s", opts.ofilename);
				ofilenamenorm[strlen(opts.ofilename) - 4] = '\0';
				strcat(ofilenamenorm, "_dtc_max-mean1.jpg");
				std::strcat(max_mean_folder_path_filename, right(ofilenamenorm, strlen(ofilenamenorm) - InRstr(ofilenamenorm,
					"\\"), tmpstring));
				pADUdtcMat.convertTo(pADUdtcImg, CV_8UC3);
				cv::cvtColor(pADUdtcImg, pADUdtcImg, CV_GRAY2BGR);

				if ((maxDtcImp->point->x != 0) && (maxDtcImp->point->y != 0))
					dtcDrawImpact(pADUdtcImg, cv::Point(maxDtcImp->point->x, maxDtcImp->point->y), CV_RGB(255, 0, 0));
				dtcDrawImpact(pADUdtcImg, cv::Point(maxDtcImg->point->x, maxDtcImg->point->y), CV_RGB(0, 255, 0));

				cv::imwrite(max_mean_folder_path_filename, pADUdtcImg, img_save_params);

				pADUdtcMat.convertTo(impactFrame, CV_8U);
				//double maxLum2;
				//cv::minMaxLoc(pADUdtcMat, &minLum, &maxLum2, &minPoint, &maxPoint);
				/*Max-mean non normalized image*/
				if (opts.detail) {
					if (maxLum > 255)
						pADUdtcMat.convertTo(pADUdtcMat, -1, maxLum / (255.0*255.0), 0);
					else
						pADUdtcMat.convertTo(pADUdtcMat, -1, maxLum / 255.0, 0);
					//cv::minMaxLoc(pADUdtcMat, &minLum, &maxLum2, &minPoint, &maxPoint);
					cv::Mat pADUdtcMatSmooth;
					if (pADUdtcMat.type() != CV_32F)
						pADUdtcMat.convertTo(pADUdtcMat, CV_32F);
					pADUdtcMat.copyTo(pADUdtcMatSmooth);
					cv::blur(pADUdtcMatSmooth, pADUdtcMatSmooth, cv::Size(5, 5));
					cv::minMaxLoc(pADUdtcMatSmooth, &minLum, &maxLum, &minPoint, &maxPoint);
					pADUdtcMatSmooth.release();
					pADUdtcMatSmooth = NULL;
					pADUavgMat.convertTo(pADUavgMat, CV_8U);
					pADUmaxMat.convertTo(pADUmaxMat, CV_8U);
					//pGryMat.convertTo(pGryMat, CV_8UC1);
					snprintf(ofilenamenorm, strlen(opts.ofilename) - 4, "%s", opts.ofilename);
					ofilenamenorm[strlen(opts.ofilename) - 4] = '\0';
					strcat(ofilenamenorm, "_dtc_max-mean2.jpg");
					std::strcat(max_mean2_folder_path_filename, right(ofilenamenorm, strlen(ofilenamenorm) - InRstr(ofilenamenorm,
						"\\"), tmpstring));

					pADUdtcMat.convertTo(pADUdtcImg2, CV_8UC3);
					cv::cvtColor(pADUdtcImg2, pADUdtcImg2, CV_GRAY2BGR);

					if ((maxDtcImp->point->x != 0) && (maxDtcImp->point->y != 0))
						dtcDrawImpact(pADUdtcImg2, cv::Point(maxDtcImp->point->x, maxDtcImp->point->y), CV_RGB(255, 0, 0));
					dtcDrawImpact(pADUdtcImg2, cv::Point(maxDtcImg->point->x, maxDtcImg->point->y), CV_RGB(0, 255, 0));
					cv::imwrite(max_mean2_folder_path_filename, pADUdtcImg2, img_save_params);
				}

				if (nb_impact > 0) {
					outdtc.nMinFrame = frameNumbers[outdtc.nMinFrame];
					outdtc.nMaxFrame = frameNumbers[outdtc.nMaxFrame];
					outdtc.MaxFrame = frameNumbers[outdtc.MaxFrame];
					if (distance <= 30) {
						if (!opts.ADUdtconly) {
							logmessage = std::to_string(nb_impact) + " potential impact(s) from frames " +
								std::to_string(outdtc.nMinFrame) + " to " + std::to_string(outdtc.nMaxFrame) + " (max @ " +
								std::to_string(outdtc.MaxFrame) + "). Confidence: " + std::to_string(confidence) + ".";
							CDeTeCtMFCDlg::getLog()->AddString((CString)getDateTime().str().c_str() +
								(CString)std::to_string(nb_impact).c_str() + L" potential impact(s) detected in frames ranging from " +
								(CString)std::to_string(outdtc.nMinFrame).c_str() + L" to " +
								(CString)std::to_string(outdtc.nMaxFrame).c_str() + L" (max @ " +
								(CString)std::to_string(outdtc.MaxFrame).c_str() + L"). Confidence: " +
								(CString)std::to_string(confidence).c_str() + L".");
							output_log << getDateTime().str().c_str() << nb_impact << " potential impact(s) detected in frames ranging from "
								<< outdtc.nMinFrame << " to " << outdtc.nMaxFrame << " (max @ " << outdtc.MaxFrame <<
								"). Confidence: " << confidence << ".\n";
							output_log.flush();
						} else {
							CDeTeCtMFCDlg::getLog()->AddString((CString)getDateTime().str().c_str() +
								(CString)std::to_string(nb_impact).c_str() + L" impacts detected.");
							logmessage = std::to_string(nb_impact) + " impacts detected.";
							output_log << getDateTime().str().c_str() << nb_impact << " impacts detected." << "\n";
							output_log.flush();
						}
					} else {
						confidence /= 4;
						if (!opts.ADUdtconly) {
							logmessage = std::to_string(nb_impact) + " potential impact(s) from frames " +
								std::to_string(outdtc.nMinFrame) + " to " + std::to_string(outdtc.nMaxFrame) + " (max @ " +
								std::to_string(outdtc.MaxFrame) + "). Confidence: " + std::to_string(confidence) +
								"\nWARNING: impact detection algorithm and detection images are inconsistent" +
								"\nPlease check detection image";
							CDeTeCtMFCDlg::getLog()->AddString((CString)getDateTime().str().c_str() +
								(CString)std::to_string(nb_impact).c_str() + L" potential impact(s) detected in frames ranging from " +
								(CString)std::to_string(outdtc.nMinFrame).c_str() + L" to " +
								(CString)std::to_string(outdtc.nMaxFrame).c_str() + L" (max @ " +
								(CString)std::to_string(outdtc.MaxFrame).c_str() + L"). Confidence: " +
								(CString)std::to_string(confidence).c_str() + L".");
							CDeTeCtMFCDlg::getLog()->AddString((CString)getDateTime().str().c_str() +
								+L" WARNING: impact detection algorithm "
								+ L"and detection images are inconsistent.");
							CDeTeCtMFCDlg::getLog()->AddString((CString)getDateTime().str().c_str() +
								+L" Please check detection image.");
							output_log << getDateTime().str().c_str() << nb_impact << " detected in frames ranging from "
								<< outdtc.nMinFrame << " to " << outdtc.nMaxFrame << " (max @ " << outdtc.MaxFrame <<
								"). Confidence: " << confidence << ".\n";
							output_log << getDateTime().str().c_str() << "WARNING: impact detection algorithm and detection "
								<< "images are inconsistent.\n";
							output_log << getDateTime().str().c_str() << "Please check detection image.\n";
							output_log.flush();
						} else {
							CDeTeCtMFCDlg::getLog()->AddString((CString)getDateTime().str().c_str() +
								(CString)std::to_string(nb_impact).c_str() + L" impacts detected " +
								L"with inconsistencies with the detection image.");
							CDeTeCtMFCDlg::getLog()->AddString((CString)getDateTime().str().c_str() +
								+L"WARNING: impact detection algorithm "
								+ L"and detection images are inconsistent.");
							CDeTeCtMFCDlg::getLog()->AddString((CString)getDateTime().str().c_str() +
								+L" Please check detection image.");
							logmessage = std::to_string(nb_impact) + " impacts detected" +
								"\nWARNING: impact detection algorithm and detection images are inconsistent" +
								"\nPlease check detection image";
							output_log << getDateTime().str().c_str() << nb_impact << " impacts detected\n";
							output_log << getDateTime().str().c_str() << nb_impact << " WARNING: impact detection algorithm "<<
								"and detection images are inconsistent.\n";
							output_log << getDateTime().str().c_str() << nb_impact << " Please check detection image.\n";
							output_log.flush();
						}
					}
				} else {
					CDeTeCtMFCDlg::getLog()->AddString((CString)getDateTime().str().c_str() + L"No impacts have been detected.");
					logmessage = "No impacts have been detected.";
					output_log << getDateTime().str().c_str() << "No impacts have been detected" << "\n";
					output_log.flush();
				}
				end = clock();
				log_messages.push_back(short_filename + ":");
				std::stringstream str(logmessage);
				std::string line;
				while (std::getline(str, line)) log_messages.push_back("    " + line);
				CDeTeCtMFCDlg::getLog()->AddString((CString)getDateTime().str().c_str() + L"Computation time: " +
					(CString)std::to_string(int(end - begin) / CLOCKS_PER_SEC).c_str() + " seconds," + L" showing detection image"
					+ L" (automatically closed in 4 seconds)...");
				CDeTeCtMFCDlg::getLog()->SetTopIndex(CDeTeCtMFCDlg::getLog()->GetCount() - 1);
				CDeTeCtMFCDlg::getLog()->RedrawWindow();
				output_log << getDateTime().str().c_str() << "Computation time: " << std::to_wstring(int(end - begin) / CLOCKS_PER_SEC)
					<< " seconds." << "\n";
				output_log << getDateTime().str().c_str() << "Showing detection image" << " (automatically closed in 4 seconds)...."
					<< "\n";
				output_log.flush();

				cv::imshow("Detection image", pADUdtcImg);
				cv::waitKey(4000);

				cv::destroyWindow("Detection image");

				pGryMat.release();
				pGryMat = NULL;

				pADUdarkMat.release();
				pADUdarkMat = NULL;
				pADUavgMat.release();
				pADUavgMat = NULL;
				pADUmaxMat.release();
				pADUmaxMat = NULL;
				pADUdtcMat.release();
				pADUdtcMat = NULL;
				pADUdtcImg.release();
				pADUdtcImg = NULL;
				pADUdtcImg2.release();
				pADUdtcImg2 = NULL;

				if (opts.ignore)
					dtcCorrectDatation((DtcCapture*)pCapture, &start_time, &end_time, &duration, &fps_real, &timetype, comment);
				std::string location = filename.substr(0, filename.find_last_of("\\") + 1);

				LogInfo info(opts.filename, start_time, end_time, duration, fps_real, timetype, comment, nb_impact, confidence);
				dtcWriteLog2(logcation, info);
				dtcWriteLog2(logcation2, info);

				/*FINAL CLEANING**************************************/
				if (opts.viewDif) cv::destroyWindow("Initial differential photometry");
				if (opts.viewRef) cv::destroyWindow("Reference frame");
				if (opts.viewROI) cv::destroyWindow("ROI");
				if (opts.viewTrk) cv::destroyWindow("Tracking");
				if (opts.viewMsk) cv::destroyWindow("Mask");
				if (opts.viewThr) cv::destroyWindow("Thresholded differential photometry");
				if (opts.viewSmo) cv::destroyWindow("Smoothed differential photometry");
				if (opts.viewRes) cv::destroyWindow("Resulting differential photometry");
				if (opts.viewHis) cv::destroyWindow("Histogram");

				if (opts.thrWithMask || opts.viewMsk || (opts.ovfname && (opts.ovtype == OTYPE_MSK))) {
					pMskMat.release();
					pMskMat = NULL;
					pMskImg.release();
					pMskImg = NULL;
				}
				if (opts.viewThr) {
					pThrMat.release();
					pThrMat = NULL;
					pThrImg.release();
					pThrImg = NULL;
				}
				if (opts.filter.type >= 0 || opts.viewSmo) {
					pSmoMat.release();
					pSmoMat = NULL;
					pSmoImg.release();
					pSmoImg = NULL;
				}
				if (opts.viewTrk || (opts.ovtype == OTYPE_TRK && opts.ovfname)) {
					pTrkMat.release();
					pTrkMat = NULL;
					pTrkImg.release();
					pTrkImg = NULL;
				}
				if (opts.viewDif || opts.viewRes || opts.viewHis || (opts.ovfname && (opts.ovtype == OTYPE_DIF ||
					opts.ovtype == OTYPE_HIS))) {
					pDifMat.release();
					pDifMat = NULL;
					pDifImg.release();
					pDifImg = NULL;
				}
				pRefMat.release();
				pRefMat = NULL;
				pRefImg.release();
				pRefImg = NULL;
				if (opts.viewHis || (opts.ovfname && (opts.ovtype == OTYPE_HIS))) {
					pHisImg.release();
					pHisImg = NULL;
				}
				pFirstFrameROIMat.release();
				pFirstFrameROIMat = NULL;
				pROIMat.release();
				pROIMat = NULL;
				tempROIMat.release();
				tempROIMat = NULL;
				tempGryMat.release();
				tempGryMat = NULL;
			}
			dtcReleaseCapture(pCapture);
			pCapture = NULL;
		}
		catch (std::exception& e) {
			std::string exception_message(e.what());
			output_log << getDateTime().str().c_str() << "ERROR: " << std::wstring(exception_message.begin(),
				exception_message.end()) << "\n";
			output_log.flush();
			CDeTeCtMFCDlg::getLog()->AddString((CString)getDateTime().str().c_str() + L"ERROR: ");
			CDeTeCtMFCDlg::getLog()->AddString((CString)getDateTime().str().c_str() + (CString)std::string(e.what()).c_str());
			CDeTeCtMFCDlg::getLog()->SetTopIndex(CDeTeCtMFCDlg::getLog()->GetCount() - 1);
			CDeTeCtMFCDlg::getLog()->RedrawWindow();
			logmessage = "ERROR: " + std::string(e.what());
			log_messages.push_back(short_filename + ":");
			log_messages.push_back("    " + logmessage);
		}
		message = "--------- " + short_filename + " analyzis done ---------";
		CDeTeCtMFCDlg::getLog()->AddString((CString)getDateTime().str().c_str() + (CString)message.c_str());
		output_log << getDateTime().str().c_str() << message.c_str() << "\n";
		CDeTeCtMFCDlg::getLog()->SetTopIndex(CDeTeCtMFCDlg::getLog()->GetCount() - 1);
		CDeTeCtMFCDlg::getLog()->RedrawWindow();
	}
	SendEmailDlg* email = new SendEmailDlg(NULL, log_messages);
	//CDeTeCtMFCDlg::getLog()->AddString((CString)getDateTime().str().c_str() + L"Analysis has been done");
	//output_log << getDateTime().str().c_str() << "Analysis has been done" << "\n";
	//dtcWriteWholeLog(logcation.c_str());
	//dtcWriteWholeLog(logcation.c_str(), logs);
	//dtcWriteWholeLog(logcation2.c_str(), logs);
	CDeTeCtMFCDlg::getLog()->AddString((CString)getDateTime().str().c_str() + L"Log file is available at " +
		(CString)logcation.c_str() + L"\\DeTeCt.log");
	CDeTeCtMFCDlg::getLog()->SetTopIndex(CDeTeCtMFCDlg::getLog()->GetCount() - 1);
	CDeTeCtMFCDlg::getLog()->RedrawWindow();
	output_log << getDateTime().str().c_str() << "Log file is available at " << logcation.c_str() << "\\DeTeCt.log" << "\n";
	output_log.close();
	std::wstring output_log_file2(scan_folder_path.begin(), scan_folder_path.end());
	output_log_file2 = output_log_file2.append(L"\\output.log");
	std::wofstream output_log2(output_log_file2.c_str());
	std::wifstream output_log_in(output_log_file.c_str());
	output_log2 << output_log_in.rdbuf();
	output_log2.close();
//DBOUT("Interactive=" << opts.interactive << "\n");
	if (opts.interactive) {
		email->DoModal();
	} else {
		CDeTeCtMFCDlg:dlg.OnFileExit();
		//m_pMainWnd->DestroyWindows();
	}
}