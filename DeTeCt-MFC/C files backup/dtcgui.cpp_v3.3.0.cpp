#define _WIN32_WINNT _WIN32_WINNT_WINXP

/********************************************************************************/
/*                                                                              */
/*			DTC	(c) Luis Calderon, Marc Delcroix, Jon Juaristi 2012-			*/
/*                                                                              */
/********************************************************************************/
#include "stdafx.h"
#include "processes_queue.h"
#include "dtcgui.hpp"
#include "common2.h"
#include "DeTeCt-MFC.h"
#include <windows.h> //after processes_queue.h
#include <string>
#include <vector>
#include <queue>
#include <iostream>
#include <ctime>
#include <numeric>
//#include <algorithm>
#include <regex>
#include <fstream>

#include "DeTeCt-MFCDlg.h"
#include <strsafe.h>

#include <shldisp.h>
#include <tlhelp32.h>
#include <stdio.h>

#include <direct.h>

void	LogString(CString log_cstring, CString output_filename, int *log_counter, BOOL GUI_display);
BOOL	GetOtherProcessedFiles(const int acquisition_index, int *pacquisition_index_children, int  *pacquisitions_to_be_processed, int *pnb_error_impact, int *pnb_null_impact, int *pnb_low_impact, int *pnb_high_impact, double *pduration_total, std::vector<std::string> *plog_messages, char *DeTeCtQueueFilename);
int		ForksInstances(const int maxinstances, const int detect_PID, const CString DeTeCtQueueFilename, const int scan_time, const int scan_time_random_max, int *pnbinstances);
void	DisplayProcessingTime(clock_t *pcomputing_threshold_time, clock_t *plast_time, const clock_t refresh_duration, const clock_t single_time, const clock_t total_time);
CString			TotalType();
Instance_type	InstanceType(CString *pinstance_text);
void			AcquisitionFileListToQueue(AcquisitionFilesList *pacquisition_files, const CString tag_current, const int index_current, const CString out_directory, int *acquisitions_to_be_processed);



/** @brief	Options for the algorithm */

#define MAX_RANGE_PROGRESS SHRT_MAX

OPTS opts;
char impact_detection_dirname[MAX_STRING];
//char zip_detection_dirname[MAX_STRING];
char zip_detection_location[MAX_STRING];
char zipfile[MAX_STRING] = "";
char log_detection_dirname[MAX_STRING] = "";

