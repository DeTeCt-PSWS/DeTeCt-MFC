#define _WIN32_WINNT _WIN32_WINNT_WINXP

/********************************************************************************/
/*                                                                              */
/*			DTC	(c) Luis Calderon, Marc Delcroix, Jon Juaristi 2012-			*/
/*                                                                              */
/********************************************************************************/
#include "afxwin.h"
#include "processes_queue.h"
#include "dtcgui.hpp"
#include <windows.h> //after processes_queue.h
#include <string>
#include <vector>
#include <queue>
#include <iostream>
#include <ctime>
#include <numeric>
//#include <algorithm>
#include <regex>

#include "DeTeCt-MFCDlg.h"
#include <strsafe.h>

#include <shldisp.h>
#include <tlhelp32.h>
#include <stdio.h>

#include <direct.h>

/** @brief	Options for the algorithm */

OPTS opts;
char impact_detection_dirname[MAX_STRING];
char zip_detection_dirname[MAX_STRING];
char zip_detection_location[MAX_STRING];
char zipfile[MAX_STRING] = "";
char log_detection_dirname[MAX_STRING] = "";

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

//void read_files(std::string folder, std::vector<std::string> *file_list, std::vector<std::string> *acquisition_file_list) {
void read_files(std::string folder,  AcquisitionFilesList *acquisition_files) {
	DIR *directory;
	struct dirent *entry;
	std::string acquisition_file;
	//std::vector<int> nb_prealigned_frames = {};

	//        std::vector<std::string> non_supported_ext = { "bat", "exe", "log", "txt", "Jpg", "jpg", "Png", "png", "Tif", "tif",
	// "Bmp", "bmp", "Fit", "fit"};
	std::vector<std::string> supported_videoext = { "m4v", "avi", "ser", "wmv" };
	std::vector<std::string> supported_fileext = { "bmp", "jpg", "jpeg", "jp2", "dib", "png", "p?m", "sr", "ras", "tif",
		"tiff", "fit", "fits" };
	std::vector<std::string> supported_otherext = { AUTOSTAKKERT_EXT };
	// Syntax files:
	// F0.* *0000_*.* *_000000.*  *_000001.* *_00000.* *_00001.* *_0000.* *_0001.* *_0.tif nb1.*
	// supported 0/1 number inside filename
	std::vector<std::string> supported_filename_number = { "0000_", "0001_", "_000000.", "_000001.", "_00000.", 
		"_00001.", "_0000.", "_0001.", "_0.tif" };
	// supported 0/1 number syntax for full filename
	std::vector<std::string> supported_fullfilename_number = { "0000.", "0001.", "00000.", "00001.", "000000.", 
		"000001.", "F0.", "nb1." };

	// ignored dtc own files
	std::vector<std::string> not_supported_suffix = { DTC_MAX_MEAN_SUFFIX, DTC_MAX_MEAN1_SUFFIX, DTC_MAX_MEAN2_SUFFIX, DTC_MEAN_SUFFIX, DTC_MEAN2_SUFFIX,
		DTC_DIFF_SUFFIX, DTC_DIFF2_SUFFIX, VIDEOTEST_SUFFIX, DTC_MAX_SUFFIX, MEAN_SUFFIX, DTC_SUFFIX };



//std::wstring debug_log_file = L".\\debug.log";
//std::wofstream debug_log(debug_log_file.c_str(), std::ios_base::app);
//debug_log << "Reading directory..." << "\n";
//debug_log.flush();


	if (!(directory = opendir(folder.c_str()))) {
		return;
	}
	if (!(entry = readdir(directory))) {
		return;
	}
	do {
		if (entry->d_type == DT_DIR) {
			if (!strcmp(entry->d_name, ".") == 0 && !strcmp(entry->d_name, "..") == 0) {
				read_files(folder + "\\" + entry->d_name, acquisition_files);
			}
		}
		else {
			std::string file(entry->d_name);
			std::string extension = file.substr(file.find_last_of(".") + 1, file.length());
			std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower); //warning C4244
			if ((!IGNORE_DARK || ((folder.find(DARK_STRING) == std::string::npos) && (file.find(DARK_STRING) == std::string::npos))) &&
				(!IGNORE_PIPP || ((folder.find(PIPP_STRING) == std::string::npos) && (file.find(PIPP_STRING) == std::string::npos))) &&
				(!IGNORE_WJ_DEROTATION || ((folder.find(WJ_DEROT_STRING) == std::string::npos) && (file.find(WJ_DEROT_STRING) == std::string::npos)))) {
				acquisition_file = "";
				if (std::find(supported_videoext.begin(), supported_videoext.end(), extension) != supported_videoext.end()) {
					acquisition_files->file_list.push_back(folder + "\\" + entry->d_name);
					acquisition_file = folder + "\\" + entry->d_name;
					acquisition_files->acquisition_file_list.push_back(acquisition_file);
					acquisition_files->nb_prealigned_frames.push_back(0);
				}
				else if (std::find(supported_fileext.begin(), supported_fileext.end(), extension) != supported_fileext.end()) {
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
						acquisition_files->file_list.push_back(folder + "\\" + entry->d_name);
						acquisition_file = folder + "\\" + entry->d_name;
						acquisition_files->acquisition_file_list.push_back(acquisition_file);
						acquisition_files->nb_prealigned_frames.push_back(0);
					}
				}
				else if (std::find(supported_otherext.begin(), supported_otherext.end(), extension) != supported_otherext.end()) {
					if (extension.compare(AUTOSTAKKERT_EXT) == 0) {
						//std::vector<cv::Point> cm_list;
						int cm_list_start = 0;
						int cm_list_end = 9999999;
						int cm_frame_count = 0;

						read_autostakkert_file(folder + "\\" + file, &acquisition_file, NULL, &cm_list_start, &cm_list_end, &cm_frame_count);

						if (acquisition_file.length() > 0) {
							acquisition_files->file_list.push_back(folder + "\\" + entry->d_name);
							acquisition_files->acquisition_file_list.push_back(acquisition_file);
							acquisition_files->nb_prealigned_frames.push_back(MIN(cm_list_end - cm_list_start+1, cm_frame_count));
						}
					}
					else {
						acquisition_files->file_list.push_back(folder + "\\" + entry->d_name);
						acquisition_file = folder + "\\" + entry->d_name;
						acquisition_files->acquisition_file_list.push_back(acquisition_file);
						acquisition_files->nb_prealigned_frames.push_back(0);
					}
				}
//if (acquisition_file.length() > 0) debug_log << "file " << folder.c_str() << "\\" << entry->d_name << "     " << acquisition_file.c_str() << "\n";
			}
		}
	} while (entry = readdir(directory));
	closedir(directory);
//Remove duplicates from as3 (keeping as3)
	acquisition_files->file_list.begin();
	acquisition_files->acquisition_file_list.begin();
	acquisition_files->nb_prealigned_frames.begin();
	for (int i = 0; i < acquisition_files->file_list.size(); i++) {
		std::string file = acquisition_files->file_list.at(i);
		std::string extension = file.substr(file.find_last_of(".") + 1, file.length());
		if (extension.compare(AUTOSTAKKERT_EXT) == 0) {
			int j = 0;
			if (j == i) j++;
			while ((j < acquisition_files->acquisition_file_list.size()) && (acquisition_files->acquisition_file_list.at(j) != acquisition_files->acquisition_file_list.at(i))) {
				j++;
				if (j == i) j++;
			}
			// Erases duplicates
			if (j < acquisition_files->acquisition_file_list.size()) {
				// Erases duplicates with less prealigned_frames from as3
				if (acquisition_files->nb_prealigned_frames.at(j) < acquisition_files->nb_prealigned_frames.at(i)) {
					DBOUT("Erasing " << acquisition_files->acquisition_file_list.at(j).c_str());
					acquisition_files->file_list.erase(acquisition_files->file_list.begin() + j);
					acquisition_files->acquisition_file_list.erase(acquisition_files->acquisition_file_list.begin() + j);
					acquisition_files->nb_prealigned_frames.erase(acquisition_files->nb_prealigned_frames.begin() + j);
					if (j < i) i--;
				}
				else {
					DBOUT("Erasing " << acquisition_files->acquisition_file_list.at(i).c_str());
					acquisition_files->file_list.erase(acquisition_files->file_list.begin() + i);
					acquisition_files->acquisition_file_list.erase(acquisition_files->acquisition_file_list.begin() + i);
					acquisition_files->nb_prealigned_frames.erase(acquisition_files->nb_prealigned_frames.begin() + i);
					if (i>0) i--;
				}
			}
		}
	}
//debug_log._close();
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