extern CDeTeCtMFCDlg dlg;
//extern CDeTeCtMFCDlg::CButton AS;
//extern CDeTeCtMFCDlg dark;

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
		closedir(directory);
		return;
	}
	if (!(entry = readdir(directory))) {
		closedir(directory);
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

					read_autostakkert_session_file(folder + "\\" + file, &acquisition_file, NULL, &cm_list_start, &cm_list_end, &cm_frame_count);

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
//debug_log.close();
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

int detect_impact(DTCIMPACT *dtc, DTCIMPACT *dtcout, double meanValue, LIST *list, ITEM** dtcMax, double radius, double incrLum, int impact_frames_min)
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

int detect(std::vector<std::string> current_file_list, OPTS *popts, std::string scan_folder_path) {
	
	// ******************** INITIALIZATION *********************
	clock_t				begin, begin_total, end;
	const int			wait_imagedisplay_seconds		= 3;
	int					queue_scan_delay				= CLOCKS_PER_SEC * 2;		// interval waiting time for scanning new jobs (ms)
	int					queue_scan_delay_random_max		= CLOCKS_PER_SEC;		// additionnal max random waiting time for scanning new jobs (ms)
	int					check_children_time_factor		= 5;
	clock_t				computing_refresh_duration		= CLOCKS_PER_SEC / 2;		//interval for refreshing computing time (ms)
	clock_t				check_threshold_time_inc = wait_imagedisplay_seconds * check_children_time_factor * CLOCKS_PER_SEC;	// for interval for checking children results during parent capture processing
	int					log_counter						= 0;

	std::stringstream	logline_tmp;
	std::string			start_runtime = getRunTime().str().c_str();
	std::wstring		wstart_time = std::wstring(start_runtime.begin(), start_runtime.end());
	CString				log_cstring;
	std::string			log_directory;
	CString				message_init;
	Instance_type		instance_type;
	CString				instance_type_cstring;
	int					nb_instances = 0;
	int					nb_new_instances;
	int					maxinstances_previous;

	std::wstring		detection_folder_fullpathname = {};	// folder to store detection results
	std::wstring		detection_folder_name = {};			// folder to store detection results
	std::wstring		details_folder_fullpathname = {};		// subfolder to store details for detection results
	char				max_folder_path_filename[MAX_STRING];
	char				diff_folder_path_filename[MAX_STRING];
	char				tmpstring[MAX_STRING];
	char				tmpstring2[MAX_STRING];
	DIR					*dir_tmp;
	BOOL				GUI_display = TRUE;

	cv::setUseOptimized(true);
	maxinstances_previous = popts->maxinstances;

	//											maxinst !as parent																												as
	// string	log_directory:					"G:\\work\\Impact\\tests\\autostakkert3.2_detect"																				"G:\\Work\\Impact\\DeTeCt-PSWS\\DeTeCt-MFC\\x64\\Release"
	// string	log_consolidated =				log_directory
	// string	log								"G:\\work\\Impact\\tests\\autostakkert3.2_detect\\Impact_detection_run@2020-05-24_07-28-20"	(from queue file or calculated)
	// wstring	detection_folder_fullpathname	"G:\\work\\Impact\\tests\\autostakkert3.2_detect\\Impact_detection_run@2020-05-24_07-28-20"
	// wstring	detection_folder_name			"Impact_detection_run@2020-05-24_07-28-20"
	// char		popts->impactdirname				detection_folder_fullpathname
	// char		impact_detection_dirname		detection_folder_fullpathname
	// wstring	details_folder_fullpathname		"G:\\work\\Impact\\tests\\autostakkert3.2_detect\\Impact_detection_run@2020-05-24_07-28-20\\details"
	// wstring	output_log_file					"G:\\work\\Impact\\tests\\autostakkert3.2_detect\\Impact_detection_run@2020-05-24_07-28-20\\output.log"

	//log directory when not in autostakkert mode and  not in multi instance mode
	if (!popts->autostakkert) log_directory = scan_folder_path;
	else {
		//log directory when autostakkert mode or multi instance mode
		log_directory = CString2string(DeTeCt_exe_folder());;
	}
	std::string log_consolidated(log_directory);	// Location where log will be written
	std::string log(log_directory);					// Location where consolidated log will be written

	if (GetItemFromQueue(&log_cstring, _T("output_dir"), (CString)popts->DeTeCtQueueFilename)) {
/*		 && (!popts->parent_instance)) {	// if detect queue exists and has already an outputdir, then child instance.
*/		log = CString2string(log_cstring);
	} else {																		// otherwise parent instance (autostakkert mode, multiple instances mode or single instance mode)
		log.append("\\Impact_detection_run@").append(start_runtime);
		CString log_string(log.c_str());
		PushItemToQueue(log_string, _T("output_dir"), (CString)popts->DeTeCtQueueFilename, TRUE);
		popts->parent_instance = TRUE;
/*		}*/
	}
	std::string detection_folder_name_string = log.substr(log.find_last_of("\\") + 1, log.length());
	detection_folder_name = std::wstring(detection_folder_name_string.begin(), detection_folder_name_string.end());

//	detection_folder_fullpathname = std::wstring(scan_folder_path.begin(), scan_folder_path.end());
//	detection_folder_fullpathname =	std::wstring(log_directory.begin(), log_directory.end());
//	detection_folder_fullpathname =	detection_folder_fullpathname.append(L"\\Impact_detection_run@").append(wstart_time);
	detection_folder_fullpathname =	std::wstring(log.begin(), log.end());
	std::string detection_folder_fullpathname_string(detection_folder_fullpathname.begin(), detection_folder_fullpathname.end());
	//detection_folder_name =			detection_folder_name.append(L"Impact_detection_run@").append(wstart_time); //bug
	//std::string detection_folder_name_string(detection_folder_name.begin(), detection_folder_name.end());
	strcpy(popts->impactdirname, detection_folder_fullpathname_string.c_str());
	strcpy(impact_detection_dirname, detection_folder_fullpathname_string.c_str());

	// usage of mkdir only solution found to handle directory names with special characters (eg. é, à, ...)
		//if (GetFileAttributes(detection_folder_fullpathname.c_str()) == INVALID_FILE_ATTRIBUTES)
		//CreateDirectory(detection_folder_fullpathname.c_str(), 0);
	if (!(dir_tmp = opendir(detection_folder_fullpathname_string.c_str()))) mkdir(detection_folder_fullpathname_string.c_str());
	else closedir(dir_tmp);
	if (popts->detail || popts->allframes) {
		details_folder_fullpathname = std::wstring(detection_folder_fullpathname.begin(), detection_folder_fullpathname.end());
		details_folder_fullpathname = details_folder_fullpathname.append(L"\\details");
		std::string details_folder_fullpathname_string(details_folder_fullpathname.begin(), details_folder_fullpathname.end());
		if (!(dir_tmp = opendir(details_folder_fullpathname_string.c_str()))) mkdir(details_folder_fullpathname_string.c_str());
		else closedir(dir_tmp);
	}

	//outputFolder = outputFolder.replace(0, log_directory.length() + 1, "");
	//outputFolder = outputFolder.replace(0, log.find_last_of("\\") + 1, "");

	//std::wstring output_log_file0(log_directory.begin(), log_directory.end());
	std::string output_log_file_string(log.begin(), log.end());
	output_log_file_string = output_log_file_string.replace(log.find_last_of("\\"), log.length() - log.find_last_of("\\"), "");
	std::wstring output_log_file(output_log_file_string.begin(), output_log_file_string.end());

	//output_log_file = output_log_file.append(L"\\Impact_detection_run@").append(wstart_time).append(L"\\").append(OUTPUT_FILENAME).append(DTC_LOG_SUFFIX); //bug
	output_log_file = output_log_file.append(L"\\").append(detection_folder_name).append(L"\\").append(OUTPUT_FILENAME).append(DTC_LOG_SUFFIX);
	
	dtcWriteLogHeader(log_consolidated);
	dtcWriteLogHeader(log);

	message_init = _T("\n");
	instance_type = InstanceType(&instance_type_cstring);
	message_init = message_init + instance_type_cstring + _T(" instance");
	switch (instance_type) {
	case autostakkert_parent:
	case parent:
		message_init = message_init + L", DO NOT CLOSE unless told to do so\n";
		queue_scan_delay = 1;								// waiting time (ms) for scanning new jobs - parent instance
		queue_scan_delay_random_max = 1;
		GUI_display = FALSE;
		break;
	case autostakkert_single:
		message_init = message_init + L", DO NOT CLOSE unless told to do so\n";
		queue_scan_delay = 1;								// waiting time (ms) for scanning new jobs - parent instance
		queue_scan_delay_random_max = 1;
		GUI_display = TRUE;
		break;
	case single:
		queue_scan_delay = 1;								// waiting time (ms) for scanning new jobs - parent instance
		queue_scan_delay_random_max = 1;
		GUI_display = TRUE;
		break;
	case autostakkert_child:
		message_init = message_init + L", will CLOSE AUTOMATICALLY\n";
		queue_scan_delay = CLOCKS_PER_SEC / 10;				// waiting time (ms) for scanning new jobs - child instance
		queue_scan_delay_random_max = CLOCKS_PER_SEC / 2;
		GUI_display = TRUE;
		break;
	case child:
		message_init = message_init + L", will CLOSE AUTOMATICALLY\n";
		queue_scan_delay = CLOCKS_PER_SEC / 2;				// waiting time (ms) for scanning new jobs - child instance
		queue_scan_delay_random_max = CLOCKS_PER_SEC / 2;
		computing_refresh_duration = CLOCKS_PER_SEC * 60;	// hidden mode for child, do not need to display
		GUI_display = TRUE;
		break;
	}
	message_init = message_init + _T("\n");
	LogString(message_init, output_log_file.c_str(), &log_counter, TRUE);
	
	if (opts.parent_instance) {
		nb_instances = 1; // Forks not launched yet, gain of computing time
		instance_type = DisplayInstanceType(&nb_instances);
	} else nb_instances = 0;
	
	std::vector<int> img_save_params = { CV_IMWRITE_JPEG_QUALITY, 100 };
	
	std::vector<LogInfo> logs;
	std::vector<LPCTSTR> logMessages;
	std::vector<std::string> log_messages;	// For SendMailDlg
	
	//log_messages.push_back("");
	if (popts->dateonly) log_messages.push_back("WARNING, datation info only, no detection analysis was performed");

	std::string logmessage;
	std::string short_logmessage;
	Rating_type rating = Error;
	std::string logmessage2;
	std::string logmessage3;
	std::wstring wlogmessage;
	CString Clogmessage;
	CString message_cstring;

	AcquisitionFilesList local_acquisition_files_list;
	
	local_acquisition_files_list.file_list = current_file_list;
	local_acquisition_files_list.acquisition_file_list = current_file_list;
	local_acquisition_files_list.nb_prealigned_frames = {};
	for (int i = 0; i < local_acquisition_files_list.file_list.size(); i++) local_acquisition_files_list.nb_prealigned_frames.push_back(0);
	
	int acquisition_index = 0;
	int acquisition_index_children = 0;
	int acquisitions_processed = 0;
	int acquisitions_to_be_processed = 0;
	
	double duration_total = 0;
	double computation_time_total = 0;
	double start_time_min = gregorian_calendar_to_jd(2080, 1, 1, 0, 0, 0);
	double start_time_max = gregorian_calendar_to_jd(1980, 1, 1, 0, 0, 0);
	double JD_min = gregorian_calendar_to_jd(1980, 1, 1, 0, 0, 0);
	
	Planet_type planet;
	int planet_jupiter = 0;
	int planet_saturn = 0;
	int nb_error_impact = 0;
	int nb_null_impact = 0;
	int nb_low_impact = 0;
	int nb_high_impact = 0;
	float progress_all_status = 0;
	
	clock_t computing_threshold_time = 0;
	clock_t begin_imagedisplay_time = 0;
	clock_t check_threshold_time = 0;
	if (file_exists(CString2string((CString)opts.DeTeCtQueueFilename))) SetIntParamToQueue(popts->maxinstances, _T("max_instances"), (CString)popts->DeTeCtQueueFilename);

	CDeTeCtMFCDlg::getAuto()->SetCheck(!popts->interactive);
	// Initializes the impacts classification in dialog window
	CDeTeCtMFCDlg::getimpactNull()->SetWindowText(std::to_wstring(nb_null_impact + nb_error_impact).c_str());
	CDeTeCtMFCDlg::getimpactLow()->SetWindowText(std::to_wstring(nb_low_impact).c_str());
	CDeTeCtMFCDlg::getimpactHigh()->SetWindowText(std::to_wstring(nb_high_impact).c_str());
	CDeTeCtMFCDlg::getduration()->SetWindowText((CString)"Duration processed (" + TotalType() + "): " + std::to_wstring((int)duration_total).c_str() + (CString)"s");
	CDeTeCtMFCDlg::getprobability()->SetWindowText((CString)("Probability ") + (CString)("(") + (CString)(TotalType()) + (CString)(") :"));

	std::wstring totalProgress_wstring;
	Datation_source datation_source;

	//if ((popts->parent_instance) && (popts->maxinstances > 1) && (!file_exists(CString2string((CString)popts->DeTeCtQueueFilename)))) dlg.OnFileExit();	// exits DeTeCt if Queuefile does not exists (should not occur because output_dir line created)
	if ((popts->parent_instance) && (strlen(popts->DeTeCtQueueFilename) > 0)) SetFileProcessingFromQueue((CString)local_acquisition_files_list.file_list.at(0).c_str(), (CString)popts->DeTeCtQueueFilename);
	LogString(_T("Processing file : ") + (CString)local_acquisition_files_list.file_list.at(0).c_str(), output_log_file.c_str(), &log_counter, GUI_display);
	
	//std::vector<std::string> local_file_list;
	//local_file_list = file_list;
	
	CDeTeCtMFCDlg::getProgress_all()->SetRange(0, MAX_RANGE_PROGRESS);
	CDeTeCtMFCDlg::getProgress_all()->SetStep(1);
	CDeTeCtMFCDlg::getProgress_all()->SetPos(0);
	CDeTeCtMFCDlg::getProgress()->SetRange(0, MAX_RANGE_PROGRESS);
	CDeTeCtMFCDlg::getProgress()->SetStep(1);
	CDeTeCtMFCDlg::getProgress()->SetPos(0);

	DisplayProcessingTime(&computing_threshold_time, &begin_total, computing_refresh_duration, 0, 0);
//	processing_time_cstring.Format(L"Processing time: %.*fs (file)  %.*fs (total)", 1, 0.0, 1, 0.0);
	begin = begin_total;
	check_threshold_time = begin + (clock_t)(check_threshold_time_inc);
	std::string filename;

	do
	{
		if ((popts->parent_instance) && (strlen(popts->DeTeCtQueueFilename) > 0)) acquisitions_to_be_processed = NbFilesFromQueue((CString)popts->DeTeCtQueueFilename);
		else acquisitions_to_be_processed += local_acquisition_files_list.file_list.size();

		//for (std::string filename : local_acquisition_files_list.acquisition_file_list) {
		while (acquisition_index < local_acquisition_files_list.file_list.size()) {
			std::string filename_acquisition;
			filename =				local_acquisition_files_list.file_list.at(acquisition_index);
			filename_acquisition =	local_acquisition_files_list.acquisition_file_list.at(acquisition_index);
			acquisition_index++;

// Debug
//			if ((!file_exists(CString2string((CString)opts.DeTeCtQueueFilename))))  AcquisitionFileListToQueue(&local_acquisition_files_list, _T("file_processing"), acquisition_index - 1, (CString)log.c_str(), &acquisitions_to_be_processed);
// Debug


			if (file_exists(CString2string((CString)opts.DeTeCtQueueFilename))) popts->maxinstances = GetIntParamFromQueue(_T("max_instances"), (CString)popts->DeTeCtQueueFilename);
			maxinstances_previous = popts->maxinstances;
			if ((popts->maxinstances > 1) && (!file_exists(CString2string((CString)opts.DeTeCtQueueFilename)))) AcquisitionFileListToQueue(&local_acquisition_files_list, _T("file_processing"), acquisition_index - 1, (CString)log.c_str(), &acquisitions_to_be_processed);
			nb_new_instances = ForksInstances(popts->maxinstances, popts->detect_PID, (CString)popts->DeTeCtQueueFilename, queue_scan_delay, queue_scan_delay_random_max, &nb_instances);
			if (nb_new_instances > 1)		LogString((CString)std::to_string(nb_new_instances).c_str() + _T(" new instances launched"), output_log_file.c_str(), &log_counter, TRUE);
			else if (nb_new_instances == 1) LogString((CString)std::to_string(nb_new_instances).c_str() + _T(" new instance launched"), output_log_file.c_str(), &log_counter, TRUE);

			/********** Init **********/

			std::vector<cv::Point> cm_list = {};
			
			int cm_list_start = 0;
			int cm_list_end = 9999999;
			int cm_frame_count = 0;
			
			begin_imagedisplay_time = 0;
			CDeTeCtMFCDlg::getAS()->SetCheck(0);
			CDeTeCtMFCDlg::getdark()->SetCheck(0);
			CDeTeCtMFCDlg::getacquisitionLog()->SetCheck(0);
			CDeTeCtMFCDlg::getSER()->SetCheck(0);
			CDeTeCtMFCDlg::getSERtimestamps()->SetCheck(0);
			CDeTeCtMFCDlg::getFITS()->SetCheck(0);
			CDeTeCtMFCDlg::getFileInfo()->SetCheck(0);
			CDeTeCtMFCDlg::getacquisitionSW()->SetWindowText(L"");

//***** gets acquisition file and number of framesfrom autostakkert session file
			std::string extension = filename.substr(filename.find_last_of(".")+1, filename.size()-filename.find_last_of(".")-1);
			std::string filename_autostakkert = "";
			if (extension.compare(AUTOSTAKKERT_EXT) == 0) {

				filename_autostakkert = filename;
				CDeTeCtMFCDlg::getAS()->SetCheck(1);
				read_autostakkert_session_file(filename, &filename_acquisition, &cm_list, &cm_list_start, &cm_list_end, &cm_frame_count);
				//filename = filename_acquisition;
				//local_acquisition_files_list.nb_prealigned_frames.push_back(MIN(cm_list_end - cm_list_start + 1, cm_frame_count));
				local_acquisition_files_list.nb_prealigned_frames.at(acquisition_index - 1) = MIN(cm_list_end - cm_list_start + 1, cm_frame_count);
			}
			else {
				filename_autostakkert = "";
				CDeTeCtMFCDlg::getAS()->SetCheck(0);
				//local_acquisition_files_list.nb_prealigned_frames.push_back(cm_frame_count);
				local_acquisition_files_list.nb_prealigned_frames.at(acquisition_index - 1) = cm_frame_count;
			}
			popts->filename = strdup(filename_acquisition.c_str());
			std::string outputFolder = filename_acquisition.substr(0, filename_acquisition.find_last_of("\\") + 1);
//			outputFolder = outputFolder.replace(0, scan_folder_path.length() + 1, "");
// BUG filename incomplet?			
//outputFolder = outputFolder.replace(0, log_directory.length() + 1, "");
			outputFolder = outputFolder.replace(0, log.find_last_of("\\") + 1, "");
			std::replace(outputFolder.begin(), outputFolder.end(), '\\', '_');
			std::replace(outputFolder.begin(), outputFolder.end(), ' ', '_');

			std::string filePath = filename_acquisition.substr(filename_acquisition.find_last_of("\\") + 1, filename_acquisition.find_last_of("."));
			filePath = filePath.substr(0, filePath.find_last_of("."));

			std::string folderPath = filename_acquisition.substr(0, filename_acquisition.find_last_of("\\") + 1);
			std::string outputfilename = folderPath.append(outputFolder).append(filePath).append(".jpg");
			popts->ofilename = strdup(outputfilename.c_str());
			std::wstring filename_wstring(filename_acquisition.begin(), filename_acquisition.end());
			std::string short_filename = filename_acquisition.substr(filename_acquisition.find_last_of("\\") + 1, filename_acquisition.length());
			std::wstring short_filename_wstring(short_filename.begin(), short_filename.end());

			std::string message = "----- ";
			if ((instance_type == single) || (instance_type == autostakkert_single)) message = message + std::to_string(acquisitions_processed + 1) + "/" + std::to_string(acquisitions_to_be_processed) + " : ";
			message = message + short_filename + " start -----";
//				std::string message2 = "----- " + std::to_string(acquisition_index) + "/" + std::to_string(acquisitions_to_be_processed) + " : " + filename + " start  --------------";
			totalProgress_wstring = L"Total\n(" + std::to_wstring(acquisitions_processed + acquisition_index_children) + L"/" + std::to_wstring(acquisitions_to_be_processed) + L")";
			CDeTeCtMFCDlg::gettotalProgress()->SetWindowText(totalProgress_wstring.c_str());
			CDeTeCtMFCDlg::getfileName()->SetWindowText(short_filename_wstring.c_str());
			message_cstring = (filename_wstring).c_str();
			//message_cstring = filename_wstring.c_str();
			CDeTeCtMFCDlg::getfileName()->SetWindowText(message_cstring);

			//TODO: usage of cmlist and quality information from as3

			//impactDetectionLog.AddString((CString)getDateTime().str().c_str() + ss2.str().c_str());
			LogString((CString)message.c_str(), output_log_file.c_str(), &log_counter, GUI_display);
//				output_log << getDateTime().str().c_str() << message2.c_str() << "\n";
				
			init_string(tmpstring);
			init_string(tmpstring2);
			std::string detail_folder_path_string(details_folder_fullpathname.begin(), details_folder_fullpathname.end());


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
			cv::Mat pFlatADUmaxMat;	// For flat generation
			cv::Mat pGryFullMat;	// Grey fullframe

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
			cv::Mat pFlatADUmaxImg;

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

			int pGryImg_height = 0;
			int pGryImg_width = 0;
			char ofilenamediff[MAX_STRING] = "";
			char ofilenamemax[MAX_STRING] = "";

			char comment[MAX_STRING] = "";
			char rating_classification[MAX_STRING] = "";
			char rating_filename_suffix[MAX_STRING] = "";
			double duration = 0;

			double start_time;
			double end_time;
			TIME_TYPE timetype;

			int nb_impact = -1;
				
			int nframe = 0;
			int frame_error = 0;
			int frame_errors = 0;
			int frame_number;

			int darkfile_ok = 0;
			lum.val[0] = 0.0;
		

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
				//clock_t check_threshold_time = begin + (clock_t) (check_threshold_time_inc);

				std::vector<long> frames;

				//***** Opens acquisition file
				if (!(pCapture = dtcCaptureFromFile2(popts->filename, &nframe))) {
					LogString(L"Cannot open file " + (CString)popts->filename, output_log_file.c_str(), &log_counter, TRUE);
					continue;
				}
				frame_number = nframe;
				// sets progress bar configuration
								//CDeTeCtMFCDlg::getProgress()->SetRange(0, (short) (frame_number));
				CDeTeCtMFCDlg::getProgress()->SetPos(0);

				// ***** Gets datation info from acquisition
				dtcGetDatation(pCapture, popts->filename, nframe, &start_time, &end_time, &duration, &fps_real, &timetype, comment, &planet, &datation_source);
				if (datation_source.acquisition_log_file) {
					CDeTeCtMFCDlg::getacquisitionLog()->SetCheck(1);
					CString datation_source_cstring(datation_source.acquisition_software);
					CDeTeCtMFCDlg::getacquisitionSW()->SetWindowText(datation_source_cstring);
				}
				if (datation_source.ser_file)				CDeTeCtMFCDlg::getSER()->SetCheck(1);
				if (datation_source.ser_file_timestamp)		CDeTeCtMFCDlg::getSERtimestamps()->SetCheck(1);
				if (datation_source.fits_file)				CDeTeCtMFCDlg::getFITS()->SetCheck(1);
				if (datation_source.file_info)				CDeTeCtMFCDlg::getFileInfo()->SetCheck(1);

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
				impact_frames_min = (int)ceil(MAX(popts->incrFrameImpact, fps * popts->impact_duration_min));
				/*********************************DATE ONLY MODE******************************************/
				if (popts->dateonly) {
					LogString(L"Datation for capture of " + (CString)std::to_string(nframe).c_str() + L" frames @ " + (CString)std::to_string(fps_int).c_str() + L" fps", output_log_file.c_str(), &log_counter, TRUE);
					message = "-------------- " + short_filename + " end --------------";
					LogString(+(CString)message.c_str(), output_log_file.c_str(), &log_counter, TRUE);
					double fake_stat[3] = { 0.0, 0.0, 0.0 };
					//LogInfo info(popts->filename, start_time, end_time, duration, fps_real, timetype, "WARNING, datation info only", 0, 0);
					LogInfo info(popts->filename, start_time, end_time, duration, fps_real, timetype, comment, 0, 0, 0, fake_stat, fake_stat, fake_stat, fake_stat, fake_stat, fake_stat, rating_classification);

					std::stringstream logline;
					dtcWriteLog2(log_consolidated, info, &logline);
					log_messages.push_back(logline.str() + "\n");
					dtcWriteLog2(log, info, &logline_tmp);
					dtcReleaseCapture(pCapture);
					pCapture = NULL;
					continue;
				}

				//****************** NON DATE ONLY MODE *******************************/
				//message = "Initialization & differential photometry: " + std::to_string(nframe) + " frames @ " + std::to_string(fps_int) + " fps (" + std::to_string((int)duration) + "s duration)";
				message = std::to_string(nframe) + " frames @ " + std::to_string(fps_int) + " fps (" + std::to_string((int)duration) + "s duration)";
				message_cstring = message_cstring + (CString)"\n" + (CString)message.c_str() + (CString)"\n";
				CDeTeCtMFCDlg::getfileName()->SetWindowText(message_cstring);

				/*					CDeTeCtMFCDlg::getLog()->AddString((CString)getDateTime().str().c_str() + L"Initializing capture: " +
									(CString)std::to_string(nframe).c_str() + L" frames @ " + (CString)std::to_string(fps_int).c_str() + L" fps" +
									L" (" + (CString)std::to_string(impact_frames_min).c_str() + L" frames min for impact)");*/
				LogString(+(CString)message.c_str(), output_log_file.c_str(), &log_counter, GUI_display);
				/*					output_log << getDateTime().str().c_str() << "Initializing capture:  " << nframe << " frames @ " << fps_int
									<< " fps (" << impact_frames_min << " frames min for impact)\n";*/
									//Gets ROI, check if ROI is not big enough and exit then
				if (popts->wROI && popts->hROI) {
					croi = cv::Rect(0, 0, popts->wROI, popts->hROI);
				}
				else {
					croi = dtcGetFileROIcCM(pCapture, popts->ignore);
					dtcReinitCaptureRead2(&pCapture, popts->filename);
					if ((croi.width <= popts->ROI_min_size) || (croi.height <= popts->ROI_min_size)) {
						message = "-------------- " + short_filename + " end --------------";
						LogString(+(CString)message.c_str(), output_log_file.c_str(), &log_counter, GUI_display);

						LogString(L"WARNING: ROI " +
							(CString)std::to_string(croi.width).c_str() + L"x" + (CString)std::to_string(croi.height).c_str() + L" to small (" +
							(CString)std::to_string(popts->ROI_min_size).c_str() + L"x" + (CString)std::to_string(popts->ROI_min_size).c_str() + L"), ignoring stopping processing", output_log_file.c_str(), &log_counter, GUI_display);

						//message = "WARNING: ROI " + croi.x + "x" + croi.y + " to small (" + ROI_MIN + "x" + ROI_MIN + "), ignoring stopping processing";
						//CDeTeCtMFCDlg::getLog()->AddString((CString)getDateTime().str().c_str() + (CString)message.c_str());
						dtcReleaseCapture(pCapture);
						pCapture = NULL;
						continue;
					}
				}

				if (popts->viewDif) cv::namedWindow("Initial differential photometry");
				if (popts->viewRef) cv::namedWindow("Reference frame");
				if (popts->viewROI) cv::namedWindow("ROI");
				if (popts->viewTrk) cv::namedWindow("Tracking");
				if (popts->viewMsk) cv::namedWindow("Mask");
				if (popts->viewThr) cv::namedWindow("Thresholded differential photometry");
				if (popts->viewSmo) cv::namedWindow("Smoothed differential photometry");
				if (popts->viewRes) cv::namedWindow("Resulting differential photometry");
				if (popts->viewHis) cv::namedWindow("Histogram");

				nframe = 0;
				//Process dark file if existing, but not for Winjupos derotated files and PIPP files as a regular dark file would not be suitable if the images have been modified
				if ((popts->darkfilename) && (InStr(popts->filename, WJ_DEROT_STRING) < 0) && (InStr(popts->filename, PIPP_STRING) < 0)) {
					char darklongfilename[MAX_STRING];
					strncpy(darklongfilename, popts->filename, InRstr(popts->filename, "\\") + 1);
					strcat(darklongfilename, popts->darkfilename);
					if (!(pADUdarkMat = cv::imread(darklongfilename, CV_LOAD_IMAGE_GRAYSCALE)).data) {
						/*CDeTeCtMFCDlg::getLog()->AddString((CString)getDateTime().str().c_str() + L"Warning: cannot read dark frame " +
							(CString)std::string(darklongfilename).c_str());
						CDeTeCtMFCDlg::getLog()->SetTopIndex(CDeTeCtMFCDlg::getLog()->GetCount() - 1);
						CDeTeCtMFCDlg::getLog()->RedrawWindow();
						output_log << getDateTime().str().c_str() << "Warning: cannot read dark frame:  " << darklongfilename << "\n";
						output_log.flush();*/
						darkfile_ok = 0;
						CDeTeCtMFCDlg::getdark()->SetCheck(0);
					}
					else {
						LogString(+L"Reading dark frame " + (CString)std::string(darklongfilename).c_str(), output_log_file.c_str(), &log_counter, GUI_display);
						darkfile_ok = 1;
						CDeTeCtMFCDlg::getdark()->SetCheck(1);
					}
				}

				/*********************************CAPTURE READING******************************************/
				while ((pFrame = dtcQueryFrame2(pCapture, popts->ignore, &frame_error)).data) {
					if (popts->debug) {
						//cv::imshow("Frame " + std::to_string(nframe), pFrame);
						//cv::waitKey(1);
					}
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
						if (opts.flat_preparation) pGryFullMat = dtcGetGrayMat(&pFrame, pCapture);
						cv::medianBlur(pGryMat, pGryMat, 1);
						//cv::imshow("Frame read 2", pGryMat);
						//cv::waitKey(0);
													//dtcApplyMaskToFrame(pGryMat);
													//cv::GaussianBlur(pGryMat, pGryMat, cv::Size(1, 1), 1);
						if (darkfile_ok == 1) {
							if ((pADUdarkMat.rows != pGryMat.rows) || (pADUdarkMat.cols != pGryMat.cols)) {
								LogString(+L"Warning: dark frame " +
									(CString)std::string(popts->darkfilename).c_str() + L" differs from the frame properties " +
									(CString)std::to_string(pADUdarkMat.rows).c_str() + L" vs " +
									(CString)std::to_string(pGryMat.rows).c_str() + L" rows, " +
									(CString)std::to_string(pADUdarkMat.cols).c_str() + L" vs " +
									(CString)std::to_string(pGryMat.cols).c_str() + L" cols", output_log_file.c_str(), &log_counter, GUI_display);
								darkfile_ok = 0;
								CDeTeCtMFCDlg::getdark()->SetCheck(0);
							}
							else {
								cv::Mat pGryDarkMat;
								pGryDarkMat = cv::Mat(pGryMat.size(), pGryMat.type());
								cv::subtract(pGryMat, pADUdarkMat, pGryDarkMat);
								cv::threshold(pGryDarkMat, pGryMat, 0, 0, CV_THRESH_TOZERO);
								pGryDarkMat.release();
								pGryDarkMat = NULL;
								CDeTeCtMFCDlg::getdark()->SetCheck(1);
							}
						}

						/*******************FIRST FRAME PROCESSING*******************/
						if (nframe == 1) {
							pGryMat.copyTo(pFirstFrameROIMat);
							pGryMat.convertTo(pGryMat, CV_8U);
							if (opts.flat_preparation) pGryFullMat.convertTo(pGryFullMat, CV_8U);
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

							if (!popts->wait && (popts->viewROI || popts->viewTrk || popts->viewDif || popts->viewRef ||
								popts->viewThr || popts->viewSmo || popts->viewRes || popts->viewHis)) {
								if (fps_int > 0) {
									popts->wait = (int)(1000 / std::ceil(fps_int));
								}
								else {
									popts->wait = (int)(1000 / 25);
								}
							}

							nb_impact = 0;
							//init_list(&ptlist, (fps_int * popts->timeImpact));
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

							if (opts.flat_preparation) pFlatADUmaxMat = cv::Mat::zeros(pGryFullMat.size(), CV_32F);
							if ((popts->ofilename) && (popts->allframes)) {
								pADUdtcMat = cv::Mat(pFirstFrameROIMat.size(), CV_32F);
								pADUavgMatFrame = cv::Mat(pFirstFrameROIMat.size(), CV_32F);
							}
							if (popts->thrWithMask || popts->viewMsk || (popts->ovfname && (popts->ovtype == OTYPE_MSK))) {
								pMskMat = cv::Mat(pFirstFrameROIMat.size(), CV_32F);
							}
							if (popts->viewThr) {
								pThrMat = cv::Mat(pFirstFrameROIMat.size(), CV_32F);
							}
							if (popts->filter.type >= 0 || popts->viewSmo) {
								pSmoMat = cv::Mat(pFirstFrameROIMat.size(), CV_32F);
							}
							if (popts->viewTrk || (popts->ovtype == OTYPE_TRK && popts->ovfname)) {
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
						if (opts.flat_preparation) pGryFullMat.convertTo(pGryFullMat, CV_8U);
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
							pFrameROI = dtcGetGrayImageROIcCM(maskedGryMat, cm, (float)popts->medSize, popts->facSize, popts->secSize);

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
								if ((opts.flat_preparation) & (pGryFullMat.type() != CV_32F)) pGryFullMat.convertTo(pGryFullMat, CV_32F);
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
								if (opts.flat_preparation) pGryFullMat.convertTo(pGryFullMat, CV_32F);
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
									if (popts->viewDif) {
										cv::minMaxLoc(pDifMat, &minLum, &maxLum, &minPoint, &maxPoint);
										pDifMat.convertTo(pDifImg, -1, 255.0 / maxLum, 0);
										pDifImg.convertTo(pDifImg, CV_8U);
										cv::imshow("Initial differential photometry", pDifImg);
										cv::waitKey(1);
										pDifImg.release();
										pDifImg = NULL;
									}
									if (nframe == popts->nsaveframe && popts->ofilename && popts->ostype == OTYPE_DIF) {
										cv::imwrite(popts->ofilename, pDifMat, img_save_params);
									}
								}


								//cv::threshold(pDifMat, pThrMat, popts->threshold, 0.0, CV_THRESH_TOZERO);
								//cv::threshold(pDifMat, pDifMat, popts->threshold, 0.0, CV_THRESH_TOZERO);

								if (popts->filter.type > 0) {
									switch (popts->filter.type) {
									case FILTER_BLUR:
										//Size 5x5
										cv::blur(pDifMat, pDifMat, cv::Size(popts->filter.param[0], popts->filter.param[0]));
										break;
									case FILTER_MEDIAN:
										//Size 5
										cv::medianBlur(pDifMat, pDifMat, popts->filter.param[0]);
										break;
									case FILTER_GAUSSIAN:
										//Size 5x5 Sigma 0
										cv::GaussianBlur(pDifMat, pDifMat, cv::Size(popts->filter.param[0],
											popts->filter.param[1]), popts->filter.param[2]);
										break;
									}
								}

								pDifMat.copyTo(pSmoMat);

								if (popts->viewSmo && pSmoMat.data) {
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

								if (popts->viewMsk && pMskMat.data) {
									//pMskMat.convertTo(pMskImg, CV_8U);
									cv::minMaxLoc(pMskMat, &minLum, &maxLum, &minPoint, &maxPoint);
									pMskMat.convertTo(pMskImg, -1, 255.0 / maxLum, 0);
									pMskImg.convertTo(pMskImg, CV_8U);
									cv::imshow("Mask", pMskImg);
									cv::waitKey(1);
								}

								//cv::blur(pDifMat, pDifMat, cv::Size(3,3));
								//cv::threshold(pDifMat, pDifMat, popts->threshold, 0.0, CV_THRESH_TOZERO);

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
								if (opts.flat_preparation) cv::max(pFlatADUmaxMat, pGryFullMat, pFlatADUmaxMat);
								if (popts->ofilename && popts->allframes) {
									pADUavgMat.convertTo(pADUavgMatFrame, -1, 1.0 / (nframe - frame_errors), 0);
									pADUavgDiffMat.convertTo(pADUavgDiffMat, -1, 1.0 / (nframe - frame_errors), 0);
									cv::subtract(pADUmaxMat, pADUavgMatFrame, pADUdtcMat);
									cv::minMaxLoc(pADUdtcMat, &minLum, &maxLum, &minPoint, &maxPoint);
									pADUdtcMat.convertTo(pADUdtcMat, -1, 255.0 / maxLum, 0);
									strncpy(ofilenamemax, popts->ofilename, strlen(popts->ofilename) - 4);
									ofilenamemax[std::strlen(popts->ofilename) - 4] = '\0';
									sprintf(ofilenamemax, "%s_dtc_max_frame%05d.jpg", ofilenamemax, nframe);
									std::strcat(max_folder_path_filename, right(ofilenamemax, strlen(ofilenamemax) - InRstr(ofilenamemax,
										"\\"), tmpstring));
									cv::imwrite(max_folder_path_filename, pADUdtcMat, img_save_params);
									cv::minMaxLoc(pADUdtcMat, &minLum, &maxLum, &minPoint, &maxPoint);
									pADUdtcMat.convertTo(pADUdtcMat, -1, 255.0 / maxLum, 0);
									strncpy(ofilenamediff, popts->ofilename, strlen(popts->ofilename) - 4);
									ofilenamediff[std::strlen(popts->ofilename) - 4] = '\0';
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

								cv::threshold(pDifMat, pThrMat, popts->threshold, 0.0, CV_THRESH_TOZERO);
								cv::threshold(pDifMat, pDifMat, popts->threshold, 0.0, CV_THRESH_TOZERO);

								cv::minMaxLoc(pDifMat, &minLum, &maxLum, &minPoint, &maxPoint);

								if (popts->viewThr && pThrMat.data) {
									//pThrMat.convertTo(pThrImg, CV_8U);
									cv::minMaxLoc(pThrMat, &minLum, &maxLum, &minPoint, &maxPoint);
									pThrMat.convertTo(pThrImg, -1, 255.0 / maxLum, 0);
									pThrImg.convertTo(pThrImg, CV_8U);
									cv::imshow("Thresholded differential photometry", pThrImg);
									cv::waitKey(1);
								}

								if (popts->viewRef && pRefMat.data) {
									//pRefMat.convertTo(pRefImg, CV_8U);
									cv::minMaxLoc(pRefMat, &minLum, &maxLum, &minPoint, &maxPoint);
									pRefMat.convertTo(pRefImg, -1, 255.0 / maxLum, 0);
									pRefImg.convertTo(pRefImg, CV_8U);
									cv::imshow("Reference frame", pRefImg);
									cv::waitKey(1);
								}

								pMskMat.convertTo(pMskMat, CV_8U);
								if (nframe > 1) {
									if (nframe <= (long)popts->nframesRef) {
										cv::accumulateWeighted(pGryMat, pRefMat, 1 / nframe, popts->thrWithMask ? pMskMat : cv::noArray());
									}
									else {
										cv::add(pRefMat, pGryMat / popts->nframesRef, pRefMat, popts->thrWithMask ? pMskMat : cv::noArray());
										cv::Mat frontMat = refFrameQueue.front();
										cv::subtract(pRefMat, frontMat / popts->nframesRef, pRefMat,
											popts->thrWithMask ? pMskMat : cv::noArray());
										frontMat.release();
										refFrameQueue.pop();
									}
								}
								if (pDifMat.data && popts->viewRes) {
									cv::minMaxLoc(pDifMat, &minLum, &maxLum, &minPoint, &maxPoint);
									pDifMat.convertTo(pDifImg, -1, 255.0 / maxLum, 0);
									pDifImg.convertTo(pDifImg, CV_8U);
									cv::imshow("Resulting differential photometry", pDifImg);
									cv::waitKey(1);
								}
								if (popts->viewHis || (popts->ovfname && (popts->ovtype == OTYPE_HIS))) {
									pHisImg = dtcGetHistogramImage(pDifMat, (float)popts->histScale, popts->threshold);
									if (popts->viewHis) {
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

								if (popts->viewROI && pGryMat.data) {
									/*										double minLumroi, maxLumroi;
																		cv::Point minPointroi, maxPointroi;
																		cv::minMaxLoc(pGryMat, &minLumroi, &maxLumroi, &minPointroi, &maxPointroi);
																		pGryMat.convertTo(pGryImg, -1, 255.0 / maxLumroi, 0);*/
									pGryMat.convertTo(pGryImg, CV_8U);
									cv::imshow("ROI", pGryImg);
									cv::waitKey(1);
								}
								if (pTrkMat.data && popts->viewTrk) {
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
								if (popts->ovfname && popts->ovtype) {
									switch (popts->ovtype) {
									case OTYPE_DIF: pOVdMat = pDifMat; break;
									case OTYPE_TRK: pOVdMat = pTrkMat; break;
									case OTYPE_ROI: pOVdMat = pGryMat; break;
									case OTYPE_HIS: pOVdMat = pHisImg; break;
									case OTYPE_MSK: pOVdMat = pMskMat; break;
									}
									pWriter = dtcWriteVideo(popts->ovfname, *pWriter, pCapture, pOVdMat);
								}
								if (popts->wait && (cvWaitKey(popts->wait) == 27)) {
									break;
								}
								pGryImg_height = pGryMat.rows;
								pGryImg_width = pGryMat.cols;
							}
						}
					}

					if (clock() > computing_threshold_time) {			// refreshed progress bar and computing time at a limited interval
						if (!popts->parent_instance && !file_exists(CString2string((CString)popts->DeTeCtQueueFilename))) dlg.OnFileExit(); 	// exits DeTeCt if Queuefile does not exists (removed at parent exit) for a child instance. Added because of difficulty to terminate children processes when exiting parent instance

						CDeTeCtMFCDlg::getProgress()->SetPos((short)(MAX_RANGE_PROGRESS * ((float)nframe / (float)frame_number)));
						CDeTeCtMFCDlg::getProgress()->UpdateWindow();
						progress_all_status = MAX_RANGE_PROGRESS * ((acquisitions_processed + acquisition_index_children + ((float)nframe / frame_number)) / acquisitions_to_be_processed);
						CDeTeCtMFCDlg::getProgress_all()->SetPos((short)(progress_all_status));
						CDeTeCtMFCDlg::getProgress_all()->UpdateWindow();

						DisplayProcessingTime(&computing_threshold_time, &end, computing_refresh_duration, begin, begin_total);
					}
					if ((popts->parent_instance) && (clock() > check_threshold_time)) {
						BOOL ExistsProcessedFiles = FALSE;
						if ((popts->maxinstances>1) && (strlen(popts->DeTeCtQueueFilename) > 0)) {	// Gets other processed files by other instances
							double duration_total_others = 0;
							if (GetOtherProcessedFiles(acquisitions_processed, &acquisition_index_children, &acquisitions_to_be_processed, &nb_error_impact, &nb_null_impact, &nb_low_impact, &nb_high_impact, &duration_total_others, &log_messages, popts->DeTeCtQueueFilename)) {
								duration_total += duration_total_others;
								CDeTeCtMFCDlg::getduration()->SetWindowText((CString)"Duration processed (" + TotalType() + "): " + std::to_wstring((int)duration_total).c_str() + (CString)"s");
								ExistsProcessedFiles = TRUE;
							}
						}
						maxinstances_previous = popts->maxinstances;
						if (file_exists(CString2string((CString)opts.DeTeCtQueueFilename))) popts->maxinstances = GetIntParamFromQueue(_T("max_instances"), (CString)popts->DeTeCtQueueFilename);
						//maxinstances_previous = popts->maxinstances;
						//if ((popts->maxinstances != maxinstances_previous) || (nb_instances < popts->maxinstances) || (ExistsProcessedFiles)) {
						// Forks attempt if more instances possible, maximum # of instances not reached at last check or new files processed (hence child detect process exited)
						if ((popts->maxinstances > maxinstances_previous) || (nb_instances < popts->maxinstances) || (ExistsProcessedFiles)) {
							if (popts->debug) LogString(_T("Forks") + (CString)std::to_string(popts->maxinstances).c_str() + _T(" ") + (CString)std::to_string(maxinstances_previous).c_str() + _T(" ") + (CString)std::to_string(nb_instances).c_str(), output_log_file.c_str(), &log_counter, TRUE);
							//if ((popts->maxinstances > 1) && (!file_exists(CString2string((CString)opts.DeTeCtQueueFilename))))	AcquisitionFileListToQueue(&local_acquisition_files_list, _T("file_processing"), acquisition_index - 1, (CString)log.c_str(), &acquisitions_to_be_processed);
							//nb_new_instances = ForksInstances(popts->maxinstances, popts->detect_PID, (CString)popts->DeTeCtQueueFilename, queue_scan_delay, queue_scan_delay_random_max, &nb_instances);
							//DisplayInstanceType(&nb_instances);
							nb_new_instances = 0;
							if ((popts->maxinstances > 1) && (!file_exists(CString2string((CString)opts.DeTeCtQueueFilename)))) AcquisitionFileListToQueue(&local_acquisition_files_list, _T("file_processing"), acquisition_index - 1, (CString)log.c_str(), &acquisitions_to_be_processed);
							nb_new_instances = ForksInstances(popts->maxinstances, popts->detect_PID, (CString)popts->DeTeCtQueueFilename, queue_scan_delay, queue_scan_delay_random_max, &nb_instances);
							if (nb_new_instances > 1)		LogString((CString)std::to_string(nb_new_instances).c_str() + _T(" new instances launched"), output_log_file.c_str(), &log_counter, TRUE);
							else if (nb_new_instances == 1) LogString((CString)std::to_string(nb_new_instances).c_str() + _T(" new instance launched"), output_log_file.c_str(), &log_counter, TRUE);
							else if (ExistsProcessedFiles) {
								nb_instances = 0;
								DisplayInstanceType(&nb_instances);
							}
							//AfxMessageBox(_T("2:") + (CString)std::to_string((int)nframe).c_str() + _T(" ") + (CString)std::to_string((int)nb_new_instances).c_str() + _T(" ") + (CString)std::to_string((int)popts->maxinstances).c_str());
						}
						//						else if (ExistsProcessedFiles) {
						/*							if (popts->parent_instance) {
														nb_instances = 0;
														DisplayInstanceType(&nb_instances); // Display number of instances only if files processed (Forks does the display) and if not child instance
													}*/
						check_threshold_time = clock() + check_threshold_time_inc;
						if (popts->debug) LogString(_T("Ends"), output_log_file.c_str(), &log_counter, TRUE);
					}
					//if (popts->debug) cv::destroyWindow("Frame " + std::to_string(nframe-1));
				}

				// ********************************* END OF CAPTURE READING******************************************

// *******************************************************************************************
// *********************************FINAL PROCESSING******************************************
// *******************************************************************************************

//cv::imshow("End", pADUavgDiffMat);
				//cv::waitKey(0);
				//cv::imshow("End", pADUmaxMat);
				//cv::waitKey(0);
				//cv::imshow("End", pADUavgMat);
				//cv::waitKey(0);

				CDeTeCtMFCDlg::getProgress()->SetPos(MAX_RANGE_PROGRESS);
				CDeTeCtMFCDlg::getProgress()->UpdateWindow();
				//CDeTeCtMFCDlg::getProgress_all()->StepIt();
				CDeTeCtMFCDlg::getProgress_all()->SetPos((short)(MAX_RANGE_PROGRESS * (float)(acquisitions_processed + 1 + acquisition_index_children) / (acquisitions_to_be_processed)));
				CDeTeCtMFCDlg::getProgress_all()->UpdateWindow();
				//					message_cstring = message_cstring + (CString)"\n" + (CString)"Impact detection";
								//CDeTeCtMFCDlg::getfileName()->SetWindowText(message_cstring);
				CDeTeCtMFCDlg::getduration()->SetWindowText((CString)"Duration processed (" + TotalType() + "): " + std::to_wstring((int)duration_total).c_str() + (CString)"s");

				// *********************************IMPACT PROCESSING******************************************

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

					if (popts->ofilename && popts->allframes) {
						pADUdtcMat.release();
						pADUdtcMat = NULL;
						pADUavgMatFrame.release();
						pADUavgMatFrame = NULL;
						pADUavgDiffMat.release();
						pADUavgDiffMat = NULL;
					}
					if (popts->ovfname && popts->ovtype) {
						if (pWriter) {
							pWriter->release();
							pWriter = nullptr;
						}
					}

					/********** Process all matrix **********/
					pADUavgDiffMat.convertTo(pADUavgDiffMat, CV_32F);
					pADUmaxMat.convertTo(pADUmaxMat, CV_32F);
					if (opts.flat_preparation) pFlatADUmaxMat.convertTo(pFlatADUmaxMat, CV_32F);
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
				if (popts->detail)
					cv::imwrite(dtc_full_filename(popts->ofilename, DTC_MEAN2_SUFFIX, detail_folder_path_string.c_str(), tmpstring), pADUavgMat2, img_save_params);
				mean2_stat[1] = mean(pADUavgMat2)[0];*/
					pADUavgMat.convertTo(pADUavgMat, CV_8U);
					if (popts->detail)
						cv::imwrite(dtc_full_filename(popts->ofilename, DTC_MEAN2_SUFFIX, detail_folder_path_string.c_str(), tmpstring), pADUavgMat, img_save_params);
					mean2_stat[1] = mean(pADUavgMat)[0];
					cv::minMaxLoc(pADUavgMat, &mean2_stat[0], &mean2_stat[2], NULL, NULL);
					if (mean2_stat[2] < popts->ROI_min_px_val) is_image_correct = false;
					/* normalizes mean  */
					pADUavgMat.convertTo(pADUavgMat, -1, 255.0 / mean2_stat[2], 0);
					pADUavgMat.convertTo(pADUavgMat, CV_8U);
					mean_stat[1] = mean(pADUavgMat)[0];
					cv::minMaxLoc(pADUavgMat, &mean_stat[0], &mean_stat[2], NULL, NULL);
					cv::imwrite(dtc_full_filename(popts->ofilename, DTC_MEAN_SUFFIX, detection_folder_fullpathname_string.c_str(), tmpstring), pADUavgMat, img_save_params);

					/********** diff image **********/
					pADUavgDiffMat.convertTo(pADUavgDiffMat, -1, 1.0 / (nframe - frame_errors), 0);
					/*pADUavgDiffMat2 = cv::Mat(pADUavgDiffMat.rows, pADUavgDiffMat.cols, CV_32F);
					pADUavgDiffMat2.convertTo(pADUavgDiffMat, CV_8U);*/
					pADUavgDiffMat.convertTo(pADUavgDiffMat, CV_8U);
					if (popts->detail)
						cv::imwrite(dtc_full_filename(popts->ofilename, DTC_DIFF2_SUFFIX, detail_folder_path_string.c_str(), tmpstring), pADUavgDiffMat, img_save_params);
					diff2_stat[1] = mean(pADUavgDiffMat)[0];
					cv::minMaxLoc(pADUavgDiffMat, &diff2_stat[0], &diff2_stat[2], NULL, NULL);
					/*if (popts->detail)
						cv::imwrite(dtc_full_filename(popts->ofilename, DTC_DIFF2_SUFFIX, detail_folder_path_string.c_str(), tmpstring), pADUavgDiffMat2, img_save_params);*/
						/* normalizes diff  */
					pADUavgDiffMat.convertTo(pADUavgDiffMat, -1, 255.0 / diff2_stat[2], 0);
					pADUavgDiffMat.convertTo(pADUavgDiffMat, CV_8U);
					diff_stat[1] = mean(pADUavgDiffMat)[0];
					cv::minMaxLoc(pADUavgDiffMat, &diff_stat[0], &diff_stat[2], NULL, NULL);
					if (popts->detail)
						cv::imwrite(dtc_full_filename(popts->ofilename, DTC_DIFF_SUFFIX, detail_folder_path_string.c_str(), tmpstring), pADUavgDiffMat, img_save_params);

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

					/*Flat image*/
					if (opts.flat_preparation) {
						cv::minMaxLoc(pFlatADUmaxMat, &minLum, &maxLum, &minPoint, &maxPoint);
						if (maxLum > 255)
							pFlatADUmaxMat.convertTo(pFlatADUmaxMat, -1, maxLum / (255.0*255.0), 0);
						else
							pFlatADUmaxMat.convertTo(pFlatADUmaxMat, -1, maxLum / 255.0, 0);
						//if (pFlatADUmaxMat.type() != CV_32F) pFlatADUmaxMat.convertTo(pFlatADUmaxMat, CV_32F);
						pFlatADUmaxMat.convertTo(pFlatADUmaxMat, CV_8U);
						pFlatADUmaxMat.convertTo(pFlatADUmaxImg, CV_8UC3);
						cv::cvtColor(pFlatADUmaxImg, pFlatADUmaxImg, CV_GRAY2BGR);
						cv::imshow("pFlatADUmaxImg", pFlatADUmaxMat);
						cv::waitKey(1000);
					}
				}

				refFrameQueue = std::queue<cv::Mat>();
				totalMean /= (nframe - frame_errors);

				LogString( + L"Differential photometry done, running impact detection...", output_log_file.c_str(), &log_counter, GUI_display);

				if (popts->darkfilename) {
					if (darkfile_ok == 1) {
						pADUdarkMat.release();
						pADUdarkMat = NULL;
					}
				}

				int radius = std::min(std::min(20.0, pGryMat.rows / 10.0), pGryMat.cols / 10.0);
				radius = radius > 5 ? radius : 5; //std::max gives error

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
				std::string dir_csv_name = detection_folder_fullpathname_string;
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
					brightness_factor = (maxBright / (maxMean + popts->threshold)) - 1;
					//double brightness_factor = (maxBright / (maxMean + popts->threshold));
					//DBOUT("Brightness factor 1: " << brightness_factor << "\n");
					//brightness_factor = (maxBright / (maxMean + popts->threshold)) - (1 / popts->threshold);
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
					nb_impact += detect_impact(&dtc, &outdtc, maxMean, &ptlist, &maxDtcImp, radius, popts->incrLumImpact,
						impact_frames_min);

				delete_list(&ptlist);

				//(&outdtc)->nMaxFrame = frameNumbers[(&outdtc)->nMaxFrame];
				//(&outdtc)->nMinFrame = frameNumbers[(&outdtc)->nMinFrame];
				//(&outdtc)->MaxFrame = frameNumbers[(&outdtc)->MaxFrame];

				double impact_frames = (&outdtc)->nMaxFrame - (&outdtc)->nMinFrame;
				//double log10_value = impact_frames != 0 ? std::log10((impact_frames / (popts->incrFrameImpact)) * 10) : 0;
				double log10_value = impact_frames != 0 ? std::log10((impact_frames / impact_frames_min) * 10) : 0;
				double confidence = 0;
				if (log10_value > 0) confidence = (brightness_factor / popts->incrLumImpact) * log10_value;
				//double confidence = (stdev / popts->incrLumImpact) * log10_value;

				if (nframe > 0) {
					/* reprise ADUdtc algorithm******************************************/
					/* max-mean normalized image */
					if ((maxDtcImp->point->x != 0) && (maxDtcImp->point->y != 0)) {
						dtcDrawImpact(pADUdtcImg, cv::Point(maxDtcImp->point->x, maxDtcImp->point->y), CV_RGB(255, 255, 0), 20, 30);		// Pale Yellow, was CV_RGB(255, 0, 0) initially
						distance = sqrt(pow(maxDtcImg->point->x - maxDtcImp->point->x, 2) + pow(maxDtcImg->point->y - maxDtcImp->point->y, 2));
					}
					else {
						/* algorithm did not work */
						distance = 9999;
					}
					dtcDrawImpact(pADUdtcImg, cv::Point(maxDtcImg->point->x, maxDtcImg->point->y), CV_RGB(0, 128, 255), 15, 25);			// Blue, was CV_RGB(0, 255, 0) initially
					begin_imagedisplay_time = clock();
					cv::imshow("Detection image", pADUdtcImg);
					cv::waitKey(25);							// Needed to display the image (according to opencv)
					cv::imwrite(dtc_full_filename(popts->ofilename, DTC_MAX_MEAN_SUFFIX, detection_folder_fullpathname_string.c_str(), tmpstring), pADUdtcImg, img_save_params);

					/* max-mean non normalized image */
					if (popts->detail) {
						if ((maxDtcImp->point->x != 0) && (maxDtcImp->point->y != 0)) 
							dtcDrawImpact(pADUdtcImg2, cv::Point(maxDtcImp->point->x, maxDtcImp->point->y), CV_RGB(255, 255, 0), 20, 30);	// Pale Yellow, was CV_RGB(255, 0, 0) initially
						dtcDrawImpact(pADUdtcImg2, cv::Point(maxDtcImg->point->x, maxDtcImg->point->y), CV_RGB(0, 128, 255), 15, 25);		// Blue, was CV_RGB(0, 255, 0) initially
						cv::imwrite(dtc_full_filename(popts->ofilename, DTC_MAX_MEAN2_SUFFIX, detail_folder_path_string.c_str(), tmpstring), pADUdtcImg2, img_save_params);
					}

					/* flat image */
					if (opts.flat_preparation) cv::imwrite(dtc_full_filename(popts->ofilename, DTC_FLAT_PREP_SUFFIX, detection_folder_fullpathname_string.c_str(), tmpstring), pFlatADUmaxImg, img_save_params);

					// **************************************** final end of ADUdtc algorithm (confidence, probability, ...) ******************************************
					logmessage = "";
					short_logmessage = "";
					rating = Error;
					logmessage2 = "";
					logmessage3 = "";
					strcpy(rating_filename_suffix, "");
					if (!is_image_correct) {
						nb_error_impact++;
						logmessage = logmessage + "ERROR: No planet detected in acquisition images...";
						short_logmessage = logmessage;
						LogString((CString)logmessage.c_str(), output_log_file.c_str(), &log_counter, GUI_display);
						logmessage = logmessage + "\n";
						sprintf(rating_classification, "Error        ");
						sprintf(rating_filename_suffix, "error");
						rating = Error;
						confidence = -1;
						nb_impact = -1;
					}
					else if (nb_impact > 0) {
						outdtc.nMinFrame = frameNumbers[outdtc.nMinFrame];
						outdtc.nMaxFrame = frameNumbers[outdtc.nMaxFrame];
						outdtc.MaxFrame = frameNumbers[outdtc.MaxFrame];
						if ((distance <= popts->impact_distance_max) && (confidence > popts->impact_confidence_min) && ((max_mean_stat[2] - max_mean_stat[1]) > popts->impact_max_avg_min)) {
							if (!popts->ADUdtconly) {
								/*** high probability impact */
								nb_high_impact++;
								logmessage = "ALERT: " + std::to_string(nb_impact) + " HIGH PROBABILITY IMPACT DETECTED (frames " +
									std::to_string(outdtc.nMinFrame) + "-" + std::to_string(outdtc.nMaxFrame) + ", max @" +
									std::to_string(outdtc.MaxFrame) + ").";
								short_logmessage = logmessage;
								logmessage2 = "Confidence: " + std::_Floating_to_string("%2.2f", confidence);
								logmessage3 = "CHECK DETECTION IMAGES!\n";
								LogString((CString)logmessage.c_str(), output_log_file.c_str(), &log_counter, GUI_display);
								LogString((CString)logmessage2.c_str(), output_log_file.c_str(), &log_counter, GUI_display);
								LogString((CString)logmessage3.c_str(), output_log_file.c_str(), &log_counter, GUI_display);
								logmessage = logmessage + "\n" + logmessage2 + "\n" + logmessage3;
								sprintf(rating_classification, "HIGH (@%5d)", outdtc.MaxFrame);
								sprintf(rating_filename_suffix, "high@%d", outdtc.MaxFrame);
								rating = High;
							}
							else {
								/* only initial algorithm launched, displaying only nb impacts detected */
								nb_low_impact++;
								logmessage = "WARNING: " + std::to_string(nb_impact) + " low probability impact.";
								short_logmessage = logmessage;
								LogString((CString)logmessage.c_str(), output_log_file.c_str(), &log_counter, GUI_display);
								logmessage = logmessage + "\n";
								sprintf(rating_classification, "Low          ");
								sprintf(rating_filename_suffix, "low");
							}
						}
						else if (distance <= popts->impact_distance_max) {
							/* No impact, confidence or contrast threshold not respected */
							nb_null_impact++;
							logmessage = "No impact detected (too faint candidate).";
							short_logmessage = logmessage;
							LogString((CString)logmessage.c_str(), output_log_file.c_str(), &log_counter, GUI_display);
							logmessage = logmessage + "\n";
							sprintf(rating_classification, "Null         ");
							sprintf(rating_filename_suffix, "null");
							rating = Null;
						}
						else {
							/* algorithm worked */
							/* distance incorrect */
							confidence /= 4;
							if ((max_mean_stat[2] - max_mean_stat[1]) > popts->impact_max_avg_min) {
								/* potential impact */
								if (!popts->ADUdtconly) {
									nb_low_impact++;
									logmessage = "WARNING: " + std::to_string(nb_impact) + " low probability impact (frames " +
										std::to_string(outdtc.nMinFrame) + "-" + std::to_string(outdtc.nMaxFrame) + ", max @" +
										std::to_string(outdtc.MaxFrame) + "). ";
									short_logmessage = logmessage;
									logmessage2 = "Confidence: " + std::_Floating_to_string("%2.2f", confidence);
									//if ((distance <= popts->impact_distance_max) && (confidence > popts->impact_confidence_min) && ((max_mean_stat[2] - max_mean_stat[1]) > popts->impact_max_avg_min))
									if (!((confidence > popts->impact_confidence_min) && ((max_mean_stat[2] - max_mean_stat[1]) > popts->impact_max_avg_min)))
										logmessage2 = logmessage2 + ", too faint";
									if (!(distance <= popts->impact_distance_max))
										logmessage2 = logmessage2 + ", detection image and algorithm incoherent";
									logmessage2 = logmessage2 + ".";
									logmessage3 = "Please CHECK detection images.\n";
									LogString((CString)logmessage.c_str(), output_log_file.c_str(), &log_counter, GUI_display);
									LogString((CString)logmessage2.c_str(), output_log_file.c_str(), &log_counter, GUI_display);
									LogString((CString)logmessage3.c_str(), output_log_file.c_str(), &log_counter, GUI_display);
									logmessage = logmessage + logmessage2 + "\n" + logmessage3;
									sprintf(rating_classification, "Low  (@%5d)", outdtc.MaxFrame);
									sprintf(rating_filename_suffix, "low@%d", outdtc.MaxFrame);
									rating = Low;
								}
								else {
									nb_low_impact++;
									logmessage = "WARNING: " + std::to_string(nb_impact) + " low probability impact.";
									short_logmessage = logmessage;
									LogString((CString)logmessage.c_str(), output_log_file.c_str(), &log_counter, GUI_display);
									logmessage = logmessage + "\n";
									sprintf(rating_classification, "Low          ");
									sprintf(rating_filename_suffix, "low");
									rating = Low;
								}
							}
							else {
								/* image detection failed */
								nb_null_impact++;
								logmessage = "No impact detected (too faint).";
								short_logmessage = logmessage;
								LogString((CString)logmessage.c_str(), output_log_file.c_str(), &log_counter, GUI_display);
								logmessage = logmessage + "\n";
								sprintf(rating_classification, "Null         ");
								sprintf(rating_filename_suffix, "null");
								rating = Null;
							}
						}
					} // nb_impact = 0
					else if (distance == 9999) {
						/* algorithm did not work */
						if ((max_mean_stat[2] - max_mean_stat[1]) < popts->impact_max_avg_min) {
							/* No impact, contrast threshold not respected */
							nb_null_impact++; 
							logmessage = "No impact detected by the algorithm.";
							short_logmessage = logmessage;
							LogString((CString)logmessage.c_str(), output_log_file.c_str(), &log_counter, GUI_display);
							logmessage = logmessage + "\n";
							sprintf(rating_classification, "Null         ");
							sprintf(rating_filename_suffix, "null");
							rating = Null;
						}
						else {
							/* contrast threshold respected */
							nb_low_impact++;
							logmessage = "WARNING: low probability impact in detection image but no impact detected by the algorithm.";
							short_logmessage = logmessage;
							LogString((CString)logmessage.c_str(), output_log_file.c_str(), &log_counter, GUI_display);
							logmessage = logmessage + "\n";
							sprintf(rating_classification, "Low          ");
							sprintf(rating_filename_suffix, "low");
							rating = Low;
						}
					} else { // distance < 9999
						nb_null_impact++;
						logmessage = "No impact detected by the algorithm.";
						short_logmessage = logmessage;
						LogString((CString)logmessage.c_str(), output_log_file.c_str(), &log_counter, GUI_display);
						logmessage = logmessage + "\n";
						sprintf(rating_classification, "Null         ");
						sprintf(rating_filename_suffix, "null");
						rating = Null;
					}

					if (strlen(rating_filename_suffix)>0) {
						sprintf(tmpstring, "_%s", rating_filename_suffix);
						strcpy(rating_filename_suffix, tmpstring);
						if (popts->detail) {
							rename(dtc_full_filename(popts->ofilename, DTC_MEAN2_SUFFIX, detail_folder_path_string.c_str(), tmpstring), dtc_full_filename_2suffix(popts->ofilename, rating_filename_suffix, DTC_MEAN2_SUFFIX, detail_folder_path_string.c_str(), tmpstring2));
							rename(dtc_full_filename(popts->ofilename, DTC_DIFF2_SUFFIX, detail_folder_path_string.c_str(), tmpstring), dtc_full_filename_2suffix(popts->ofilename, rating_filename_suffix, DTC_DIFF2_SUFFIX, detail_folder_path_string.c_str(), tmpstring2));
							rename(dtc_full_filename(popts->ofilename, DTC_MAX_MEAN2_SUFFIX, detail_folder_path_string.c_str(), tmpstring), dtc_full_filename_2suffix(popts->ofilename, rating_filename_suffix, DTC_MAX_MEAN2_SUFFIX, detail_folder_path_string.c_str(), tmpstring2));
							rename(dtc_full_filename(popts->ofilename, DTC_DIFF_SUFFIX, detail_folder_path_string.c_str(), tmpstring), dtc_full_filename_2suffix(popts->ofilename, rating_filename_suffix, DTC_DIFF_SUFFIX, detail_folder_path_string.c_str(), tmpstring2));
						}
						rename(dtc_full_filename(popts->ofilename, DTC_MEAN_SUFFIX, detection_folder_fullpathname_string.c_str(), tmpstring), dtc_full_filename_2suffix(popts->ofilename, rating_filename_suffix, DTC_MEAN_SUFFIX, detection_folder_fullpathname_string.c_str(), tmpstring2));
						rename(dtc_full_filename(popts->ofilename, DTC_MAX_MEAN_SUFFIX, detection_folder_fullpathname_string.c_str(), tmpstring), dtc_full_filename_2suffix(popts->ofilename, rating_filename_suffix, DTC_MAX_MEAN_SUFFIX, detection_folder_fullpathname_string.c_str(), tmpstring2));
					}
					message_cstring = message_cstring + (CString)"\n" + (CString)logmessage.c_str();
					if (logmessage2.size() > 0) message_cstring = message_cstring + (CString)"\n" + (CString)logmessage2.c_str();
					if (logmessage3.size() > 0) message_cstring = message_cstring + (CString)"\n" + (CString)logmessage3.c_str();
					CDeTeCtMFCDlg::getfileName()->SetWindowText(message_cstring);

					// Calculates computing time ***************
					DisplayProcessingTime(&computing_threshold_time, &end, computing_refresh_duration, begin, begin_total);
					computation_time_total += (double)(end - begin) / (double)CLOCKS_PER_SEC;

					std::string s = "";
//						log_messages.push_back(short_filename + ":");
					std::stringstream str(logmessage);
					std::string line;
					std::getline(str, line);
					int computation_duration = int(end - begin) / CLOCKS_PER_SEC;
					if (computation_duration > 1) s = "s";
					log_messages.push_back(short_filename + ":" + "   " + line);
					while (std::getline(str, line)) log_messages.push_back("    " + line);
					LogString(_T("Computation time: ") +
						(CString)(std::to_string(int(end - begin) / CLOCKS_PER_SEC).c_str()) + _T(" second") + (CString)s.c_str() + _T(", showing detection image")
						+ _T(" (automatically closed in ") + (CString)(std::to_string(wait_imagedisplay_seconds).c_str()) + _T(" second") + (CString)s.c_str() +_T(")..."), output_log_file.c_str(), &log_counter, GUI_display);

					// Refreshes the impacts classification in dialog window
					CDeTeCtMFCDlg::getimpactNull()->SetWindowText(std::to_wstring(nb_null_impact + nb_error_impact).c_str());
					CDeTeCtMFCDlg::getimpactLow()->SetWindowText(std::to_wstring(nb_low_impact).c_str());
					CDeTeCtMFCDlg::getimpactHigh()->SetWindowText(std::to_wstring(nb_high_impact).c_str());
						
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
					pFlatADUmaxMat.release();
					pFlatADUmaxMat = NULL;
					pADUdtcMat.release();
					pADUdtcMat = NULL;
					pADUdtcImg.release();
					pADUdtcImg = NULL;
					pADUdtcImg2.release();
					pADUdtcImg2 = NULL;

					if (popts->ignore)
						dtcCorrectDatation((DtcCapture*)pCapture, &start_time, &end_time, &duration, &fps_real, &timetype, comment);
					std::string location = filename_acquisition.substr(0, filename_acquisition.find_last_of("\\") + 1);

					if (extension.compare(AUTOSTAKKERT_EXT) == 0) strcat(comment, "; from as3");

//						if (!is_image_correct) {

					LogInfo info(popts->filename, start_time, end_time, duration, fps_real, timetype, comment, nb_impact, confidence, distance, mean_stat, mean2_stat, max_mean_stat, max_mean2_stat, diff_stat, diff2_stat, rating_classification);
					dtcWriteLog2(log_consolidated, info, &logline_tmp);
					dtcWriteLog2(log, info, &logline_tmp);

					/*FINAL CLEANING**************************************/
					if (popts->viewDif) cv::destroyWindow("Initial differential photometry");
					if (popts->viewRef) cv::destroyWindow("Reference frame");
					if (popts->viewROI) cv::destroyWindow("ROI");
					if (popts->viewTrk) cv::destroyWindow("Tracking");
					if (popts->viewMsk) cv::destroyWindow("Mask");
					if (popts->viewThr) cv::destroyWindow("Thresholded differential photometry");
					if (popts->viewSmo) cv::destroyWindow("Smoothed differential photometry");
					if (popts->viewRes) cv::destroyWindow("Resulting differential photometry");
					if (popts->viewHis) cv::destroyWindow("Histogram");

					if (popts->thrWithMask || popts->viewMsk || (popts->ovfname && (popts->ovtype == OTYPE_MSK))) {
						pMskMat.release();
						pMskMat = NULL;
						pMskImg.release();
						pMskImg = NULL;
					}
					if (popts->viewThr) {
						pThrMat.release();
						pThrMat = NULL;
						pThrImg.release();
						pThrImg = NULL;
					}
					if (popts->filter.type >= 0 || popts->viewSmo) {
						pSmoMat.release();
						pSmoMat = NULL;
						pSmoImg.release();
						pSmoImg = NULL;
					}
					if (popts->viewTrk || (popts->ovtype == OTYPE_TRK && popts->ovfname)) {
						pTrkMat.release();
						pTrkMat = NULL;
						pTrkImg.release();
						pTrkImg = NULL;
					}
					if (popts->viewDif || popts->viewRes || popts->viewHis || (popts->ovfname && (popts->ovtype == OTYPE_DIF ||
						popts->ovtype == OTYPE_HIS))) {
						pDifMat.release();
						pDifMat = NULL;
						pDifImg.release();
						pDifImg = NULL;
					}
					pRefMat.release();
					pRefMat = NULL;
					pRefImg.release();
					pRefImg = NULL;
					if (popts->viewHis || (popts->ovfname && (popts->ovtype == OTYPE_HIS))) {
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
				} else { // frame = 0
						nb_error_impact++;
						logmessage = "No valid frame.";
						short_logmessage = logmessage;
						LogString((CString)logmessage.c_str(), output_log_file.c_str(), &log_counter, GUI_display);
						logmessage = logmessage + "\n";
						sprintf(rating_classification, "Error        ");
						sprintf(rating_filename_suffix, "error");
						rating = Null;
				}
				dtcReleaseCapture(pCapture);
				pCapture = NULL;
				// acquition has been processed, increasing counter					
				acquisitions_processed++;
				totalProgress_wstring = L"Total\n(" + std::to_wstring(acquisitions_processed + acquisition_index_children) + L"/" + std::to_wstring(acquisitions_to_be_processed) + L")";
				CDeTeCtMFCDlg::gettotalProgress()->SetWindowText(totalProgress_wstring.c_str());
			}

			catch (std::exception& e) {
					std::string exception_message(e.what());
//						output_log << getDateTime().str().c_str() << "ERROR: " << std::wstring(exception_message.begin(), exception_message.end()) << "\n";
					LogString(L"ERROR: ", output_log_file.c_str(), &log_counter, GUI_display);
					LogString((CString)std::string(e.what()).c_str(), output_log_file.c_str(), &log_counter, GUI_display);
					logmessage = "ERROR: " + std::string(e.what());
					short_logmessage = logmessage;
					log_messages.push_back(short_filename + ":" + "    " + logmessage);
					rating = Error;
					nb_error_impact++;
					// log_messages.push_back("    " + logmessage);
				}
				//message = "--------- " + short_filename + " analysis done ---------";
			LogString(_T(""), output_log_file.c_str(), &log_counter, GUI_display);
//				output_log << getDateTime().str().c_str() << message.c_str() << "\n";
//				if (popts->filename) {
			if (strlen(popts->DeTeCtQueueFilename) > 0) {
				CString objectname(popts->filename);
				RemoveFileFromQueue(objectname, (CString)popts->DeTeCtQueueFilename, TRUE);

				CString objectname_cstring;
				if (filename_autostakkert.size() > 0) {
					objectname_cstring = filename_autostakkert.c_str();
				}
				else {
					objectname_cstring = objectname;
				}
				CString tag;
				if (popts->parent_instance)	tag = "file_ok        ";
				else						tag = "file_processed ";
				if (!popts->parent_instance && !file_exists(CString2string((CString)popts->DeTeCtQueueFilename))) dlg.OnFileExit(); 	// exits DeTeCt if Queuefile does not exists (removed at parent exit) for a child instance. Added because of difficulty to terminate children processes when exiting parent instance
				else {
					SetProcessingFileProcessedFromQueue(objectname_cstring, _T("|") + objectname + _T("|") + (CString)short_logmessage.c_str() + _T("|") + (CString)std::to_string(rating).c_str() + _T("|") + (CString)std::to_string(duration).c_str() + _T("|") + (CString)std::to_string(nframe).c_str() + _T("|") + (CString)std::to_string(fps_int).c_str(), tag, (CString)popts->DeTeCtQueueFilename);
					LogString(_T("Processed file : ") + objectname_cstring, output_log_file.c_str(), &log_counter, GUI_display);
				}
				// Display shortened message in case of multi instances mode 
				if (!GUI_display) {
					CDeTeCtMFCDlg::getLog()->AddString((CString)getDateTime().str().c_str() + "----- " + objectname_cstring + " -----");
					CDeTeCtMFCDlg::getLog()->AddString((CString)getDateTime().str().c_str() + std::to_string(nframe).c_str() + (CString)" frames @ " + std::to_string(fps_int).c_str() + (CString)" fps (" + std::to_string((int)duration).c_str() + (CString)"s duration)");
					//CDeTeCtMFCDlg::getLog()->AddString((CString)getDateTime().str().c_str() + "(" + std::to_wstring((int)duration).c_str() + "s duration)");
					CDeTeCtMFCDlg::getLog()->AddString((CString)getDateTime().str().c_str() + short_logmessage.c_str());
					CDeTeCtMFCDlg::getLog()->AddString((CString)getDateTime().str().c_str());
					CDeTeCtMFCDlg::getLog()->SetTopIndex(CDeTeCtMFCDlg::getLog()->GetCount() - 1);
					CDeTeCtMFCDlg::getLog()->RedrawWindow();
				}
			}
			DisplayProcessingTime(&computing_threshold_time, &end, computing_refresh_duration, begin, begin_total);
			if ((begin_imagedisplay_time > 0) && (acquisition_index < local_acquisition_files_list.acquisition_file_list.size())) {
//AfxMessageBox((CString)std::to_string((int)((begin_imagedisplay_time)+wait_imagedisplay_seconds * 1000 - clock())).c_str());
				cv::waitKey((int)(begin_imagedisplay_time)+wait_imagedisplay_seconds * 1000 - clock());
				cv::destroyWindow("Detection image");
				begin_imagedisplay_time = 0;
			}
		}

		if (popts->dirname) {
			CString objectname(popts->dirname);
			RemoveFileFromQueue(objectname, (CString) popts->DeTeCtQueueFilename,TRUE);
		}
		local_acquisition_files_list.file_list = std::vector<std::string>();
		local_acquisition_files_list.acquisition_file_list = std::vector<std::string>();
		local_acquisition_files_list.nb_prealigned_frames = {};

// ***********************************************************************
// ******************** Looks for new job in queue ***********************
// ***********************************************************************

//		if (!popts->interactive) {
		if (local_acquisition_files_list.file_list.size() == 0) {
			std::string waiting_message = "";
			if (NbItemFromQueue(_T("file"), (CString)popts->DeTeCtQueueFilename, TRUE) == 0) {
				if ((popts->parent_instance) && (popts->autostakkert) && (IsParentAutostakkertRunning(popts->autostakkert_PID))) {
					waiting_message = "checking for pending or new files to be processed from AutoStakkert...\n(close Autostakkert if done with your processing!)\n";
					if (logmessage3.size() > 0) message_cstring = message_cstring + (CString)"\n" + (CString)logmessage3.c_str(); // ???
				}
				if ((popts->parent_instance) && (ChildrenProcessesNumber() > 0))
					waiting_message =	waiting_message + "checking for other DeTeCt process(es) running to finish ...\n";
			} else waiting_message =	waiting_message + "checking new files to be processed ...\n";
			if (waiting_message.size() > 1) {
				LogString((CString)"PLEASE WAIT, " + (CString)waiting_message.c_str(), output_log_file.c_str(), &log_counter, TRUE);
				CDeTeCtMFCDlg::getfileName()->SetWindowText((CString)waiting_message.c_str());
			}
		}
		BOOL QueueListEmpty			= FALSE;
		int total_sleep_duration	= queue_scan_delay + rand() % queue_scan_delay_random_max; 	// wait (in ms)
		int sleep_duration			= 0;

		if (popts->parent_instance) { 
			nb_instances = 0;
			DisplayInstanceType(&nb_instances); // Display number of instances only if files processed (Forks does the display) and if not child instance
		}
		// looks for new job for a child only if not too many children
		int nbChildren = ParentChildrenProcessesNumber(MAX(popts->detect_PID, popts->autostakkert_PID));
		maxinstances_previous = popts->maxinstances;
		if (file_exists(CString2string((CString)opts.DeTeCtQueueFilename))) popts->maxinstances = GetIntParamFromQueue(_T("max_instances"), (CString)popts->DeTeCtQueueFilename);
if (popts->debug) LogString(_T("Check queue: parent=") + (CString)std::to_string(popts->parent_instance).c_str() + _T("\nmax=") + (CString)std::to_string(popts->maxinstances).c_str() + _T("\nnb children=") + (CString)std::to_string(nbChildren).c_str() + _T("\nDeTeCt PID=") + (CString)std::to_string(popts->detect_PID).c_str() + _T("\nAS PID=") + (CString)std::to_string(popts->autostakkert_PID).c_str(), output_log_file.c_str(), &log_counter, GUI_display);
		if ((!((!popts->parent_instance) && ((popts->maxinstances - nbChildren - 1) < 0))) && (local_acquisition_files_list.file_list.size() == 0)) do {
			// ******************** Looks for other processed files*******************
			if ((popts->parent_instance) && (strlen(popts->DeTeCtQueueFilename) > 0)) { // get other processed files by other instances
				double duration_total_others = 0;
				if ((popts->maxinstances > 1) && (GetOtherProcessedFiles(acquisitions_processed, &acquisition_index_children, &acquisitions_to_be_processed, &nb_error_impact, &nb_null_impact, &nb_low_impact, &nb_high_impact, &duration_total_others, &log_messages, popts->DeTeCtQueueFilename))) {
					duration_total += duration_total_others;
					CDeTeCtMFCDlg::getduration()->SetWindowText((CString)"Duration processed (total): " + std::to_wstring((int)duration_total).c_str() + (CString)"s");
					if ((popts->maxinstances > 1) && (!file_exists(CString2string((CString)opts.DeTeCtQueueFilename))))  AcquisitionFileListToQueue(&local_acquisition_files_list, _T("file_ok"), acquisition_index - 1, (CString)log.c_str(), &acquisitions_to_be_processed);
					nb_new_instances = ForksInstances(popts->maxinstances, popts->detect_PID, (CString)popts->DeTeCtQueueFilename, queue_scan_delay, queue_scan_delay_random_max, &nb_instances);
					if (nb_new_instances > 1)		LogString((CString)std::to_string(nb_new_instances).c_str() + _T(" new instances launched"), output_log_file.c_str(), &log_counter, TRUE);
					else if (nb_new_instances == 1) LogString((CString)std::to_string(nb_new_instances).c_str() + _T(" new instance launched"), output_log_file.c_str(), &log_counter, TRUE);
					else if (opts.parent_instance) { //&&((popts->maxinstances > maxinstances_previous) || (nb_instances < popts->maxinstances))) {
						nb_instances = 0;
						DisplayInstanceType(&nb_instances);
					}
				}
			}
			if (total_sleep_duration > computing_refresh_duration) {
				sleep_duration = 0;
				do {
					Sleep(computing_refresh_duration);				// sleep but only for computing_refresh_duration
					sleep_duration += computing_refresh_duration;
					if (clock() > computing_threshold_time)			// refreshed computing time at a limited interval
						DisplayProcessingTime(&computing_threshold_time, &end, computing_refresh_duration, begin, begin_total);
				} while (sleep_duration < total_sleep_duration);
			}
			else {													// duration < refresh
				Sleep(total_sleep_duration);
				sleep_duration += total_sleep_duration;				// sleep all sleep duration
				if (clock() > computing_threshold_time)				// refreshed computing time at a limited interval
					DisplayProcessingTime(&computing_threshold_time, &end, computing_refresh_duration, begin, begin_total);
			}			
			if (begin_imagedisplay_time > 0) {
				int wait_time_rest = (int)(begin_imagedisplay_time)+wait_imagedisplay_seconds * 1000 - clock();
				if (wait_time_rest < 0) { // stop displaying image as wait time is over
					begin_imagedisplay_time = 0;
					cv::destroyWindow("Detection image"); 
				}
			}
			CString objectname;
			QueueListEmpty = FALSE;
			if (!popts->parent_instance && !file_exists(CString2string((CString)popts->DeTeCtQueueFilename))) dlg.OnFileExit(); 	// exits DeTeCt if Queuefile does not exists (removed at parent exit) for a child instance. Added because of difficulty to terminate children processes when exiting parent instance
			if (GetFileFromQueue(&objectname, (CString) popts->DeTeCtQueueFilename)) {
				LogString(_T("Fetched file : ") + objectname, output_log_file.c_str(), &log_counter, TRUE);
				std::ifstream file_stream(objectname);
				if (file_stream) {
					CT2A objectnamechar(objectname);
					popts->filename = objectnamechar;
					file_stream.close();
//						local_acquisition_files_list.acquisition_file_list.push_back(std::string(popts->filename));
//						dlg.OnFileOpenfile();
					std::wstringstream ss;
					std::string file;
					std::string filename_acquisition;
					int nframe = -1;

					file = std::string(popts->filename);
					std::string extension = file.substr(file.find_last_of(".") + 1, file.size() - file.find_last_of(".") - 1);

					if ((Is_Capture_OK_from_File(file, &filename_acquisition, &nframe, &ss)) &&
						// ********* Error if acquisition has not enough frames
						(Is_Capture_Long_Enough(file, nframe, &ss)) &&
							// ********* Ignores dark, pipp, winjupos derotated files
							(!Is_Capture_Special_Type(file, &ss)) &&
								(Is_CaptureFile_To_Be_Processed(filename_acquisition, &ss))) {
									// ***** if option noreprocessing on, check in detect log file if file already processed or processed with in datation only mode

									// ********* Finally adds file to the list !
									// Set-up global variable
									scan_folder_path = file.substr(0, file.find_last_of("\\"));
									local_acquisition_files_list.file_list.push_back(std::string(popts->filename));
									file = file.substr(file.find_last_of("\\") + 1, file.length());
									if (extension.compare(AUTOSTAKKERT_EXT) != 0) {
										ss << "Adding " << file.c_str() << " (in " << scan_folder_path.c_str() << ") for analysis\n";
										local_acquisition_files_list.acquisition_file_list.push_back(std::string(popts->filename));
									}
									else {
										ss << "Adding " << file.c_str() << " (acquisition file " << filename_acquisition.c_str() << " in " << scan_folder_path.c_str() << ") for analysis\n";
										local_acquisition_files_list.acquisition_file_list.push_back(std::string(filename_acquisition));
									}
									local_acquisition_files_list.nb_prealigned_frames.push_back(nframe);
									CDeTeCtMFCDlg::getfileName()->SetWindowText(L"");
									acquisition_index = 0;
					}
					else
						RemoveItemsFromQueue(objectname, (CString)"file_processing", (CString)popts->DeTeCtQueueFilename, TRUE);
					// Prints message
					LogString((CString)ss.str().c_str(), output_log_file.c_str(), &log_counter, TRUE);
				}
				else {
					DIR *folder_object;
					CT2A objectnamechar(objectname);
					if (folder_object = opendir(objectnamechar)) {
						popts->dirname = objectnamechar;	//strcpy(popts->dirname,objectnamechar);
						std::string path = std::string(popts->dirname);
						read_files(path, &local_acquisition_files_list);
						closedir(folder_object);

						if (local_acquisition_files_list.file_list.size() > 0) {
							int index = 0;
							std::string filename_folder;
							while (index < local_acquisition_files_list.file_list.size()) {
								filename_folder = local_acquisition_files_list.file_list.at(index);
								std::wstringstream ss3;
								std::string filename_acquisition;
								int nframe = -1;

								if (
									(Is_Capture_OK_from_File(filename_folder, &filename_acquisition, &nframe, &ss3))
									// ********* Error if acquisition has not enough frames
									&& (Is_Capture_Long_Enough(filename_folder, nframe, &ss3))
									// ********* Ignores if less than minimum frames
									&& (!Is_Capture_Special_Type(filename_folder, &ss3))
									// ********* Ignores dark, pipp, winjupos derotated files
									&& (Is_CaptureFile_To_Be_Processed(filename_acquisition, &ss3))
									) {
									// ***** if option noreprocessing on, check in detect log file if file already processed or processed with in datation only mode

												// ********* Finally adds file to the list !
									std::string extension		= filename_folder.substr(filename_folder.find_last_of(".") + 1, filename_folder.size() - filename_folder.find_last_of(".") - 1);
									std::string filename_path	= filename_folder.substr(0, filename_folder.find_last_of("\\"));
									std::string file			= filename_folder.substr(filename_folder.find_last_of("\\") + 1, filename_folder.length());
									if (extension.compare(AUTOSTAKKERT_EXT) != 0) ss3 << "Adding " << file.c_str() << " (in " << filename_path.c_str() << ") for analysis\n";
									else ss3 << "Adding " << file.c_str() << " (acquisition file " << filename_acquisition.c_str() << " in " << filename_path.c_str() << ") for analysis\n";
									index++;
								}
								else {
									local_acquisition_files_list.file_list.erase(local_acquisition_files_list.file_list.begin() + index);
									local_acquisition_files_list.acquisition_file_list.erase(local_acquisition_files_list.acquisition_file_list.begin() + index);
									local_acquisition_files_list.nb_prealigned_frames.erase(local_acquisition_files_list.nb_prealigned_frames.begin() + index); // WARNING in debug, error in .begin()
								}
								LogString((CString)ss3.str().c_str(), output_log_file.c_str(), &log_counter, TRUE);
							}
						}

					}
				}
				file_stream.close();
			}
			else QueueListEmpty = TRUE;
		} while		((local_acquisition_files_list.file_list.size() == 0) &&													// Waiting mode  if no file in the list
						((!QueueListEmpty) ||																								// but queue still not empty
							((popts->parent_instance) && (((popts->autostakkert) && (IsParentAutostakkertRunning(popts->autostakkert_PID)))			// or parent instance and AutoStakkert! parent still running
														|| (ChildrenProcessesNumber() > 0))															// or parent instance with other instances of DeTeCt running
							)
						)
					);
//		}
		if (begin_imagedisplay_time > 0) {
			int wait_time_rest = (int)(begin_imagedisplay_time)+wait_imagedisplay_seconds * 1000 - clock();
			if (wait_time_rest > 0) {
				cv::waitKey(wait_time_rest);  // wait time left before wait time defined, before stopping displaying image
				begin_imagedisplay_time = 0;
				cv::destroyWindow("Detection image");
			}
		}
	} while (local_acquisition_files_list.file_list.size() > 0);


// **************************************************************************
// ******************** End of acquisition processing ***********************
// **************************************************************************

	// Last update of file counts with actual figures
	int acquisitions_finally_processed = MAX(NbItemFromQueue(_T("file_ok        "), (CString)popts->DeTeCtQueueFilename, TRUE), acquisitions_to_be_processed) ;
	totalProgress_wstring = L"Total\n(" + std::to_wstring(acquisitions_processed + acquisition_index_children) + L"/" + std::to_wstring(acquisitions_finally_processed) + L")";
	CDeTeCtMFCDlg::gettotalProgress()->SetWindowText(totalProgress_wstring.c_str());
	if (popts->parent_instance) {
		nb_instances = 0;
		DisplayInstanceType(&nb_instances); // Display number of instances only if files processed (Forks does the display) and if not child instance
	}
	DisplayProcessingTime(&computing_threshold_time, &end, computing_refresh_duration, begin, begin_total);
	begin_imagedisplay_time = 0;
	cv::destroyWindow("Detection image");

	// ******************* end of processing configuration
	if ((popts->parent_instance) && (!IsParentAutostakkertRunning(popts->autostakkert_PID))) {
		popts->autostakkert = FALSE;
		popts->autostakkert_PID = 0;
		dlg.execAS.SetCheck(0);
		LogString(L"Automatic execution from parent AutoStakkert terminated", output_log_file.c_str(), &log_counter, TRUE);
	}

	// * delete process queue if parent instance *
	if ((popts->parent_instance) && (strlen(popts->DeTeCtQueueFilename) > 0)) {
		UnlockQueue((CString)popts->DeTeCtQueueFilename);
		remove(popts->DeTeCtQueueFilename);
		strcpy(popts->DeTeCtQueueFilename, "");
	}
	LogString(_T(""), output_log_file.c_str(), &log_counter, TRUE);

	if (acquisitions_processed > 0) {
		// * defines new name for logfile in impact_detection directory 
		double second_min, second_max;
		int minute_min, minute_max;
		int hour_min, hour_max;
		int day_min, day_max;
		int month_min, month_max;
		int year_min, year_max;
		char suffix_char[MAX_STRING];
		char planet_char[MAX_STRING] = "";

		if (planet_jupiter > 0) strcat(planet_char, "_jupiter");;
		if (planet_saturn > 0) strcat(planet_char, "_saturn");;

		jd_to_date(start_time_min, &second_min, &minute_min, &hour_min, &day_min, &month_min, &year_min);
		jd_to_date(start_time_max, &second_max, &minute_max, &hour_max, &day_max, &month_max, &year_max);
		sprintf(suffix_char, "%s_%04d%02d%02d_%02d%02d", planet_char, year_min, month_min, day_min, hour_min, minute_min);
		if ((day_min != day_max) || (month_min != month_max) || (year_min != year_max)) sprintf(suffix_char, "%s-%04d%02d%02d_%02d%02d", suffix_char, year_max, month_max, day_max, hour_max, minute_max);
		else if ((hour_min != hour_max) || (minute_min != minute_max) || (second_min != second_max)) sprintf(suffix_char, "%s-%02d%02d", suffix_char, hour_max, minute_max);
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
			message = message + std::to_string(seconds) + "s)";
		}

		// * Final message *

		char tmpchar[MAX_STRING];
		LogString((CString)message.c_str(), output_log_file.c_str(), &log_counter, TRUE);
		//LogString(L"In " + (CString)(log_consolidated.c_str()) + L", please find:", output_log_file.c_str());
		//LogString(L" * log file " + (CString)(left(DeTeCtFileName(tmpchar), InRstr(DeTeCtFileName(tmpchar), "."), tmpchar)) + DTC_LOG_SUFFIX, output_log_file.c_str());
		//LogString(L" * folder " + (CString)(right(detection_folder_fullpathname_string.c_str(), strlen(detection_folder_fullpathname_string.c_str()) - InRstr(detection_folder_fullpathname_string.c_str(), "\\") - 1, tmpchar)) + L" for checking images", output_log_file.c_str());

		if (popts->dateonly) LogString(L"WARNING, datation info only, no detection analysis was performed\n", output_log_file.c_str(), &log_counter, TRUE);

		CT2A LogOrgFilename(DeTeCt_additional_filename_from_folder(CString(log.c_str()), DTC_LOG_SUFFIX));
		CT2A LogNewFilename(DeTeCt_additional_filename_from_folder(CString(log.c_str()), (CString)suffix_char));

		CT2A tmp_log_detection_dirname(DeTeCt_additional_filename_from_folder(_T(""), (CString)suffix_char));
		strcpy(log_detection_dirname, tmp_log_detection_dirname);

		CT2A OutOrgFilename2(CString(log.c_str()) + L"\\" + OUTPUT_FILENAME + DTC_LOG_SUFFIX);
		CT2A OutNewFilename2(CString(log.c_str()) + L"\\" + OUTPUT_FILENAME + (CString)suffix_char);

		// renames logs and shows links, creates zip if parent instance
		char item_to_be_zipped_shortname[MAX_STRING] = "";
		if (popts->parent_instance) {
			rename(LogOrgFilename, LogNewFilename);

//			CDeTeCtMFCDlg::getdetectLoglink()->SetURL((CString)log_consolidated.c_str() + _T("\\") + DeTeCt_additional_filename_from_folder(_T(""), DTC_LOG_SUFFIX));
			CDeTeCtMFCDlg::getdetectLoglink()->SetURL((CString)LogNewFilename);
			dlg.EnableLogLink(TRUE);
			dlg.GetDlgItem(IDC_MFCLINK_DETECTLOG)->SetWindowTextW(_T("Detection log"));

			CDeTeCtMFCDlg::getdetectImageslink()->SetURL((CString)detection_folder_fullpathname.c_str());
			dlg.EnableImagesLink(TRUE);
			dlg.GetDlgItem(IDC_MFCLINK_DETECTIMAGES)->SetWindowTextW(_T("Detection images to check"));

			// *************** creates zip file ***********/
			strcpy(zipfile, "\0");
			strcpy(zip_detection_location, zipfile);
			strcpy(popts->zipname, zipfile);
			if (popts->zip) {
				CDeTeCtMFCDlg::getfileName()->SetWindowText(L"Creating zip file, please wait ...");
				LogString(L"Creating zip file, please wait ...", output_log_file.c_str(), &log_counter, TRUE);


				char item_to_be_zipped[MAX_STRING] = "";
//				char item_to_be_zipped_location[MAX_STRING] = "";

				strcat(item_to_be_zipped, detection_folder_fullpathname_string.c_str());		// eg "G:\\work\\Impact\\tests\\autostakkert3.2_detect\\Impact_detection_run@2020-05-08_00-39-34"
				strcat(item_to_be_zipped, "\0");

				strcat(item_to_be_zipped_shortname, detection_folder_name_string.c_str());
				strcat(item_to_be_zipped_shortname, ".zip");
				strcat(item_to_be_zipped_shortname, "\0");
				strcpy(popts->zipname, item_to_be_zipped_shortname);

//				strcat(item_to_be_zipped_location, log_consolidated.c_str());					// eg 0x000000b79c3fdf90 "G:\\work\\Impact\\DeTeCt-PSWS\\DeTeCt-MFC\\x64\\Release"
//				strcat(item_to_be_zipped_location, "\0");

				strcat(zipfile, detection_folder_fullpathname_string.c_str());					// eg "G:\\work\\Impact\\tests\\autostakkert3.2_detect\\Impact_detection_run@2020-05-08_00-39-34"
				strcat(zipfile, ".zip");
				strcat(zipfile, "\0");															// eg "G:\\work\\Impact\\tests\\autostakkert3.2_detect\\Impact_detection_run@2020-05-08_00-39-34.zip"

//				strcpy(zip_detection_dirname, zipfile);											// eg "G:\\work\\Impact\\tests\\autostakkert3.2_detect\\Impact_detection_run@2020-05-08_00-39-34"
				strcpy(zip_detection_location, log_consolidated.c_str());						// eg "G:\\work\\Impact\\tests\\autostakkert3.2_detect"

				// Deactivated and replaced by new version below
				zip(zipfile, item_to_be_zipped);

				struct stat st;
				if (stat(zipfile, &st) == 0) if (st.st_size < 23) remove(zipfile);

				// see project https://www.codeproject.com/articles/4135/xzip-and-xunzip-add-zip-and-or-unzip-to-your-app-w
				//USES_CONVERSION;
				//HZIP newZip0 = CreateZip(L"E:\\Sample.zip", NULL, ZIP_FILENAME);
				//BOOL retval0 = AddFolderContent(newZip0, L"E:", L"TEMP");
				//CloseZip(newZip0);

				std::ifstream filetest(zipfile);
				if (filetest) {
					CDeTeCtMFCDlg::getzipFilelink()->SetURL((CString)zip_detection_location);
					dlg.EnableZipLink(TRUE);
					dlg.GetDlgItem(IDC_MFCLINK_ZIPFILE)->SetWindowTextW(_T("Folder with zip file to send"));

					LogString(L"Zip file " + (CString)(right(zipfile, strlen(zipfile) - InRstr(zipfile, "\\") - 1, tmpchar)) + L" created", output_log_file.c_str(), &log_counter, TRUE);
					message = "(Click) check detection images and send email with zip file!\n";
					CDeTeCtMFCDlg::getfileName()->SetWindowText((CString) message.c_str());
				}
				else {
					dlg.EnableZipLink(FALSE);
					LogString(L"ERROR: zip file " + (CString)(right(zipfile, strlen(zipfile) - InRstr(zipfile, "\\") - 1, tmpchar)) + L" not created!", output_log_file.c_str(), &log_counter, TRUE);
					message = "(Click) Check detection images and send email with log!\n";
					CDeTeCtMFCDlg::getfileName()->SetWindowText((CString)message.c_str());
				}
				filetest.close();
			}
			CWnd *resultsbtn = dlg.GetDlgItem(IDC_BUTTON_CHECKRESULTS);
			if (resultsbtn) {
				resultsbtn->EnableWindow(TRUE);
			}
		}
		if (popts->dateonly) LogString(L"WARNING, datation info only, no detection analysis was performed\n", output_log_file.c_str(), &log_counter, TRUE);
		LogString(_T(""), output_log_file.c_str(), &log_counter, TRUE);

		std::string message_new = "";
		message = message + "\n";
		log_messages.push_back("");
		std::string plural;
		if (nb_high_impact > 0) {
			if (nb_high_impact > 1) plural = "s";
			else plural = "";
			message_new = std::to_string(nb_high_impact) + " acquisition" + plural + " with high probability impact" + plural + "\n";
			log_messages.push_back(message_new);
			message = message + message_new;
		}
		if (nb_low_impact > 0) {
			if (nb_low_impact > 1) plural = "s";
			else plural = "";
			message_new = std::to_string(nb_low_impact) + " acquisition" + plural + " with low probability impact" + plural + "\n";
			log_messages.push_back(message_new);
			message = message + message_new;
		}
		if (nb_null_impact > 0) {
			if (nb_null_impact > 1) plural = "s";
			else plural = "";
			message_new = std::to_string(nb_null_impact) + " acquisition" + plural + " without any impact" + plural + "\n";
			log_messages.push_back(message_new);
			message = message + message_new;
		}
		if (nb_error_impact > 0) {
			if (nb_error_impact > 1) plural = "s";
			else plural = "";
			message_new = std::to_string(nb_error_impact) + " acquisition" + plural + " with error" + plural + "\n";
			log_messages.push_back(message_new);
			message = message + message_new;
		}
		message = message + "\n" + "CHECK and SEND the RESULTS to:   delcroix.marc@free.fr   NO DETECTION also MATTERS!\n";
		CDeTeCtMFCDlg::getfileName()->SetWindowText((CString)message.c_str());

		if (popts->parent_instance) {
			log_messages.push_back("");
			log_messages.push_back("Click \"Check detection images...\" button to open in \"" + log_consolidated + "\" :");
			log_messages.push_back(" - an explorer in \"" + detection_folder_name_string + "\" to check the detection images");
			std::ifstream filetest(zipfile);
			if (filetest) {
				std::string stritem_to_be_zipped_shortname(item_to_be_zipped_shortname);
				log_messages.push_back(" - an explorer where \"" + stritem_to_be_zipped_shortname + "\" to be sent by email is  (along with " + left(DeTeCtFileName(tmpchar), InRstr(DeTeCtFileName(tmpchar), "."), tmpchar) + " log)");
				if (popts->email)
					log_messages.push_back(" - an email to send the results by attaching \"" + stritem_to_be_zipped_shortname + "\" file");
				else log_messages.push_back("Please send an email with the results by attaching \"" + stritem_to_be_zipped_shortname + "\" file");
			}
			else {
				std::string strlog_detection_dirname(log_detection_dirname);
				if (popts->email) log_messages.push_back(" - an email to send the results by attaching the detection images and \"" + strlog_detection_dirname + "\" from the \"" + detection_folder_name_string + "\" folder");
				else log_messages.push_back("Please send an email with the results by attaching the detection images and \"" + strlog_detection_dirname + "\" from the \"" + detection_folder_name_string + "\" folder");
			}
			filetest.close();
			log_messages.push_back("");
			log_messages.push_back("CHECK the DETECTION IMAGES for impacts and SEND the RESULTS, NO DETECTION also MATTERS!");
			log_messages.push_back("delcroix.marc@free.fr");
		}
//		if (popts->autostakkert) {
		//if (popts->parent_instance) {
		if (!popts->exit) {
			log_messages.push_back("");
			log_messages.push_back("You can SAFELY CLOSE this window.");
			log_messages.push_back("================================================================================");
		}

		for (std::string msg : log_messages) {
			std::wstring wmsg = std::wstring(msg.begin(), msg.end());
			CString Cmsg = CString(wmsg.c_str(), (int)wmsg.length());
			LogString(Cmsg, output_log_file.c_str(), &log_counter, TRUE);
		}

		if (popts->parent_instance) {
			//saves detect.ini parameters in output.log
			std::wofstream output_log_out(output_log_file.c_str(), std::ios_base::app);
			std::wifstream parameter_ini_in(DeTeCt_additional_filename_exe_fullpath(DTC_INI_SUFFIX));
			output_log_out << "======================================================================================================\n  Parameters:\n";
			output_log_out << parameter_ini_in.rdbuf();
			output_log_out.flush();
			output_log_out.close();
			parameter_ini_in.close();
			// copies output log to central directory
			std::wstring output_log_file2(log_directory.begin(), log_directory.end());
			output_log_file2 = output_log_file2.append(L"\\").append(OUTPUT_FILENAME).append(DTC_LOG_SUFFIX);
			std::wofstream output_log2(output_log_file2.c_str(), std::ios_base::app);
			if (popts->dateonly) output_log2 << "WARNING, datation info only, no detection analysis was performed\n";
			std::wifstream output_log_in(output_log_file.c_str());
			output_log2 << output_log_in.rdbuf();
			output_log_in.close();
			//		output_log2 << getDateTime().str().c_str() << "\n";
			//		output_log2 << getDateTime().str().c_str() << message.c_str() << "\n";
			output_log2 << "======================================================================================================\n\n";
			output_log2.flush();
			output_log2.close();
			rename(OutOrgFilename2, OutNewFilename2);
			output_log_in.close();
		}
//		SendEmailDlg* email = new SendEmailDlg(NULL, log_messages);
//DBOUT("Interactive=" << popts->interactive << "\n");

//		if ((popts->interactive) || ((popts->autostakkert) && (popts->parent_instance))) { 
//			SendEmailDlg* email = new SendEmailDlg(NULL, log_messages); 
//			email->DoModal();
//		} else {
//			dlg.OnFileExit();
			//m_pMainWnd->DestroyWindows();
//		}
} // end if acquisition > 0
	
	else {
		// TODO: Message nothing has been done
		LogString(L"WARNING, no file has been processed\n", output_log_file.c_str(), &log_counter, TRUE);
		LogString(_T(""), output_log_file.c_str(), &log_counter, TRUE);
		
		// TODO: Delete log if empty
	}
	if (popts->exit) {
		if (popts->shutdown) ShellExecute(NULL, L"open", _T("cmd"), _T("/c shutdown /s /t 30 /d u:0:0"), NULL, SW_NORMAL);
		dlg.OnFileExit();
	}

	/*if (((popts->autostakkert) && (!popts->parent_instance)) || ((!popts->interactive) && (!popts->parent_instance))) // Automatically exit for child instances in autostakkert mode or automatic mode
		dlg.OnFileExit();

	if (!popts->interactive)	dlg.OnFileExit();  // Automatically exit of parent instance in automatic mode
	*/

	// Reactivate file/folder management
	CWnd *openfolderbtn = dlg.GetDlgItem(IDOK3);
	if (openfolderbtn) {
		openfolderbtn->EnableWindow(TRUE);
	}
	CWnd *openfilebtn = dlg.GetDlgItem(IDOK2);
	if (openfilebtn) {
		openfilebtn->EnableWindow(TRUE);
	}
	CMenu *mmenu = dlg.GetMenu();
	CMenu *submenu = mmenu->GetSubMenu(0);
	submenu->EnableMenuItem(ID_FILE_OPENFOLDER,		MF_BYCOMMAND | MF_ENABLED);
	submenu->EnableMenuItem(ID_FILE_OPENFILE,		MF_BYCOMMAND | MF_ENABLED);
	submenu->EnableMenuItem(ID_FILE_RESETFILELIST,	MF_BYCOMMAND | MF_ENABLED);

	opts.filename =	NULL;
	opts.dirname =	NULL;

	return TRUE;
}


/**********************************************************************************************//**
/**********************************************************************************************//**
									
									INTERNAL FUNCTIONS

/**********************************************************************************************//**
/**********************************************************************************************//**


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
* @fn	char *dtc_full_filenamedtc_full_filename(const char *acquisition_filename, const char *suffix, const char *path_name, char *full_filename)
*
* @brief	Returns constructed filename path_name\\(short)acquisition_filename(-extension)suffix
*
* @author	Marc
* @date		2018
*
* @param	acquisition_filename	acquisition filename char
* @param	suffix					suffix char to add
* @param	path_name				suffix char to add
* @param	full_filename			constructed filename returned char to add
*
* @return	full_filename with constructed pathname
**************************************************************************************************/

char *dtc_full_filename(const char *acquisition_filename, const char *suffix, const char *path_name, char *full_filename) {
	char tmpstring[MAX_STRING];
	char filename[MAX_STRING];
	
	init_string(tmpstring);
	init_string(filename);
	snprintf(filename, strlen(acquisition_filename) - 4, "%s", acquisition_filename);
	strcat(filename, suffix);
	filename[std::strlen(filename)] = '\0';
	strcpy(full_filename, path_name);
	std::strcat(full_filename, right(filename, strlen(filename) - InRstr(filename, "\\"), tmpstring));
	full_filename[std::strlen(full_filename)] = '\0';

	return full_filename;
}

/**********************************************************************************************//**
* @fn	char *dtc_full_filename_2suffix(const char *acquisition_filename, const char *suffix, const char *suffix2, const char *path_name, char *full_filename)
*
* @brief	Returns constructed filename path_name\\(short)acquisition_filename(-extension)suffix
*
* @author	Marc
* @date		2020
*
* @param	acquisition_filename	acquisition filename char
* @param	suffix					suffix char to add
* @param	suffix2					second suffix char to add
* @param	path_name				suffix char to add
* @param	full_filename			constructed filename returned char to add
*
* @return	full_filename with constructed pathname
**************************************************************************************************/

char *dtc_full_filename_2suffix(const char *acquisition_filename, const char *suffix, const char *suffix2, const char *path_name, char *full_filename) {
	char tmpstring[MAX_STRING];
	char filename[MAX_STRING];

	init_string(tmpstring);
	init_string(filename);
	snprintf(filename, strlen(acquisition_filename) - 4, "%s", acquisition_filename);
	strcat(filename, suffix);
	strcat(filename, suffix2);
	filename[std::strlen(filename)] = '\0';
	strcpy(full_filename, path_name);
	std::strcat(full_filename, right(filename, strlen(filename) - InRstr(filename, "\\"), tmpstring));
	full_filename[std::strlen(full_filename)] = '\0';

	return full_filename;
}


/******************************************************************************************************
*                                                                                                     *
*		zip file/folder                                                                               *
*                                                                                                     *
* from https://stackoverflow.com/questions/118547/creating-a-zip-file-on-windows-xp-2003-in-c-c       *
*                                                                                                     *
******************************************************************************************************/

void zip(char *zipfilename, char *item_to_be_zipped)
{
	#define MAX_THREADS 5000

	// Create zip file
	FILE* f = fopen(zipfilename, "wb");
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
		strlen = MultiByteToWideChar(CP_ACP, 0, zipfilename, -1, 0, 0);
		strptr1 = SysAllocStringLen(0, strlen);
		MultiByteToWideChar(CP_ACP, 0, zipfilename, -1, strptr1, strlen);

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

void LogString(CString log_cstring,  CString output_filename, int *log_counter, BOOL GUI_display)
{
	if (GUI_display) {
		CDeTeCtMFCDlg::getLog()->AddString((CString)getDateTime().str().c_str() + log_cstring);
		CDeTeCtMFCDlg::getLog()->SetTopIndex(CDeTeCtMFCDlg::getLog()->GetCount() - 1);
		CDeTeCtMFCDlg::getLog()->RedrawWindow();
	}
	
	GetLockQueue((CString)__FUNCTION__ + _T(" ") + log_cstring + _T(" ") + output_filename, output_filename);
	//std::ofstream output_log_stream(output_filename, std::ios_base::app, _SH_DENYRW);
	std::ofstream output_log_stream(output_filename, std::ios_base::app);
	std::string log_string = CString2string(log_cstring); 	// do not know why, but need to convert to string then back to cstring to have it work
	CString log_counter_cstr;
	log_counter_cstr.Format(L"%05i", (*log_counter));
	std::string log_counter_string = CString2string(log_counter_cstr);

	output_log_stream  << "PID " << std::to_string(GetCurrentProcessId()).c_str() << "-" << log_counter_string.c_str() << ": "  << getDateTimeMillis().str().c_str() << log_string.c_str() << "\n";
	output_log_stream.flush();
	output_log_stream.close();
	UnlockQueue(output_filename);
	(*log_counter) = (*log_counter) + 1;
}

BOOL GetOtherProcessedFiles(const int acquisition_index, int *pacquisition_index_children, int  *pacquisitions_to_be_processed, int *pnb_error_impact, int *pnb_null_impact, int *pnb_low_impact, int *pnb_high_impact, double *pduration_total, std::vector<std::string> *plog_messages, char *DeTeCtQueueFilename) {
	CString processed_filename;
	CString processed_filename_acquisition;
	CString processed_message;
	CString tmp;
	Rating_type processed_rating;
	std::wstring totalProgress_wstring_tmp;
	int		nb_otherprocessedfiles	= 0;
	int		nframe_child			= 0;
	int		fps_int_child			= 0;

	double duration = 0;
	while (GetProcessedFileFromQueue(&processed_filename, &processed_filename_acquisition, &processed_message, &processed_rating, &duration, &nframe_child, &fps_int_child, _T("file_processed "), (CString)DeTeCtQueueFilename)) {
		switch (processed_rating) {
		case Error:
			(*pnb_error_impact)++;
			break;
		case Null:
			(*pnb_null_impact)++;
			CDeTeCtMFCDlg::getimpactNull()->SetWindowText(std::to_wstring((*pnb_null_impact) + (*pnb_error_impact)).c_str());
			break;
		case Low:
			(*pnb_low_impact)++;
			CDeTeCtMFCDlg::getimpactLow()->SetWindowText(std::to_wstring((*pnb_low_impact)).c_str());
			break;
		case High:
			(*pnb_high_impact)++;
			CDeTeCtMFCDlg::getimpactHigh()->SetWindowText(std::to_wstring((*pnb_high_impact)).c_str());
			break;
		}
		(*pacquisition_index_children)++;
//(*pacquisitions_to_be_processed) = NbFilesFromQueue(char2CString(DeTeCtQueueFilename, &tmp));
		(*pduration_total) += duration;
		totalProgress_wstring_tmp = L"Total\n(" + std::to_wstring(acquisition_index + (*pacquisition_index_children)) + L"/" + std::to_wstring(*pacquisitions_to_be_processed) + L")";
		CDeTeCtMFCDlg::gettotalProgress()->SetWindowText(totalProgress_wstring_tmp.c_str());
		CDeTeCtMFCDlg::getProgress_all()->SetPos((short)(MAX_RANGE_PROGRESS * (float)(acquisition_index + (*pacquisition_index_children)) / (*pacquisitions_to_be_processed)));
		CDeTeCtMFCDlg::getProgress_all()->UpdateWindow();

		std::string processed_filename_acquisition_string = CString2string(processed_filename_acquisition);
		std::string processed_short_filename = processed_filename_acquisition_string.substr(processed_filename_acquisition_string.find_last_of("\\") + 1, processed_filename_acquisition_string.length());
		plog_messages->push_back(processed_short_filename + ":" + "    " + CString2string(processed_message));

		CDeTeCtMFCDlg::getLog()->AddString((CString)getDateTime().str().c_str() + "----- " + processed_short_filename.c_str() + " -----");
		CDeTeCtMFCDlg::getLog()->AddString((CString)getDateTime().str().c_str() + std::to_string(nframe_child).c_str() + (CString)" frames @ " + std::to_string(fps_int_child).c_str() + (CString)" fps (" + std::to_wstring((int)duration).c_str() + "s duration)");
		CDeTeCtMFCDlg::getLog()->AddString((CString)getDateTime().str().c_str() + processed_message);
		CDeTeCtMFCDlg::getLog()->AddString((CString)getDateTime().str().c_str());
		//CDeTeCtMFCDlg::getLog()->AddString((CString)getDateTime().str().c_str() + processed_short_filename.c_str() + ":" + "    " + processed_message);
		CDeTeCtMFCDlg::getLog()->SetTopIndex(CDeTeCtMFCDlg::getLog()->GetCount() - 1);
		CDeTeCtMFCDlg::getLog()->RedrawWindow();

		duration = 0;
		nb_otherprocessedfiles++;
	}
	if (nb_otherprocessedfiles == 0) return FALSE;
	else {
		return TRUE;
	}
}

int	ForksInstances(const int maxinstances, const int detect_PID, const CString DeTeCtQueueFilename, const int scan_time, const int scan_time_random_max, int *pnbinstances)
{
	int nb_forked_instances = 0;
	int files_to_be_processed = 0;
	// Forks other DeTeCt instances if work in queue

	if ((opts.parent_instance) && (maxinstances > 1)) {
		files_to_be_processed = NbItemFromQueue(L"file", DeTeCtQueueFilename, TRUE);
		if (files_to_be_processed > 0) {
			(*pnbinstances) = ChildrenProcessesNumber() + 1;
			int nb_instances_to_be_forked = MIN(files_to_be_processed, maxinstances - (*pnbinstances));	// NBItemQueue files to be processed
			// Forks DeTeCt.exe -auto
			nb_forked_instances = 0;
			while (nb_forked_instances < nb_instances_to_be_forked) { // no variable there: we do not know at which speed DeTeCt child instances will be launched
				if (nb_forked_instances > 0) Sleep(scan_time + rand() % scan_time_random_max); // in ms
				if (detect_PID > 0) { // Should not happen, as stores parent DeTeCt PID
					HINSTANCE status = ShellExecute(NULL, L"open", DeTeCt_additional_filename_exe_fullpath(_T(".exe")), _T(" -auto - dtcpid ") + (CString)std::to_string(detect_PID).c_str() + (CString) " - maxinst " + (CString)std::to_string(maxinstances).c_str(), NULL, SW_HIDE);
					if ((int)status <= 32) AfxMessageBox(_T("Error ShellExecute, code ") + (CString)std::to_string((int)status).c_str());
				}
				else {
					HINSTANCE status = ShellExecute(NULL, L"open", DeTeCt_additional_filename_exe_fullpath(_T(".exe")), _T(" -auto  -maxinst ") + (CString)std::to_string(maxinstances).c_str(), NULL, SW_HIDE);
					if ((int)status <= 32) AfxMessageBox(_T("Error ShellExecute, code ") + (CString)std::to_string((int)status).c_str());
				}
				nb_forked_instances++;
			}
			(*pnbinstances) += nb_forked_instances;
			DisplayInstanceType(pnbinstances);
		}
	}
	return nb_forked_instances;
}

void DisplayProcessingTime(clock_t *pcomputing_threshold_time, clock_t *plast_time, const clock_t refresh_duration, const clock_t single_time, const clock_t total_time) {
	CString processing_time_str;
	CString total_type;

	(*plast_time) = clock();
	(*pcomputing_threshold_time) = (*plast_time) + refresh_duration;

	processing_time_str.Format(L"Processing time: %.*fs (file)  %.*fs (%s)", 1, ((double)((*plast_time) - single_time) / (double)CLOCKS_PER_SEC), 1, ((double)((*plast_time) - total_time) / (double)CLOCKS_PER_SEC), static_cast<LPCTSTR>(TotalType()));
	CDeTeCtMFCDlg::getcomputingTime()->SetWindowText(processing_time_str);
}

CString TotalType() {
	if (opts.parent_instance) return _T("total");
	else return _T("instance");
}

Instance_type InstanceType(CString *pinstance_text) {
	Instance_type instance_type;

	if (opts.parent_instance)
		if (opts.maxinstances > 1)
				if (opts.autostakkert)	instance_type = autostakkert_parent;
				else					instance_type = parent;
		else if (opts.autostakkert)		instance_type = autostakkert_single;
		else							instance_type = single;
	else if (opts.autostakkert)			instance_type = autostakkert_child; 
	else								instance_type = child;

	switch (instance_type) {
		case autostakkert_parent:
			(*pinstance_text) = "AS! PARENT";
			break;
		case parent:
			(*pinstance_text) = "PARENT";
			break;
		case autostakkert_single:
			(*pinstance_text) = "AS! single";
			break;
		case single:
			(*pinstance_text) = "Single";
			break;
		case autostakkert_child:
			(*pinstance_text) = "AS! child";
			break;
		case child:
			(*pinstance_text) = "Child";
			break;
	}
	if (opts.debug) (*pinstance_text) = (*pinstance_text) + (CString)" instance #" + (CString)std::to_string(GetCurrentProcessId()).c_str();

	return instance_type;
}

Instance_type DisplayInstanceType(int *nbinstances) {
	CString			instance_cstring;
	CString			nbinstances_cstr;
	CString			max_nbinstances_cstr;
	Instance_type	instance_type;
	CString			instance_type_cstring;
//	(*nbinstances) = 0;

	instance_type = InstanceType(&instance_type_cstring);
	switch (instance_type) {
		case autostakkert_parent:
		case parent:
			if ((*nbinstances) == 0) {
				(*nbinstances) = ChildrenProcessesNumber() + 1;	// heavy computing, does not compute and uses value if not nul
			}
//			if (nbinstances > 1) {
				if (opts.autostakkert) instance_cstring = _T("Executed from AS!, ") +  instance_cstring;
				nbinstances_cstr.Format(L"%d", (*nbinstances));
				max_nbinstances_cstr.Format(L"%d", opts.maxinstances);
				if (opts.debug) instance_cstring = instance_type_cstring + _T(" (") + nbinstances_cstr + _T("/") + max_nbinstances_cstr + _T(")");
				else instance_cstring = nbinstances_cstr + _T("/") + max_nbinstances_cstr + _T(" instances");
				/*if (opts.debug) instance_cstring = instance_type_cstring + _T(" (") + nbinstances_cstr + _T("/") + max_nbinstances_cstr + _T(")");
				else {
					instance_cstring = nbinstances_cstr + _T(" instance");
					if (nbinstances > 1) instance_cstring = instance_cstring + _T("s");
					instance_cstring = instance_cstring + _T(" running");
				}*/
	//		}
			break;
// No display in child mode, so no calculation of nbinstances
		case autostakkert_child:
		case child:
		case autostakkert_single:
		case single:
			break;
	}
	CDeTeCtMFCDlg::getInstance()->SetWindowText(instance_cstring);
	
	return instance_type;
}

void WriteIni() {
	CString str;
	CString DeTeCtIniFilename = DeTeCt_additional_filename_exe_fullpath(DTC_INI_SUFFIX);
	//char DeTeCtIniFilename_char[MAX_STRING];
	//remove(CString2char(DeTeCtIniFilename, DeTeCtIniFilename_char));

	str.Format(L"%.2f", opts.incrLumImpact);
	::WritePrivateProfileString(L"impact", L"min_strength", str, DeTeCtIniFilename);
	str.Format(L"%d", opts.nframesRef);
	::WritePrivateProfileString(L"other", L"refmin", str, DeTeCtIniFilename);
	str.Format(L"%d", opts.incrFrameImpact);
	::WritePrivateProfileString(L"impact", L"frames", str, DeTeCtIniFilename);
	str.Format(L"%.2f", opts.impact_duration_min);
	::WritePrivateProfileString(L"impact", L"impact_duration_min", str, DeTeCtIniFilename);
	str.Format(L"%.2f", opts.radius);
	::WritePrivateProfileString(L"impact", L"radius", str, DeTeCtIniFilename);
	str.Format(L"%d", opts.threshold);
	::WritePrivateProfileString(L"impact", L"thresh", str, DeTeCtIniFilename);
	str.Format(L"%.2f", opts.facSize);
	::WritePrivateProfileString(L"roi", L"sizfac", str, DeTeCtIniFilename);
	str.Format(L"%.2f", opts.secSize);
	::WritePrivateProfileString(L"roi", L"secfac", str, DeTeCtIniFilename);
	str.Format(L"%.2f", opts.medSize);
	::WritePrivateProfileString(L"roi", L"medbuf", str, DeTeCtIniFilename);
	str.Format(L"%d", opts.ROI_min_px_val);
	::WritePrivateProfileString(L"roi", L"ROI_min_px_val", str, DeTeCtIniFilename);
	str.Format(L"%d", opts.ROI_min_size);
	::WritePrivateProfileString(L"roi", L"ROI_min_size", str, DeTeCtIniFilename);
	str.Format(L"%d", opts.minframes);
	::WritePrivateProfileString(L"other", L"frmin", str, DeTeCtIniFilename);
	str.Format(L"%.2f", opts.histScale);
	::WritePrivateProfileString(L"other", L"histscale", str, DeTeCtIniFilename);
	str.Format(L"%d", opts.thrWithMask);
	::WritePrivateProfileString(L"impact", L"mask", str, DeTeCtIniFilename);
	str.Format(L"%d", opts.detail);
	::WritePrivateProfileString(L"impact", L"detail", str, DeTeCtIniFilename);
	str.Format(L"%d", opts.allframes);
	::WritePrivateProfileString(L"impact", L"inter", str, DeTeCtIniFilename);
	str.Format(L"%.2f", opts.impact_distance_max);
	::WritePrivateProfileString(L"impact", L"impact_distance_max", str, DeTeCtIniFilename);
	str.Format(L"%.2f", opts.impact_max_avg_min);
	::WritePrivateProfileString(L"impact", L"impact_max_avg_min", str, DeTeCtIniFilename);
	str.Format(L"%.2f", opts.impact_confidence_min);
	::WritePrivateProfileString(L"impact", L"impact_confidence_min", str, DeTeCtIniFilename);

	str.Format(L"%d", opts.viewROI);
	::WritePrivateProfileString(L"view", L"roi", str, DeTeCtIniFilename);
	str.Format(L"%d", opts.viewTrk);
	::WritePrivateProfileString(L"view", L"trk", str, DeTeCtIniFilename);
	str.Format(L"%d", opts.viewRef);
	::WritePrivateProfileString(L"view", L"ref", str, DeTeCtIniFilename);
	str.Format(L"%d", opts.viewMsk);
	::WritePrivateProfileString(L"view", L"msk", str, DeTeCtIniFilename);
	str.Format(L"%d", opts.viewThr);
	::WritePrivateProfileString(L"view", L"thr", str, DeTeCtIniFilename);
	str.Format(L"%d", opts.viewSmo);
	::WritePrivateProfileString(L"view", L"smo", str, DeTeCtIniFilename);
	str.Format(L"%d", opts.viewRes);
	::WritePrivateProfileString(L"view", L"res", str, DeTeCtIniFilename);
	str.Format(L"%d", opts.viewDif);
	::WritePrivateProfileString(L"view", L"dif", str, DeTeCtIniFilename);
	str.Format(L"%d", opts.viewHis);
	::WritePrivateProfileString(L"view", L"his", str, DeTeCtIniFilename);
	str.Format(L"%d", opts.ignore);
	::WritePrivateProfileString(L"other", L"ignore", str, DeTeCtIniFilename);
	//int bayerCodes[] = { 0, cv::COLOR_BayerBG2RGB, cv::COLOR_BayerGB2RGB, cv::COLOR_BayerRG2RGB, cv::COLOR_BayerGR2RGB };
	str.Format(L"%d", opts.bayer);
	::WritePrivateProfileString(L"other", L"debayer", str, DeTeCtIniFilename);
	str.Format(L"%d", opts.filter.type);
	::WritePrivateProfileString(L"other", L"filter", str, DeTeCtIniFilename);
	::WritePrivateProfileString(L"other", L"darkfile", CA2T(opts.darkfilename), DeTeCtIniFilename);

	str.Format(L"%d", opts.debug);
	::WritePrivateProfileString(L"processing", L"debug", str, DeTeCtIniFilename);
	str.Format(L"%d", opts.dateonly);
	::WritePrivateProfileString(L"processing", L"dateonly", str, DeTeCtIniFilename);
	str.Format(L"%d", opts.zip);
	::WritePrivateProfileString(L"processing", L"zip", str, DeTeCtIniFilename);
	str.Format(L"%d", opts.email);
	::WritePrivateProfileString(L"processing", L"email", str, DeTeCtIniFilename);
	// From main window checkboxes
	str.Format(L"%d", !opts.interactive);
	::WritePrivateProfileString(L"processing", L"autoprocessing", str, DeTeCtIniFilename);
	str.Format(L"%d", opts.exit);
	::WritePrivateProfileString(L"processing", L"autoexit", str, DeTeCtIniFilename);
	str.Format(L"%d", opts.shutdown);
	::WritePrivateProfileString(L"processing", L"autoshutdown", str, DeTeCtIniFilename);
	str.Format(L"%d", opts.maxinstances);
	::WritePrivateProfileString(L"processing", L"maxinstances", str, DeTeCtIniFilename);
}

void FileListToQueue(std::vector<std::string> file_list, CString QueueFilename) {
	for (std::string filename : file_list) {
		PushFileToQueue((CString)filename.c_str(), QueueFilename);
		file_list.erase(file_list.begin());
	}
}

void AcquisitionFileListToQueue(AcquisitionFilesList *pacquisition_files, const CString tag_current, const int index_current, const CString out_directory, int *acquisitions_to_be_processed) {
	CString tmp, tmp2;
	if (!file_exists(CString2string((CString)opts.DeTeCtQueueFilename))) {
		CreateQueueFileName();
		/*DWORD pid;
		if (opts.autostakkert_PID > 0) {				// Autostakkert mode
			CString pid_cstring;
			pid_cstring.Format(L"%d", opts.autostakkert_PID);
			CString2char(DeTeCt_additional_filename_exe_fullpath(CString(_T(DTC_QUEUE_PREFIX)) + _T("_as") + pid_cstring + _T(DTC_QUEUE_EXT)), opts.DeTeCtQueueFilename);
		}
		else {
			CString pid_cstring;
			pid = GetCurrentProcessId();
			pid_cstring.Format(L"%d", GetCurrentProcessId());
			opts.detect_PID = pid;
			CString2char(DeTeCt_additional_filename_exe_fullpath(CString(_T(DTC_QUEUE_PREFIX)) + _T("_dtc") + pid_cstring + _T(DTC_QUEUE_EXT)), opts.DeTeCtQueueFilename);
		}*/
		CString log_cstring;
		if (!GetItemFromQueue(&log_cstring, _T("output_dir"), (CString)opts.DeTeCtQueueFilename)) PushItemToQueue(out_directory, _T("output_dir"), char2CString(opts.DeTeCtQueueFilename, &tmp2), TRUE);
		SetIntParamToQueue(opts.maxinstances, _T("max_instances"), (CString)opts.DeTeCtQueueFilename);
	}
	if (file_exists(CString2string((CString)opts.DeTeCtQueueFilename))) {
		std::string filename;
		int index = 0;
		//int initial_file_list_size = pacquisition_files->file_list.size();
		(*acquisitions_to_be_processed) = pacquisition_files->file_list.size();
		while (index < pacquisition_files->file_list.size()) {
			filename = pacquisition_files->file_list.at(index);
			if (index < index_current)			PushItemToQueue(char2CString(filename.c_str(), &tmp), _T("file_ok"), char2CString(opts.DeTeCtQueueFilename, &tmp2), TRUE);
			else if (index == index_current)	PushItemToQueue(char2CString(filename.c_str(), &tmp), tag_current, char2CString(opts.DeTeCtQueueFilename, &tmp2), TRUE);
			else								PushFileToQueue(char2CString(filename.c_str(), &tmp), char2CString(opts.DeTeCtQueueFilename, &tmp2));
			if (index > index_current) {
				pacquisition_files->file_list.erase(pacquisition_files->file_list.begin() + index);
				pacquisition_files->acquisition_file_list.erase(pacquisition_files->acquisition_file_list.begin() + index);
				pacquisition_files->nb_prealigned_frames.erase(pacquisition_files->nb_prealigned_frames.begin() + index); // WARNING in debug, error in .begin()
			}
			else index++;
			//index++;
		}
	}
}