int detect_impact(DTCIMPACT *dtc, DTCIMPACT *dtcout, double meanValue, LIST *list, ITEM** dtcMax, double fps, double radius, double incrLum, int impact_frames_min)
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
	//int impact_frame_num = incrFrame;
	
	int impact_frame_num;

	/* use of minimum impact frames or minimum impact time */
	impact_frame_num = impact_frames_min;

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
				lastivalFrame = (int)potential_impact.size();
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
	if (lastivalFrame >= impact_frames_min) {
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
 * @param	current_file_list	List of files.
 * @param	opts	 	The options for the algorithm execution
 *
 * @return	An integer which is unused.
 **************************************************************************************************/

int detect(std::vector<std::string> current_file_list, OPTS *opts, std::string scan_folder_path) {
	clock_t begin, end;
	const int wait_seconds = 3;

	cv::setUseOptimized(true);
	std::string logcation(scan_folder_path);
	std::string logcation2(scan_folder_path);
	std::stringstream logline_tmp;

	std::string start_runtime = getRunTime().str().c_str();
	std::wstring wstart_time = std::wstring(start_runtime.begin(), start_runtime.end());
	logcation2.append("\\Impact_detection_run@").append(start_runtime);
	std::vector<int> img_save_params = { CV_IMWRITE_JPEG_QUALITY, 100 };
	
	std::wstring detection_folder_path = {};
	std::wstring detection_folder_name = {};
	std::wstring details_folder_path = {};

	char max_folder_path_filename[MAX_STRING];
	char diff_folder_path_filename[MAX_STRING];
	char tmpstring[MAX_STRING];
	DIR *dir_tmp;

	detection_folder_path = std::wstring(scan_folder_path.begin(), scan_folder_path.end());
	detection_folder_path = detection_folder_path.append(L"\\Impact_detection_run@").append(wstart_time);
	std::string detection_folder_path_string(detection_folder_path.begin(), detection_folder_path.end());
	detection_folder_name = detection_folder_name.append(L"Impact_detection_run@").append(wstart_time);
	std::string detection_folder_name_string(detection_folder_name.begin(), detection_folder_name.end());
	strcpy(opts->impactdirname, detection_folder_path_string.c_str());
	strcpy(impact_detection_dirname, detection_folder_path_string.c_str());

	// usage of mkdir only solution found to handle directory names with special characters (eg. é, à, ...)
		//if (GetFileAttributes(detection_folder_path.c_str()) == INVALID_FILE_ATTRIBUTES)
		//CreateDirectory(detection_folder_path.c_str(), 0);
	if (!(dir_tmp = opendir(detection_folder_path_string.c_str()))) mkdir(detection_folder_path_string.c_str());
	else closedir(dir_tmp);
	if (opts->detail || opts->allframes) {
		details_folder_path = std::wstring(detection_folder_path.begin(), detection_folder_path.end());
		details_folder_path = details_folder_path.append(L"\\details");
		std::string details_folder_path_string(details_folder_path.begin(), details_folder_path.end());
		// usage of mkdir only solution found to handle directory names with special characters (eg. é, à, ...)
			//if (GetFileAttributes(details_folder_path.c_str()) == INVALID_FILE_ATTRIBUTES)
			//CreateDirectory(details_folder_path.c_str(), 0);
		if (!(dir_tmp = opendir(detection_folder_path_string.c_str()))) mkdir(detection_folder_path_string.c_str());
		else closedir(dir_tmp);
	}

	dtcWriteLogHeader(logcation);
	dtcWriteLogHeader(logcation2);

	std::wstring output_log_file(scan_folder_path.begin(), scan_folder_path.end());
	output_log_file = output_log_file.append(L"\\Impact_detection_run@").append(wstart_time).append(L"\\").append(OUTPUT_FILENAME).append(DTC_LOG_SUFFIX);
	std::wofstream output_log(output_log_file.c_str(), std::ios_base::app);

	std::vector<LogInfo> logs;
	std::vector<LPCTSTR> logMessages;
	std::vector<std::string> log_messages;	// For SendMailDlg
	
	//log_messages.push_back("");
	if (opts->dateonly) log_messages.push_back("WARNING, datation info only, no detection analysis was performed");

	std::string logmessage;
	std::string logmessage2;
	std::string logmessage3;
	std::wstring wlogmessage;
	CString Clogmessage;

	AcquisitionFilesList local_acquisition_files_list;
	
	local_acquisition_files_list.file_list = current_file_list;
	local_acquisition_files_list.acquisition_file_list = current_file_list;
	local_acquisition_files_list.nb_prealigned_frames = {};
	
	int acquisition_index = 0;
	double duration_total = 0;
	double computation_time_total = 0;
	double start_time_min = gregorian_calendar_to_jd(2080, 1, 1, 0, 0, 0);
	double start_time_max = gregorian_calendar_to_jd(1980, 1, 1, 0, 0, 0);
	double JD_min = gregorian_calendar_to_jd(1980, 1, 1, 0, 0, 0);
	Planet_type planet;
	int planet_jupiter = 0;
	int planet_saturn = 0;
	int nb_null_impact = 0;
	int nb_low_impact = 0;
	int nb_high_impact = 0;

	//std::vector<std::string> local_file_list;
	//local_file_list = file_list;
	do
	{
			//for (std::string filename : local_file_list) {
		for (std::string filename : local_acquisition_files_list.acquisition_file_list) {
//***** gets acquisition file from autostakkert session file
			acquisition_index++;
			std::vector<cv::Point> cm_list = {};
			int cm_list_start = 0;
			int cm_list_end = 9999999;
			int cm_frame_count = 0;

			std::string extension = filename.substr(filename.find_last_of(".")+1, filename.size()-filename.find_last_of(".")-1);
			if (extension.compare(AUTOSTAKKERT_EXT) == 0) {
				std::string filename_acquisition;

				read_autostakkert_file(filename, &filename_acquisition, &cm_list, &cm_list_start, &cm_list_end, &cm_frame_count);
				filename = filename_acquisition;
				local_acquisition_files_list.nb_prealigned_frames.push_back(MIN(cm_list_end - cm_list_start + 1, cm_frame_count));
			}
			else {
				local_acquisition_files_list.nb_prealigned_frames.push_back(cm_frame_count);
			}
//***** if option noreprocessing on, check in detect log file if file already processed or processed with in datation only mode
			BOOL process = TRUE;
			if (!opts->reprocessing) {
				CT2A DeTeCtLogFilename(DeTeCt_additional_filename(logcation.c_str(), DTC_LOG_SUFFIX));
				std::ifstream input_file(DeTeCtLogFilename, std::ios_base::in);
				if (input_file) {
					std::string line;
	//0.0000		 0       ; 2015/01/17 04:42,059300 UT; 2015/01/17 04:44,058567 UT; 2015/01/17 04:43,058933 UT; 119.9560 s; 43.000 fr/s; G:\Work\Impact\tests\data_set\bugs\ACo\J_2015Jan17_044203_RGB.avi; DeTeCt v3.1.8.20190512_x64 (Firecapture 2.4beta); Win8(or_above)_64b
					while ((std::getline(input_file, line)) && (process == TRUE)) {
						if ((!starts_with(line, "DeTeCt")) && (!starts_with(line, "PLEASE")) && (!starts_with(line, "confidence"))) {
							BOOL line_rated;
							if (starts_with(line, "N/A")) line_rated = FALSE;
							else line_rated = TRUE;
							int nb_separators = 0;
							int pos_separator;
							do {
								pos_separator = (int)line.find_first_of(";");
								if (pos_separator != std::string::npos) {
									nb_separators++;
									line = line.substr(pos_separator + 1, line.size() - pos_separator - 1);
								}
							} while ((pos_separator != std::string::npos) && (nb_separators < 6));
							while (starts_with(line, " ")) line = line.substr(1, line.size() - 1);
							while (starts_with(line, "\t")) line = line.substr(1, line.size() - 1);
							if ((nb_separators == 6)
								&& (line.find_first_of(";") != std::string::npos)
									&& (starts_with(line, filename))
										&& (!opts->dateonly)
											&& (line_rated)) process = FALSE;
						}
					}
					input_file._close();
				}
			}
			opts->filename = strdup(filename.c_str());
			std::string outputFolder = filename.substr(0, filename.find_last_of("\\") + 1);
			outputFolder = outputFolder.replace(0, scan_folder_path.length() + 1, "");
			std::replace(outputFolder.begin(), outputFolder.end(), '\\', '_');
			std::replace(outputFolder.begin(), outputFolder.end(), ' ', '_');
			std::string folderPath = filename.substr(0, filename.find_last_of("\\") + 1);
			std::string filePath = filename.substr(filename.find_last_of("\\") + 1, filename.find_last_of("."));
			filePath = filePath.substr(0, filePath.find_last_of("."));
			std::string outputfilename = folderPath.append(outputFolder).append(filePath).append(".jpg");
			opts->ofilename = strdup(outputfilename.c_str());
			std::wstring fname(filename.begin(), filename.end());
			std::string short_filename = filename.substr(filename.find_last_of("\\") + 1, filename.length());

			if (process) {
				std::string message  = "----- " + std::to_string(acquisition_index) + "/" + std::to_string(local_acquisition_files_list.acquisition_file_list.size()) + " : " + short_filename + " start --------------";
				std::string message2 = "----- " + std::to_string(acquisition_index) + "/" + std::to_string(local_acquisition_files_list.acquisition_file_list.size()) + " : " + filename + " start  --------------";
				//TODO: usage of cmlist and quality information from as3

				//impactDetectionLog.AddString((CString)getDateTime().str().c_str() + ss2.str().c_str());
				CDeTeCtMFCDlg::getLog()->SetTopIndex(CDeTeCtMFCDlg::getLog()->GetCount() - 1);
				CDeTeCtMFCDlg::getLog()->AddString((CString)getDateTime().str().c_str() + (CString)message.c_str());
				output_log << getDateTime().str().c_str() << message2.c_str() << "\n";
				output_log.flush();

				init_string(tmpstring);
				std::string detail_folder_path_string(details_folder_path.begin(), details_folder_path.end());


				int fps_int = 0;
				double fps_real = 0;
				int impact_frames_min;
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
				cv::Mat pADUavgMat2; // ADU average frame
				cv::Mat pADUmaxMat; // ADU max frame
				cv::Mat pADUdtcMat; // ADU detect frame
				cv::Mat pSmoADUdtcMat; // ADU detect frame (smoothed)
				cv::Mat pADUavgDiffMat; // ADU average difference frame
				cv::Mat pADUavgDiffMat2; // ADU average difference frame
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
				char ofilenamediff[MAX_STRING] = "";
				char ofilenamemax[MAX_STRING] = "";

				char comment[MAX_STRING] = "";
				char rating_classification[MAX_STRING] = "";
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
					//***** Opens acquisition file
					if (!(pCapture = dtcCaptureFromFile2(opts->filename, &framecount))) {
						CDeTeCtMFCDlg::getLog()->AddString((CString)getDateTime().str().c_str() + L"Cannot open file " +
							(CString)opts->filename);
						CDeTeCtMFCDlg::getLog()->SetTopIndex(CDeTeCtMFCDlg::getLog()->GetCount() - 1);
						CDeTeCtMFCDlg::getLog()->RedrawWindow();
						output_log << getDateTime().str().c_str() << "Cannot open file " << opts->filename << "\n";
						output_log.flush();
						continue;
					}
					switch (pCapture->type) {
					case CAPTURE_SER:
						nframe = (int)pCapture->u.sercapture->header.FrameCount;
						break;
					case CAPTURE_FITS:
					case CAPTURE_FILES:
						nframe = (int) pCapture->u.filecapture->FrameCount;
						break;
					default: // CAPTURE_CV
						nframe = (int)(dtcGetCaptureProperty(pCapture, CV_CAP_PROP_FRAME_COUNT));
					}
					frame_number = nframe;
					CDeTeCtMFCDlg::getProgress()->SetRange(0, (short)nframe);
					CDeTeCtMFCDlg::getProgress()->SetPos(0);
					//***** Checks if acquisition has a minimum number of frames
					if ((nframe > 0) && (nframe < opts->minframes)) {
						logmessage = "INFO: only " + std::to_string(nframe) + " ";
						if (nframe == 1)  logmessage = logmessage + "frame";
						else logmessage = logmessage + "frames";
						logmessage = logmessage + " (minimum is " + std::to_string(opts->minframes) +
							"), stopping processing\n";

						CDeTeCtMFCDlg::getLog()->AddString((CString)getDateTime().str().c_str() + logmessage.c_str());
						CDeTeCtMFCDlg::getLog()->SetTopIndex(CDeTeCtMFCDlg::getLog()->GetCount() - 1);
						CDeTeCtMFCDlg::getLog()->RedrawWindow();
						output_log << getDateTime().str().c_str() << logmessage.c_str();
						output_log.flush();
						sprintf(rating_classification, "Unknown      ");
						dtcReleaseCapture(pCapture);
						pCapture = NULL;

						if (!opts->dateonly) {
							log_messages.push_back(short_filename + ":" + "    " + logmessage);
							// log_messages.push_back("    " + logmessage);
						}
						continue;
					}
					//***** Gets datation info from acquisition
					dtcGetDatation(pCapture, opts->filename, nframe, &start_time, &end_time, &duration, &fps_real, &timetype, comment, &planet);

					if (planet == Jupiter) planet_jupiter++;
					else if (planet == Saturn) planet_saturn++;

					if (start_time > JD_min) { 	/* for renaming logfile in impact_detection directory */
						if (start_time < start_time_min) start_time_min = start_time;
						if (start_time > start_time_max) start_time_max = start_time;
					}

					duration_total += duration;
					double fps = fps_real;
					if (fps < 0.02)	fps = dtcGetCaptureProperty(pCapture, CV_CAP_PROP_FPS);
						fps_int = (int)fps;
					impact_frames_min = (int)ceil(MAX(opts->incrFrameImpact, fps * opts->impact_duration_min));
					/*********************************DATE ONLY MODE******************************************/
					if (opts->dateonly) {
						CDeTeCtMFCDlg::getLog()->AddString((CString)getDateTime().str().c_str() + L"Datation for capture of " +
							(CString)std::to_string(nframe).c_str() + L" frames @ " + (CString)std::to_string(fps_int).c_str() + L" fps");
						CDeTeCtMFCDlg::getLog()->SetTopIndex(CDeTeCtMFCDlg::getLog()->GetCount() - 1);
						CDeTeCtMFCDlg::getLog()->RedrawWindow();
						output_log << getDateTime().str().c_str() << "Datation for capture of  " << nframe << " frames @ " << fps_int
							<< " fps" << "\n";
						output_log.flush();
						message = "-------------- " + short_filename + " end --------------";
						CDeTeCtMFCDlg::getLog()->AddString((CString)getDateTime().str().c_str() + (CString)message.c_str());
						output_log << getDateTime().str().c_str() << message.c_str() << "\n";
						double fake_stat[3] = { 0.0, 0.0, 0.0 };
						//LogInfo info(opts->filename, start_time, end_time, duration, fps_real, timetype, "WARNING, datation info only", 0, 0);
						LogInfo info(opts->filename, start_time, end_time, duration, fps_real, timetype, comment, 0, 0, 0, fake_stat, fake_stat, fake_stat, fake_stat, fake_stat, fake_stat, rating_classification);
						
						std::stringstream logline;
						dtcWriteLog2(logcation, info, &logline);
						log_messages.push_back(logline.str() + "\n");
						dtcWriteLog2(logcation2, info, &logline_tmp);
						dtcReleaseCapture(pCapture);
						pCapture = NULL;
						continue;
					}

					//****************** NON DATE ONLY MODE *******************************/
					message = "Initializing capture: " + std::to_string(nframe) + " frames @ " + std::to_string(fps_int) + " (" + std::to_string((int)duration) + "s duration)";
/*					CDeTeCtMFCDlg::getLog()->AddString((CString)getDateTime().str().c_str() + L"Initializing capture: " +
						(CString)std::to_string(nframe).c_str() + L" frames @ " + (CString)std::to_string(fps_int).c_str() + L" fps" +
						L" (" + (CString)std::to_string(impact_frames_min).c_str() + L" frames min for impact)");*/
					CDeTeCtMFCDlg::getLog()->AddString((CString)getDateTime().str().c_str() + (CString)message.c_str());
					CDeTeCtMFCDlg::getLog()->SetTopIndex(CDeTeCtMFCDlg::getLog()->GetCount() - 1);
					CDeTeCtMFCDlg::getLog()->RedrawWindow();
/*					output_log << getDateTime().str().c_str() << "Initializing capture:  " << nframe << " frames @ " << fps_int
						<< " fps (" << impact_frames_min << " frames min for impact)\n";*/
					output_log << getDateTime().str().c_str() << message.c_str() << "\n";
					output_log.flush();
					//Gets ROI, check if ROI is not big enough and exit then
					if (opts->wROI && opts->hROI) {
						croi = cv::Rect(0, 0, opts->wROI, opts->hROI);
					}
					else {
						croi = dtcGetFileROIcCM(pCapture, opts->ignore);
						dtcReinitCaptureRead2(&pCapture, opts->filename);
						if ((croi.width <= opts->ROI_min_size) || (croi.height <= opts->ROI_min_size)) {
							message = "-------------- " + short_filename + " end --------------";
							CDeTeCtMFCDlg::getLog()->AddString((CString)getDateTime().str().c_str() + (CString)message.c_str());
							output_log << getDateTime().str().c_str() << message.c_str() << "\n";

							CDeTeCtMFCDlg::getLog()->AddString((CString)getDateTime().str().c_str() + L"WARNING: ROI " +
								(CString)std::to_string(croi.width).c_str() + L"x" + (CString)std::to_string(croi.height).c_str() + L" to small (" +
								(CString)std::to_string(opts->ROI_min_size).c_str() + L"x" + (CString)std::to_string(opts->ROI_min_size).c_str() + L"), ignoring stopping processing");

							//message = "WARNING: ROI " + croi.x + "x" + croi.y + " to small (" + ROI_MIN + "x" + ROI_MIN + "), ignoring stopping processing";
							//CDeTeCtMFCDlg::getLog()->AddString((CString)getDateTime().str().c_str() + (CString)message.c_str());
							output_log << getDateTime().str().c_str() << "WARNING: ROI " << croi.width << "x" << croi.height << " to small (" << opts->ROI_min_size << "x" << opts->ROI_min_size << "), ignoring stopping processing" << "\n";
							dtcReleaseCapture(pCapture);
							pCapture = NULL;
							continue;
						}
					}

					if (opts->viewDif) cv::namedWindow("Initial differential photometry");
					if (opts->viewRef) cv::namedWindow("Reference frame");
					if (opts->viewROI) cv::namedWindow("ROI");
					if (opts->viewTrk) cv::namedWindow("Tracking");
					if (opts->viewMsk) cv::namedWindow("Mask");
					if (opts->viewThr) cv::namedWindow("Thresholded differential photometry");
					if (opts->viewSmo) cv::namedWindow("Smoothed differential photometry");
					if (opts->viewRes) cv::namedWindow("Resulting differential photometry");
					if (opts->viewHis) cv::namedWindow("Histogram");

					nframe = 0;
//Process dark file if existing, but not for Winjupos derotated files and PIPP files as a regular dark file would not be suitable if the images have been modified
					if ((opts->darkfilename) && (InStr(opts->filename, WJ_DEROT_STRING) < 0) && (InStr(opts->filename, PIPP_STRING) < 0)) {
						char darklongfilename[MAX_STRING];
						strncpy(darklongfilename,opts->filename,InRstr(opts->filename,"\\")+1);
						strcat(darklongfilename, opts->darkfilename);
						if (!(pADUdarkMat = cv::imread(darklongfilename, CV_LOAD_IMAGE_GRAYSCALE)).data) {
							/*CDeTeCtMFCDlg::getLog()->AddString((CString)getDateTime().str().c_str() + L"Warning: cannot read dark frame " +
								(CString)std::string(darklongfilename).c_str());
							CDeTeCtMFCDlg::getLog()->SetTopIndex(CDeTeCtMFCDlg::getLog()->GetCount() - 1);
							CDeTeCtMFCDlg::getLog()->RedrawWindow();
							output_log << getDateTime().str().c_str() << "Warning: cannot read dark frame:  " << darklongfilename << "\n";
							output_log.flush();*/
							darkfile_ok = 0;
						}
						else {
							CDeTeCtMFCDlg::getLog()->AddString((CString)getDateTime().str().c_str() + L"Reading dark frame " +
								(CString)std::string(darklongfilename).c_str());
							CDeTeCtMFCDlg::getLog()->SetTopIndex(CDeTeCtMFCDlg::getLog()->GetCount() - 1);
							CDeTeCtMFCDlg::getLog()->RedrawWindow();
							output_log << getDateTime().str().c_str() << "Reading dark frame:  " << darklongfilename << "\n";
							output_log.flush();
							darkfile_ok = 1;
						}
					}

					/*********************************CAPTURE READING******************************************/
 					while ((pFrame = dtcQueryFrame2(pCapture, opts->ignore, &frame_error)).data) {
//				cv::imshow("Frame read 1", pFrame);
//				cv::waitKey(0);
						cv::medianBlur(pFrame, pFrame, 3);
						video_duration += (int)dtcGetCaptureProperty(pCapture, CV_CAP_PROP_POS_MSEC);
						nframe++;
						if (!(frame_error) == 0) {
							frame_errors += 1;
						}
						else {
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
							//cv::imshow("Frame read 2", pGryMat);
							//cv::waitKey(0);
														//dtcApplyMaskToFrame(pGryMat);
														//cv::GaussianBlur(pGryMat, pGryMat, cv::Size(1, 1), 1);
							if (darkfile_ok == 1) {
								if ((pADUdarkMat.rows != pGryMat.rows) || (pADUdarkMat.cols != pGryMat.cols)) {
									CDeTeCtMFCDlg::getLog()->AddString((CString)getDateTime().str().c_str() + L"Warning: dark frame " +
										(CString)std::string(opts->darkfilename).c_str() + L" differs from the frame properties " +
										(CString)std::to_string(pADUdarkMat.rows).c_str() + L" vs " +
										(CString)std::to_string(pGryMat.rows).c_str() + L" rows, " +
										(CString)std::to_string(pADUdarkMat.cols).c_str() + L" vs " +
										(CString)std::to_string(pGryMat.cols).c_str() + L" cols");
									CDeTeCtMFCDlg::getLog()->SetTopIndex(CDeTeCtMFCDlg::getLog()->GetCount() - 1);
									CDeTeCtMFCDlg::getLog()->RedrawWindow();
									output_log << getDateTime().str().c_str() << "Warning: dark frame  " << opts->darkfilename <<
										" differs from the frame properties " << pADUdarkMat.rows << " vs " << pGryMat.rows << "rows, "
										<< pADUdarkMat.cols << " vs " << pGryMat.cols << " cols " << "\n";
									output_log.flush();
									darkfile_ok = 0;
								}
								else {
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
								//cv::imshow("Frame read 3", pGryMat);
								//cv::waitKey(0);

								pFirstFrameROIMat = dtcApplyMask(pFirstFrameROIMat);
								//dtcApplyMaskToFrame(pFirstFrameROIMat);
								//pFirstFrameROIMat.copyTo(pFirstFrameROIMat, dtcGetMask(pFirstFrameROIMat));
								//*** checks croi for correcting incoherent values (outside of image range)
								if (croi.x < 0) {
									croi.width += croi.x;
									croi.x = 0;
								}
								if (croi.y < 0) {
									croi.height += croi.y;
									croi.y = 0;
								}
								if (croi.x + croi.width > pFirstFrameROIMat.cols) croi.width = pFirstFrameROIMat.cols - croi.x;
								if (croi.y + croi.height > pFirstFrameROIMat.rows) croi.height = pFirstFrameROIMat.rows - croi.y;

								firstFrameCm.x = croi.x + croi.width / 2;
								firstFrameCm.y = croi.y + croi.height / 2;

								pFirstFrameROI = cv::Rect(croi);
								pFirstFrameROIMat = dtcReduceMatToROI(pGryMat, pFirstFrameROI);
//cv::imshow("Frame read 4", pFirstFrameROIMat);
//cv::waitKey(0);

								if (!opts->wait && (opts->viewROI || opts->viewTrk || opts->viewDif || opts->viewRef ||
									opts->viewThr || opts->viewSmo || opts->viewRes || opts->viewHis)) {
									if (fps_int > 0) {
										opts->wait = (int)(1000 / std::ceil(fps_int));
									}
									else {
										opts->wait = (int)(1000 / 25);
									}
								}

								nb_impact = 0;
								//init_list(&ptlist, (fps_int * opts->timeImpact));
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
								if ((opts->ofilename) && (opts->allframes)) {
									pADUdtcMat = cv::Mat(pFirstFrameROIMat.size(), CV_32F);
									pADUavgMatFrame = cv::Mat(pFirstFrameROIMat.size(), CV_32F);
								}
								if (opts->thrWithMask || opts->viewMsk || (opts->ovfname && (opts->ovtype == OTYPE_MSK))) {
									pMskMat = cv::Mat(pFirstFrameROIMat.size(), CV_32F);
								}
								if (opts->viewThr) {
									pThrMat = cv::Mat(pFirstFrameROIMat.size(), CV_32F);
								}
								if (opts->filter.type >= 0 || opts->viewSmo) {
									pSmoMat = cv::Mat(pFirstFrameROIMat.size(), CV_32F);
								}
								if (opts->viewTrk || (opts->ovtype == OTYPE_TRK && opts->ovfname)) {
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
//cv::imshow("All frames read 1", pGryMat);
//cv::waitKey(0);
							pGryMat.convertTo(pGryMat, CV_8U);
							//cv::imshow("All frames read 2", pGryMat);
							//cv::waitKey(0);

							cv::Mat maskedGryMat = dtcApplyMask(pGryMat.clone());
							//cv::imshow("All frames read 3", pGryMat);
							//cv::waitKey(0);

														//pGryMat.copyTo(pGryMat, dtcGetMask(pGryMat.clone()));
							//AS3
							if (((cm_list.size() + cm_list_start) >= nframe) && (nframe > cm_list_start))
								cm = cm_list[nframe - cm_list_start - 1];
							else cm = dtcGetGrayMatCM(maskedGryMat);
							//cv::imshow("All frames read 4", pGryMat);
							//cv::waitKey(0);

							currentFrameMean = cv::mean(pGryMat)[0];

							//if ((cm.x <= 0) || (cm.y <= 0) || ((firstFrameMean / 10) > currentFrameMean)) {		
							//if ((cm.x <= 0) || (cm.y <= 0) || (currentFrameMean <= (0.2 * firstFrameMean))) {
							//if ((cm.x <= 0) || (cm.y <= 0) || (currentFrameMean == 0.0)) {
							//if ((cm.x <= 0) || (cm.y <= 0) || (currentFrameMean <= (0.1 * firstFrameMean))) {

//Modification v3.2.1 comparison first ROI with full frame: applying size ratio and 80% tolerance in transparency
							//if ((cm.x <= 0) || (cm.y <= 0) || (currentFrameMean < 5.0)) {
							if ((cm.x <= 0) || (cm.y <= 0) || (currentFrameMean <= (0.2 * firstFrameMean * pFirstFrameROIMat.rows * pFirstFrameROIMat.cols / (pGryMat.rows * pGryMat.cols)))) {
								frame_errors++;
							}
							else {
								pFrameROI = dtcGetGrayImageROIcCM(maskedGryMat, cm, (float)opts->medSize, opts->facSize, opts->secSize);

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
								}
								else {
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
//cv::imshow("All frames read 4", tempROIMat);
//cv::waitKey(0);

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
									//cv::imshow("All frames read 6", pGryMat);
									//cv::waitKey(0);
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

									//cv::imshow("All frames read 6", pGryMat);
									//cv::waitKey(0);
																		/* Normalise image */
									pGryMat *= (firstFrameMean / cv::mean(pGryMat)[0]);
									pGryMat.convertTo(pGryMat, CV_32F);
									//cv::imshow("All frames read 7", pGryMat);
									//cv::waitKey(0);
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
										if (opts->viewDif) {
											cv::minMaxLoc(pDifMat, &minLum, &maxLum, &minPoint, &maxPoint);
											pDifMat.convertTo(pDifImg, -1, 255.0 / maxLum, 0);
											pDifImg.convertTo(pDifImg, CV_8U);
											cv::imshow("Initial differential photometry", pDifImg);
											cv::waitKey(1);
											pDifImg.release();
											pDifImg = NULL;
										}
										if (nframe == opts->nsaveframe && opts->ofilename && opts->ostype == OTYPE_DIF) {
											cv::imwrite(opts->ofilename, pDifMat, img_save_params);
										}
									}


									//cv::threshold(pDifMat, pThrMat, opts->threshold, 0.0, CV_THRESH_TOZERO);
									//cv::threshold(pDifMat, pDifMat, opts->threshold, 0.0, CV_THRESH_TOZERO);

									if (opts->filter.type > 0) {
										switch (opts->filter.type) {
										case FILTER_BLUR:
											//Size 5x5
											cv::blur(pDifMat, pDifMat, cv::Size(opts->filter.param[0], opts->filter.param[0]));
											break;
										case FILTER_MEDIAN:
											//Size 5
											cv::medianBlur(pDifMat, pDifMat, opts->filter.param[0]);
											break;
										case FILTER_GAUSSIAN:
											//Size 5x5 Sigma 0
											cv::GaussianBlur(pDifMat, pDifMat, cv::Size(opts->filter.param[0],
												opts->filter.param[1]), opts->filter.param[2]);
											break;
										}
									}

									pDifMat.copyTo(pSmoMat);

									if (opts->viewSmo && pSmoMat.data) {
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

									if (opts->viewMsk && pMskMat.data) {
										//pMskMat.convertTo(pMskImg, CV_8U);
										cv::minMaxLoc(pMskMat, &minLum, &maxLum, &minPoint, &maxPoint);
										pMskMat.convertTo(pMskImg, -1, 255.0 / maxLum, 0);
										pMskImg.convertTo(pMskImg, CV_8U);
										cv::imshow("Mask", pMskImg);
										cv::waitKey(1);
									}

									//cv::blur(pDifMat, pDifMat, cv::Size(3,3));
									//cv::threshold(pDifMat, pDifMat, opts->threshold, 0.0, CV_THRESH_TOZERO);

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
									if (opts->ofilename && opts->allframes) {
										pADUavgMat.convertTo(pADUavgMatFrame, -1, 1.0 / (nframe - frame_errors), 0);
										pADUavgDiffMat.convertTo(pADUavgDiffMat, -1, 1.0 / (nframe - frame_errors), 0);
										cv::subtract(pADUmaxMat, pADUavgMatFrame, pADUdtcMat);
										cv::minMaxLoc(pADUdtcMat, &minLum, &maxLum, &minPoint, &maxPoint);
										pADUdtcMat.convertTo(pADUdtcMat, -1, 255.0 / maxLum, 0);
										strncpy(ofilenamemax, opts->ofilename, strlen(opts->ofilename) - 4);
										ofilenamemax[std::strlen(opts->ofilename) - 4] = '\0';
										sprintf(ofilenamemax, "%s_dtc_max_frame%05d.jpg", ofilenamemax, nframe);
										std::strcat(max_folder_path_filename, right(ofilenamemax, strlen(ofilenamemax) - InRstr(ofilenamemax,
											"\\"), tmpstring));
										cv::imwrite(max_folder_path_filename, pADUdtcMat, img_save_params);
										cv::minMaxLoc(pADUdtcMat, &minLum, &maxLum, &minPoint, &maxPoint);
										pADUdtcMat.convertTo(pADUdtcMat, -1, 255.0 / maxLum, 0);
										strncpy(ofilenamediff, opts->ofilename, strlen(opts->ofilename) - 4);
										ofilenamediff[std::strlen(opts->ofilename) - 4] = '\0';
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

									cv::threshold(pDifMat, pThrMat, opts->threshold, 0.0, CV_THRESH_TOZERO);
									cv::threshold(pDifMat, pDifMat, opts->threshold, 0.0, CV_THRESH_TOZERO);

									cv::minMaxLoc(pDifMat, &minLum, &maxLum, &minPoint, &maxPoint);

									if (opts->viewThr && pThrMat.data) {
										//pThrMat.convertTo(pThrImg, CV_8U);
										cv::minMaxLoc(pThrMat, &minLum, &maxLum, &minPoint, &maxPoint);
										pThrMat.convertTo(pThrImg, -1, 255.0 / maxLum, 0);
										pThrImg.convertTo(pThrImg, CV_8U);
										cv::imshow("Thresholded differential photometry", pThrImg);
										cv::waitKey(1);
									}

									if (opts->viewRef && pRefMat.data) {
										//pRefMat.convertTo(pRefImg, CV_8U);
										cv::minMaxLoc(pRefMat, &minLum, &maxLum, &minPoint, &maxPoint);
										pRefMat.convertTo(pRefImg, -1, 255.0 / maxLum, 0);
										pRefImg.convertTo(pRefImg, CV_8U);
										cv::imshow("Reference frame", pRefImg);
										cv::waitKey(1);
									}

									pMskMat.convertTo(pMskMat, CV_8U);
									if (nframe > 1) {
										if (nframe <= (long)opts->nframesRef) {
											cv::accumulateWeighted(pGryMat, pRefMat, 1 / nframe, opts->thrWithMask ? pMskMat : cv::noArray());
										}
										else {
											cv::add(pRefMat, pGryMat / opts->nframesRef, pRefMat, opts->thrWithMask ? pMskMat : cv::noArray());
											cv::Mat frontMat = refFrameQueue.front();
											cv::subtract(pRefMat, frontMat / opts->nframesRef, pRefMat,
												opts->thrWithMask ? pMskMat : cv::noArray());
											frontMat.release();
											refFrameQueue.pop();
										}
									}
									if (pDifMat.data && opts->viewRes) {
										cv::minMaxLoc(pDifMat, &minLum, &maxLum, &minPoint, &maxPoint);
										pDifMat.convertTo(pDifImg, -1, 255.0 / maxLum, 0);
										pDifImg.convertTo(pDifImg, CV_8U);
										cv::imshow("Resulting differential photometry", pDifImg);
										cv::waitKey(1);
									}
									if (opts->viewHis || (opts->ovfname && (opts->ovtype == OTYPE_HIS))) {
										pHisImg = dtcGetHistogramImage(pDifMat, (float)opts->histScale, opts->threshold);
										if (opts->viewHis) {
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

									if (opts->viewROI && pGryMat.data) {
										pGryMat.convertTo(pGryImg, CV_8U);
										cv::imshow("ROI", pGryImg);
										cv::waitKey(1);
									}
									if (pTrkMat.data && opts->viewTrk) {
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
									if (opts->ovfname && opts->ovtype) {
										switch (opts->ovtype) {
										case OTYPE_DIF: pOVdMat = pDifMat; break;
										case OTYPE_TRK: pOVdMat = pTrkMat; break;
										case OTYPE_ROI: pOVdMat = pGryMat; break;
										case OTYPE_HIS: pOVdMat = pHisImg; break;
										case OTYPE_MSK: pOVdMat = pMskMat; break;
										}
										pWriter = dtcWriteVideo(opts->ovfname, *pWriter, pCapture, pOVdMat);
									}
									if (opts->wait && (cvWaitKey(opts->wait) == 27)) {
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
					/********************************* END OF CAPTURE READING******************************************/

					/*********************************FINAL PROCESSING******************************************/
					//cv::imshow("End", pADUavgDiffMat);
					//cv::waitKey(0);
					//cv::imshow("End", pADUmaxMat);
					//cv::waitKey(0);
					//cv::imshow("End", pADUavgMat);
					//cv::waitKey(0);

							/*********************************IMPACT PROCESSING******************************************/
					double distance = 0;
					ITEM* maxDtcImg = NULL; // Detection image
					// img stats {min, average, max }
					double mean_stat[] = { 0,0,0 };
					double mean2_stat[] = { 0,0,0 };
					double max_mean_stat[] = { 0,0,0 };
					double max_mean2_stat[] = { 0,0,0 };
					double diff_stat[] = { 0,0,0 };
					double diff2_stat[] = { 0,0,0 };

					bool is_image_correct = true;

					if (nframe > 0) {
						/*ADUdtc algorithm******************************************/

						if (opts->ofilename && opts->allframes) {
							pADUdtcMat.release();
							pADUdtcMat = NULL;
							pADUavgMatFrame.release();
							pADUavgMatFrame = NULL;
							pADUavgDiffMat.release();
							pADUavgDiffMat = NULL;
						}
						if (opts->ovfname && opts->ovtype) {
							if (pWriter) {
								pWriter->release();
								pWriter = nullptr;
							}
						}

						/********** Process all matrix **********/
						pADUavgDiffMat.convertTo(pADUavgDiffMat, CV_32F);
						pADUmaxMat.convertTo(pADUmaxMat, CV_32F);
						pADUavgMat.convertTo(pADUavgMat, CV_32F);
						pADUavgMat.convertTo(pADUavgMat, -1, 1.0 / (nframe - frame_errors), 0);
						//pGryMat.convertTo(pGryMat, CV_32FC1);
						/* Compute Max-mean image */
						pADUdtcMat = cv::Mat(pGryImg_height, pGryImg_width, CV_32F);
						cv::subtract(pADUmaxMat, pADUavgMat, pADUdtcMat);
						//dtcApplyMaskToFrame(pADUdtcMat);

					/********** mean image **********/
/*					pADUavgMat2 = cv::Mat(pADUavgMat.rows, pADUavgMat.cols, CV_32F);
					pADUavgMat2.convertTo(pADUavgMat, CV_8U);
					if (opts->detail)
						cv::imwrite(dtc_full_filename(opts->ofilename, DTC_MEAN2_SUFFIX, detail_folder_path_string.c_str(), tmpstring), pADUavgMat2, img_save_params);
					mean2_stat[1] = mean(pADUavgMat2)[0];*/
						pADUavgMat.convertTo(pADUavgMat, CV_8U);
						if (opts->detail)
							cv::imwrite(dtc_full_filename(opts->ofilename, DTC_MEAN2_SUFFIX, detail_folder_path_string.c_str(), tmpstring), pADUavgMat, img_save_params);
						mean2_stat[1] = mean(pADUavgMat)[0];
						cv::minMaxLoc(pADUavgMat, &mean2_stat[0], &mean2_stat[2], NULL, NULL);
						if (mean2_stat[2] < opts->ROI_min_px_val) is_image_correct = false;
						/* normalizes mean  */
						pADUavgMat.convertTo(pADUavgMat, -1, 255.0 / mean2_stat[2], 0);
						pADUavgMat.convertTo(pADUavgMat, CV_8U);
						mean_stat[1] = mean(pADUavgMat)[0];
						cv::minMaxLoc(pADUavgMat, &mean_stat[0], &mean_stat[2], NULL, NULL);
						cv::imwrite(dtc_full_filename(opts->ofilename, DTC_MEAN_SUFFIX, detection_folder_path_string.c_str(), tmpstring), pADUavgMat, img_save_params);

						/********** diff image **********/
						pADUavgDiffMat.convertTo(pADUavgDiffMat, -1, 1.0 / (nframe - frame_errors), 0);
						/*pADUavgDiffMat2 = cv::Mat(pADUavgDiffMat.rows, pADUavgDiffMat.cols, CV_32F);
						pADUavgDiffMat2.convertTo(pADUavgDiffMat, CV_8U);*/
						pADUavgDiffMat.convertTo(pADUavgDiffMat, CV_8U);
						if (opts->detail)
							cv::imwrite(dtc_full_filename(opts->ofilename, DTC_DIFF2_SUFFIX, detail_folder_path_string.c_str(), tmpstring), pADUavgDiffMat, img_save_params);
						diff2_stat[1] = mean(pADUavgDiffMat)[0];
						cv::minMaxLoc(pADUavgDiffMat, &diff2_stat[0], &diff2_stat[2], NULL, NULL);
						/*if (opts->detail)
							cv::imwrite(dtc_full_filename(opts->ofilename, DTC_DIFF2_SUFFIX, detail_folder_path_string.c_str(), tmpstring), pADUavgDiffMat2, img_save_params);*/
							/* normalizes diff  */
						pADUavgDiffMat.convertTo(pADUavgDiffMat, -1, 255.0 / diff2_stat[2], 0);
						pADUavgDiffMat.convertTo(pADUavgDiffMat, CV_8U);
						diff_stat[1] = mean(pADUavgDiffMat)[0];
						cv::minMaxLoc(pADUavgDiffMat, &diff_stat[0], &diff_stat[2], NULL, NULL);
						if (opts->detail)
							cv::imwrite(dtc_full_filename(opts->ofilename, DTC_DIFF_SUFFIX, detail_folder_path_string.c_str(), tmpstring), pADUavgDiffMat, img_save_params);

						/********** Max-mean image **********/
						cv::medianBlur(pADUdtcMat, pSmoADUdtcMat, 3); // blur image
						cv::minMaxLoc(pSmoADUdtcMat, &minLum, &maxLum, &minPoint, &maxPoint);
						pSmoADUdtcMat.release();
						(ITEM*)(maxDtcImg) = create_item(create_point(0, 0, maxPoint.x, maxPoint.y));
						cv::minMaxLoc(pADUdtcMat, &minLum, &maxLum, &minPoint, &maxPoint);
						/* Max-mean normalized image */
						pADUdtcMat.convertTo(pADUdtcMat, -1, 255.0 / maxLum, 0);
						pADUdtcMat.convertTo(pADUdtcImg, CV_8UC3);
						cv::cvtColor(pADUdtcImg, pADUdtcImg, CV_GRAY2BGR);
						max_mean_stat[1] = mean(pADUdtcImg)[0];
						cv::minMaxLoc(pADUdtcImg, &max_mean_stat[0], &max_mean_stat[2], NULL, NULL);

						pADUdtcMat.convertTo(impactFrame, CV_8U);

						/*Max-mean non normalized image*/
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
						pADUdtcMat.convertTo(pADUdtcImg2, CV_8UC3);
						cv::cvtColor(pADUdtcImg2, pADUdtcImg2, CV_GRAY2BGR);
						max_mean2_stat[1] = mean(pADUdtcImg2)[0];
						cv::minMaxLoc(pADUdtcImg2, &max_mean2_stat[0], &max_mean2_stat[2], NULL, NULL);
						/* temporary end of ADUdtc algorithm******************************************/
					}

					refFrameQueue = std::queue<cv::Mat>();
					totalMean /= (nframe - frame_errors);

					CDeTeCtMFCDlg::getLog()->AddString((CString)getDateTime().str().c_str() +
						L"Differential photometry done, running impact detection...");
					CDeTeCtMFCDlg::getLog()->RedrawWindow();
					CDeTeCtMFCDlg::getLog()->SetTopIndex(CDeTeCtMFCDlg::getLog()->GetCount() - 1);
					output_log << getDateTime().str().c_str() << "Differential photometry done, running impact detection..." << "\n";
					output_log.flush();

					if (opts->darkfilename) {
						if (darkfile_ok == 1) {
							pADUdarkMat.release();
							pADUdarkMat = NULL;
						}
					}

					int radius = std::min(std::min(20.0, pGryMat.rows / 10.0), pGryMat.cols / 10.0);
					radius = radius > 5 ? radius : 5; //std::max gives error

					output_log << getDateTime().str().c_str() << "Running impact detection..." << "\n";
					output_log.flush();

					ITEM* maxDtcImp = create_item(create_point(0, 0, 0, 0)); // Algorithm

					bMat = cv::Mat(cv::Size(1, nframe), CV_8UC1, maxPtB.data());
					//cv::medianBlur(bMat, bMat, 3);
					//maxPtB = bMat.data;

					/*for (int i = 0; i < nframe; i++) {
						add_tail_item(&ptlist, create_item(create_point(i + 1 - frameErrors[i], maxPtB[i], maxPtX[i], maxPtY[i])));
						maxList.push_back(maxPtB[i]);
					}*/

					// usage of mkdir only solution found to handle directory names with special characters (eg. é, à, ...)
					//CreateDirectory(wdir_csv_name.c_str(), 0);
					std::string dir_csv_name = detection_folder_path_string;
					dir_csv_name = dir_csv_name.append("\\csv");
					if (!(dir_tmp = opendir(dir_csv_name.c_str()))) mkdir(dir_csv_name.c_str());
					else closedir(dir_tmp);

					std::ofstream output_csv(dir_csv_name.append("\\").append(filePath).append(".csv"));
					output_csv << "x,y,B\n";
					for (int i = 0; i < (nframe - frame_errors); i++) {
						add_tail_item(&ptlist, create_item(create_point(i + 1, maxPtB[i], maxPtX[i], maxPtY[i])));
						output_csv << (int)maxPtX[i] << "," << (int)maxPtY[i] << "," << (int)maxPtB[i] << "\n";
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
						brightness_factor = (maxBright / (maxMean + opts->threshold)) - 1;
						//double brightness_factor = (maxBright / (maxMean + opts->threshold));
						//DBOUT("Brightness factor 1: " << brightness_factor << "\n");
						//brightness_factor = (maxBright / (maxMean + opts->threshold)) - (1 / opts->threshold);
						//DBOUT("Brightness factor 2: " << brightness_factor << "\n");
						brightnessFactor = maxMean / totalMean;
						stdDevAccum = 0.0;
						std::for_each(maxList.begin(), maxList.end(), [&](const double d) {
							stdDevAccum += (d - maxMean) * (d - maxMean);
						});

						stdev = sqrt(stdDevAccum / (maxList.size() - 1));
					}

					bMat.release();

					if (ptlist.size <= ptlist.maxsize && ptlist.size > impact_frames_min)
						nb_impact += detect_impact(&dtc, &outdtc, maxMean, &ptlist, &maxDtcImp, fps, radius, opts->incrLumImpact,
							impact_frames_min);

					delete_list(&ptlist);

					//(&outdtc)->nMaxFrame = frameNumbers[(&outdtc)->nMaxFrame];
					//(&outdtc)->nMinFrame = frameNumbers[(&outdtc)->nMinFrame];
					//(&outdtc)->MaxFrame = frameNumbers[(&outdtc)->MaxFrame];

					double impact_frames = (&outdtc)->nMaxFrame - (&outdtc)->nMinFrame;
					//double log10_value = impact_frames != 0 ? std::log10((impact_frames / (opts->incrFrameImpact)) * 10) : 0;
					double log10_value = impact_frames != 0 ? std::log10((impact_frames / impact_frames_min) * 10) : 0;
					double confidence = 0;
					if (log10_value > 0) confidence = (brightness_factor / opts->incrLumImpact) * log10_value;
					//double confidence = (stdev / opts->incrLumImpact) * log10_value;

					if (nframe > 0) {
						/* reprise ADUdtc algorithm******************************************/
						/* max-mean normalized image */
						if ((maxDtcImp->point->x != 0) && (maxDtcImp->point->y != 0)) {
							dtcDrawImpact(pADUdtcImg, cv::Point(maxDtcImp->point->x, maxDtcImp->point->y), CV_RGB(255, 0, 0), 20, 30);
							distance = sqrt(pow(maxDtcImg->point->x - maxDtcImp->point->x, 2) + pow(maxDtcImg->point->y - maxDtcImp->point->y, 2));
						}
						else {
							/* algorithm did not work */
							distance = 9999;
						}
						dtcDrawImpact(pADUdtcImg, cv::Point(maxDtcImg->point->x, maxDtcImg->point->y), CV_RGB(0, 255, 0), 15, 25);
						cv::imshow("Detection image", pADUdtcImg); // moved up
						cv::imwrite(dtc_full_filename(opts->ofilename, DTC_MAX_MEAN_SUFFIX, detection_folder_path_string.c_str(), tmpstring), pADUdtcImg, img_save_params);

						/* max-mean non normalized image */
						if (opts->detail) {
							if ((maxDtcImp->point->x != 0) && (maxDtcImp->point->y != 0))
								dtcDrawImpact(pADUdtcImg2, cv::Point(maxDtcImp->point->x, maxDtcImp->point->y), CV_RGB(255, 0, 0), 20, 30);
							dtcDrawImpact(pADUdtcImg2, cv::Point(maxDtcImg->point->x, maxDtcImg->point->y), CV_RGB(0, 255, 0), 15, 25);
							cv::imwrite(dtc_full_filename(opts->ofilename, DTC_MAX_MEAN2_SUFFIX, detail_folder_path_string.c_str(), tmpstring), pADUdtcImg2, img_save_params);
						}

						/* final end of ADUdtc algorithm******************************************/
						if (!is_image_correct) {
							logmessage = logmessage + "ERROR: No planet detected in acquisition images...\n";
							CDeTeCtMFCDlg::getLog()->AddString((CString)getDateTime().str().c_str() + logmessage.c_str());
							output_log << getDateTime().str().c_str() << logmessage.c_str();
							output_log.flush();
							sprintf(rating_classification, "Error        ");
							confidence = -1;
							nb_impact = -1;
						}
						else if (nb_impact > 0) {
							outdtc.nMinFrame = frameNumbers[outdtc.nMinFrame];
							outdtc.nMaxFrame = frameNumbers[outdtc.nMaxFrame];
							outdtc.MaxFrame = frameNumbers[outdtc.MaxFrame];
							if ((distance <= opts->impact_distance_max) && (confidence > opts->impact_confidence_min) && ((max_mean_stat[2] - max_mean_stat[1]) > opts->impact_max_avg_min)) {
								if (!opts->ADUdtconly) {
									/*** high probability impact */
									nb_high_impact++;
									logmessage = "ALERT: " + std::to_string(nb_impact) + " HIGH PROBABILITY IMPACT DETECTED (frames " +
										std::to_string(outdtc.nMinFrame) + "-" + std::to_string(outdtc.nMaxFrame) + ", max @" +
										std::to_string(outdtc.MaxFrame) + ").";
									logmessage2 = "Confidence: " + std::_Floating_to_string("%2.2f", confidence);
									logmessage3 = "CHECK DETECTION IMAGES!\n";
									CDeTeCtMFCDlg::getLog()->AddString((CString)getDateTime().str().c_str() + logmessage.c_str());
									CDeTeCtMFCDlg::getLog()->AddString((CString)getDateTime().str().c_str() + logmessage2.c_str());
									CDeTeCtMFCDlg::getLog()->AddString((CString)getDateTime().str().c_str() + logmessage3.c_str());
									output_log << getDateTime().str().c_str() << logmessage.c_str() << "\n";
									output_log << getDateTime().str().c_str() << logmessage2.c_str() << "\n";
									output_log << "    " << logmessage3.c_str();
									output_log.flush();
									logmessage = logmessage + "\n" + logmessage2 + "\n" + logmessage3;
									sprintf(rating_classification, "HIGH (@%5d)", outdtc.MaxFrame);
								}
								else {
									/* only initial algorithm launched, displaying only nb impacts detected */
									nb_low_impact++;
									logmessage = "WARNING: " + std::to_string(nb_impact) + " low probability impact.";
									CDeTeCtMFCDlg::getLog()->AddString((CString)getDateTime().str().c_str() + logmessage.c_str());
									output_log << getDateTime().str().c_str() << logmessage.c_str() << "\n";
									output_log.flush();
									logmessage = logmessage + "\n";
									sprintf(rating_classification, "Low          ");
								}
							}
							else if (distance <= opts->impact_distance_max) {
								/* No impact, confidence or contrast threshold not respected */
								nb_null_impact++;
								logmessage = "No impact detected (too faint candidate).\n";
								CDeTeCtMFCDlg::getLog()->AddString((CString)getDateTime().str().c_str() + logmessage.c_str());
								output_log << getDateTime().str().c_str() << logmessage.c_str();
								output_log.flush();
								sprintf(rating_classification, "Null         ");
							}
							else {
								/* algorithm worked */
								/* distance incorrect */
								confidence /= 4;
								if ((max_mean_stat[2] - max_mean_stat[1]) > opts->impact_max_avg_min) {
									/* potential impact */
									if (!opts->ADUdtconly) {
										nb_low_impact++;
										logmessage = "WARNING: " + std::to_string(nb_impact) + " low probability impact (frames " +
											std::to_string(outdtc.nMinFrame) + "-" + std::to_string(outdtc.nMaxFrame) + ", max @" +
											std::to_string(outdtc.MaxFrame) + "). ";
										logmessage2 = "Confidence: " + std::_Floating_to_string("%2.2f", confidence);
										//if ((distance <= opts->impact_distance_max) && (confidence > opts->impact_confidence_min) && ((max_mean_stat[2] - max_mean_stat[1]) > opts->impact_max_avg_min))
										if (!((confidence > opts->impact_confidence_min) && ((max_mean_stat[2] - max_mean_stat[1]) > opts->impact_max_avg_min)))
											logmessage2 = logmessage2 + ", too faint";
										if (!(distance <= opts->impact_distance_max))
											logmessage2 = logmessage2 + ", detection image and algorithm incoherent";
										logmessage2 = logmessage2 + ".";
										logmessage3 = "Please CHECK detection images.\n";
										CDeTeCtMFCDlg::getLog()->AddString((CString)getDateTime().str().c_str() + logmessage.c_str());
										CDeTeCtMFCDlg::getLog()->AddString((CString)getDateTime().str().c_str() + logmessage2.c_str());
										CDeTeCtMFCDlg::getLog()->AddString((CString)getDateTime().str().c_str() + logmessage3.c_str());
										output_log << getDateTime().str().c_str() << logmessage.c_str() << "\n";
										output_log << getDateTime().str().c_str() << logmessage2.c_str() << "\n";
										output_log << "    " << logmessage3.c_str();
										output_log.flush();
										logmessage = logmessage + logmessage2 + "\n" + logmessage3;
										sprintf(rating_classification, "Low  (@%5d)", outdtc.MaxFrame);
									}
									else {
										nb_low_impact++;
										logmessage = "WARNING: " + std::to_string(nb_impact) + " low probability impact.";
										CDeTeCtMFCDlg::getLog()->AddString((CString)getDateTime().str().c_str() + logmessage.c_str());
										output_log << getDateTime().str().c_str() << logmessage.c_str() << "\n";
										output_log.flush();
										logmessage = logmessage + "\n";
										sprintf(rating_classification, "Low          ");
									}
								}
								else {
									/* image detection failed */
									nb_null_impact++;
									logmessage = "No impact detected (too faint).\n";
									CDeTeCtMFCDlg::getLog()->AddString((CString)getDateTime().str().c_str() + logmessage.c_str());
									output_log << getDateTime().str().c_str() << logmessage.c_str();
									output_log.flush();
									sprintf(rating_classification, "Null         ");
								}
							}
						}
						else if (distance == 9999) {
							/* algorithm did not work */
							if ((max_mean_stat[2] - max_mean_stat[1]) < opts->impact_max_avg_min) {
								/* No impact, contrast threshold not respected */
								nb_null_impact++; 
								logmessage = "No impact detected by the algorithm.\n";
								CDeTeCtMFCDlg::getLog()->AddString((CString)getDateTime().str().c_str() + logmessage.c_str());
								output_log << getDateTime().str().c_str() << logmessage.c_str();
								output_log.flush();
								sprintf(rating_classification, "Null         ");
							}
							else {
								/* contrast threshold respected */
								nb_low_impact++;
								logmessage = "WARNING: low probability impact in detection image but no impact detected by the algorithm.";
								CDeTeCtMFCDlg::getLog()->AddString((CString)getDateTime().str().c_str() + logmessage.c_str());
								output_log << getDateTime().str().c_str() << logmessage.c_str() << "\n";
								output_log.flush();
								logmessage = logmessage + "\n";
								sprintf(rating_classification, "Low          ");
							}
						}

/*						else {
							logmessage = "No impact detected.\n";
							CDeTeCtMFCDlg::getLog()->AddString((CString)getDateTime().str().c_str() + logmessage.c_str());
							output_log << getDateTime().str().c_str() << logmessage.c_str();
							output_log.flush();
							sprintf(rating_classification, "Null         ");
						}*/

						end = clock();
						computation_time_total += (end - begin) / CLOCKS_PER_SEC;
						std::string s = "";
//						log_messages.push_back(short_filename + ":");
						std::stringstream str(logmessage);
						std::string line;
						std::getline(str, line);
						int computation_duration = int(end - begin) / CLOCKS_PER_SEC;
						if (computation_duration > 1) s = "s";
						log_messages.push_back(short_filename + ":" + "   " + line);
						while (std::getline(str, line)) log_messages.push_back("    " + line);
						CDeTeCtMFCDlg::getLog()->AddString((CString)getDateTime().str().c_str() + L"Computation time: " +
							(CString)std::to_string(int(end - begin) / CLOCKS_PER_SEC).c_str() + " second" + s.c_str() + "," + L" showing detection image"
							+ L" (automatically closed in " + (CString)std::to_string(wait_seconds).c_str() + " second" + s.c_str() + ")...");
						CDeTeCtMFCDlg::getLog()->SetTopIndex(CDeTeCtMFCDlg::getLog()->GetCount() - 1);
						CDeTeCtMFCDlg::getLog()->RedrawWindow();
						output_log << getDateTime().str().c_str() << "Computation time: " << std::to_wstring(int(end - begin) / CLOCKS_PER_SEC)
							<< " second" << s.c_str() << "." << "\n";
						output_log << getDateTime().str().c_str() << "Showing detection image" << " (automatically closed in " << wait_seconds << " seconds)...."
							<< "\n";
						output_log.flush();

						//					cv::imshow("Detection image", pADUdtcImg);
						//					cv::waitKey(3000);
						//					cv::destroyWindow("Detection image");

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

						if (opts->ignore)
							dtcCorrectDatation((DtcCapture*)pCapture, &start_time, &end_time, &duration, &fps_real, &timetype, comment);
						std::string location = filename.substr(0, filename.find_last_of("\\") + 1);

						if (extension.compare(AUTOSTAKKERT_EXT) == 0) strcat(comment, "; from as3");

//						if (!is_image_correct) {

						LogInfo info(opts->filename, start_time, end_time, duration, fps_real, timetype, comment, nb_impact, confidence, distance, mean_stat, mean2_stat, max_mean_stat, max_mean2_stat, diff_stat, diff2_stat, rating_classification);
						dtcWriteLog2(logcation, info, &logline_tmp);
						dtcWriteLog2(logcation2, info, &logline_tmp);

						/*FINAL CLEANING**************************************/
						if (opts->viewDif) cv::destroyWindow("Initial differential photometry");
						if (opts->viewRef) cv::destroyWindow("Reference frame");
						if (opts->viewROI) cv::destroyWindow("ROI");
						if (opts->viewTrk) cv::destroyWindow("Tracking");
						if (opts->viewMsk) cv::destroyWindow("Mask");
						if (opts->viewThr) cv::destroyWindow("Thresholded differential photometry");
						if (opts->viewSmo) cv::destroyWindow("Smoothed differential photometry");
						if (opts->viewRes) cv::destroyWindow("Resulting differential photometry");
						if (opts->viewHis) cv::destroyWindow("Histogram");

						if (opts->thrWithMask || opts->viewMsk || (opts->ovfname && (opts->ovtype == OTYPE_MSK))) {
							pMskMat.release();
							pMskMat = NULL;
							pMskImg.release();
							pMskImg = NULL;
						}
						if (opts->viewThr) {
							pThrMat.release();
							pThrMat = NULL;
							pThrImg.release();
							pThrImg = NULL;
						}
						if (opts->filter.type >= 0 || opts->viewSmo) {
							pSmoMat.release();
							pSmoMat = NULL;
							pSmoImg.release();
							pSmoImg = NULL;
						}
						if (opts->viewTrk || (opts->ovtype == OTYPE_TRK && opts->ovfname)) {
							pTrkMat.release();
							pTrkMat = NULL;
							pTrkImg.release();
							pTrkImg = NULL;
						}
						if (opts->viewDif || opts->viewRes || opts->viewHis || (opts->ovfname && (opts->ovtype == OTYPE_DIF ||
							opts->ovtype == OTYPE_HIS))) {
							pDifMat.release();
							pDifMat = NULL;
							pDifImg.release();
							pDifImg = NULL;
						}
						pRefMat.release();
						pRefMat = NULL;
						pRefImg.release();
						pRefImg = NULL;
						if (opts->viewHis || (opts->ovfname && (opts->ovtype == OTYPE_HIS))) {
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

						cv::waitKey(wait_seconds*1000); // put at the end
						cv::destroyWindow("Detection image"); // put at the end
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
						log_messages.push_back(short_filename + ":" + "    " + logmessage);
						// log_messages.push_back("    " + logmessage);
					}
					//message = "--------- " + short_filename + " analysis done ---------";
					message = ""; 
					CDeTeCtMFCDlg::getLog()->AddString((CString)getDateTime().str().c_str() + (CString)message.c_str());
					output_log << getDateTime().str().c_str() << message.c_str() << "\n";
					CDeTeCtMFCDlg::getLog()->SetTopIndex(CDeTeCtMFCDlg::getLog()->GetCount() - 1);
					CDeTeCtMFCDlg::getLog()->RedrawWindow();
				if (opts->filename) {
					CString objectname(opts->filename);
					RemoveFromQueue(objectname, DeTeCt_additional_filename_exe_folder(DTC_QUEUE_SUFFIX));
				}
			} else {
				std::string message = "INFO: " + short_filename + " already processed, ignoring";
				std::string message2 = "INFO: " + short_filename + " already processed, ignoring";
				CDeTeCtMFCDlg::getLog()->SetTopIndex(CDeTeCtMFCDlg::getLog()->GetCount() - 1);
				CDeTeCtMFCDlg::getLog()->AddString((CString)getDateTime().str().c_str() + (CString)message.c_str());
				CDeTeCtMFCDlg::getLog()->RedrawWindow();
				output_log << getDateTime().str().c_str() << message2.c_str() << "\n";
				output_log.flush();
				log_messages.push_back(message);
			}
		}

		if (opts->dirname) {
			CString objectname(opts->dirname);
			RemoveFromQueue(objectname, DeTeCt_additional_filename_exe_folder(DTC_QUEUE_SUFFIX));
		}
		local_acquisition_files_list.acquisition_file_list = std::vector<std::string>();
		if (!opts->interactive) {
			CString objectname;
			if (PopFromQueue(&objectname, DeTeCt_additional_filename_exe_folder(DTC_QUEUE_SUFFIX))) {
				std::ifstream file(objectname);
				if (file) {
					CT2A objectnamechar(objectname);
					opts->filename = objectnamechar;
					file.close();
					local_acquisition_files_list.acquisition_file_list.push_back(std::string(opts->filename));
				}
				else {
					DIR *folder_object;
					CT2A objectnamechar(objectname);
					if (folder_object = opendir(objectnamechar)) {
						opts->dirname = objectnamechar;
						//strcpy(opts->dirname,objectnamechar);
						std::string path = std::string(opts->dirname);
						read_files(path, &local_acquisition_files_list);
						closedir(folder_object);
					}
				}
			}
		}
	} while (local_acquisition_files_list.acquisition_file_list.size() > 0);

	/* Renames logfile in impact_detection directory */
	double second;
	int minute;
	int hour;
	int day_min, day_max;
	int month_min, month_max;
	int year_min,year_max;
	char suffix_char[MAX_STRING];
	char planet_char[MAX_STRING] = "";

/*	switch (planet) {
		case Jupiter:	strcpy(planet_char, "_jupiter");
						break;
		case Saturn:	strcpy(planet_char, "_saturn");
						break;
		default: strcpy(planet_char, "");
	}*/

	if (planet_jupiter>0) strcat(planet_char, "_jupiter");;
	if (planet_saturn > 0) strcat(planet_char, "_saturn");;

	jd_to_date(start_time_min, &second, &minute, &hour, &day_min, &month_min, &year_min);
	jd_to_date(start_time_max, &second, &minute, &hour, &day_max, &month_max, &year_max);
	sprintf(suffix_char, "%s_%04d%02d%02d", planet_char, year_min, month_min, day_min);
	if ((day_min != day_max) || (month_min != month_max) || (year_min != year_max)) sprintf(suffix_char, "%s-%04d%02d%02d", suffix_char, year_max, month_max, day_max);
	sprintf(suffix_char, "%s.log", suffix_char);

	std::string message = "Total duration analyzed ";
	int days;
	int hours;
	int minutes;
	int seconds;

	days = (int)floor(duration_total / 60 / 60 / 24);
	if (days > 0)  message = message + std::to_string(days) + "d";
	hours = (int)floor((duration_total - days * 24 * 60 * 60) / 60 / 60);
	if (hours > 0)  message = message + std::to_string(hours) + "h";
	minutes = (int)floor((duration_total - (days * 24 + hours) * 60 * 60) / 60);
	if (minutes > 0)  message = message + std::to_string(minutes) + "m";
	seconds = (int)floor((duration_total - ((days * 24 + hours) * 60 + minutes) * 60));
	message = message + std::to_string(seconds) + "s";

	if (acquisition_index > 1) {
		message = message + " (" + std::to_string(acquisition_index) + " acquisitions processed in ";
		days = (int)floor(computation_time_total / 60 / 60 / 24);
		if (days > 0)  message = message + std::to_string(days) + "d";
		hours = (int)floor((computation_time_total - days * 24 * 60 * 60) / 60 / 60);
		if (hours > 0)  message = message + std::to_string(hours) + "h";
		minutes = (int)floor((computation_time_total - (days * 24 + hours) * 60 * 60) / 60);
		if (minutes > 0)  message = message + std::to_string(minutes) + "m";
		seconds = (int)floor((computation_time_total - ((days * 24 + hours) * 60 + minutes) * 60));
		message = message + std::to_string(seconds) + "s";
		message = message + ")";
	}

	char tmpchar[MAX_STRING];
	CDeTeCtMFCDlg::getLog()->AddString((CString)getDateTime().str().c_str() + (CString)message.c_str());
	CDeTeCtMFCDlg::getLog()->AddString((CString)getDateTime().str().c_str() + L"In " + (CString)(logcation.c_str()) + L", please find:");
	CDeTeCtMFCDlg::getLog()->AddString((CString)getDateTime().str().c_str() + L" * log file " + (CString)(left(DeTeCtFileName(tmpchar), InRstr(DeTeCtFileName(tmpchar), "."), tmpchar)) + DTC_LOG_SUFFIX);
	CDeTeCtMFCDlg::getLog()->AddString((CString)getDateTime().str().c_str() + L" * folder " + (CString)(right(detection_folder_path_string.c_str(), strlen(detection_folder_path_string.c_str()) - InRstr(detection_folder_path_string.c_str(), "\\") - 1, tmpchar)) + L" for checking images");
	CDeTeCtMFCDlg::getLog()->SetTopIndex(CDeTeCtMFCDlg::getLog()->GetCount() - 1);
	CDeTeCtMFCDlg::getLog()->RedrawWindow();

	//	CT2A pathlog (DeTeCt_additional_filename(CString(logcation.c_str()), DTC_LOG_SUFFIX));
	//	std::string pathlogstr(pathlog);
	output_log << getDateTime().str().c_str() << "Log file available at " << CW2A(DeTeCt_additional_filename(CString(logcation.c_str()), DTC_LOG_SUFFIX)) << ", zip file created for sending\n";
	if (opts->dateonly) output_log << getDateTime().str().c_str() << "WARNING, datation info only, no detection analysis was performed\n";
	output_log << getDateTime().str().c_str() << message.c_str() << "\n";

	std::wstring output_log_file2(scan_folder_path.begin(), scan_folder_path.end());
	output_log_file2 = output_log_file2.append(L"\\").append(OUTPUT_FILENAME).append(DTC_LOG_SUFFIX);
	std::wofstream output_log2(output_log_file2.c_str(), std::ios_base::app);
	if (opts->dateonly) output_log2 << "WARNING, datation info only, no detection analysis was performed\n";
	std::wifstream output_log_in(output_log_file.c_str());
	output_log2 << output_log_in.rdbuf();
	output_log_in.close();
	output_log2 << getDateTime().str().c_str() << "\n";
	output_log2 << getDateTime().str().c_str() << message.c_str() << "\n";
	output_log2 << "======================================================================================================\n\n";
	output_log2.flush();
	output_log2.close();
	output_log.flush();
	output_log.close();

	CT2A LogOrgFilename(DeTeCt_additional_filename(CString(logcation2.c_str()), DTC_LOG_SUFFIX));
	CT2A LogNewFilename(DeTeCt_additional_filename(CString(logcation2.c_str()), (CString)suffix_char));
	rename(LogOrgFilename, LogNewFilename);

	CT2A tmp_log_detection_dirname(DeTeCt_additional_filename("", suffix_char));
	strcpy(log_detection_dirname, tmp_log_detection_dirname);
		
	CT2A OutOrgFilename2(CString(logcation2.c_str()) + L"\\" + OUTPUT_FILENAME + DTC_LOG_SUFFIX);
	CT2A OutNewFilename2(CString(logcation2.c_str()) + L"\\" + OUTPUT_FILENAME + (CString)suffix_char);
	rename(OutOrgFilename2, OutNewFilename2);

	strcpy(zipfile, "");
	strcat(zipfile, "\0");
	strcpy(zip_detection_dirname, zipfile);
	strcpy(opts->zipname, zipfile);
	char item_to_be_zipped_shortname[MAX_STRING] = "";
	if (opts->zip) {
		char item_to_be_zipped[MAX_STRING] = "";
		char item_to_be_zipped_location[MAX_STRING] = "";

		strcat(item_to_be_zipped, detection_folder_path_string.c_str());
		strcat(item_to_be_zipped, "\0");
		
		strcat(item_to_be_zipped_shortname, detection_folder_name_string.c_str());
		strcat(item_to_be_zipped_shortname, ".zip");
		strcat(item_to_be_zipped_shortname, "\0");
		strcpy(opts->zipname, item_to_be_zipped_shortname);
		
		strcat(item_to_be_zipped_location, logcation.c_str());
		strcat(item_to_be_zipped_location, "\0");

		strcat(zipfile, detection_folder_path_string.c_str());
		strcat(zipfile, ".zip");
		strcat(zipfile, "\0");

		strcpy(zip_detection_dirname, zipfile);
		strcpy(zip_detection_location, item_to_be_zipped_location);

		// Deactivated and replaced by new version below
		zip(zipfile, item_to_be_zipped);
		
		struct stat st;
		if (stat(zipfile, &st) == 0) if (st.st_size < 23) remove(zipfile);

		// see project https://www.codeproject.com/articles/4135/xzip-and-xunzip-add-zip-and-or-unzip-to-your-app-w
		
		//USES_CONVERSION;
		//HZIP newZip0 = CreateZip(L"E:\\Sample.zip", NULL, ZIP_FILENAME);
		//BOOL retval0 = AddFolderContent(newZip0, L"E:", L"TEMP");
		//CloseZip(newZip0);

		//HZIP newZip = CreateZip(A2T(zipfile), NULL, ZIP_FILENAME);
		//BOOL retval = AddFolderContent(newZip, A2T(item_to_be_zipped_location), A2T(item_to_be_zipped_shortname));
		//CloseZip(newZip);

		std::ifstream filetest(zipfile);
		if (filetest) CDeTeCtMFCDlg::getLog()->AddString((CString)getDateTime().str().c_str() + L" * zip file " + (CString)(right(zipfile, strlen(zipfile) - InRstr(zipfile, "\\") - 1, tmpchar)) + L" for sending");
		else CDeTeCtMFCDlg::getLog()->AddString((CString)getDateTime().str().c_str() + L" * ERROR: zip file " + (CString)(right(zipfile, strlen(zipfile) - InRstr(zipfile, "\\") - 1, tmpchar)) + L" not created!");
	}

	if (opts->dateonly) CDeTeCtMFCDlg::getLog()->AddString((CString)getDateTime().str().c_str() + L"WARNING, datation info only, no detection analysis was performed\n");
	CDeTeCtMFCDlg::getLog()->AddString((CString)getDateTime().str().c_str());
	CDeTeCtMFCDlg::getLog()->SetTopIndex(CDeTeCtMFCDlg::getLog()->GetCount() - 1);
	CDeTeCtMFCDlg::getLog()->RedrawWindow();

	log_messages.push_back("");
	std::string plural;
	if (nb_high_impact > 0) {
		if (nb_high_impact > 1) plural = "s";
		else plural = "";
		log_messages.push_back(std::to_string(nb_high_impact) + " acquisition" + plural + " with high probability impact" + plural);
	}
	if (nb_low_impact > 0) {
		if (nb_low_impact > 1) plural = "s";
		else plural = "";
		log_messages.push_back(std::to_string(nb_low_impact) + " acquisition" + plural + " with low probability impact" + plural);
	}
	if (nb_null_impact > 0) {
		if (nb_null_impact > 1) plural = "s";
		else plural = "";
		log_messages.push_back(std::to_string(nb_null_impact) + " acquisition" + plural + " without any impact" + plural);
	}
	log_messages.push_back("");
	log_messages.push_back("Please click on \"Check detection images\" button to open :");
	log_messages.push_back(" - an explorer in the \"" + detection_folder_name_string + "\" folder to check the detection images stored there.");
	
	std::ifstream filetest(zipfile);
	if (filetest) {
		std::string stritem_to_be_zipped_shortname(item_to_be_zipped_shortname);
		log_messages.push_back(" - an explorer in the folder where the \"" + stritem_to_be_zipped_shortname + "\" file is, to be sent by email.");
		if (opts->email)
			log_messages.push_back(" - an email to send the results by attaching the \"" + stritem_to_be_zipped_shortname + "\" file.");
		else log_messages.push_back("Please send an email with the results by attaching the\"" + stritem_to_be_zipped_shortname + "\" file.");
	}
	else {
		std::string strlog_detection_dirname(log_detection_dirname);
		if (opts->email) log_messages.push_back(" - an email to send the results by attaching the detection images and \"" + strlog_detection_dirname + "\" from the \"" + detection_folder_name_string + "\" folder.");
		else log_messages.push_back("Please send an email with the results by attaching the detection images and \"" + strlog_detection_dirname + "\" from the \"" + detection_folder_name_string + "\" folder.");
	}

	SendEmailDlg* email = new SendEmailDlg(NULL, log_messages);

//DBOUT("Interactive=" << opts->interactive << "\n");

	if (opts->interactive) {
		email->DoModal();
	} else {
		dlg.OnFileExit();
		//m_pMainWnd->DestroyWindows();
	}
	return TRUE;
}

char *dtc_full_filename(const char *acquisition_filename, const char *suffix, const char *path_name, char *full_filename) {
	char tmpstring[MAX_STRING];
	char filename[MAX_STRING];
	
	init_string(tmpstring);
	init_string(filename);
	snprintf(filename, strlen(acquisition_filename) - 4, "%s", acquisition_filename);
	filename[std::strlen(filename)] = '\0';
	strcat(filename, suffix);
	strcpy(full_filename, path_name);
	std::strcat(full_filename, right(filename, strlen(filename) - InRstr(filename, "\\"), tmpstring));

	return full_filename;
}

/******************************************************************************************************
*                                                                                                     *
*		zip file/folder                                                                               *
*                                                                                                     *
* from https://stackoverflow.com/questions/118547/creating-a-zip-file-on-windows-xp-2003-in-c-c       *
*                                                                                                     *
******************************************************************************************************/


void zip(char *zipfile, char *item_to_be_zipped)
{
	#define MAX_THREADS 500

	// Create zip file
	FILE* f = fopen(zipfile, "wb");
	fwrite("\x50\x4B\x05\x06\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0", 22, 1, f);
	fclose(f);


	DWORD strlen = 0;
	HRESULT hResult;
	IShellDispatch *pISD;
	Folder *pToFolder = NULL;
	VARIANT vDir, vFile, vOpt;
	BSTR strptr1, strptr2;

	CoInitialize(NULL);
	hResult = CoCreateInstance(CLSID_Shell, NULL, CLSCTX_INPROC_SERVER, IID_IShellDispatch, (void **)&pISD);

	if (SUCCEEDED(hResult) && pISD != NULL)
	{
		strlen = MultiByteToWideChar(CP_ACP, 0, zipfile, -1, 0, 0);
		strptr1 = SysAllocStringLen(0, strlen);
		MultiByteToWideChar(CP_ACP, 0, zipfile, -1, strptr1, strlen);

		VariantInit(&vDir);
		vDir.vt = VT_BSTR;
		vDir.bstrVal = strptr1;
		hResult = pISD->NameSpace(vDir, &pToFolder);

		if (SUCCEEDED(hResult))
		{
			strlen = MultiByteToWideChar(CP_ACP, 0, item_to_be_zipped, -1, 0, 0);
			strptr2 = SysAllocStringLen(0, strlen);
			MultiByteToWideChar(CP_ACP, 0, item_to_be_zipped, -1, strptr2, strlen);

			VariantInit(&vFile);
			vFile.vt = VT_BSTR;
			vFile.bstrVal = strptr2;

			VariantInit(&vOpt);
			vOpt.vt = VT_I4;
			vOpt.lVal = 4;          // Do not display a progress dialog box


	/* Attempt to log current existing threads - failed */
	
	//HANDLE hThrd0[MAX_THREADS];
			DWORD ThreadID0[MAX_THREADS];
			HANDLE h0 = CreateToolhelp32Snapshot(TH32CS_SNAPALL, 0);  //TH32CS_SNAPMODULE, 0);
			DWORD NUM_THREADS0 = 0;
			if (h0 != INVALID_HANDLE_VALUE) {
				THREADENTRY32 te;
				te.dwSize = sizeof(te);
				if (Thread32First(h0, &te)) {
					do {
						if (te.dwSize >= (FIELD_OFFSET(THREADENTRY32, th32OwnerProcessID) + sizeof(te.th32OwnerProcessID))) {
							//only enumerate threads that are called by this process and not the main thread
							if ((te.th32OwnerProcessID == GetCurrentProcessId()) && (te.th32ThreadID != GetCurrentThreadId())) {
								ThreadID0[NUM_THREADS0] = te.th32ThreadID;
								//printf("Process 0x%04x Thread 0x%04x\n", te.th32OwnerProcessID, te.th32ThreadID);
		//						hThrd0[NUM_THREADS0] = OpenThread(THREAD_ALL_ACCESS, FALSE, te.th32ThreadID);
								NUM_THREADS0++;
							}
						}
						te.dwSize = sizeof(te);
					} while (Thread32Next(h0, &te));
				}
				CloseHandle(h0);
			}
			

			hResult = NULL;
			//Copying
			hResult = pToFolder->CopyHere(vFile, vOpt); //NOTE: this appears to always return S_OK even on error
			/*
				* 1) Enumerate current threads in the process using Thread32First/Thread32Next
				* 2) Start the operation
				* 3) Enumerate the threads again
				* 4) Wait for any new threads using WaitForMultipleObjects
				*
				* Of course, if the operation creates any new threads that don't exit, then you have a problem.
				*/
			
			if (hResult == S_OK) {
				//NOTE: hard-coded for testing - be sure not to overflow the array if > 5 threads exist
				HANDLE hThrd[MAX_THREADS];
				DWORD ThreadID[MAX_THREADS];
				HANDLE h = CreateToolhelp32Snapshot(TH32CS_SNAPALL, 0);  //TH32CS_SNAPMODULE, 0);
				DWORD NUM_THREADS = 0;
				if (h != INVALID_HANDLE_VALUE) {
					THREADENTRY32 te;
					int Threads_all_nb = 0;
					te.dwSize = sizeof(te);
					if (Thread32First(h, &te)) {
						do {
							if (te.dwSize >= (FIELD_OFFSET(THREADENTRY32, th32OwnerProcessID) + sizeof(te.th32OwnerProcessID))) {
								//only enumerate threads that are called by this process and not the main thread
								if ((te.th32OwnerProcessID == GetCurrentProcessId()) && (te.th32ThreadID != GetCurrentThreadId())) {
									DWORD ThreadID_index = 0;
									BOOL Is_ZipThread = TRUE;
									Threads_all_nb++;
									while (Is_ZipThread && ThreadID_index < NUM_THREADS0) if (te.th32ThreadID == ThreadID0[ThreadID_index]) Is_ZipThread = FALSE; else ThreadID_index++;

									if (Is_ZipThread) {
										//printf("Process 0x%04x Thread 0x%04x\n", te.th32OwnerProcessID, te.th32ThreadID);
										ThreadID[NUM_THREADS] = te.th32ThreadID;
										hThrd[NUM_THREADS] = OpenThread(THREAD_ALL_ACCESS, FALSE, te.th32ThreadID);
										NUM_THREADS++;
									}
								}
							}
							te.dwSize = sizeof(te);
						} while (Thread32Next(h, &te));
					}
					CloseHandle(h);

					//Wait for all threads to exit
					WaitForMultipleObjects(NUM_THREADS, hThrd, TRUE, INFINITE);
					//(Usually object/thread closed is the last one)
					//DWORD object_closed = WaitForMultipleObjects(NUM_THREADS, hThrd, TRUE, INFINITE) - WAIT_OBJECT_0;
					//WaitForSingleObject(hThrd[NUM_THREADS - 1], INFINITE);

					//Close All handles
					for (DWORD i = 0; i < NUM_THREADS; i++) {
						CloseHandle(hThrd[i]);
					}
				} //if invalid handle
			} //if CopyHere() hResult is S_OK
			

			SysFreeString(strptr2);
			pToFolder->Release();
		}

		SysFreeString(strptr1);
		pISD->Release();
	}
	CoUninitialize();
}