#define _WIN32_WINNT _WIN32_WINNT_WINXP

/********************************************************************************/
/*                                                                              */
/*			DTC	(c) Luis Calderon, Marc Delcroix, Jon Juaristi 2012-			*/
/*                                                                              */
/********************************************************************************/
#include "stdafx.h"
#include "processes_queue.hpp"
#include "dtcgui.hpp"
#include "common2.hpp"
#include "DeTeCt-MFC.hpp"
#include <windows.h> //after processes_queue.h
#include <string>
#include <vector>
#include <queue>
#include <iostream>
#include <ctime>
#include <numeric>
#include <regex>
#include <fstream>

#include "DeTeCt-MFCDlg.hpp"
#include <strsafe.h>

#include <shldisp.h>
#include <tlhelp32.h>
#include <stdio.h>

#include <direct.h>

#include <numeric>      // std::iota
#include <algorithm>    // std::sort, std::stable_sort

#include <opencv2/imgcodecs/legacy/constants_c.h>  // test OpenCV 4.7.0 
#include <opencv2/imgproc.hpp>  // test OpenCV 4.7.0 
#include <opencv2/videoio.hpp>  // test OpenCV 4.7.0 
//#include <opencv2/videoio/videoio_c.h>  // test OpenCV 4.7.0 
#include <opencv2/highgui.hpp>

#ifndef _SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING
#define _SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING
#endif
#include <experimental\filesystem>
#include <iomanip> // test OpenCV 4.7.0 

namespace filesys = std::experimental::filesystem;

#define CROSS_DIFFERENTIAL_PHOTOMETRY_LMAX 32
#define CROSS_MAX_LMAX 20
#define CROSS_SIZEMIN 1
#define CROSS_SIZEMAX 2
#define CROSS_ROI_MIN opts.ROI_min_size
#define CROSS_ROI_MAX 250



//#include <opencv2/imgproc.hpp>  //TEST opencv3

void			LogString(CString log_cstring, CString output_filename, int *log_counter, BOOL GUI_display, int* pwaitms);
int				GetOtherProcessedFiles(const size_t acquisition_index, size_t* pacquisition_index_children, size_t* pacquisitions_to_be_processed, int *pnb_error_impact, int *pnb_null_impact, int *pnb_low_impact, int *pnb_high_impact, double *pduration_total, std::vector<std::string> *plog_messages, char *DeTeCtQueueFilename, clock_t* computing_threshold_time, clock_t* end, clock_t computing_refresh_duration, clock_t begin, clock_t begin_total, const int nframe, const int frame_number);
int				GetOtherProcessedFiles2(const int acquisitions_processed, int* pacquisition_index_children, int* pacquisitions_to_be_processed, int* pnb_error_impact, int* pnb_null_impact, int* pnb_low_impact, int* pnb_high_impact, double* pduration_total, std::vector<std::string>* plog_messages, char* DeTeCtQueueFilename, clock_t* computing_threshold_time, clock_t* end, clock_t computing_refresh_duration, clock_t begin, clock_t begin_total, const int nframe, const int frame_number);
int				ForksInstances(const int maxinstances, const int PID, const CString DeTeCtQueueFilename, const int scan_time, const int scan_time_random_max, int *pnbinstances);
int				ASorDeTeCtPID(const int AutoStakkert_ID, const int DeTeCt_PID);
void			DisplayProcessingTime(clock_t *pcomputing_threshold_time, clock_t *plast_time, const clock_t refresh_duration, const clock_t single_time, const clock_t total_time);
CString			TotalType();
Instance_type	InstanceType(CString *pinstance_text);
int				rename_replace(const char* src, const char* dest, const char* foldername, const char* function);
void			UpdateProgress(const size_t acquisitions_to_be_processed, const size_t acquisitions_processed, const size_t acquisition_index_children, const int nframe, const int frame_bumber, const char *DeTeCtQueueFilename);

void			Show_matrix(const cv::Mat matrix, const char *title, const bool normalize_image, const int wait_ms);

bool			is_point_black_in_frame(cv::Mat FrameMat, const cv::Point Point_to_check, const int delta_pixels, const float background_level);
bool			is_zone_black_around_point(const cv::Mat FrameMat, const cv::Point Point_to_check, const int delta_pixels, const float background_level);
bool			is_bright_point_valid(cv::Mat pGryMat, const cv::Point Point_to_check, const float background, double *pplanet_radius_estimation, double *pdistance_to_planet_center, bool* pIs_black, bool* pIs_outside_of_planet, bool* pIs_not_on_planet);
bool			check_bright_point_on_bright_line_column(cv::Mat pADUdtcMat, cv::Mat pADUdtcMat_invalid_points_corrected, const cv::Point brightestDtcImgPoint, bool *pIs_bright_line, bool *pIs_bright_column, const double distance_factor_from_edge, const double line_column_avg_min, const bool correct_line, const int correction_value);

double			planet_radius(const cv::Mat FrameMat, const double background_level);
double			planet_radius_single_estimation(const cv::Mat FrameMat, const double background_level, const unsigned int init_x, const int delta_x, const unsigned int last_x, const unsigned int init_y, const int delta_y, const unsigned int last_y);
void			dtcDrawImpact(const cv::Mat frame, const cv::Point point, const cv::Scalar colour, const double length, const bool variable_thickness, const unsigned int ROI_size);
bool			Is_frame_similarity_valid(const Similarity_type similarity_method, const Comparison_type comparison_frame_type, const int nframe, const int frame_errors, bool* pis_frame_errors, bool* pis_frame_duplicate, bool* pis_frame_errors_too_different,
					std::array<double, max_similarity>* psimilarity_reference, std::array<double, max_similarity>* psimilarity_reference_last_valid, std::array<double, max_similarity>* pdelta_similarity_reference, const cv::Mat pReferenceMat,
					std::array<double, max_similarity>* psimilarity_previous_frame, std::array<double, max_similarity>* psimilarity_previous_frame_last_valid, std::array<double, max_similarity>* pdelta_similarity_previous_frame, std::array<cv::Mat, max_similarity>* pPreviousFrameMat,
					const cv::Mat pGryMat);


/** @brief	Options for the algorithm */

#define MAX_RANGE_PROGRESS SHRT_MAX

char impact_detection_dirname[MAX_STRING]		= {0};
char zip_detection_location[MAX_STRING]			= {0};
char zipfile[MAX_STRING]						= {0};
char log_detection_dirname[MAX_STRING]			= {0};
char email_subject_probabilities[MAX_STRING]	= {0};
char email_body_probabilities[MAX_STRING]		= {0};

extern CDeTeCtMFCDlg dlg;

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

void read_files(std::string folder,  AcquisitionFilesList *acquisition_files) {
	DIR				*directory;
	struct dirent	*entry;
	std::string		acquisition_file;

	std::vector<std::string> supported_videoext				= { VIDEOS_EXT };
	std::vector<std::string> supported_fileext				= { FILES_EXT };
	std::vector<std::string> supported_otherext				= { AUTOSTAKKERT_EXT };
	// Syntax files:
	// F0.* *0000_*.* *_000000.*  *_000001.* *_00000.* *_00001.* *_0000.* *_0001.* *_0.tif nb1.*
	// supported 0/1 number syntax for full filename
	std::vector<std::string> supported_fullfilename_number = { FULLFILENAME_NUMBER };
	// supported 0/1 number inside filename
	std::vector<std::string> supported_filename_number		= { FILENAME_NUMBER };

	// ignored dtc own files
	std::vector<std::string> not_supported_suffix = { DTC_MAX_MEAN_SUFFIX, DTC_MAX_MEAN1_SUFFIX, DTC_MAX_MEAN2_SUFFIX, DTC_MEAN_SUFFIX, DTC_MEAN2_SUFFIX,
		DTC_DIFF_SUFFIX, DTC_DIFF2_SUFFIX, VIDEOTEST_SUFFIX, DTC_MAX_SUFFIX, MEAN_SUFFIX, DTC_SUFFIX, DTC_DIFF_FRAME_PREFIX, DTC_MAX_FRAME_PREFIX };

	if (!(directory = opendir(folder.c_str()))) {
		closedir(directory);
		return;
	}
	if (!(entry = readdir(directory))) {
		closedir(directory);
		return;
	}
	do {
		if (entry->d_type == DT_DIR) { //directory
			if ((strcmp(entry->d_name, ".") != 0) && (strcmp(entry->d_name, "..") != 0)) {
				read_files(folder + "\\" + entry->d_name, acquisition_files);
			}
		}
		else {
			std::string file(entry->d_name);
			std::string extension = file.substr(file.find_last_of(".") + 1, file.length());
			lowercase_string(&extension);

			acquisition_file = "";
			if (std::find(supported_videoext.begin(), supported_videoext.end(), extension) != supported_videoext.end()) { //video file
				acquisition_files->file_list.push_back(folder + "\\" + entry->d_name);
				acquisition_file = folder + "\\" + entry->d_name;
				acquisition_files->acquisition_file_list.push_back(acquisition_file);
				acquisition_files->nb_prealigned_frames.push_back(0);
				acquisition_files->acquisition_size.push_back(filesize(acquisition_file.c_str()));
			}
			else if (std::find(supported_fileext.begin(), supported_fileext.end(), extension) != supported_fileext.end()) { // file extensions
				int found = false;
				for (std::string filename_number : supported_fullfilename_number) { // number just before extension
					if (file.find(filename_number) != std::string::npos) {
						found = true;
						/*if (isNumeric(file.substr(file.find(filename_number) + filename_number.length() + 1, 1))) {
							found = false;
								break;
						}*/
						for (std::string suffix : not_supported_suffix) {		// no  detect suffix
							if (file.find(suffix) != std::string::npos) {
								found = false;
								break;
							}
						}
						if (found) break;
					}
				}
				if (!found) {
					for (std::string filename_number : supported_filename_number) { // number format inside
						if (file.find(filename_number) != std::string::npos) {
							found = true;
							for (std::string suffix : not_supported_suffix) {		// no  detect suffix
								if (file.find(suffix) != std::string::npos) {
									found = false;
									break;
								}
							}
							if (((found) && ((filename_number.substr(filename_number.size() - 1, 1) == "0") || (filename_number.substr(filename_number.size() - 1, 1) == "1"))) && (isNumeric(file.substr(file.find(filename_number)+ filename_number.size(), 1)))) found = FALSE; // no 0001[0-9]
						}
						if (found) break;
					}
				}
				if (found) {
					acquisition_files->file_list.push_back(folder + "\\" + entry->d_name);
					acquisition_file = folder + "\\" + entry->d_name;
					acquisition_files->acquisition_file_list.push_back(acquisition_file);
					acquisition_files->nb_prealigned_frames.push_back(0);
					acquisition_files->acquisition_size.push_back(filesize(acquisition_file.c_str()));
					//break;			//avoid picking *1, *10-*19 when *0 or *1 found
				}
			}
			else if (std::find(supported_otherext.begin(), supported_otherext.end(), extension) != supported_otherext.end()) {
				if (extension.compare(AUTOSTAKKERT_EXT) == 0) {
					//std::vector<cv::Point> cm_list;
					int cm_list_start = 0;
					int cm_list_end = INT_MAX;
					int cm_frame_count = 0;

					read_autostakkert_session_file(folder + "\\" + file, &acquisition_file, NULL, &cm_list_start, &cm_list_end, &cm_frame_count);

					if (acquisition_file.length() > 0) {
						acquisition_files->file_list.push_back(folder + "\\" + entry->d_name);
						acquisition_files->acquisition_file_list.push_back(acquisition_file);
						acquisition_files->nb_prealigned_frames.push_back(MIN(cm_list_end - cm_list_start+1, cm_frame_count));
						acquisition_files->acquisition_size.push_back(filesize(acquisition_file.c_str()));
					}
				}
				else {
					acquisition_files->file_list.push_back(folder + "\\" + entry->d_name);
					acquisition_file = folder + "\\" + entry->d_name;
					acquisition_files->acquisition_file_list.push_back(acquisition_file);
					acquisition_files->nb_prealigned_frames.push_back(0);
					acquisition_files->acquisition_size.push_back(filesize(acquisition_file.c_str()));
				}
			}
		}
	} while (entry = readdir(directory));
	closedir(directory);
//Remove duplicates from as3 (keeping as3)
	std::vector<std::string>::iterator acquisition_files_vector_string =	acquisition_files->file_list.begin();
	acquisition_files_vector_string =									acquisition_files->acquisition_file_list.begin();
	std::vector<int>::iterator acquisition_files_vector_int =			acquisition_files->nb_prealigned_frames.begin();
	std::vector<int64>::iterator acquisition_files_vector_long =		acquisition_files->acquisition_size.begin();
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
					acquisition_files->acquisition_size.erase(acquisition_files->acquisition_size.begin() + j);
					if (j < i) i--;
				}
				else {
					DBOUT("Erasing " << acquisition_files->acquisition_file_list.at(i).c_str());
					acquisition_files->file_list.erase(acquisition_files->file_list.begin() + i);
					acquisition_files->acquisition_file_list.erase(acquisition_files->acquisition_file_list.begin() + i);
					acquisition_files->nb_prealigned_frames.erase(acquisition_files->nb_prealigned_frames.begin() + i);
					acquisition_files->acquisition_size.erase(acquisition_files->acquisition_size.begin() + i);
					if (i>0) i--;
				}
			}
		}
	}
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

int detect_impact(DTCIMPACT* dtc, DTCIMPACT* dtcout, double meanValue, LIST* list, ITEM** dtcMax, double *ptemporal_density, double radius, double incrLum, double radius_share, int impact_frames_min, double temporal_density_min, std::vector<std::string> *pdetect_impact_log_messages)
{
	int		c;
	int		x0;
	int		y0;
	int		lastivalFrame;
	//double	maxMeanValue;
	double	d				= DBL_MAX;		// long frame_distance;
	ITEM**	ord;
	ITEM**	tmp;
	ITEM	*tmpSrc;
	int		nb_impact		= 0;
	std::vector<ITEM*> items;

	struct FrameOrder		frameOrder;
	struct BrightnessOrder	brightnessOrder;

	std::string ok_or_ko_string;
	std::string bool_string;
	bool debug_impact_detection = false;

	//if (fps < 0) throw std::logic_error("Negative fps value, can't operate with impact detection");
	if (list->size <= 0) return 0;
	if (!(ord = (ITEM **)calloc(list->size, sizeof(ITEM *)))) throw std::bad_alloc();

/////////////////////
///////////////////// 1. not an impact if brightness increase is lower than limit parameter
////////////////////	(uses meanValue, incrLum)
	// Sorts per brightness value list into ord to identify the brightest one(s)
	for (tmpSrc = list->head, tmp = ord, c = 0;		tmpSrc && c < list->size;	tmpSrc = tmpSrc->next, tmp++, c++) *tmp = tmpSrc; //populates ord
	qsort(ord, list->size, sizeof(ITEM *), item_point_val_cmp);
	//maxMeanValue = get_item_point_val_list_mean_value(list);
	ok_or_ko_string = "OK";
	if (ord[0]->point->val <= meanValue * (1 + incrLum)) {
		ok_or_ko_string = "ko";
if (debug_impact_detection) (*pdetect_impact_log_messages).push_back("Impact (" + std::to_string(ord[0]->point->x) + "," + std::to_string(ord[0]->point->y) + ") detection " + ok_or_ko_string + ": brightness of best candidate : " + std::to_string(ord[0]->point->val) + " >= ? " + std::to_string(meanValue * (1 + incrLum)) + " (incr Lum limit = " + std::to_string(ord[0]->point->val/meanValue - 1) + " vs " + std::to_string(incrLum) + ")");
		return 0; // brightest point is not bright enough, no impact
	}
if (debug_impact_detection) (*pdetect_impact_log_messages).push_back("Impact (" + std::to_string(ord[0]->point->x) + "," + std::to_string(ord[0]->point->y) + ") detection " + ok_or_ko_string + ": brightness of best candidate : " + std::to_string(ord[0]->point->val) + " >= ? " + std::to_string(meanValue * (1 + incrLum)) + " (incr Lum limit = " + std::to_string(ord[0]->point->val / meanValue - 1) + " vs " + std::to_string(incrLum) + ")");
	x0 = ord[0]->point->x;
	y0 = ord[0]->point->y;
	dtc->MaxFrame = ord[0]->point->frame;
/////////////////////
	/*	ITEM* max = ord[0];
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
		if ((d <= radius) && (frame_distance <= 20)) potential_impact.push_back(current);
	}
	dtc->MaxFrame = max->point->frame;
	std::sort(potential_impact.begin(), potential_impact.end(), frameOrder);
	dtc->nMinFrame = potential_impact.front()->point->frame;
	dtc->nMaxFrame = potential_impact.back()->point->frame;*/

	//int impact_frame_num = (int)std::ceil(fps * opts.timeImpact);
	//int impact_frame_num = incrFrame;

	//int					impact_frame_num	= impact_frames_min;	// use of minimum impact frames or minimum impact time
	std::deque<ITEM*>	potential_impact;
	ITEM*				impactBrightest	= nullptr;
	ITEM*				brightest		= nullptr;
	double				maxMean			= 0.0;
	
	/*double minStdDev = DBL_MAX;
	double stdDev = 0.0; */

	// Resorts per frame index value list into ord to reorder list
	qsort(ord, list->size, sizeof(ITEM *), item_frame_rank_cmp);
	if (list->size >= impact_frames_min) ok_or_ko_string = "OK"; else ok_or_ko_string = "ko";
if (debug_impact_detection) (*pdetect_impact_log_messages).push_back("Impact detection: min number of frames " + ok_or_ko_string + ": " + std::to_string(list->size) + " >= ? " + std::to_string(impact_frames_min));
	for (int i = 0; i < list->size; i++) {
		if (i >= impact_frames_min) {

/////////////////////
///////////////////// 2. identify brightest impact (by constructing potential_impact list)
/////////////////////		(uses impact_frame_num,  incrLum)
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

			if (mean <= meanValue * (1 + incrLum)) ok_or_ko_string = "ko"; else ok_or_ko_string = "OK";
if (debug_impact_detection) (*pdetect_impact_log_messages).push_back("Impact detection " + ok_or_ko_string + ": brightness of candidate : " + std::to_string(mean) + " >= ? " + std::to_string(meanValue * (1 + incrLum)) + " (incr Lum limit = " + std::to_string(mean / meanValue - 1) + " vs " + std::to_string(incrLum) + ")");
			if (mean < (meanValue * (1 + incrLum))) {	// use of minimum impact frames or minimum impact time
				potential_impact.pop_front();
				potential_impact.push_back(ord[i]);
				continue;
			}
			std::sort(potential_impact.begin(), potential_impact.end(), brightnessOrder);
			brightest = potential_impact.front();
			// Starts from the second element, since the first is the brightest point of the queue
			/*bool candidate = std::all_of(potential_impact.begin() + 1, potential_impact.end(), [&](const ITEM* it) {
				d = sqrt(pow(it->point->x - brightest->point->x, 2) + pow(it->point->y - brightest->point->y, 2));
				return d <= radius;
			});	*/

			/* make that only the 70% of the frames have to be in the place of impact */
/////////////////////
///////////////////// 3. validate brightest impact if count of increase brightness frames is ok
/////////////////////		(uses radius, radius_share)
			 
			int		count = 0;
			(*ptemporal_density) = 0.0;
			std::for_each(potential_impact.begin(), potential_impact.end(), [&](const ITEM* it) {
				d = sqrt(pow(it->point->x - brightest->point->x, 2) + pow(it->point->y - brightest->point->y, 2));
				if (d <= radius) {
					count++;
					// computation of temporal density (cumulation of	frame distance /	[nb frames] *	brightness increase, divided finally by count afterwards)
					//																?		list->size;	?	
					double frame_distance = abs(it->point->frame - brightest->point->frame);
					(*ptemporal_density) += it->point->val / (frame_distance + 1);
				}
				});
			bool candidate = ((double(count - 1) / double(potential_impact.size())) >= radius_share);
			if (candidate) ok_or_ko_string = "OK"; else ok_or_ko_string = "ko";
if (debug_impact_detection) (*pdetect_impact_log_messages).push_back("Impact (" + std::to_string(brightest->point->x) + "," + std::to_string(brightest->point->y) + ") detection " + ok_or_ko_string + ": radius share : " + std::to_string(double(count - 1) / double(potential_impact.size())) + " >= ? " + std::to_string(radius_share));
			if (candidate && (count > 0)) {
				(*ptemporal_density) /= count; 
				if (temporal_density_min > 0) (*pdetect_impact_log_messages).push_back("Impact (" + std::to_string(brightest->point->x) + "," + std::to_string(brightest->point->y) + ") detection: temporal density: " + std::to_string((*ptemporal_density)) + " < ? " + std::to_string(temporal_density_min)); // if temporal_density = 0, final test will always fail / test deactivated
				if ((*ptemporal_density) < temporal_density_min) candidate = false;
			}
			std::sort(potential_impact.begin(), potential_impact.end(), frameOrder);
			//if (candidate && stdDev < minStdDev) {
//LOGSTRING
			if (candidate) bool_string = "true"; else bool_string = "false";
			if (candidate && mean > maxMean) ok_or_ko_string = "OK"; else ok_or_ko_string = "ko";
if (debug_impact_detection) (*pdetect_impact_log_messages).push_back("Impact (" + std::to_string(brightest->point->x) + "," + std::to_string(brightest->point->y) + ") detection " + ok_or_ko_string + ": brightest candidate : " + bool_string + " = true ? ; " + std::to_string(mean) + " > ? " + std::to_string(maxMean));
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
				while (potential_impact.size() > impact_frames_min)
					potential_impact.pop_front();
				potential_impact.push_back(ord[i]);
				continue;
			}
/////////////////////
		}
		potential_impact.push_back(ord[i]);
	}

	lastivalFrame = dtc->nMaxFrame - dtc->nMinFrame + 1;
	dtcout->MaxFrame = dtc->MaxFrame;
	dtcout->nMinFrame = dtc->nMinFrame;
	dtcout->nMaxFrame = dtc->nMaxFrame;
//LOGSTRING

	if ((lastivalFrame >= impact_frames_min) && (impactBrightest)) {
		//TCHAR	buffer[MAX_STRING];
		//StringCchPrintf(buffer, sizeof(buffer) / sizeof(TCHAR), TEXT("Max lum %d at frame %ld, point (%ld, %ld).\n"), (int)impactBrightest->point->val, (int)impactBrightest->point->frame, (int)impactBrightest->point->x, (int)impactBrightest->point->y);
		//OutputDebugString(buffer);
		//fflush(stdout);
		*dtcMax = create_item(create_point(impactBrightest->point->frame, impactBrightest->point->val, impactBrightest->point->x, impactBrightest->point->y));
		nb_impact++;
		ok_or_ko_string = "OK";
		//delete_list(list);
	} else ok_or_ko_string = "ko";
	if (impactBrightest != nullptr) {
		bool_string = "true";
if (debug_impact_detection) (*pdetect_impact_log_messages).push_back("Impact (" + std::to_string(impactBrightest->point->x) + "," + std::to_string(impactBrightest->point->y) + ") detection: brightest candidate: " + bool_string + "=true ? ; " + std::to_string(lastivalFrame) + " >= ? " + std::to_string(impact_frames_min));
	} else bool_string = "false";
//(*pdetect_impact_log_messages).push_back("Impact (" + std::to_string(impactBrightest->point->x) + "," + std::to_string(impactBrightest->point->y) + ") detection: brightest candidate: " + bool_string + "=true ? ; " + std::to_string(lastivalFrame) + " >= ? " + std::to_string(impact_frames_min));
free(ord);
	ord = NULL;
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
		if (d <= radius && brightness_delta <= opts.impact_brightness_increase_min_factor && frame_difference <= frame_delta)
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
 * @fn	int detect(std::vector<std::string> file_list, std::string scan_folder_path)
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

int detect(std::vector<std::string> current_file_list, std::string scan_folder_path) {
	
// **************************************************************************
// ***************************** INITIALIZATION *****************************
// **************************************************************************
	clock_t				begin, begin_total, end;
	int					queue_scan_delay				= CLOCKS_PER_SEC * 2;	// interval waiting time for scanning new jobs (s)
	int					queue_scan_delay_random_max		= CLOCKS_PER_SEC;		// additionnal max random waiting time for scanning new jobs (s)
	int					wait_imagedisplay_seconds		= 3;					// display time for detection/mean image display (s). No limit if set to 0s (was 3s)
	int					check_children_time_factor		= 4;
	clock_t				check_threshold_time_inc		= wait_imagedisplay_seconds * check_children_time_factor * CLOCKS_PER_SEC;	// for interval for checking children results during parent capture processing
	clock_t				computing_refresh_duration		= CLOCKS_PER_SEC / 2;	//interval for refreshing computing time (s)
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

	std::wstring		detection_folder_fullpathname			= {};	// folder to store detection results
	std::wstring		detection_folder_name					= {};			// folder to store detection results
	std::wstring		details_folder_fullpathname				= {};		// subfolder to store details for detection results
	char				max_folder_path_filename[MAX_STRING]	= { 0 };
	char				diff_folder_path_filename[MAX_STRING]	= { 0 };
	char				single_folder_path_filename[MAX_STRING] = { 0 };
	char				tmpstring[MAX_STRING]					= { 0 };
	char				tmpstring2[MAX_STRING]					= { 0 };
	DIR					*dir_tmp;
	BOOL				GUI_display = TRUE;
	int					wait_count_total = 0;


	char buffer[MAX_STRING] = { 0 };
	sprintf_s(buffer, MAX_STRING, "detect1:				opts    : %p	opts->ignore	:	%i\n", &opts, opts.ignore);
	OutputDebugStringA(buffer);

	cv::setUseOptimized(true);
	maxinstances_previous = opts.maxinstances;

		//log directory when not in autostakkert mode and  not in multi instance mode
	if ((!opts.autostakkert) || (!AS_IMPACT_DETECTION_DIR_DETECT)) log_directory = scan_folder_path;
	else {
		//log directory when autostakkert mode or multi instance mode
		log_directory = CString2string(DeTeCt_exe_folder());
	}
	std::string log_consolidated_directory(log_directory);		// Location where consolidatedlog will be written
	std::string log(log_directory);						// Location where log will be written

	if (GetItemFromQueue(&log_cstring, _T("output_dir: "), (CString)opts.DeTeCtQueueFilename, NULL, TRUE)) {
		log = CString2string(log_cstring);
		log_consolidated_directory = CString2string(log_cstring.Left(log_cstring.ReverseFind(_T('\\'))));
	} else {								// otherwise parent instance (autostakkert mode, multiple instances mode or single instance mode)
		log.append("\\Impact_detection_run@").append(start_runtime);
		CString log_string(log.c_str());
		PushItemToQueue(log_string, _T("output_dir"), (CString)opts.DeTeCtQueueFilename, NULL, TRUE);
		opts.parent_instance = TRUE;
	}
	strcpy_s(opts.LogConsolidatedDirname, sizeof(opts.LogConsolidatedDirname), log_consolidated_directory.c_str());
	std::string detection_folder_name_string = log.substr(log.find_last_of("\\") + 1, log.length());
	detection_folder_name = std::wstring(detection_folder_name_string.begin(), detection_folder_name_string.end());

	detection_folder_fullpathname =	std::wstring(log.begin(), log.end());
	std::string detection_folder_fullpathname_string = wstring2string(detection_folder_fullpathname);
	strcpy_s(opts.impactdirname, sizeof(opts.impactdirname), detection_folder_fullpathname_string.c_str());
	strcpy_s(impact_detection_dirname, sizeof(impact_detection_dirname), detection_folder_fullpathname_string.c_str());

	// usage of mkdir only solution found to handle directory names with special characters (eg. �, �, ...)
	if (!(dir_tmp = opendir(detection_folder_fullpathname_string.c_str())))
		if (mkdir(detection_folder_fullpathname_string.c_str()) != 0) {
			char msgtext[MAX_STRING] = { 0 };
			snprintf(msgtext, MAX_STRING, "cannot create directory %s", detection_folder_fullpathname_string.c_str());
			ErrorExit(TRUE, TRUE, "cannot create directory", __func__, msgtext);
		}
	else closedir(dir_tmp);
	if (opts.detail || opts.allframes) {
		details_folder_fullpathname = std::wstring(detection_folder_fullpathname.begin(), detection_folder_fullpathname.end());
		details_folder_fullpathname = details_folder_fullpathname.append(L"\\details");
		std::string details_folder_fullpathname_string = wstring2string(details_folder_fullpathname);
		if (!(dir_tmp = opendir(details_folder_fullpathname_string.c_str()))) 
			if (mkdir(details_folder_fullpathname_string.c_str()) != 0) {
				char msgtext[MAX_STRING] = { 0 };
				snprintf(msgtext,MAX_STRING, "cannot create directory %s\n", details_folder_fullpathname_string.c_str());
				Warning(WARNING_MESSAGE_BOX, "cannot create directory", __func__, msgtext);
			}
		else closedir(dir_tmp);
	}

	std::string output_log_file_string(log.begin(), log.end());
	output_log_file_string = output_log_file_string.replace(log.find_last_of("\\"), log.length() - log.find_last_of("\\"), "");
	std::wstring output_log_file(output_log_file_string.begin(), output_log_file_string.end());
	output_log_file = output_log_file.append(L"\\").append(detection_folder_name).append(L"\\").append(OUTPUT_FILENAME).append(DTC_LOG_SUFFIX);
	std::wstring warnings_log_file(output_log_file_string.begin(), output_log_file_string.end());
	warnings_log_file = warnings_log_file.append(L"\\").append(detection_folder_name).append(L"\\").append(WARNINGS_FILENAME).append(DTC_LOG_SUFFIX);
	std::wstring errors_log_file(output_log_file_string.begin(), output_log_file_string.end());
	errors_log_file = errors_log_file.append(L"\\").append(detection_folder_name).append(L"\\").append(ERRORS_FILENAME).append(DTC_LOG_SUFFIX);


	strcpy_s(opts.WarningsFilename,	sizeof(opts.WarningsFilename), wstring2string(warnings_log_file).c_str());
	strcpy_s(opts.ErrorsFilename,	sizeof(opts.ErrorsFilename), wstring2string(errors_log_file).c_str());

	//DBOUT("DBOUT test " << "\n");	// works
	//fprintf(stderr, "stderr test\n"); // does not work
	//fprintf(stdout, "stdout test\n"); // does not work
	//Warning(WARNING_MESSAGE_BOX, "Warning test", __func__, "Warning display test"); // works

	dtcWriteLogHeader(log_consolidated_directory);
	dtcWriteLogHeader(log);

	message_init = L"DeTeCt v";
	message_init = message_init + _T(VERSION_NB);
	message_init = message_init + _T(" \n");
	instance_type = InstanceType(&instance_type_cstring);
	message_init = message_init + instance_type_cstring + _T(" instance");
	std::wofstream output_log_out(output_log_file.c_str(), std::ios_base::app);
	std::wifstream parameter_ini_in(DeTeCt_additional_filename_exe_fullpath(DTC_INI_SUFFIX));
	switch (instance_type) {
		case Instance_type::autostakkert_parent:
		case Instance_type::parent:
			message_init = message_init + L", DO NOT CLOSE unless told to do so";
			queue_scan_delay = FILEACCESS_WAIT_MS;								// waiting time (ms) for scanning new jobs - parent instance
			queue_scan_delay_random_max = FILEACCESS_WAIT_MS;
			GUI_display = FALSE;
			//saves detect.ini parameters in output.log
			output_log_out << "======================================================================================================\n  Parameters:\n";
			output_log_out << parameter_ini_in.rdbuf();
			output_log_out << "======================================================================================================\n\n";
			output_log_out.flush();
			break;
		case Instance_type::autostakkert_single:
			message_init = message_init + L", DO NOT CLOSE unless told to do so";
			queue_scan_delay = FILEACCESS_WAIT_MS;								// waiting time (ms) for scanning new jobs - parent instance
			queue_scan_delay_random_max = FILEACCESS_WAIT_MS;
			GUI_display = TRUE;
			break;
		case Instance_type::single:
			queue_scan_delay = FILEACCESS_WAIT_MS;								// waiting time (ms) for scanning new jobs - parent instance
			queue_scan_delay_random_max = FILEACCESS_WAIT_MS;
			GUI_display = TRUE;
			//saves detect.ini parameters in output.log
			output_log_out << "======================================================================================================\n  Parameters:\n";
			output_log_out << parameter_ini_in.rdbuf();
			output_log_out << "======================================================================================================\n\n";
			output_log_out.flush();
			break;
		case Instance_type::autostakkert_child:
			message_init = message_init + L", will CLOSE AUTOMATICALLY";
			//queue_scan_delay = CLOCKS_PER_SEC / 10;				// waiting time (ms) for scanning new jobs - child instance
			//queue_scan_delay_random_max = CLOCKS_PER_SEC / 2;
			queue_scan_delay = FILEACCESS_WAIT_MS;
			queue_scan_delay_random_max = FILEACCESS_WAIT_MS;
			GUI_display = TRUE;
			break;
		case Instance_type::child:
			message_init = message_init + L", will CLOSE AUTOMATICALLY";
			//queue_scan_delay = CLOCKS_PER_SEC / 20;				// waiting time (ms) for scanning new jobs - child instance
			//queue_scan_delay_random_max = CLOCKS_PER_SEC / 2;
			queue_scan_delay = FILEACCESS_WAIT_MS;
			queue_scan_delay_random_max = FILEACCESS_WAIT_MS;
			computing_refresh_duration = CLOCKS_PER_SEC * 60;	// hidden mode for child, do not need to display
			GUI_display = TRUE;
			break;
	}
	output_log_out.close();
	parameter_ini_in.close();
	message_init = message_init + _T("\n");
	LogString(message_init, output_log_file.c_str(), &log_counter, TRUE, &wait_count_total);
	
	
	if (opts.parent_instance) {
		nb_instances = 1; // Forks not launched yet, gain of computing time
		instance_type = DisplayInstanceType(&nb_instances);
	} else nb_instances = 0;
	
	std::vector<int> img_save_params = { CV_IMWRITE_JPEG_QUALITY, 100 };
	
	std::vector<LogInfo> logs;
	std::vector<LPCTSTR> logMessages;
	std::vector<std::string> log_messages;	// For SendMailDlg
	
	//log_messages.push_back("");
	if (opts.dateonly) log_messages.push_back("WARNING, datation info only, no detection analysis was performed");

	std::string logmessage;
	std::string short_logmessage;
	Rating_type rating = Rating_type::Error;
	std::string logmessage2;
	std::string logmessage3;
	std::wstring wlogmessage;
	CString Clogmessage;
	CString message_cstring;

	AcquisitionFilesList local_acquisition_files_list;
	
	local_acquisition_files_list.file_list = current_file_list;
	local_acquisition_files_list.acquisition_file_list = current_file_list;
	local_acquisition_files_list.nb_prealigned_frames = {};
	local_acquisition_files_list.acquisition_size = {};
	for (int i = 0; i < local_acquisition_files_list.file_list.size(); i++) {
		local_acquisition_files_list.nb_prealigned_frames.push_back(0);
		local_acquisition_files_list.acquisition_size.push_back(0);
	}
	size_t acquisition_index = 0;
	size_t acquisition_index_children = 0;
	size_t acquisitions_processed = 0;
	size_t acquisitions_to_be_processed = 0;
	
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
	//float progress_all_status = 0;
	
	clock_t computing_threshold_time = 0;
	clock_t begin_imagedisplay_time = 0;
	clock_t check_threshold_time = 0;

	clock_t display_update_duration = 0;
	clock_t processing_update_duration = 0;
	clock_t instances_update_duration = 0;
	clock_t start_update_time = 0;
	int		update_count = 0;

	if (filesys::exists(CString2string((CString)opts.DeTeCtQueueFilename))) SetIntParamToQueue(opts.maxinstances, _T("max_instances"), (CString)opts.DeTeCtQueueFilename);

	CDeTeCtMFCDlg::getAuto()->SetCheck(!opts.interactive);
	// Initializes the impacts classification in dialog window
	CDeTeCtMFCDlg::getimpactNull()->SetWindowText(std::to_wstring(nb_null_impact + nb_error_impact).c_str());
	CDeTeCtMFCDlg::getimpactLow()->SetWindowText(std::to_wstring(nb_low_impact).c_str());
	CDeTeCtMFCDlg::getimpactHigh()->SetWindowText(std::to_wstring(nb_high_impact).c_str());
	CDeTeCtMFCDlg::getduration()->SetWindowText((CString)"Duration processed (" + TotalType() + "): " + std::to_wstring((int)duration_total).c_str() + (CString)"s");
	CDeTeCtMFCDlg::getprobability()->SetWindowText((CString)("Probability ") + (CString)("(") + (CString)(TotalType()) + (CString)(") :"));

	std::wstring totalProgress_wstring;
	Datation_source datation_source;

if (opts.debug) LogString(_T("!Debug info: Setting processing file from queue"), output_log_file.c_str(), &log_counter, TRUE, &wait_count_total);
	if ((opts.parent_instance) && (strlen(opts.DeTeCtQueueFilename) > 0)) SetFileProcessingFromQueue((CString)local_acquisition_files_list.file_list.at(0).c_str(), (CString)opts.DeTeCtQueueFilename);
	if (opts.debug) LogString(_T("File in processing : ") + (CString)local_acquisition_files_list.file_list.at(0).c_str(), output_log_file.c_str(), &log_counter, GUI_display, &wait_count_total);
	
	CDeTeCtMFCDlg::getProgress_all()->SetRange(0, MAX_RANGE_PROGRESS);
	CDeTeCtMFCDlg::getProgress_all()->SetStep(1);
	CDeTeCtMFCDlg::getProgress_all()->SetPos(0);
	CDeTeCtMFCDlg::getProgress()->SetRange(0, MAX_RANGE_PROGRESS);
	CDeTeCtMFCDlg::getProgress()->SetStep(1);
	CDeTeCtMFCDlg::getProgress()->SetPos(0);

	DisplayProcessingTime(&computing_threshold_time, &begin_total, computing_refresh_duration, 0, 0);
	begin = begin_total;
	check_threshold_time = begin + check_threshold_time_inc;
	std::string filename;

// **************************************************************************
// ******************* Start of acquisition processing **********************
// **************************************************************************

	//if ((opts.parent_instance) && (strlen(opts.DeTeCtQueueFilename) > 0)) acquisitions_to_be_processed = NbFilesFromQueue((CString)opts.DeTeCtQueueFilename) - NbItemFromQueue(_T("file_ko"), (CString)opts.DeTeCtQueueFilename, NULL, TRUE);
	do
	{
		if ((opts.parent_instance) && (strlen(opts.DeTeCtQueueFilename) > 0))	acquisitions_to_be_processed = NbFilesFromQueue((CString)opts.DeTeCtQueueFilename) - NbItemFromQueue(_T("file_ko"), (CString)opts.DeTeCtQueueFilename, NULL, TRUE);
		else																	acquisitions_to_be_processed += (int) local_acquisition_files_list.file_list.size();
//		if ((!opts.parent_instance) || (strlen(opts.DeTeCtQueueFilename) == 0)) acquisitions_to_be_processed = NbFilesFromQueue((CString)opts.DeTeCtQueueFilename);
		while (acquisition_index < local_acquisition_files_list.file_list.size()) {
			std::string filename_acquisition;
			filename =				local_acquisition_files_list.file_list.at(acquisition_index);
			filename_acquisition =	local_acquisition_files_list.acquisition_file_list.at(acquisition_index);
			acquisition_index++;
			if (filesys::exists(CString2string((CString)opts.DeTeCtQueueFilename))) opts.maxinstances = GetIntParamFromQueue(_T("max_instances"), (CString)opts.DeTeCtQueueFilename);
			maxinstances_previous = opts.maxinstances;
			if ((opts.maxinstances > 1) && (!filesys::exists(CString2string((CString)opts.DeTeCtQueueFilename)))) AcquisitionFileListToQueue(&local_acquisition_files_list, _T("file_processing"), acquisition_index - 1, (CString)log.c_str(), &acquisitions_to_be_processed);
			float CPULoad = GetCPULoad(FALSE);
			nb_new_instances = ForksInstances(opts.maxinstances, ASorDeTeCtPID(opts.autostakkert_PID, opts.detect_PID), (CString)opts.DeTeCtQueueFilename, queue_scan_delay, queue_scan_delay_random_max, &nb_instances);
			if (nb_new_instances > 1)		LogString((CString)std::to_string(nb_new_instances).c_str() + _T(" new instances launched (") + (CString)std::to_string(nb_instances).c_str() + _T(" in total)") + _T(" (") + (CString)std::to_string((int)(100 - CPULoad * 100)).c_str() + _T("% CPU available)"), output_log_file.c_str(), &log_counter, TRUE, &wait_count_total);
			else if (nb_new_instances == 1) LogString((CString)std::to_string(nb_new_instances).c_str() + _T(" new instance launched (") + (CString)std::to_string(nb_instances).c_str() + _T(" in total)") + _T(" (") + (CString)std::to_string((int)(100 - CPULoad * 100)).c_str() + _T("% CPU available)"), output_log_file.c_str(), &log_counter, TRUE, &wait_count_total);

			/********** Init **********/

			std::vector<cv::Point> cm_list = {};
			int cm_list_start = 0;
			int cm_list_end = INT_MAX;
			int cm_frame_count = 0;

			int nframe = 0;
			int frame_error = 0;
			int frame_errors = 0;
			int frame_errors_not_readable = 0;
			int frame_errors_incorrect = 0;
			int frame_errors_too_dark = 0;
			int frame_errors_too_shifted = 0;
			int frame_errors_too_different = 0;
			int frame_duplicates = 0;
			int frame_number = 1;

			begin_imagedisplay_time = 0;
			CDeTeCtMFCDlg::getAS()->SetCheck(false);
			CDeTeCtMFCDlg::getdark()->SetCheck(false);
			CDeTeCtMFCDlg::getacquisitionLog()->SetCheck(false);
			CDeTeCtMFCDlg::getSER()->SetCheck(false);
			CDeTeCtMFCDlg::getSERtimestamps()->SetCheck(false);
			CDeTeCtMFCDlg::getFITS()->SetCheck(false);
			CDeTeCtMFCDlg::getFileInfo()->SetCheck(false);
			CDeTeCtMFCDlg::getacquisitionSW()->SetWindowText(L"");

//***** gets acquisition file and number of framesfrom autostakkert session file
			std::string extension = filename.substr(filename.find_last_of(".")+1, filename.size()-filename.find_last_of(".")-1);
			std::string filename_autostakkert = "";
			if (extension.compare(AUTOSTAKKERT_EXT) == 0) {

				filename_autostakkert = filename;
				CDeTeCtMFCDlg::getAS()->SetCheck(true);
				read_autostakkert_session_file(filename, &filename_acquisition, &cm_list, &cm_list_start, &cm_list_end, &cm_frame_count);
				local_acquisition_files_list.nb_prealigned_frames.at(acquisition_index - 1) = MIN(cm_list_end - cm_list_start + 1, cm_frame_count);
			}
			else {
				filename_autostakkert = "";
				CDeTeCtMFCDlg::getAS()->SetCheck(false);
				local_acquisition_files_list.nb_prealigned_frames.at(acquisition_index - 1) = cm_frame_count;
			}
			strcpy_s(opts.filename, strdup(filename_acquisition.c_str()));
			std::string outputFolder = filename_acquisition.substr(0, filename_acquisition.find_last_of("\\") + 1);
// BUG filename incomplet?			
//outputFolder = outputFolder.replace(0, log_directory.length() + 1, "");
			outputFolder = outputFolder.replace(0, log.find_last_of("\\") + 1, "");
			std::replace(outputFolder.begin(), outputFolder.end(), '\\', '_');
			std::replace(outputFolder.begin(), outputFolder.end(), ' ', '_');

			std::string filePath = filename_acquisition.substr(filename_acquisition.find_last_of("\\") + 1, filename_acquisition.find_last_of("."));
			filePath = filePath.substr(0, filePath.find_last_of("."));

			std::string folderPath = filename_acquisition.substr(0, filename_acquisition.find_last_of("\\") + 1);
			std::string outputfilename = folderPath.append(outputFolder).append(filePath).append(".jpg");
			strcpy_s(opts.ofilename, strdup(outputfilename.c_str()));
			std::wstring filename_wstring(filename_acquisition.begin(), filename_acquisition.end());
			std::string short_filename = filename_acquisition.substr(filename_acquisition.find_last_of("\\") + 1, filename_acquisition.length());
			std::wstring short_filename_wstring(short_filename.begin(), short_filename.end());

			std::string message = "----- ";
			if ((instance_type == Instance_type::single) || (instance_type == Instance_type::autostakkert_single)) message = message + std::to_string(acquisitions_processed + 1) + "/" + std::to_string(acquisitions_to_be_processed) + " : ";
			message = message + short_filename + " start -----";
			//totalProgress_wstring = L"Total\n(" + std::to_wstring(acquisitions_processed + acquisition_index_children) + L"/" + std::to_wstring(acquisitions_to_be_processed) + L")";
//if (opts.parent_instance) LogString(_T("1: parent / children / done / tobe = ") + (CString) (std::to_string(acquisitions_processed).c_str()) + (CString)(" / ") + (CString) (std::to_string(acquisition_index_children).c_str()) + (CString)(" / ") + (CString)(std::to_string(acquisitions_processed + acquisition_index_children).c_str()) + (CString)(" / ") + (CString)(std::to_string(acquisitions_to_be_processed).c_str()), output_log_file.c_str(), &log_counter, TRUE, &wait_count_total);

			//CDeTeCtMFCDlg::gettotalProgress()->SetWindowText(totalProgress_wstring.c_str());
			//CDeTeCtMFCDlg::getfileName()->SetWindowText(short_filename_wstring.c_str());
			UpdateProgress(acquisitions_to_be_processed, acquisitions_processed, acquisition_index_children, nframe, frame_number, opts.DeTeCtQueueFilename);
			message_cstring = (filename_wstring).c_str();
			CDeTeCtMFCDlg::getfileName()->SetWindowText(message_cstring);

			//TODO: usage of cmlist and quality information from as3

			LogString((CString)message.c_str(), output_log_file.c_str(), &log_counter, GUI_display, &wait_count_total);
				
			std::string detail_folder_path_string = wstring2string(details_folder_fullpathname);

			int fps_int		= 0;
			double fps_real = 0;
			int impact_frames_min;
			DtcCapture *pCapture;
			LIST ptlist =		{ 0,0,NULL,NULL }; // list of brightest differential points
			DTCIMPACT dtc;
			DTCIMPACT outdtc =	{ 0,0,0 };

			LIST candidates = { 0, 0, NULL, NULL };
			LIST impact = { 0, 0, NULL, NULL };

			cv::Mat pFrame; // Input frame
			cv::Mat pGryMat; // Grey frame-
			std::array<cv::Mat, max_similarity> PreviousFrameMat;	 // Last similarity valid frey frame
			cv::Mat pRefMat; // Reference frame (running accumulation of frames)
			cv::Mat pDifMat; // Differential photometry frame-
			cv::Mat pMskMat; // Mask frame-
			//cv::UMat pHisMat; // Histogram frame
			cv::Mat pThrMat; // Threshold frame								//Umat vs data
			cv::Mat pSmoMat; // Smooth frame								//Umat vs data
			cv::Mat pTrkMat; // Tracking frame-
			cv::Mat pOVdMat; // Output video frame
			
			///cv::UMat pADUavgMat; // ADU average frame
			cv::Mat pADUavgMat; // ADU average frame
			
			cv::UMat pADUmaxMat; // ADU max frame						//was cv::Mat
			cv::Mat pADUdtcMat; // ADU detect frame					//was cv::Mat
			cv::UMat pSmoADUdtcMat; // ADU detect frame (smoothed)		//was cv::Mat
			cv::Mat pADUavgDiffMat; // ADU average difference frame			//was cv::Mat- to be checked w/ next
			cv::Mat pADUavgMatFrame; // ADU average frame					//was cv::Mat- to be checked with previous
			cv::Mat pADUdarkMat; // ADU dark frame							//Umat vs imread
			cv::Mat pFirstFrameROIMat; // Region of interest, obtained from the first frame
			cv::Rect pFirstFrameROI; // Aforementioned region of interest as a delimited rectangle
			cv::Mat pROIMat; // Region of interest, obtained for the rest of the frames
			
			cv::Rect pROI; // Aforementioned region of interest as a delimited rectangle
			cv::Rect pFrameROI; // ROI of current frame
			
			//cv::Mat pAvgMat;
			cv::UMat pFlatADUmaxMat;	// For flat generation			//was cv::Mat
			cv::Mat pGryFullMat;	// Grey fullframe

			cv::Mat tempROIMat; // For matrices in which the ROI covers non-existing data
			cv::Mat tempGryMat; // For matrices in which the ROI covers non-existing data

			//cv::Mat previousGrayMat;
			std::queue<cv::Mat> ReferenceFrameQueue; // Queue to make a moving reference frame
						/* Images to be shown and/or saved */
			cv::UMat pGryImg;					//was cv::Mat
			cv::UMat pMskImg;					//was cv::Mat
			cv::UMat pDifImg;					//was cv::Mat
			cv::Mat pHisImg;
			cv::UMat pTrkImg;					//was cv::Mat
			cv::UMat pOVdImg;					//was cv::Mat
			cv::Mat pADUdtcImg;
	cv::Mat pADUmaxImg;
	cv::Mat pADUavgImg;
			cv::Mat pADUdtcImg2;				//was cv::YMat
			//cv::UMat pADUavgImg;				//was cv::Mat
			cv::UMat pADUdarkImg;				//was cv::Mat
			cv::UMat pFlatADUmaxImg;			//was cv::Mat

			int x_shift = 10;
			int y_shift = 10;

			//std::vector<double> xList;		// List of brightness increases //not used
			std::vector<double> maxList;		// List of brightest points brightness
//			double pDif_totalMean =0;			//not used

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
			char ofilenamediff[MAX_STRING]			= { 0 };
			char ofilenamemax[MAX_STRING]			= { 0 };
			char ofilenamesingle[MAX_STRING] = { 0 };

			char comment[MAX_STRING]				= { 0 };
			char rating_classification[MAX_STRING]	= { 0 };
			char rating_filename_suffix[MAX_STRING] = { 0 };
			char tmpchar[MAX_STRING]				= { 0 };
			double duration = 0;

			double start_time;
			double end_time;
			TIME_TYPE timetype;

			int nb_impact = -1;
				
			int darkfile_ok = 0;
			lum.val[0] = 0.0;
		

			std::vector<ITEM*>					candidateFrames;
			//std::vector<ITEM*>				brightestPoints;
			std::vector<cv::Mat>				frameList;
			std::vector<cv::Point>				cmShifts;
			std::vector<DiffImage>				diffImages;
			std::vector<unsigned int>			maxFrameNb;						// Y of brightness differential frame point
			std::vector<unsigned int>			maxPtX;						// X of brightness differential frame point
			std::vector<unsigned int>			maxPtY;						// Y of brightness differential frame point
			std::vector<double>					maxPtB;						// Brightness differential frame point
			std::vector<std::array<double, max_similarity>>	ReferenceFramePtSimilarity_decrease;			// Similarity decrease of frame
			std::vector<std::array<double, max_similarity>>	PreviousFramePtSimilarity_decrease;	// Similarity decrease of frame
			std::vector<std::array<double, max_similarity>>	ReferenceFrameSimilarity;						// Similarity of reference frames
			std::vector<std::array<double, max_similarity>>	LastValidFrameSimilarity;				// Similarity of frames
			//std::vector<unsigned long> frameErrors;
			std::vector<unsigned long>	frameNumbers;// Frame index of brightness differential frame point
			cv::Mat xMat;
			cv::Mat yMat;
			//cv::Mat bMat;
			cv::Mat impactFrame;
			cv::Mat pOrigGryMat;

			double firstFrameMean = 0;
			double currentFrameMean;

			int tempCols = 0;
			int tempRows = 0;

// ********************************************************************
// ****************** Start of capture processing *********************
// ********************************************************************

			try {
/*********************************INITIALIZATION******************************************/

				double video_duration = 0.0;
				begin = clock();
				std::vector<long> frames;
				bool is_ROI_too_small	= false;
				bool is_ROI_null		= false;
				bool is_ROI_too_dark	= false;
				bool is_ROI_ok			= true;
				bool is_image_correct	= true;

				//***** Opens acquisition file
				if (!(pCapture = dtcCaptureFromFile2(opts.filename, &nframe))) {
					LogString(L"ERROR: cannot open file " + (CString)opts.filename + L" correctly", output_log_file.c_str(), &log_counter, TRUE, &wait_count_total);
					//TO DO: CLOSE PROPERLY
					nframe = 0;
					//continue;
				}
				frame_number = nframe;
				CDeTeCtMFCDlg::getProgress()->SetPos(0);  // sets progress bar configuration

				// ***** Gets datation info from acquisition
				std::wstringstream pipp_message;
				PIPPInfo pipp_info;

				Is_PIPP_OK(opts.filename, &pipp_info, &pipp_message);
				dtcGetDatation(pCapture, opts.filename, nframe, &start_time, &end_time, &duration, &fps_real, &timetype, &pipp_info, comment, &planet, &datation_source);
				if (datation_source.acquisition_log_file) {
					CDeTeCtMFCDlg::getacquisitionLog()->SetCheck(true);
					CString datation_source_cstring(datation_source.acquisition_software);
					CDeTeCtMFCDlg::getacquisitionSW()->SetWindowText(datation_source_cstring);
				}
				if (datation_source.ser_file)				CDeTeCtMFCDlg::getSER()->SetCheck(true);
				if (datation_source.ser_file_timestamp)		CDeTeCtMFCDlg::getSERtimestamps()->SetCheck(true);
				if (datation_source.fits_file)				CDeTeCtMFCDlg::getFITS()->SetCheck(true);
				if (datation_source.file_info)				CDeTeCtMFCDlg::getFileInfo()->SetCheck(true);

				if (planet == Jupiter) planet_jupiter++;
				else if (planet == Saturn) planet_saturn++;
				else if (pCapture != NULL) {
					if (InStr(lcase(pCapture->CaptureInfo.profile, tmpchar), "jupiter") >= 0)  planet_jupiter++;
					else if (InStr(lcase(pCapture->CaptureInfo.profile, tmpchar), "saturn") >= 0)  planet_saturn++;
				}

				if (start_time > JD_min) { 	/* for renaming logfile in impact_detection directory */
					if (start_time < start_time_min) start_time_min = start_time;
					if (start_time > start_time_max) start_time_max = start_time;
				}

				duration_total += duration;

				double fps = fps_real;
				if (fps < 0.02)	fps = dtcGetCaptureProperty(pCapture, cv::CAP_PROP_FPS);   // test OpenCV 4.7.0 
				fps_int = (int)fps;
				//impact_frames_min = (int)ceil(MAX(opts.incrFrameImpact, fps * opts.impact_duration_min));    // moved down just before usage to use nbframe if fps is invalid
				/*********************************DATE ONLY MODE******************************************/
				if (opts.dateonly) {
						if (nframe != 0) LogString(L"Datation for capture of " + (CString)std::to_string(nframe).c_str() + L" frames @ " + (CString)std::to_string(fps_int).c_str() + L" fps", output_log_file.c_str(), &log_counter, TRUE, &wait_count_total);
					message = "-------------- " + short_filename + " end --------------";
					LogString(+(CString)message.c_str(), output_log_file.c_str(), &log_counter, TRUE, &wait_count_total);
					double fake_stat[3] = { 0.0, 0.0, 0.0 };
					LogInfo info(opts.filename, start_time, end_time, duration, fps_real, timetype, comment, 0, 0, 0, fake_stat, fake_stat, fake_stat, fake_stat, fake_stat, fake_stat, 0.0, rating_classification, croi.width, croi.height);

					std::stringstream logline;
					DtcCaptureInfo CaptureInfo = {};
					if (pCapture != NULL) CaptureInfo = pCapture->CaptureInfo;
					else {
						info.confidence = -1.0;
						info.nb_impact = -1;
						sprintf(info.rating_classification, "Error        ");
					}
					//dtcWriteLog2(log_consolidated_directory, info, (pCapture->CaptureInfo), &logline, &wait_count_total);
					dtcWriteLog2(log_consolidated_directory, info, CaptureInfo, &logline, &wait_count_total);
					if (nframe > 0) log_messages.push_back(logline.str() + "\n");
					dtcWriteLog2(log, info, (CaptureInfo), &logline, &wait_count_total);
					dtcReleaseCapture(pCapture);
				//	continue;
				}
				else if (nframe > 0) {
					//****************** NON DATE ONLY MODE *******************************/
					message = std::to_string(nframe) + " frames @ " + std::to_string(fps_int) + " fps (" + std::to_string((int)duration) + "s duration)";
					message_cstring = message_cstring + (CString)"\n" + (CString)message.c_str() + (CString)"\n";
					CDeTeCtMFCDlg::getfileName()->SetWindowText(message_cstring);
										LogString(+(CString)message.c_str(), output_log_file.c_str(), &log_counter, GUI_display, &wait_count_total);
					
					//
					// Gets ROI, check if ROI is not big enough and exit then
					//
					if (opts.wROI && opts.hROI) {
						croi = cv::Rect(0, 0, opts.wROI, opts.hROI);
					}
					else {
						croi = dtcGetFileROIcCM(pCapture, opts.ignore);
						dtcReinitCaptureRead2(&pCapture, opts.filename);
						if ((croi.width <= opts.ROI_min_size) || (croi.height <= opts.ROI_min_size)) {
							message = "-------------- " + short_filename + " end --------------";
							LogString(+(CString)message.c_str(), output_log_file.c_str(), &log_counter, GUI_display, &wait_count_total);

							if ((croi.width <= 0) || (croi.height <= 0)) {
								LogString(L"ERROR: ROI cannot be obtained, ignoring acquisition and stopping processing", output_log_file.c_str(), &log_counter, GUI_display, &wait_count_total);
								is_ROI_null = true;
								is_image_correct = false;
							}
							else {
							LogString(L"WARNING: ROI " +
								(CString)std::to_string(croi.width).c_str() + L"x" + (CString)std::to_string(croi.height).c_str() + L" too small "
								+ L"(" + (CString)std::to_string(croi.width).c_str() + L"x" + (CString)std::to_string(croi.height).c_str()
								+ L") < " + L"(" + (CString)std::to_string(opts.ROI_min_size).c_str() + L"x" + (CString)std::to_string(opts.ROI_min_size).c_str()
								+ L", ignoring acquisition and stopping processing", output_log_file.c_str(), &log_counter, GUI_display, &wait_count_total);
								is_ROI_too_small = true;
								is_image_correct = false;
							}
							is_ROI_ok = !(is_ROI_null || is_ROI_too_small);
							//message_cstring = (CString)"\n" + (CString)message.c_str() + (CString)"\n";
							//CDeTeCtMFCDlg::getfileName()->SetWindowText(message_cstring);
						//RemoveFileFromQueue((CString)filename_acquisition.c_str(), (CString)opts.DeTeCtQueueFilename, NULL, TRUE);
						//dtcReleaseCapture(pCapture);
						//pCapture = NULL;
						//acquisitions_to_be_processed--;
							//totalProgress_wstring = L"Total\n(" + std::to_wstring(acquisitions_processed + acquisition_index_children) + L"/" + std::to_wstring(acquisitions_to_be_processed) + L")";
							//CDeTeCtMFCDlg::gettotalProgress()->SetWindowText(totalProgress_wstring.c_str());
							//CDeTeCtMFCDlg::getProgress_all()->SetPos((short)(MAX_RANGE_PROGRESS * (float)(acquisitions_processed + 1 + acquisition_index_children) / (acquisitions_to_be_processed)));
							//CDeTeCtMFCDlg::getProgress_all()->UpdateWindow();
							UpdateProgress(acquisitions_to_be_processed, acquisitions_processed, acquisition_index_children, nframe, frame_number, opts.DeTeCtQueueFilename);
							//continue;
						}
					}

					if (is_ROI_ok) {
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
						//
						//Process dark file if existing, but not for Winjupos derotated files and PIPP files as a regular dark file would not be suitable if the images have been modified
						//
						if ((strlen(opts.darkfilename) > 0) && (InStr(opts.filename, WJ_DEROT_STRING) < 0) && (InStr(opts.filename, PIPP_STRING) < 0)) {
							char darklongfilename[MAX_STRING] = { 0 };
							strncpy_s(darklongfilename, sizeof(darklongfilename), opts.filename, InRstr(opts.filename, "\\") + 1);
							strcat_s(darklongfilename, sizeof(darklongfilename), opts.darkfilename);
							if (!(pADUdarkMat = cv::imread(darklongfilename, CV_LOAD_IMAGE_GRAYSCALE)).data) {
								darkfile_ok = 0;
								CDeTeCtMFCDlg::getdark()->SetCheck(false);
							}
							else {
								LogString(+L"Reading dark frame " + (CString)std::string(darklongfilename).c_str(), output_log_file.c_str(), &log_counter, GUI_display, &wait_count_total);
								darkfile_ok = 1;
								CDeTeCtMFCDlg::getdark()->SetCheck(true);
							}
						}
						char buffer2[MAX_STRING] = { 0 };
						sprintf_s(buffer2, MAX_STRING, "detect2:				opts    : %p	opts->ignore	:	%i\n", &opts, opts.ignore);
						OutputDebugStringA(buffer2);

// *******************************************************************************************************
// ************************************ Start of frames processing ***************************************
// *******************************************************************************************************
											//while ((pFrame = dtcQueryFrame2(pCapture, opts.ignore, &frame_error)).data && (pFrame.dims > 0)) {
						std::array<double, max_similarity> similarity_reference					= { 0.0 };	// [SSIM, ME, NCC]
						std::array<double, max_similarity> similarity_reference_last_valid		= { 0.0 };	// [SSIM, ME, NCC]
						std::array<double, max_similarity> delta_similarity_reference			= { 0.0 };	// [SSIM, ME, NCC]
						std::array<double, max_similarity> similarity_previous_frame			= { 0.0 };	// [SSIM, ME, NCC]
						std::array<double, max_similarity> similarity_previous_frame_last_valid	= { 0.0 };	// [SSIM, ME, NCC]
						std::array<double, max_similarity> delta_similarity_previous_frame		= { 0.0 };	// [SSIM, ME, NCC]

						while (!(pFrame = dtcQueryFrame2(pCapture, opts.ignore, &frame_error)).empty()) {
							cv::medianBlur(pFrame, pFrame, 3);
							video_duration += (int)dtcGetCaptureProperty(pCapture, cv::CAP_PROP_POS_MSEC);   // test OpenCV 4.7.0 
							nframe++;
							if ((frame_error) != 0) {
								frame_errors += 1;
								frame_errors_not_readable++;
								LogString(L"Ignoring not readable frame #" + (CString)std::to_string(nframe).c_str(), output_log_file.c_str(), &log_counter, FALSE, &wait_count_total);
							}
							else {
								init_string(ofilenamemax);
								init_string(ofilenamediff);
								init_string(ofilenamesingle);
								init_string(max_folder_path_filename);
								init_string(diff_folder_path_filename);
								init_string(single_folder_path_filename);
								strcpy_s(max_folder_path_filename, sizeof(max_folder_path_filename), detail_folder_path_string.c_str());
								strcpy_s(diff_folder_path_filename, sizeof(diff_folder_path_filename), detail_folder_path_string.c_str());
								strcpy_s(single_folder_path_filename, sizeof(single_folder_path_filename), detail_folder_path_string.c_str());
								cv::Point cm;
								cv::Rect roi;
								DiffImage diffImage;
								pGryMat = dtcGetGrayMat(&pFrame, pCapture);
								//deactivated as background different for each frame
								//int background = dtcGetBackgroundFromHistogram(pGryMat, opts.bg_detection_peak_factor, opts.bg_detection_consecutive_values, 0);
								//cv::threshold(pGryMat, pGryMat, background, 0, CV_THRESH_TOZERO); //deactivated as background ddifferent for each frame
								//LogString(L"Background = " + (CString)std::to_string(background).c_str(), output_log_file.c_str(), &log_counter, GUI_display, &wait_count_total);
								if (opts.flat_preparation) pGryFullMat = dtcGetGrayMat(&pFrame, pCapture);
								//dtcApplyMaskToFrame(pGryMat);
								//cv::GaussianBlur(pGryMat, pGryMat, cv::Size(1, 1), 1);
								if (darkfile_ok == 1) {
									if ((pADUdarkMat.rows != pGryMat.rows) || (pADUdarkMat.cols != pGryMat.cols)) {
										LogString(+L"Warning: dark frame " +
											(CString)std::string(opts.darkfilename).c_str() + L" differs from the frame properties " +
											(CString)std::to_string(pADUdarkMat.rows).c_str() + L" vs " +
											(CString)std::to_string(pGryMat.rows).c_str() + L" rows, " +
											(CString)std::to_string(pADUdarkMat.cols).c_str() + L" vs " +
											(CString)std::to_string(pGryMat.cols).c_str() + L" cols", output_log_file.c_str(), &log_counter, GUI_display, &wait_count_total);
										darkfile_ok = 0;
										CDeTeCtMFCDlg::getdark()->SetCheck(false);
									}
									else {
										cv::Mat pGryDarkMat;
										pGryDarkMat = cv::Mat(pGryMat.size(), pGryMat.type());
										cv::subtract(pGryMat, pADUdarkMat, pGryDarkMat);
										cv::threshold(pGryDarkMat, pGryMat, 0, 0, CV_THRESH_TOZERO);
										pGryDarkMat.~Mat();
										CDeTeCtMFCDlg::getdark()->SetCheck(true);
									}
								}

/*******************FIRST FRAME PROCESSING*******************/
								if (nframe == 1) {
									pGryMat.copyTo(pFirstFrameROIMat);
									pGryMat.convertTo(pGryMat, CV_8U);
									if (opts.flat_preparation) pGryFullMat.convertTo(pGryFullMat, CV_8U);

									pFirstFrameROIMat = dtcApplyMask(pFirstFrameROIMat);
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
									if (!opts.wait && (opts.viewROI || opts.viewTrk || opts.viewDif || opts.viewRef ||
										opts.viewThr || opts.viewSmo || opts.viewRes || opts.viewHis)) {
										if (fps_int > 0) {
											opts.wait = (int)(1000 / std::ceil(fps_int));
										}
										else {
											opts.wait = (int)(1000 / 25);
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
									pADUmaxMat = cv::UMat::zeros(pFirstFrameROIMat.size(), CV_32F);

									if (opts.flat_preparation) pFlatADUmaxMat = cv::UMat::zeros(pGryFullMat.size(), CV_32F);
									if ((strlen(opts.ofilename) > 0) && (opts.allframes)) {
										pADUdtcMat = cv::Mat(pFirstFrameROIMat.size(), CV_32F);
										pADUavgMatFrame = cv::Mat(pFirstFrameROIMat.size(), CV_32F);
									}
									if (opts.thrWithMask || opts.viewMsk || ((strlen(opts.ovfname) > 0) && (opts.ovtype == OTYPE_MSK))) pMskMat = cv::Mat(pFirstFrameROIMat.size(), CV_32F);
									if (opts.viewThr)																					pThrMat = cv::Mat(pFirstFrameROIMat.size(), CV_32F);
									if (opts.filter.type >= 0 || opts.viewSmo) 															pSmoMat = cv::Mat(pFirstFrameROIMat.size(), CV_32F);
									if (opts.viewTrk || ((opts.ovtype == OTYPE_TRK) && (strlen(opts.ovfname) > 0)))						pTrkMat = cv::Mat(pFrame.size(), CV_32F);
									//pAvgMat = cv::Mat(pFirstFrameROIMat.size(), CV_64F);

									firstFrameMean = cv::mean(pFirstFrameROIMat)[0];
									pFirstFrameROIMat.convertTo(pRefMat, CV_32F);
									cv::Rect bigROI = pFirstFrameROI + cv::Size(x_shift, y_shift);
									pROIMat = cv::Mat::zeros(bigROI.size(), pFirstFrameROIMat.type());
									tempGryMat = cv::Mat::zeros(pFirstFrameROI.size(), pFirstFrameROIMat.type());
								}
// ******************************************************************************************************
// ************************************ Start of frame processing ***************************************
// ******************************************************************************************************

								pGryMat.convertTo(pGryMat, CV_8U);
								if (opts.flat_preparation) pGryFullMat.convertTo(pGryFullMat, CV_8U);

								cv::Mat maskedGryMat = dtcApplyMask(pGryMat.clone());
								//AS3
								if (((cm_list.size() + cm_list_start) >= nframe) && (nframe > cm_list_start))
									cm = cm_list[nframe - cm_list_start - 1];
								else cm = dtcGetGrayMatCM(maskedGryMat);

								currentFrameMean = cv::mean(pGryMat)[0];

								//if ((cm.x <= 0) || (cm.y <= 0) || ((firstFrameMean / 10) > currentFrameMean)) {		
								//if ((cm.x <= 0) || (cm.y <= 0) || (currentFrameMean <= (0.2 * firstFrameMean))) {
								//if ((cm.x <= 0) || (cm.y <= 0) || (currentFrameMean == 0.0)) {
								//if ((cm.x <= 0) || (cm.y <= 0) || (currentFrameMean <= (0.1 * firstFrameMean))) {

								//Modification v3.2.1 comparison first ROI with full frame: applying size ratio and 80% tolerance in transparency
								BOOL CurrentMeanBrightness_ok = (currentFrameMean > ((opts.transparency_min_pc / 100.0) * firstFrameMean * pFirstFrameROIMat.rows * pFirstFrameROIMat.cols / (pGryMat.rows * pGryMat.cols)));
								if ((cm.x < 0) || (cm.y < 0) || (!CurrentMeanBrightness_ok)) {
									frame_errors++;
									if (CurrentMeanBrightness_ok) {
										LogString(L"Ignoring incorrect ROI frame #" + (CString)std::to_string(nframe).c_str(), output_log_file.c_str(), &log_counter, FALSE, &wait_count_total);
										frame_errors_incorrect++;
									}
									else {
										LogString(L"Ignoring too dark ROI frame #" + (CString)std::to_string(nframe).c_str(), output_log_file.c_str(), &log_counter, FALSE, &wait_count_total);
										frame_errors_too_dark++;
									}
								}
								else {
									pFrameROI = dtcGetGrayImageROIcCM(maskedGryMat, cm, (float)opts.medSize, opts.facSize, opts.secSize);

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
										LogString(L"Ignoring too shifted ROI frame #" + (CString)std::to_string(nframe).c_str(), output_log_file.c_str(), &log_counter, FALSE, &wait_count_total);
										frame_errors++;
										frame_errors_too_shifted++;
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
										tempCols = pROI.br().x > pGryMat.cols ? 0 : pROIMat.cols - tempROIMat.cols;
										tempRows = pROI.br().y > pGryMat.rows ? 0 : pROIMat.rows - tempROIMat.rows;

										tempROIMat.copyTo(pROIMat(cv::Rect(tempCols, tempRows, tempROIMat.cols, tempROIMat.rows)));
										if (pGryMat.type() != CV_32F) pGryMat.convertTo(pGryMat, CV_32F);

										if ((opts.flat_preparation) && (pGryFullMat.type() != CV_32F)) pGryFullMat.convertTo(pGryFullMat, CV_32F);
										if (pFirstFrameROIMat.type() != CV_32F)	pFirstFrameROIMat.convertTo(pFirstFrameROIMat, CV_32F);
										if (pROIMat.type() != CV_32F) pROIMat.convertTo(pROIMat, CV_32F);

										//x and y are top left (tl) coordinates of the ROI
										roi = dtcCorrelateROI(pROIMat, pFirstFrameROIMat, pROI.tl(), pFirstFrameROI.size());
										pROI = roi;
										if (pROI.x < 0) pROI.x = 0;
										if (pROI.y < 0) pROI.y = 0;
										//if ((roi.x + roi.width > pFirstFrameROI.cols) )
										tempGryMat.setTo(cv::Scalar::all(0));
										if (pROI.width + pROI.x > pGryMat.cols) pROI.width = pGryMat.cols - pROI.x;
										if (pROI.height + pROI.y > pGryMat.rows) pROI.height = pGryMat.rows - pROI.y;
										pGryMat = dtcReduceMatToROI(pGryMat, pROI);
										tempCols = pROI.br().x > pGryMat.cols ? 0 : pROIMat.cols - tempROIMat.cols;
										tempRows = pROI.br().y > pGryMat.rows ? 0 : pROIMat.rows - tempROIMat.rows;

										/*** Following added to avoid writing outside of matrix size - algorithm correctness to be checked ? ***/
										tempCols = MIN(tempCols, tempGryMat.cols - pGryMat.cols);
										tempRows = MIN(tempRows, tempGryMat.rows - pGryMat.rows);
										pGryMat.copyTo(tempGryMat(cv::Rect(tempCols, tempRows, pGryMat.cols, pGryMat.rows)));
										tempGryMat.copyTo(pGryMat);

							//
							// checks similarity of the current frame with reference frame, or last valid frame
							//
										
										// prevoir pour chaque m�thode diff�rents is_frame_valid, frame_errors, frame_duplicates, frame_errors_too_different et comment les g�rer
										bool	is_frame_valid = true;
										if (opts.ignore) {
											std::array<bool, max_similarity>	is_frame_errors					= { false };
											std::array<bool, max_similarity>	is_frame_duplicate				= { false };
											std::array<bool, max_similarity>	is_frame_errors_too_different	= { false };

											bool is_frame_errors_tmp				= false;
											bool is_frame_duplicate_tmp				= false;
											bool is_frame_errors_too_different_tmp	= false;
											bool is_frame_valid_tmp					= true;
											Comparison_type comparison_frame_type	= Comparison_type::reference;
											Similarity_type similarity_method		= MSE;
											
											if (opts.use_all_algo_for_test) for (int sim = 0; sim < max_similarity; sim++) 	for (int comp = 0; comp < (int)Comparison_type::max_comparison_type; comp++) {
												is_frame_valid_tmp = Is_frame_similarity_valid(static_cast<Similarity_type>(sim), static_cast<Comparison_type>(comp), nframe, frame_errors, &is_frame_errors_tmp, &is_frame_duplicate_tmp, &is_frame_errors_too_different_tmp,
													&similarity_reference, &similarity_reference_last_valid, &delta_similarity_reference, pFirstFrameROIMat,
													&similarity_previous_frame, &similarity_previous_frame_last_valid, &delta_similarity_previous_frame, &PreviousFrameMat, pGryMat);
											}
											else {
												comparison_frame_type = Comparison_type::reference;
												similarity_method = MSE;

												if (opts.use_reference_similarity[similarity_method]) {
													comparison_frame_type = Comparison_type::reference;
													is_frame_valid_tmp = Is_frame_similarity_valid(similarity_method, comparison_frame_type, nframe, frame_errors, &is_frame_errors_tmp, &is_frame_duplicate_tmp, &is_frame_errors_too_different_tmp,
														&similarity_reference, &similarity_reference_last_valid, &delta_similarity_reference,
														pFirstFrameROIMat, &similarity_previous_frame, &similarity_previous_frame_last_valid, &delta_similarity_previous_frame, &PreviousFrameMat, pGryMat);
												}
												if (!(!is_frame_valid_tmp && opts.use_one_algo_to_reject_frame)) {  // not the case that one invalid frame is enough

													if (opts.use_previous_frame_similarity[similarity_method]) {
														comparison_frame_type = Comparison_type::previous_frame;
														is_frame_valid_tmp = Is_frame_similarity_valid(similarity_method, comparison_frame_type, nframe, frame_errors, &is_frame_errors_tmp, &is_frame_duplicate_tmp, &is_frame_errors_too_different_tmp,
															&similarity_reference, &similarity_reference_last_valid, &delta_similarity_reference, pFirstFrameROIMat,
															&similarity_previous_frame, &similarity_previous_frame_last_valid, &delta_similarity_previous_frame, &PreviousFrameMat, pGryMat);
													}
													if (!(!is_frame_valid_tmp && opts.use_one_algo_to_reject_frame)) { // not the case that one invalid frame is enough

														similarity_method = NCC;
														if (opts.use_reference_similarity[similarity_method]) {
															comparison_frame_type = Comparison_type::reference;
															is_frame_valid_tmp = Is_frame_similarity_valid(similarity_method, comparison_frame_type, nframe, frame_errors, &is_frame_errors_tmp, &is_frame_duplicate_tmp, &is_frame_errors_too_different_tmp,
																&similarity_reference, &similarity_reference_last_valid, &delta_similarity_reference, pFirstFrameROIMat,
																&similarity_previous_frame, &similarity_previous_frame_last_valid, &delta_similarity_previous_frame, &PreviousFrameMat, pGryMat);
														}
														if (!(!is_frame_valid_tmp && opts.use_one_algo_to_reject_frame)) { // not the case that one invalid frame is enough

															if (opts.use_previous_frame_similarity[similarity_method]) {
																comparison_frame_type = Comparison_type::previous_frame;
																is_frame_valid_tmp = Is_frame_similarity_valid(similarity_method, comparison_frame_type, nframe, frame_errors, &is_frame_errors_tmp, &is_frame_duplicate_tmp, &is_frame_errors_too_different_tmp,
																	&similarity_reference, &similarity_reference_last_valid, &delta_similarity_reference, pFirstFrameROIMat,
																	&similarity_previous_frame, &similarity_previous_frame_last_valid, &delta_similarity_previous_frame, &PreviousFrameMat, pGryMat);
															}
															if (!(!is_frame_valid_tmp && opts.use_one_algo_to_reject_frame)) { // not the case that one invalid frame is enough

																similarity_method = SSIM;
																if (opts.use_reference_similarity[similarity_method]) {
																	comparison_frame_type = Comparison_type::reference;
																	is_frame_valid_tmp = Is_frame_similarity_valid(similarity_method, comparison_frame_type, nframe, frame_errors, &is_frame_errors_tmp, &is_frame_duplicate_tmp, &is_frame_errors_too_different_tmp,
																		&similarity_reference, &similarity_reference_last_valid, &delta_similarity_reference, pFirstFrameROIMat,
																		&similarity_previous_frame, &similarity_previous_frame_last_valid, &delta_similarity_previous_frame, &PreviousFrameMat, pGryMat);
																}
																if (!(!is_frame_valid_tmp && opts.use_one_algo_to_reject_frame)) { // not the case that one invalid frame is enough

																	if (opts.use_previous_frame_similarity[SSIM]) {
																		comparison_frame_type = Comparison_type::previous_frame;
																		is_frame_valid_tmp = Is_frame_similarity_valid(SSIM, comparison_frame_type, nframe, frame_errors, &is_frame_errors_tmp, &is_frame_duplicate_tmp, &is_frame_errors_too_different_tmp,
																			&similarity_reference, &similarity_reference_last_valid, &delta_similarity_reference, pFirstFrameROIMat,
																			&similarity_previous_frame, &similarity_previous_frame_last_valid, &delta_similarity_previous_frame, &PreviousFrameMat, pGryMat);
																	}
																}
															}
														}
													}
												}
												is_frame_valid = is_frame_valid_tmp;
												if (is_frame_errors_tmp)				frame_errors++;
												if (is_frame_duplicate_tmp)				frame_duplicates++;
												if (is_frame_errors_too_different_tmp)	frame_errors_too_different++;
											}

											// Logs if frame rejected
											if (is_frame_duplicate_tmp) {
												if (comparison_frame_type == Comparison_type::reference)			LogString(L"Duplicate of frame #" + (CString)std::to_string(nframe).c_str() + L" (reference similarity(" + (CString)std::to_string(similarity_method).c_str() + "): " + (CString)std::to_string((similarity_reference)[similarity_method]).c_str() + L")", output_log_file.c_str(), &log_counter, FALSE, &wait_count_total);
												else if (comparison_frame_type == Comparison_type::previous_frame)	LogString(L"Duplicate of frame #" + (CString)std::to_string(nframe).c_str() + L" (previous frame similarity(" + (CString)std::to_string(similarity_method).c_str() + "): " + (CString)std::to_string((similarity_previous_frame)[similarity_method]).c_str() + L")", output_log_file.c_str(), &log_counter, FALSE, &wait_count_total);
											}
											if (is_frame_errors_too_different_tmp) {
												if (comparison_frame_type == Comparison_type::reference)			LogString(L"Ignoring different frame #" + (CString)std::to_string(nframe).c_str() + L" (reference similarity(" + (CString)std::to_string(similarity_method).c_str() + "): " + (CString)std::to_string((similarity_reference)[similarity_method]).c_str() + L" vs " + (CString)std::to_string((similarity_reference_last_valid)[similarity_method]).c_str() + L" (" + (CString)std::to_string((similarity_reference)[similarity_method] - (similarity_reference_last_valid)[similarity_method]).c_str() + L")", output_log_file.c_str(), &log_counter, FALSE, &wait_count_total);
												else if (comparison_frame_type == Comparison_type::previous_frame)	LogString(L"Ignoring different frame #" + (CString)std::to_string(nframe).c_str() + L" (previous frame similarity(" + (CString)std::to_string(similarity_method).c_str() + "): " + (CString)std::to_string((similarity_previous_frame)[similarity_method]).c_str() + L" vs " + (CString)std::to_string((similarity_previous_frame_last_valid)[similarity_method]).c_str() + L" (" + (CString)std::to_string((similarity_previous_frame)[similarity_method] - (similarity_previous_frame_last_valid)[similarity_method]).c_str() + L")", output_log_file.c_str(), &log_counter, FALSE, &wait_count_total);
											}
										}
										if ((is_frame_valid) || (!opts.ignore)) {
			/* Normalise image */
											pGryMat *= (firstFrameMean / cv::mean(pGryMat)[0]);
											pGryMat.convertTo(pGryMat, CV_32F);
											if (opts.flat_preparation) pGryFullMat.convertTo(pGryFullMat, CV_32F);
											ReferenceFrameQueue.push(pGryMat);

											pDifMat = pGryMat - pRefMat;

											cv::Mat ifDif; // intelligent median
											cv::medianBlur(pDifMat, ifDif, 1);

											double maxDifVal = 0;

											cv::minMaxLoc(pDifMat, NULL, &maxDifVal, NULL, NULL);

											cv::Mat ifMask = ifDif - pDifMat > 5;
											cv::Mat pDifMat2 = pDifMat.clone();
											ifMask.~Mat();
											ifDif.~Mat();
											pDifMat2.~Mat();

											if (!pDifMat.empty()) { // if relevant display and / or save differential frame
												if (opts.viewDif) Show_matrix(pDifMat, "Initial differential photometry", true, 1);
												if (nframe == opts.nsaveframe && opts.ofilename && opts.ostype == OTYPE_DIF) {
													cv::imwrite(opts.ofilename, pDifMat, img_save_params);
												}
											}
											if (opts.filter.type > 0) { // if relevant applies filter to differential frame
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
											// if relevant displays smooth frame
											if (opts.viewSmo && !pSmoMat.empty()) Show_matrix(pSmoMat, "Smoothed differential photometry", true, 1);
											
											if (!pMskMat.empty()) { // if relevant applies and displays mask
												cv::threshold(pDifMat, pMskMat, 0.0, 255.0, CV_THRESH_BINARY_INV);
												if (opts.viewMsk) Show_matrix(pMskMat, "Mask", true, 1);
											}

							//
							// IMPACT DETECTION algorithm - prepare dtc images
							//	
							//  prepare dtc images
							//
																											//cv::blur(pDifMat, pDifMat, cv::Size(3,3));
																					//cv::threshold(pDifMat, pDifMat, popts->threshold, 0.0, CV_THRESH_TOZERO);

																					/* not used
																					double pDif_mean = cv::mean(pDifMat)[0];
																					pDif_totalMean += pDif_mean;
																					double x = std::abs(double(maxLum) / double(pDif_mean)) - 1;
																					if (x >= 0.7) {
																						xList.push_back(x); // pushes luminosity increase if > 170% of mean
																					}*/

																					/*ADUdtc algorithm******************************************/
											cv::add(pADUavgMat, pGryMat, pADUavgMat);							// mean image preparation
											cv::add(pADUavgDiffMat, pDifMat, pADUavgDiffMat);
											cv::max(pADUmaxMat, pGryMat, pADUmaxMat);							// detection image preparation
											if (opts.flat_preparation) cv::max(pFlatADUmaxMat, pGryFullMat, pFlatADUmaxMat);
											if ((strlen(opts.ofilename) > 0) && opts.allframes) {
												pADUavgMat.convertTo(pADUavgMatFrame, -1, 1.0 / (nframe - frame_errors), 0);
												pADUavgDiffMat.convertTo(pADUavgDiffMat, -1, 1.0 / (nframe - frame_errors), 0);

												// detection image construction
												cv::subtract(pADUmaxMat, pADUavgMatFrame, pADUdtcMat);

												cv::minMaxLoc(pADUdtcMat, &minLum, &maxLum, &minPoint, &maxPoint);
												pADUdtcMat.convertTo(pADUdtcMat, -1, 255.0 / maxLum, 0);
												strncpy_s(ofilenamemax, sizeof(ofilenamemax), opts.ofilename, strlen(opts.ofilename) - 4);
												ofilenamemax[std::strlen(opts.ofilename) - 4] = '\0';
												sprintf(ofilenamemax, "%s_dtc_max_frame%05d.jpg", ofilenamemax, nframe);
												strcat_s(max_folder_path_filename, sizeof(max_folder_path_filename), right(ofilenamemax, strlen(ofilenamemax) - InRstr(ofilenamemax, "\\"), tmpstring));
												cv::imwrite(max_folder_path_filename, pADUdtcMat, img_save_params);
//useless as done before?
//cv::minMaxLoc(pADUdtcMat, &minLum, &maxLum, &minPoint, &maxPoint);
//pADUdtcMat.convertTo(pADUdtcMat, -1, 255.0 / maxLum, 0);
//

												// differential accumulated frames image construction
												strncpy_s(ofilenamediff, sizeof(ofilenamediff), opts.ofilename, strlen(opts.ofilename) - 4);
												ofilenamediff[std::strlen(opts.ofilename) - 4] = '\0';
												sprintf(ofilenamediff, "%s_dtc_diff_frame%05d.jpg", ofilenamediff, nframe);
												strcat_s(diff_folder_path_filename, sizeof(diff_folder_path_filename), right(ofilenamediff, strlen(ofilenamediff) - InRstr(ofilenamediff, "\\"), tmpstring));

												cv::minMaxLoc(pDifMat, &minLum, &maxLum, &minPoint, &maxPoint);
												pDifMat.convertTo(pDifImg, -1, 255.0 / maxLum, 0);
												pDifImg.convertTo(pDifImg, CV_8U);
												cv::imwrite(diff_folder_path_filename, pDifImg, img_save_params);
												pDifImg.~UMat();

												// single frame image construction
												strncpy_s(ofilenamesingle, sizeof(ofilenamesingle), opts.ofilename, strlen(opts.ofilename) - 4);
												ofilenamesingle[std::strlen(opts.ofilename) - 4] = '\0';
												sprintf(ofilenamesingle, "%s_frame%05d.jpg", ofilenamesingle, nframe);
												strcat_s(single_folder_path_filename, sizeof(single_folder_path_filename), right(ofilenamesingle, strlen(ofilenamesingle) - InRstr(ofilenamesingle, "\\"), tmpstring));

												cv::minMaxLoc(pGryMat, &minLum, &maxLum, &minPoint, &maxPoint);
												pGryMat.convertTo(pGryImg, -1, 255.0 / maxLum, 0);
												pGryImg.convertTo(pGryImg, CV_8U);
												cv::imwrite(single_folder_path_filename, pGryImg, img_save_params);
												//pGryImg.~UMat();
											}

											cv::threshold(pDifMat, pThrMat, opts.threshold, 0.0, CV_THRESH_TOZERO);
											cv::threshold(pDifMat, pDifMat, opts.threshold, 0.0, CV_THRESH_TOZERO);
											cv::minMaxLoc(pDifMat, &minLum, &maxLum, &minPoint, &maxPoint);

											// displays Threshold frame if relevant
											if (opts.viewThr && !pThrMat.empty()) Show_matrix(pThrMat, "Thresholded differential photometry", true, 1);

											// displays reference frame if relevant
											if (opts.viewRef && !pRefMat.empty()) Show_matrix(pRefMat, "Reference frame", true, 1);

											pMskMat.convertTo(pMskMat, CV_8U);
											// updates reference frame (all frames or running window)
											if (nframe > 1) {
												if (nframe <= (long)opts.nframesRef) {
													cv::accumulateWeighted(pGryMat, pRefMat, 1.0 / nframe, opts.thrWithMask ? pMskMat : cv::noArray());
												}
												else {
													cv::add(pRefMat, pGryMat / opts.nframesRef, pRefMat, opts.thrWithMask ? pMskMat : cv::noArray());
													cv::Mat frontMat = ReferenceFrameQueue.front();
													cv::subtract(pRefMat, frontMat / opts.nframesRef, pRefMat, opts.thrWithMask ? pMskMat : cv::noArray());
													frontMat.~Mat();
													ReferenceFrameQueue.pop();
												}
											}
											
											// displays differential frame if relevant
											if (!pDifMat.empty() && opts.viewRes) Show_matrix(pDifMat, "Resulting differential photometry", true, 1);

											// displays histogram frame if relevant
											if (opts.viewHis || ((strlen(opts.ovfname) > 0) && (opts.ovtype == OTYPE_HIS))) { 
												pHisImg = dtcGetHistogramImage(pDifMat, (float)opts.histScale, opts.threshold);
												if (opts.viewHis) Show_matrix(pHisImg, "Histogram", false, 1);
											}

											if (opts.viewROI && !pGryMat.empty()) { // displays ROI frame if relevant
												/*									double minLumroi, maxLumroi;
																					cv::Point minPointroi, maxPointroi;
																					cv::minMaxLoc(pGryMat, &minLumroi, &maxLumroi, &minPointroi, &maxPointroi);
																					pGryMat.convertTo(pGryImg, -1, 255.0 / maxLumroi, 0);*/
												pGryMat.convertTo(pGryImg, CV_8U);
												cv::imshow("ROI", pGryImg);
												cv::waitKey(1);
											}
											if (pTrkMat.data && opts.viewTrk) { // displays tracking frame if relevant
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
	//
	// IMPACT DETECTION alggorithm - single frames level tests
	//	
	//  Checks validity of brightest differential point by various method on frame level and if not sets maxLum to zero to neutralize it then
	//		
											//find brightest point in differential frame
											pDifMat.convertTo(pDifMat, CV_8U);
											cv::minMaxLoc(pDifMat, &minLum, &maxLum, &minPoint, &maxPoint);
											//brightestPoints.push_back(create_item(create_point(nframe - frame_errors, maxLum, maxPoint.x, maxPoint.y)));

											//get backgroung of normal frame
											cv::Mat pGryMat_8b(pGryMat);
											pGryMat_8b.convertTo(pGryMat_8b, CV_8U);
											float frame_background_level = dtcGetBackgroundFromHistogram(pGryMat_8b, opts.bg_detection_peak_factor, opts.bg_detection_consecutive_values, 0, false);
											bool is_black = false;
											bool is_outside_of_planet = false;
											bool is_not_on_planet = false;
											double planet_radius_estimation = 0.0;
											double distance_to_planet_center = 0.0;

											//check validity of brightest point in normal frame
											if (!is_bright_point_valid(pGryMat_8b, maxPoint, frame_background_level, &planet_radius_estimation, &distance_to_planet_center, &is_black, &is_outside_of_planet, &is_not_on_planet)) maxLum = 0;

											/*
											// Checks if maxpoint value in frame is black (case: satellite shadow transit (bright in differential matrix, black in mean/current frame))
											bool is_black = is_point_black_in_frame(pGryMat, maxPoint, 1, frame_background_level);

											// Checks if outside of planet disk checking black around (case satellite aside planet) and exclude solution
											bool is_outside_of_planet = is_zone_black_around_point(pGryMat, maxPoint, CROSS_DIFFERENTIAL_PHOTOMETRY_LMAX / 2, frame_background_level);
											
											// Checks if outside of planet disk checking distance from planet disk (case satellite aside planet) and exclude solution by setting maxLum to zero(other points will be used then)
											//double planet_radius2 = MAX(pGryMat.cols, pGryMat.rows) / 2.0;//
											bool is_not_on_planet = false;
											if (!is_black && !is_outside_of_planet) {
												double planet_radius_estimation = planet_radius(pGryMat, frame_background_level);
												double distance_to_planet_center = sqrt(pow(maxPoint.x - pGryMat.cols / 2.0, 2) + pow(maxPoint.y - pGryMat.rows / 2.0, 2));
												is_not_on_planet = (distance_to_planet_center > planet_radius_estimation);
											}*/

											maxFrameNb.push_back(nframe); /// to display real frame number (not only valid)
											maxPtB.push_back(maxLum);
											maxPtX.push_back(maxPoint.x);
											maxPtY.push_back(maxPoint.y);
											ReferenceFramePtSimilarity_decrease.push_back({ delta_similarity_reference[SSIM], delta_similarity_reference[MSE], delta_similarity_reference[NCC]});
											ReferenceFrameSimilarity.push_back({ similarity_reference[SSIM], similarity_reference[MSE], similarity_reference[NCC] });
											PreviousFramePtSimilarity_decrease.push_back({ delta_similarity_previous_frame[SSIM], delta_similarity_previous_frame[MSE], delta_similarity_previous_frame[NCC] });
											LastValidFrameSimilarity.push_back({ similarity_previous_frame[SSIM], similarity_previous_frame[MSE], similarity_previous_frame[NCC] });

											//frameErrors.push_back(nframe - frame_errors);
											frameNumbers.push_back(nframe);
											// end Impact detection

											if ((strlen(opts.ovfname) > 0) && opts.ovtype) {
												switch (opts.ovtype) {
												case OTYPE_DIF: pOVdMat = pDifMat; break;
												case OTYPE_TRK: pOVdMat = pTrkMat; break;
												case OTYPE_ROI: pOVdMat = pGryMat; break;
												case OTYPE_HIS: pOVdMat = pHisImg; break;
												case OTYPE_MSK: pOVdMat = pMskMat; break;
												}
												//pWriter = dtcWriteVideo(opts.ovfname, *pWriter, pCapture, pOVdMat);   // test OpenCV 4.7.0 
											}
											if (opts.wait && (cvWaitKey(opts.wait) == 27)) {
												break;
											}
											pGryImg_height = pGryMat.rows;
											pGryImg_width = pGryMat.cols;
										}
									}
								}
							}
							// Regular display update
							if (clock() > computing_threshold_time) {			// refreshed progress bar and computing time at a limited interval
								start_update_time = clock();
								if (!opts.parent_instance && !filesys::exists(CString2string((CString)opts.DeTeCtQueueFilename))) dlg.OnFileExit(); 	// exits DeTeCt if Queuefile does not exists (removed at parent exit) for a child instance. Added because of difficulty to terminate children processes when exiting parent instance
								UpdateProgress(acquisitions_to_be_processed, acquisitions_processed, acquisition_index_children, nframe, frame_number, opts.DeTeCtQueueFilename);
								DisplayProcessingTime(&computing_threshold_time, &end, computing_refresh_duration, begin, begin_total);
								display_update_duration += clock() - start_update_time;
							}
				//
				//	Regular processing update
				//
							if ((opts.parent_instance) && (clock() > check_threshold_time) && ((acquisitions_to_be_processed - acquisitions_processed - acquisition_index_children) > 1)) { //still 1 or more acquisitions being processed by children hence processing update needed
								//Regular processing update
								update_count++;
								start_update_time = clock();
								BOOL ExistsProcessedFiles = FALSE;
								if ((opts.maxinstances > 1) && (filesys::exists(CString2string((CString)opts.DeTeCtQueueFilename)))) {	// Gets other processed files by other instances
									double duration_total_others = 0;
									int nb_processed_files_fetched = GetOtherProcessedFiles(acquisitions_processed, &acquisition_index_children, &acquisitions_to_be_processed, &nb_error_impact, &nb_null_impact, &nb_low_impact, &nb_high_impact, &duration_total_others, &log_messages, opts.DeTeCtQueueFilename, &computing_threshold_time, &end, computing_refresh_duration, begin, begin_total, nframe, frame_number);
									if (nb_processed_files_fetched > 0) {
										if (opts.debug) LogString(L"File(s) processed fetched: " + (CString)std::to_string(nb_processed_files_fetched).c_str(), output_log_file.c_str(), &log_counter, TRUE, &wait_count_total);
										duration_total += duration_total_others;
										CDeTeCtMFCDlg::getduration()->SetWindowText((CString)"Duration processed (" + TotalType() + "): " + std::to_wstring((int)duration_total).c_str() + (CString)"s");
										ExistsProcessedFiles = TRUE;
									}
									if ((opts.parent_instance) && (strlen(opts.DeTeCtQueueFilename) > 0)) {  //Update correctly figure if children instances ignore some files
										acquisitions_to_be_processed = NbFilesFromQueue((CString)opts.DeTeCtQueueFilename) - NbItemFromQueue(_T("file_ko"), (CString)opts.DeTeCtQueueFilename, NULL, TRUE);
										UpdateProgress(acquisitions_to_be_processed, acquisitions_processed, acquisition_index_children, nframe, frame_number, opts.DeTeCtQueueFilename);
									}
								}
								processing_update_duration += clock() - start_update_time;
								
								//Regular instances update
								start_update_time = clock();
								maxinstances_previous = opts.maxinstances;
								if (filesys::exists(CString2string((CString)opts.DeTeCtQueueFilename))) opts.maxinstances = GetIntParamFromQueue(_T("max_instances"), (CString)opts.DeTeCtQueueFilename);
								// Forks attempt if more instances possible, maximum # of instances not reached at last check or new files processed (hence child detect process exited)
								if ((opts.maxinstances > maxinstances_previous) || (nb_instances < opts.maxinstances) || (ExistsProcessedFiles)) {
									if (opts.debug) LogString(_T("!Debug info: Forks") + (CString)std::to_string(opts.maxinstances).c_str() + _T(" ") + (CString)std::to_string(maxinstances_previous).c_str() + _T(" ") + (CString)std::to_string(nb_instances).c_str(), output_log_file.c_str(), &log_counter, TRUE, &wait_count_total);
									if ((opts.maxinstances > 1) && (!filesys::exists(CString2string((CString)opts.DeTeCtQueueFilename)))) AcquisitionFileListToQueue(&local_acquisition_files_list, _T("file_processing"), acquisition_index - 1, (CString)log.c_str(), &acquisitions_to_be_processed);
									nb_new_instances = 0;
									CPULoad = GetCPULoad(FALSE);
									nb_new_instances = ForksInstances(opts.maxinstances, ASorDeTeCtPID(opts.autostakkert_PID, opts.detect_PID), (CString)opts.DeTeCtQueueFilename, queue_scan_delay, queue_scan_delay_random_max, &nb_instances);
									if (nb_new_instances > 1)		LogString((CString)std::to_string(nb_new_instances).c_str() + _T(" new instances launched (") + (CString)std::to_string(nb_instances).c_str() + _T(" in total)") + _T(" (") + (CString)std::to_string((int)(100 - CPULoad * 100)).c_str() + _T("% CPU available)"), output_log_file.c_str(), &log_counter, TRUE, &wait_count_total);
									else if (nb_new_instances == 1) LogString((CString)std::to_string(nb_new_instances).c_str() + _T(" new instance launched (") + (CString)std::to_string(nb_instances).c_str() + _T(" in total)") + _T(" (") + (CString)std::to_string((int)(100 - CPULoad * 100)).c_str() + _T("% CPU available)"), output_log_file.c_str(), &log_counter, TRUE, &wait_count_total);
									else if (ExistsProcessedFiles) {
										nb_instances = 0;
										DisplayInstanceType(&nb_instances);
									}
								}
								check_threshold_time = clock() + check_threshold_time_inc;
								if (opts.debug) LogString(_T("!Debug info: Ends"), output_log_file.c_str(), &log_counter, TRUE, &wait_count_total);
								instances_update_duration += clock() - start_update_time;
							}
							pFrame.~Mat();
						}
					} // end of is_ROI_ok

// *****************************************************************
// ****************** End of frames processing *********************
// *****************************************************************
										

					char buffer3[MAX_STRING] = { 0 };
					sprintf_s(buffer3, MAX_STRING, "detect2:				opts    : %p	opts->ignore	:	%i\n", &opts, opts.ignore);
					OutputDebugStringA(buffer3);
				}
// *******************************************************************************************
// *********************************FINAL PROCESSING******************************************
// *******************************************************************************************

				//CDeTeCtMFCDlg::getProgress()->SetPos(MAX_RANGE_PROGRESS);
				//CDeTeCtMFCDlg::getProgress()->UpdateWindow();
				//CDeTeCtMFCDlg::getProgress_all()->SetPos((short)(MAX_RANGE_PROGRESS * (float)(acquisitions_processed + 1 + acquisition_index_children) / (acquisitions_to_be_processed)));
				//CDeTeCtMFCDlg::getProgress_all()->UpdateWindow();
				UpdateProgress(acquisitions_to_be_processed, acquisitions_processed, acquisition_index_children, nframe, frame_number, opts.DeTeCtQueueFilename);

				CDeTeCtMFCDlg::getduration()->SetWindowText((CString)"Duration processed (" + TotalType() + "): " + std::to_wstring((int)duration_total).c_str() + (CString)"s");

				// *********************************IMPACT PROCESSING******************************************

				double distance				= 0.0;
				ITEM* maxDtcImg				= NULL; // Detection image
				// img stats {min, average, max }
				double mean_stat[]			= { 0,0,0 };
				double mean2_stat[]			= { 0,0,0 };
				double max_mean_stat[]		= { 0,0,0 };
				double max_mean2_stat[]		= { 0,0,0 };
				double diff_stat[]			= { 0,0,0 };
				double diff2_stat[]			= { 0,0,0 };
				double temporal_density		= 0.0;
				double temporal_density_min	= 0.0;
				double confidence			= 0.0;

				if ((nframe > 0) && (is_ROI_ok)) {
					/*ADUdtc algorithm******************************************/

					if ((strlen(opts.ofilename) > 0) && opts.allframes) {
						pADUdtcMat.~Mat();
						pADUavgMatFrame.~Mat();
						pADUavgDiffMat.~Mat();
					}
					if ((strlen(opts.ovfname) > 0) && opts.ovtype && (pWriter)) pWriter->~VideoWriter();
							//{pWriter->release();
							//pWriter = nullptr;

					/********** Process all matrix **********/ //types changed from CV_32F to CV_8U
					pADUavgDiffMat.convertTo(pADUavgDiffMat, CV_32F);

					if (opts.flat_preparation) pFlatADUmaxMat.convertTo(pFlatADUmaxMat, CV_32F);
					
					/* compute average image */
					pADUavgMat.convertTo(pADUavgMat, CV_32F);
					pADUavgMat.convertTo(pADUavgMat, -1, 1.0 / (nframe - frame_errors), 0);
					//pGryMat.convertTo(pGryMat, CV_32FC1);
					/* Compute Max-mean image */
					pADUdtcMat = cv::Mat(pGryImg_height, pGryImg_width, CV_32F);
					cv::subtract(pADUmaxMat, pADUavgMat, pADUdtcMat);
//dtcApplyMaskToFrame(pADUdtcMat);

					/********** mean image **********/
					pADUavgMat.convertTo(pADUavgMat, CV_8U);
					mean2_stat[1] = mean(pADUavgMat)[0];
					cv::minMaxLoc(pADUavgMat, &mean2_stat[0], &mean2_stat[2], NULL, NULL);
					if (mean2_stat[2] < opts.ROI_min_px_val) {
						is_ROI_too_dark = true;
						is_image_correct = false;
					}
					/* normalizes mean  */
					pADUavgMat.convertTo(pADUavgMat, -1, 255.0 / mean2_stat[2], 0);
					pADUavgMat.convertTo(pADUavgMat, CV_8U);
					mean_stat[1] = mean(pADUavgMat)[0];
					cv::minMaxLoc(pADUavgMat, &mean_stat[0], &mean_stat[2], NULL, NULL);
					if (is_image_correct) cv::imwrite(dtc_full_filename(opts.ofilename, DTC_MEAN_SUFFIX, detection_folder_fullpathname_string.c_str(), tmpstring), pADUavgMat, img_save_params);

					/********** diff image **********/
					pADUavgDiffMat.convertTo(pADUavgDiffMat, -1, 1.0 / (nframe - frame_errors), 0);
					pADUavgDiffMat.convertTo(pADUavgDiffMat, CV_8U);
					diff2_stat[1] = mean(pADUavgDiffMat)[0];
					cv::minMaxLoc(pADUavgDiffMat, &diff2_stat[0], &diff2_stat[2], NULL, NULL);
						/* normalizes diff  */
					pADUavgDiffMat.convertTo(pADUavgDiffMat, -1, 255.0 / diff2_stat[2], 0);
					pADUavgDiffMat.convertTo(pADUavgDiffMat, CV_8U);
					diff_stat[1] = mean(pADUavgDiffMat)[0];
					cv::minMaxLoc(pADUavgDiffMat, &diff_stat[0], &diff_stat[2], NULL, NULL);
					if (opts.detail)
						if (is_image_correct) cv::imwrite(dtc_full_filename(opts.ofilename, DTC_DIFF_SUFFIX, detail_folder_path_string.c_str(), tmpstring), pADUavgDiffMat, img_save_params);

					/********** Max-mean image **********/
					//Computes max loc in temporary blurred dtc image to give less importance to flat pixels
					cv::medianBlur(pADUdtcMat, pSmoADUdtcMat, 3); // blur image
					cv::minMaxLoc(pSmoADUdtcMat, &minLum, &maxLum, &minPoint, &maxPoint);
					pSmoADUdtcMat.~UMat();
					(ITEM*)(maxDtcImg) = create_item(create_point(0, 0, maxPoint.x, maxPoint.y));

					/* Max-mean normalized image */
					cv::minMaxLoc(pADUdtcMat, &minLum, &maxLum, &minPoint, &maxPoint);
					pADUdtcMat.convertTo(pADUdtcMat, -1, 255.0 / maxLum, 0);
					//pADUdtcMat.convertTo(pADUdtcImg, CV_8UC3);	//3 channels for color image
					pADUdtcMat.convertTo(pADUdtcImg, CV_8U);	//3 channels for color image
					//cv::cvtColor(pADUdtcImg, pADUdtcImg, CV_GRAY2BGR);
					max_mean_stat[1] = mean(pADUdtcImg)[0];
					cv::minMaxLoc(pADUdtcImg, &max_mean_stat[0], &max_mean_stat[2], NULL, NULL);
					pADUdtcMat.convertTo(impactFrame, CV_8U);

					/*Max-mean non normalized image*/
					//works on 8 or 16 bits images
					if (maxLum > 255)	pADUdtcMat.convertTo(pADUdtcMat, -1, maxLum / (255.0 * 255.0), 0);
					else				pADUdtcMat.convertTo(pADUdtcMat, -1, maxLum / 255.0, 0);
					cv::Mat pADUdtcMatSmooth;
					if (pADUdtcMat.type() != CV_32F) pADUdtcMat.convertTo(pADUdtcMat, CV_32F);
					pADUdtcMat.copyTo(pADUdtcMatSmooth);
//smoths image by 5px for max loc detection
//is this beneficial?	
					cv::blur(pADUdtcMatSmooth, pADUdtcMatSmooth, cv::Size(5, 5));
					cv::minMaxLoc(pADUdtcMatSmooth, &minLum, &maxLum, &minPoint, &maxPoint);
					pADUdtcMatSmooth.~Mat();
					//Back to 8 bits
					pADUavgMat.convertTo(pADUavgMat, CV_8U);
					pADUmaxMat.convertTo(pADUmaxMat, CV_8U);
					pADUdtcMat.convertTo(pADUdtcImg2, CV_8UC3);
					cv::cvtColor(pADUdtcImg2, pADUdtcImg2, CV_GRAY2BGR);
					//cv::cvtColor(pADUdtcMat, pADUdtcMat, cv::COLOR_BGR2GRAY);
					cv::minMaxLoc(pADUdtcMat, &minLum, &maxLum, &minPoint, &maxPoint);
					pADUdtcMat.convertTo(pADUdtcMat, -1, 255.0 / maxLum, 0);
			//pADUdtcMat.convertTo(pADUdtcMat, CV_8U);

			//cv::minMaxLoc(pADUavgMat, &minLum, &maxLum, &minPoint, &maxPoint);
			//pADUavgMat.convertTo(pADUavgMat, -1, 255.0 / maxLum, 0);
			//pADUavgMat.convertTo(pADUavgMat, CV_8U);

//max-mean stats on first channel
//is this optimal?
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
						pFlatADUmaxMat.convertTo(pFlatADUmaxMat, CV_8U);
						pFlatADUmaxMat.convertTo(pFlatADUmaxImg, CV_8UC3);
						cv::cvtColor(pFlatADUmaxImg, pFlatADUmaxImg, CV_GRAY2BGR);
						cv::imshow("pFlatADUmaxImg", pFlatADUmaxMat);
						cv::waitKey(1000);
					}
//				}

					ReferenceFrameQueue = std::queue<cv::Mat>();
					//pDif_totalMean /= (nframe - frame_errors); //not used

// Replace GUI_display by TRUE
//if (dev_mode) GUI_display = true;

					if (frame_errors > 0) {
						CString frame_errors_cstring = (CString)(std::to_string(frame_errors)).c_str() + L" frame(s) rejected";
						if (frame_errors_not_readable > 0)	frame_errors_cstring += L", " + (CString)(std::to_string(frame_errors_not_readable)).c_str()	+ L" not readable";
						if (frame_errors_incorrect > 0)		frame_errors_cstring += L", " + (CString)(std::to_string(frame_errors_incorrect)).c_str()		+ L" incorrect ROI";
						if (frame_errors_too_dark > 0)		frame_errors_cstring += L", " + (CString)(std::to_string(frame_errors_too_dark)).c_str()		+ L" too dark";
						if (frame_errors_too_shifted > 0)	frame_errors_cstring += L", " + (CString)(std::to_string(frame_errors_too_shifted)).c_str()		+ L" ROI too shifted";
						if (frame_errors_too_different > 0)	frame_errors_cstring += L", " + (CString)(std::to_string(frame_errors_too_different)).c_str()	+ L" too different";
						LogString(frame_errors_cstring, output_log_file.c_str(), &log_counter, GUI_display, &wait_count_total);
					}

					if (frame_duplicates > 0) LogString((CString)(std::to_string(frame_duplicates)).c_str() + L" duplicate frames", output_log_file.c_str(), &log_counter, GUI_display, &wait_count_total);
					if (nframe > 0) LogString(+L"Differential photometry done, running impact detection...", output_log_file.c_str(), &log_counter, GUI_display, &wait_count_total);

					if ((strlen(opts.darkfilename) > 0) && (darkfile_ok == 1)) pADUdarkMat.~Mat();
 
if (is_image_correct) {

	//
	// IMPACT DETECTION algorithm - all frames level tests
	//	
	//  Checks validity of brightest differential point by various method on all frames level and if not sets maxLum to zero to neutralize it then
	//
					//bMat = cv::Mat(cv::Size(1, nframe), CV_8UC1, maxPtB.data());
					//cv::medianBlur(bMat, bMat, 3);
					//maxPtB = bMat.data;
					/*for (int i = 0; i < nframe; i++) {
					add_tail_item(&ptlist, create_item(create_point(i + 1 - frameErrors[i], maxPtB[i], maxPtX[i], maxPtY[i])));
					maxList.push_back(maxPtB[i]);
					}*/

					//double radius = std::min(std::min(20.0, pGryMat.rows / 10.0), pGryMat.cols / 10.0);
					//radius = radius > 5 ? radius : 5; //std::max gives error - //minimum value = 5
					//double	impact_radius	= std::max(std::min(opts.impact_radius_max, std::min(pGryMat.rows, pGryMat.cols) / opts.impact_radius_ratio), opts.impact_radius_min);	//minimum value = opts.impact_radius_min
					double	impact_radius	= std::min(std::max(opts.impact_radius_min, std::min(pGryMat.rows, pGryMat.cols) / opts.impact_radius_ratio), opts.impact_radius_max);	//minimum value = opts.impact_radius_min
					ITEM*	bestDtcImgImpact		= create_item(create_point(0, 0, 0, 0)); // Algorithm

					// ************* CSV
					//CreateDirectory(wdir_csv_name.c_str(), 0);
					std::string dir_csv_name = detection_folder_fullpathname_string;
					dir_csv_name = dir_csv_name.append("\\csv");
					if (!(dir_tmp = opendir(dir_csv_name.c_str())))
						if (mkdir(dir_csv_name.c_str()) != 0) { // usage of mkdir only solution found to handle directory names with special characters (eg. �, �, ...)
							Sleep(FILEACCESS_WAIT_MS * 10);
							if (!(dir_tmp = opendir(dir_csv_name.c_str()))) {
								char msgtext[MAX_STRING] = { 0 };
								snprintf(msgtext, MAX_STRING, "cannot create directory %s\n", dir_csv_name.c_str());
								Warning(WARNING_MESSAGE_BOX, "cannot create directory", __func__, msgtext);
							} else closedir(dir_tmp);
						}
						else closedir(dir_tmp);
					std::ofstream output_csv(dir_csv_name.append("\\").append(filePath).append(".csv"));
					//output_csv << "x,y,B\n";
					for (int i = 0; i < (nframe - frame_errors); i++) {
						add_tail_item(&ptlist, create_item(create_point(i + 1, maxPtB[i], maxPtX[i], maxPtY[i])));	// populate the list
	//					output_csv << (int)maxPtX[i] << "," << (int)maxPtY[i] << "," << (int)maxPtB[i] << "\n";		// logs the list
						//add_tail_item(&ptlist, create_item(create_point(frameErrors[i], maxPtB[i], maxPtX[i], maxPtY[i])));
						maxList.push_back(maxPtB[i]);																// populates the max list
					}
					//output_csv.close();

					double maxMean				= 0;
					double brightness_factor	= 0;
					if (maxList.size() > 0) {
						//double accum				= std::accumulate(xList.begin(), xList.end(), 0.0);				// sum of brightness increases not used
						double maxAccum				= std::accumulate(maxList.begin(), maxList.end(), 0.0);			// sum of brightnesses
						double maxBright			= (double)*(std::max_element(maxList.begin(), maxList.end()));	// maximum of brightness
						maxMean						= maxAccum / maxList.size();									// average of brightness
						brightness_factor			= (maxBright / (maxMean + opts.threshold)) - 1;					// Brightness increase of brightest point
						//brightnessFactor			= maxMean / pDif_totalMean;												// not used
						/* not used but should it be ?
						//double stdDevAccum = 0.0;
						//std::for_each(maxList.begin(), maxList.end(), [&](const double d) {
						//	stdDevAccum += (d - maxMean) * (d - maxMean);
						//	});
						//double stdev = sqrt(stdDevAccum / (maxList.size() - 1));*/
					}
					//bMat.release();
					
					if (!IsFPSValid(fps)) fps = nframe / duration; // correction FPS with known nbframe and duration (case wmv with bad info)
					impact_frames_min = (int)ceil(MAX(opts.incrFrameImpact, fps * opts.impact_duration_min));    // moved down just before usage to use nbframe if fps is invalid

			//
			// Run detection
			//
					if (ptlist.size <= ptlist.maxsize && ptlist.size > impact_frames_min) {
						std::vector<std::string> detect_impact_log_messages;
						detect_impact_log_messages.push_back("");
						nb_impact += detect_impact(&dtc, &outdtc, maxMean, &ptlist, &bestDtcImgImpact, &temporal_density, impact_radius, opts.impact_brightness_increase_min_factor, opts.impact_radius_shared_candidates_factor_min, impact_frames_min, temporal_density_min, &detect_impact_log_messages);
						for (std::string msg : detect_impact_log_messages) {
							std::wstring wmsg = std::wstring(msg.begin(), msg.end());
							CString Cmsg = CString(wmsg.c_str(), (int)wmsg.length());
							LogString(Cmsg, output_log_file.c_str(), &log_counter, TRUE, &wait_count_total);
						}
						detect_impact_log_messages.clear();

						LogString(L"Detection : number of impacts = " + (CString)(std::to_string(nb_impact)).c_str(), output_log_file.c_str(), &log_counter, GUI_display, &wait_count_total);
					}
					else LogString(L"Detection test ko: impact duration: ptlist size = " + (CString)(std::to_string(ptlist.size)).c_str() + L" <= ? " + (CString)(std::to_string(ptlist.size)).c_str() + L", impact frames min=" + (CString)(std::to_string(impact_frames_min)).c_str(), output_log_file.c_str(), &log_counter, GUI_display, &wait_count_total);
					delete_list(&ptlist);

			//
			// Writes csv file with frames info for further analysis
			//
					output_csv << "impact_radius," << impact_radius <<"\n";
					output_csv << "maxMean," << maxMean << "\n";
					output_csv << "brightness_factor," << brightness_factor << "\n";
					output_csv << "brightness point," << bestDtcImgImpact->point->frame << "," << bestDtcImgImpact->point->x << "," << bestDtcImgImpact->point->y << "," << bestDtcImgImpact->point->val << "," << temporal_density << "\n";
					output_csv << "impact_frames_min," << impact_frames_min << "\n";
					output_csv << "frame,x,y,B,d,incrLum,sliding mean incr";
					if (opts.use_reference_similarity[SSIM] || opts.use_all_algo_for_test)		output_csv << ", ref SSIM sim dec, ref SSIM sim";
					if (opts.use_previous_frame_similarity[SSIM] || opts.use_all_algo_for_test)	output_csv << ", previous SSIM sim dec, previous SSIM sim";
					if (opts.use_reference_similarity[MSE] || opts.use_all_algo_for_test)		output_csv << ", ref MSE sim dec, ref MSE sim";
					if (opts.use_previous_frame_similarity[MSE] || opts.use_all_algo_for_test)	output_csv << ", previous MSE sim dec, previous MSE sim";
					if (opts.use_reference_similarity[NCC] || opts.use_all_algo_for_test)		output_csv << ", ref NCC sim dec, ref NCC sim";
					if (opts.use_previous_frame_similarity[NCC] || opts.use_all_algo_for_test)	output_csv << ", previous NCC sim dec, previous NCC sim";
					output_csv << "\n";
					for (int i = 0; i < (nframe - frame_errors); i++) {
						double mean_brightness_increase = 0.0;
						if (i < impact_frames_min) {
							for (int j = 0; j <= i; mean_brightness_increase += maxPtB[j], j++);
							mean_brightness_increase /= (i + 1);
						}
						else {
							for (int j = i - impact_frames_min + 1; j <= i; mean_brightness_increase += maxPtB[j], j++);
							mean_brightness_increase /= impact_frames_min;
						}
						output_csv << (int)maxFrameNb[i] << "," << (int)maxPtX[i] << "," << (int)maxPtY[i] << "," << std::fixed << std::setprecision(3) << maxPtB[i] << "," << std::fixed << std::setprecision(3) << sqrt(pow((int)maxPtX[i] - bestDtcImgImpact->point->x, 2) + pow((int)maxPtY[i] - bestDtcImgImpact->point->y, 2)) << "," << std::fixed << std::setprecision(4) << maxPtB[i] / maxMean - 1.0 << "," << std::fixed << std::setprecision(4) << mean_brightness_increase / maxMean - 1.0;		// logs the list
						if (opts.use_reference_similarity[SSIM] || opts.use_all_algo_for_test)		output_csv << "," << std::fixed << std::setprecision(7) << ReferenceFramePtSimilarity_decrease[i][SSIM] << "," << std::fixed << std::setprecision(7) << ReferenceFrameSimilarity[i][SSIM];
						if (opts.use_previous_frame_similarity[SSIM] || opts.use_all_algo_for_test)	output_csv << "," << std::fixed << std::setprecision(7) << PreviousFramePtSimilarity_decrease[i][SSIM] << "," << std::fixed << std::setprecision(7) << LastValidFrameSimilarity[i][SSIM];
						if (opts.use_reference_similarity[MSE] || opts.use_all_algo_for_test)		output_csv << "," << std::fixed << std::setprecision(7) << ReferenceFramePtSimilarity_decrease[i][MSE] << "," << std::fixed << std::setprecision(7) << ReferenceFrameSimilarity[i][MSE];
						if (opts.use_previous_frame_similarity[MSE] || opts.use_all_algo_for_test)	output_csv << "," << std::fixed << std::setprecision(7) << PreviousFramePtSimilarity_decrease[i][MSE] << "," << std::fixed << std::setprecision(7) << LastValidFrameSimilarity[i][MSE];
						if (opts.use_reference_similarity[NCC] || opts.use_all_algo_for_test)		output_csv << "," << std::fixed << std::setprecision(7) << ReferenceFramePtSimilarity_decrease[i][NCC] << "," << std::fixed << std::setprecision(7) << ReferenceFrameSimilarity[i][NCC];
						if (opts.use_previous_frame_similarity[NCC] || opts.use_all_algo_for_test)	output_csv << "," << std::fixed << std::setprecision(7) << PreviousFramePtSimilarity_decrease[i][NCC] << "," << std::fixed << std::setprecision(7) << LastValidFrameSimilarity[i][NCC];
						output_csv << "\n";
					}
					output_csv.close();

					char buffer4[MAX_STRING] = { 0 };
					sprintf_s(buffer4, MAX_STRING, "detect4:				opts    : %p	opts->ignore	:	%i\n", &opts, opts.ignore);
					OutputDebugStringA(buffer4);

					// calculate impact confidence
					double impact_frames = (&outdtc)->nMaxFrame - (&outdtc)->nMinFrame;
					double log10_value = impact_frames != 0 ? std::log10((impact_frames / impact_frames_min) * 10) : 0;
					if (log10_value > 0) confidence = (brightness_factor / opts.impact_brightness_increase_min_factor) * log10_value;
					//double confidence = (stdev / popts->impact_brightness_increase_min_factor) * log10_value;

	//
	// IMPACT DETECTION alggorithm - all frames level tests
	//	
	
	
			//  Checks validity of brightest differential point in dtc image - checked against mean image as it is the most real one and less noisy, and as algorithm are brightness/position based
			// Time=35ms
			//
					bool is_impact_point_valid			= false; // default value : algorithm did not work or is_black
					bool is_black						= false;
					bool is_outside_of_planet			= false;
					bool is_not_on_planet				= false;
					double planet_radius_estimation		= 0.0;
					double distance_to_planet_center	= 0.0;

					if ((bestDtcImgImpact->point->x > 0) && (bestDtcImgImpact->point->y > 0)) {
						CString ok_or_ko_CString;
						// test on transit detection - unsucessfull because background on mean image is difficult to detect: peak is not steep

						float image_background_level = dtcGetBackgroundFromHistogram(pADUavgMat, opts.bg_detection_peak_factor, opts.bg_detection_consecutive_values, 0, false); // x4 factor because many frames consolidated
						
						is_impact_point_valid = is_bright_point_valid(pADUavgMat, cv::Point(bestDtcImgImpact->point->x, bestDtcImgImpact->point->y), image_background_level, &planet_radius_estimation, &distance_to_planet_center, &is_black, &is_outside_of_planet, &is_not_on_planet);

						if (!is_black) ok_or_ko_CString = L"OK: max differential point not"; else ok_or_ko_CString = L"ko: max differential point";
						LogString(L"Detection test " + ok_or_ko_CString + " over black area (background = "   + (CString)(std::to_string(image_background_level)).c_str() + L")", output_log_file.c_str(), &log_counter, GUI_display, &wait_count_total);
						
						// Checks if outside of planet disk checking black around (case satellite aside planet) and exclude solution
						if (!is_black) {
							if (!is_outside_of_planet) ok_or_ko_CString = L"OK: max differential point on"; else ok_or_ko_CString = L"ko: max differential point outside of";
							LogString(L"Detection test " + ok_or_ko_CString + " planet disk (background = " + (CString)(std::to_string(image_background_level)).c_str() + L")", output_log_file.c_str(), &log_counter, GUI_display, &wait_count_total);
						}
				
						// Checks if outside of planet disk checking distance from planet disk (case satellite aside planet) and exclude solution by setting maxLum to zero(other points will be used then)
						if (!is_black && !is_outside_of_planet) {
							if (!is_not_on_planet) ok_or_ko_CString = L"OK: max differential point"; else ok_or_ko_CString = L"ko: max differential not";
							LogString(	L"Detection test "+ ok_or_ko_CString +" on planet disk : planet radius = " + (CString)(std::to_string(planet_radius_estimation)).c_str()
													+ L", distance to center = "  + (CString)(std::to_string(distance_to_planet_center)).c_str()
													+ L" (background = "   + (CString)(std::to_string(image_background_level)).c_str() + L")", output_log_file.c_str(), &log_counter, GUI_display, &wait_count_total);
						}
					}
			// EndTime=35ms
			//
			//  Checks validity (not outside planet) of brightest point in dtc image  - checked against mean image as it is the most real one and less noisy, and as algorithm are brightness/position based
			//
					cv::Point brightestDtcImgPoint = cv::Point(maxDtcImg->point->x, maxDtcImg->point->y);
					bool is_brightestDtcImgPoint_corrected = false;
					cv::minMaxLoc(pADUdtcMat, NULL, &maxLum, NULL, &brightestDtcImgPoint);
					pADUdtcMat.convertTo(pADUdtcMat, -1, 255.0 / maxLum, 0);
					float image_background_level = dtcGetBackgroundFromHistogram(pADUavgMat, opts.bg_detection_peak_factor, opts.bg_detection_consecutive_values, 0, false); // canceled because usage of pADUavgMat: x4 factor because many frames consolidated

					cv::Mat pADUdtcMat_invalid_points_corrected(pADUdtcMat);
					pADUdtcMat_invalid_points_corrected.convertTo(pADUdtcMat_invalid_points_corrected, CV_8U);		//for modification

					bool is_bright_line					= false;
					bool is_bright_column				= false;
					double distance_factor_from_edge	= 0.04;
					double line_column_avg_min			= 0.0;
					double line_column_avg_factor		= 0.7;
					double normalization_value			= 255.0;

					while (maxLum > image_background_level) {
						do { // remove bright columns/lines - cannot be tested
							if (!is_brightestDtcImgPoint_corrected) { 
								cv::minMaxLoc(pADUdtcMat, NULL, &maxLum, NULL, &brightestDtcImgPoint);	// get new brightest point
							} else cv::minMaxLoc(pADUdtcMat, NULL, &maxLum, NULL, NULL);				// do not overwrites existing brightest point
							is_brightestDtcImgPoint_corrected = false;
							if (maxLum <= image_background_level) break;
							pADUdtcMat.convertTo(pADUdtcMat, -1, normalization_value / maxLum, 0);
							line_column_avg_min = line_column_avg_factor * maxLum;
						} while (check_bright_point_on_bright_line_column(pADUdtcMat, pADUdtcMat_invalid_points_corrected, brightestDtcImgPoint, &is_bright_line, &is_bright_column,
							distance_factor_from_edge, line_column_avg_min, true, (int)image_background_level));
						// point not on bright col/line anymore
						// is point valid???
						if (maxLum <= image_background_level) break;
						if (is_bright_point_valid(pADUavgMat, cv::Point(brightestDtcImgPoint.x, brightestDtcImgPoint.y), image_background_level, &planet_radius_estimation, &distance_to_planet_center,
							&is_black, &is_outside_of_planet, &is_not_on_planet)) break;
						else {
							// black circleout outside of planet in image - would replace blacking out point by point
							//cv::Point center(pADUdtcMat_invalid_points_corrected.cols / 2, pADUdtcMat_invalid_points_corrected.rows / 2);
							//cv::Mat mask = cv::Mat::zeros(pADUdtcMat_invalid_points_corrected.size(), pADUdtcMat_invalid_points_corrected.type());
							//cv::circle(mask, center, (int)planet_radius_estimation, cv::Scalar(255), -1);
							//cv::bitwise_not(mask, mask);
							//pADUdtcMat_invalid_points_corrected.setTo(0, mask);
							//mask.~Mat();
							//find next brightest pixel candidate - erase wrong one first
							pADUdtcMat_invalid_points_corrected.at<uchar>(brightestDtcImgPoint.y, brightestDtcImgPoint.x) = 0;
							//cv::circle(pADUdtcMat_invalid_points_corrected, brightestDtcImgPoint, 5, 0, -1);	// 5 radius zone to go much faster
							cv::minMaxLoc(pADUdtcMat_invalid_points_corrected, NULL, &maxLum, NULL, &brightestDtcImgPoint);
							is_brightestDtcImgPoint_corrected = true;
						}
					}
					//end logs and cleaning
					if (maxLum <= image_background_level) LogString(L"Detection image: brightest on planet pixel fainter than background (" + (CString)(std::to_string(maxLum)).c_str() + L" <= " +
						(CString)(std::to_string(image_background_level)).c_str() + L")", output_log_file.c_str(), &log_counter, GUI_display, &wait_count_total);
					pADUdtcMat.convertTo(pADUdtcMat, -1, 255.0 / maxLum, 0);
					pADUdtcMat.convertTo(pADUdtcImg, CV_8UC3);	//3 channels for color image
					pADUdtcMat_invalid_points_corrected.~Mat();





					////do { // general loop
						////do  { // remove bright column/lines - cannot be tested
							////cv::minMaxLoc(pADUdtcMat, NULL, &maxLum, NULL, &brightestDtcImgPoint);
							////pADUdtcMat.convertTo(pADUdtcMat, -1, 255.0 / maxLum, 0);
							////line_column_avg_min = 0.6 * pADUdtcMat.at<uchar>(brightestDtcImgPoint.y, brightestDtcImgPoint.x);
//cv::imshow("First dtc image", pADUdtcImg);
//cv::waitKey(2000);
						////} while (check_bright_point_on_bright_line_column(pADUdtcMat, pADUdtcMat_invalid_points_corrected, brightestDtcImgPoint, &is_bright_line, &is_bright_column,
							////distance_factor_from_edge, line_column_avg_min, true, (int) image_background_level));
					

						////if ((!is_bright_point_valid(pADUavgMat, cv::Point(brightestDtcImgPoint.x, brightestDtcImgPoint.y), image_background_level, &planet_radius_estimation, &distance_to_planet_center,
							////&is_black, &is_outside_of_planet, &is_not_on_planet))){
/*
cv::Mat pADUdtcMat_planet_only_view(pADUdtcMat);									//for viewing purpose only
cv::minMaxLoc(pADUdtcMat_planet_only_view, &minLum, &maxLum, NULL, NULL);			//find in planet only image  maxlum
pADUdtcMat_planet_only_view.convertTo(pADUdtcMat_planet_only_view, -1, 255.0 / maxLum, 0);
pADUdtcMat_planet_only_view.convertTo(pADUdtcMat_planet_only_view, CV_8U);
dtcDrawImpact(pADUdtcMat_planet_only_view, brightestDtcImgPoint, CV_RGB(0, 128, 255), CROSS_MAX_LMAX, false, MAX(croi.width, croi.height));	// Blue, was CV_RGB(0, 255, 0) initially
*/
					//
					// black circleout outside of planet in image - would replace blacking out point by point
					//
					//cv::Point center(pADUdtcMat_invalid_points_corrected.cols / 2, pADUdtcMat_invalid_points_corrected.rows / 2);
					//cv::Mat mask = cv::Mat::zeros(pADUdtcMat_invalid_points_corrected.size(), pADUdtcMat_invalid_points_corrected.type());
					//cv::circle(mask, center, (int)planet_radius_estimation, cv::Scalar(255), -1);
					//cv::bitwise_not(mask, mask);
					//pADUdtcMat_invalid_points_corrected.setTo(0, mask);
					//mask.~Mat();
							////do { // find new valid point
								//find next brightest pixel candidate
								////pADUdtcMat_invalid_points_corrected.at<uchar>(brightestDtcImgPoint.y, brightestDtcImgPoint.x) = 0;
								//cv::circle(pADUdtcMat_invalid_points_corrected, brightestDtcImgPoint, 5, 0, -1);	// 5 radius zone to go much faster
								////cv::minMaxLoc(pADUdtcMat_invalid_points_corrected, NULL, &maxLum, NULL, &brightestDtcImgPoint);
							////} while ((maxLum > image_background_level) && (!is_bright_point_valid(pADUavgMat, cv::Point(brightestDtcImgPoint.x, brightestDtcImgPoint.y), image_background_level, &planet_radius_estimation, &distance_to_planet_center,
								////&is_black, &is_outside_of_planet, &is_not_on_planet)));
								////} // end if not point valid
					////} while ((maxLum > image_background_level) && (!is_bright_point_valid(pADUavgMat, cv::Point(brightestDtcImgPoint.x, brightestDtcImgPoint.y), image_background_level, &planet_radius_estimation, &distance_to_planet_center,
								////&is_black, &is_outside_of_planet, &is_not_on_planet)));
					////if (maxLum <= image_background_level) LogString(L"Detection image: brightest on planet pixel fainter than background (" + (CString)(std::to_string(maxLum)).c_str() + L" <= " +
																		////(CString)(std::to_string(image_background_level)).c_str() + L")", output_log_file.c_str(), &log_counter, GUI_display, &wait_count_total);
					////pADUdtcMat.convertTo(pADUdtcImg, CV_8UC3);	//3 channels for color image
					////pADUdtcMat_invalid_points_corrected.~Mat();
					//pADUdtcMat_invalid_points_corrected_view_only.~Mat();

			//
			//  Checks validity (not too close to edge) of brightest point in dtc images
			// 
					//if max point not too close to edge draw yellow cross else recalculate it outside of the edges

					// Time=1ms
					/* max-mean non normalized image */
#define MIN_EDGE_DISTANCE 1
					if ((brightestDtcImgPoint.x < MIN_EDGE_DISTANCE) || (brightestDtcImgPoint.x > pADUdtcMat.cols - MIN_EDGE_DISTANCE) || (brightestDtcImgPoint.y < MIN_EDGE_DISTANCE) || (brightestDtcImgPoint.y > pADUdtcMat.rows - MIN_EDGE_DISTANCE)) {
						cv::Mat pADUdtcImg_cropped(pADUdtcMat, cv::Rect(MIN_EDGE_DISTANCE, MIN_EDGE_DISTANCE, pADUdtcImg.cols - 2 * MIN_EDGE_DISTANCE, pADUdtcMat.rows - 2 * MIN_EDGE_DISTANCE));
						cv::minMaxLoc(pADUdtcImg_cropped, &minLum, &maxLum, &minPoint, &maxPoint);
						brightestDtcImgPoint.x = maxPoint.x + MIN_EDGE_DISTANCE;
						brightestDtcImgPoint.y = maxPoint.y + MIN_EDGE_DISTANCE;
						pADUdtcImg_cropped.~Mat();
					}
					// EndTime=1ms
					
				//
				// Draw crosses on max-mean image
				//
					cv::cvtColor(pADUdtcImg, pADUdtcImg, CV_GRAY2BGR);
					dtcDrawImpact(pADUdtcImg, brightestDtcImgPoint, CV_RGB(0, 128, 255), CROSS_MAX_LMAX, false, MAX(croi.width, croi.height));																	// Blue, was CV_RGB(0, 255, 0) initially
					LogString(L"Brightest valid point @(" + (CString)(std::to_string(brightestDtcImgPoint.x)).c_str() + L"," + (CString)(std::to_string(brightestDtcImgPoint.y)).c_str() + L")", output_log_file.c_str(), &log_counter, true, &wait_count_total);

					distance = 1.0;						// default value : algorithm did not work or is_black
					if (is_impact_point_valid) {						//if candidate valid the draw yellow cross and calculate distance
						dtcDrawImpact(pADUdtcImg, cv::Point(bestDtcImgImpact->point->x, bestDtcImgImpact->point->y), CV_RGB(255, 255, 0), CROSS_DIFFERENTIAL_PHOTOMETRY_LMAX, true, MAX(croi.width, croi.height));	// Pale Yellow, was CV_RGB(255, 0, 0) initially
						LogString(L"Differential algorithm best candidate @(" + (CString)(std::to_string(bestDtcImgImpact->point->x)).c_str() + L"," + (CString)(std::to_string(bestDtcImgImpact->point->y)).c_str() + L")", output_log_file.c_str(), &log_counter, true, &wait_count_total);
						distance = sqrt(pow(brightestDtcImgPoint.x - bestDtcImgImpact->point->x, 2) + pow(brightestDtcImgPoint.y - bestDtcImgImpact->point->y, 2));
						distance /= ((MIN(croi.width, croi.height) / opts.secSize));
					}
					cv::imwrite(dtc_full_filename(opts.ofilename, DTC_MAX_MEAN_SUFFIX, detection_folder_fullpathname_string.c_str(), tmpstring), pADUdtcImg, img_save_params);
					if (opts.flat_preparation) cv::imwrite(dtc_full_filename(opts.ofilename, DTC_FLAT_PREP_SUFFIX, detection_folder_fullpathname_string.c_str(), tmpstring), pFlatADUmaxImg, img_save_params);
				}
}	//end !is_image_correct
				//if (is_image_correct) cv::imwrite(dtc_full_filename(opts.ofilename, DTC_MAX_MEAN_SUFFIX, detection_folder_fullpathname_string.c_str(), tmpstring), pADUdtcImg, img_save_params);
// **************************************** final end of ADUdtc algorithm (confidence, probability, ...) ******************************************
				if (nframe == 0) is_image_correct = false;
				logmessage = "";
				short_logmessage = "";
				rating = Rating_type::Error;
				logmessage2 = "";
				logmessage3 = "";
				strcpy_s(rating_filename_suffix, sizeof(rating_filename_suffix),  "");
				
				if (!is_image_correct) {
					if		(is_ROI_null)		logmessage +=  "ERROR: ROI cannot be obtained.";
					else if (is_ROI_too_small)	logmessage +=  "ERROR: ROI too small." + std::to_string(croi.width) + "x" + std::to_string(croi.height) + " too small "
						+ "(" + std::to_string(croi.width) + "x" + std::to_string(croi.height) + ") < " + "(" + std::to_string(opts.ROI_min_size) + "x" + std::to_string(opts.ROI_min_size);

					else if (is_ROI_too_dark) 	logmessage +=  "ERROR: No planet detected in acquisition images...";
					else if (nframe == 0) 		logmessage += "ERROR: No valid frame.";
					nb_error_impact++; 
					short_logmessage = logmessage;
					LogString((CString)logmessage.c_str(), output_log_file.c_str(), &log_counter, GUI_display, &wait_count_total);
					logmessage += "\n";
					sprintf(rating_classification, "Error        ");
					sprintf(rating_filename_suffix, "error");
					rating = Rating_type::Error;
					confidence = -1;
					nb_impact = -1;
				}

// No error for acquisition

	//
	// IMPACT DETECTION alggorithm - dtc images/confidence level
	//	
	//  Checks validity of brightest differential point by various method on dtc images/confidence level
	//
				else if (nb_impact > 0) {
					outdtc.nMinFrame = frameNumbers[outdtc.nMinFrame];
					outdtc.nMaxFrame = frameNumbers[outdtc.nMaxFrame];
					outdtc.MaxFrame  = frameNumbers[outdtc.MaxFrame];
					CString ok_or_ko_CString;
					if (distance <= opts.impact_distance_max)								ok_or_ko_CString =L"OK"; else ok_or_ko_CString = L"ko";
					LogString(L"Detection test " + ok_or_ko_CString +": distance:                                          " + (CString)(std::to_string(distance)).c_str() + " (<= ? "+ (CString)(std::to_string(opts.impact_distance_max)).c_str() + L")",		output_log_file.c_str(), &log_counter, GUI_display, &wait_count_total);
					if (confidence >= opts.impact_confidence_min)							ok_or_ko_CString = "OK"; else ok_or_ko_CString = "ko";
					LogString(L"Detection test " + ok_or_ko_CString + ": confidence:	                                       " + (CString)(std::to_string(confidence)).c_str() + " (>= ? "+ (CString)(std::to_string(opts.impact_confidence_min)).c_str() + L")", output_log_file.c_str(), & log_counter, GUI_display, & wait_count_total);
					if ((max_mean_stat[2] - max_mean_stat[1]) >= opts.impact_max_avg_min)	ok_or_ko_CString = "OK"; else ok_or_ko_CString = "ko";
					LogString(L"Detection test " + ok_or_ko_CString + ": max-mean max - max-mean avg: " + (CString)(std::to_string(max_mean_stat[2] - max_mean_stat[1])).c_str() + " (>= ? "+ (CString)(std::to_string(opts.impact_max_avg_min)).c_str() + L")",	output_log_file.c_str(), &log_counter, GUI_display, &wait_count_total);
					if ((distance <= opts.impact_distance_max) && (confidence >= opts.impact_confidence_min) && ((max_mean_stat[2] - max_mean_stat[1]) >= opts.impact_max_avg_min)) {
						if (!opts.ADUdtconly) {
							/*** high probability impact */
							nb_high_impact++;
							logmessage = "ALERT: " + std::to_string(nb_impact) + " HIGH PROBABILITY IMPACT DETECTED (frames " +
								std::to_string(outdtc.nMinFrame) + "-" + std::to_string(outdtc.nMaxFrame) + ", max @" +
								std::to_string(outdtc.MaxFrame) + ").";
							short_logmessage = logmessage;
							std::stringstream confidence_stream;
							confidence_stream << std::fixed << std::setprecision(2) << confidence;
							std::string confidence_string = confidence_stream.str();
							//logmessage2 = "Confidence: " + confidence_string;
							logmessage3 = "CHECK DETECTION IMAGES!\n";
							LogString((CString)logmessage.c_str(), output_log_file.c_str(), &log_counter, GUI_display, &wait_count_total);
							LogString((CString)logmessage2.c_str(), output_log_file.c_str(), &log_counter, GUI_display, &wait_count_total);
							LogString((CString)logmessage3.c_str(), output_log_file.c_str(), &log_counter, GUI_display, &wait_count_total);
							//logmessage += "\n" + logmessage2 + "\n" + logmessage3;
							logmessage += "\n" + logmessage3;
							sprintf(rating_classification, "HIGH (@%5d)", outdtc.MaxFrame);
							sprintf(rating_filename_suffix, "high@%d", outdtc.MaxFrame);
							rating = Rating_type::High;
						}
						else {
							/* only initial algorithm launched, displaying only nb impacts detected */
							nb_low_impact++;
							logmessage = "WARNING: " + std::to_string(nb_impact) + " low probability impact.";
							short_logmessage = logmessage;
							LogString((CString)logmessage.c_str(), output_log_file.c_str(), &log_counter, GUI_display, &wait_count_total);
							logmessage += "\n";
							sprintf(rating_classification, "Low          ");
							sprintf(rating_filename_suffix, "low");
						}
					}
					else if (distance <= opts.impact_distance_max) {
						/* No impact, confidence or contrast threshold not respected */
						nb_null_impact++;
						logmessage = "No impact detected (too faint candidate).";
						short_logmessage = logmessage;
						LogString((CString)logmessage.c_str(), output_log_file.c_str(), &log_counter, GUI_display, &wait_count_total);
						logmessage += "\n";
						sprintf(rating_classification, "Null         ");
						sprintf(rating_filename_suffix, "null");
						rating = Rating_type::Null;
					}
					else {
						/* algorithm worked */
						/* distance incorrect */
						confidence /= 4;
						if ((max_mean_stat[2] - max_mean_stat[1]) > opts.impact_max_avg_min) {
							/* potential impact */
							if (!opts.ADUdtconly) {
								nb_low_impact++;
								logmessage = "WARNING: " + std::to_string(nb_impact) + " low probability impact (frames " +
									std::to_string(outdtc.nMinFrame) + "-" + std::to_string(outdtc.nMaxFrame) + ", max @" +
									std::to_string(outdtc.MaxFrame) + "). ";
								short_logmessage = logmessage;
								std::stringstream confidence_stream;
								confidence_stream << std::fixed << std::setprecision(2) << confidence;
								std::string confidence_string = confidence_stream.str();
								logmessage2 = "Confidence: " + confidence_string;
								//if ((distance <= popts->impact_distance_max) && (confidence > popts->impact_confidence_min) && ((max_mean_stat[2] - max_mean_stat[1]) > popts->impact_max_avg_min))
								if (!((confidence > opts.impact_confidence_min) && ((max_mean_stat[2] - max_mean_stat[1]) > opts.impact_max_avg_min)))
									logmessage2 = logmessage2 + ", too faint";
								if (!(distance <= opts.impact_distance_max))
									logmessage2 = logmessage2 + ", detection image and algorithm incoherent";
								logmessage2 = logmessage2 + ".";
								logmessage3 = "Please CHECK detection images.\n";
								LogString((CString)logmessage.c_str(), output_log_file.c_str(), &log_counter, GUI_display, &wait_count_total);
								LogString((CString)logmessage2.c_str(), output_log_file.c_str(), &log_counter, GUI_display, &wait_count_total);
								LogString((CString)logmessage3.c_str(), output_log_file.c_str(), &log_counter, GUI_display, &wait_count_total);
								logmessage += logmessage2 + "\n" + logmessage3;
								sprintf(rating_classification, "Low  (@%5d)", outdtc.MaxFrame);
								sprintf(rating_filename_suffix, "low@%d", outdtc.MaxFrame);
								rating = Rating_type::Low;
							}
							else {
								nb_low_impact++;
								logmessage = "WARNING: " + std::to_string(nb_impact) + " low probability impact.";
								short_logmessage = logmessage;
								LogString((CString)logmessage.c_str(), output_log_file.c_str(), &log_counter, GUI_display, &wait_count_total);
								logmessage += "\n";
								sprintf(rating_classification, "Low          ");
								sprintf(rating_filename_suffix, "low");
								rating = Rating_type::Low;
							}
						}
						else {
							/* image detection failed */
							nb_null_impact++;
							logmessage = "No impact detected (too faint).";
							short_logmessage = logmessage;
							LogString((CString)logmessage.c_str(), output_log_file.c_str(), &log_counter, GUI_display, &wait_count_total);
							logmessage += "\n";
							sprintf(rating_classification, "Null         ");
							sprintf(rating_filename_suffix, "null");
							rating = Rating_type::Null;
						}
					}
				} // nb_impact = 0
				else if (distance == 1.0) {
					/* algorithm did not work */
					if ((max_mean_stat[2] - max_mean_stat[1]) < opts.impact_max_avg_min) {
						/* No impact, contrast threshold not respected */
						nb_null_impact++; 
						logmessage = "No impact detected by the algorithm.";
						short_logmessage = logmessage;
						LogString((CString)logmessage.c_str(), output_log_file.c_str(), &log_counter, GUI_display, &wait_count_total);
						logmessage += "\n";
						sprintf(rating_classification, "Null         ");
						sprintf(rating_filename_suffix, "null");
						rating = Rating_type::Null;
					}
					else {
						/* contrast threshold respected */
						nb_low_impact++;
						logmessage = "WARNING: low probability impact in detection image but no impact detected by the algorithm.";
						short_logmessage = logmessage;
						LogString((CString)logmessage.c_str(), output_log_file.c_str(), &log_counter, GUI_display, &wait_count_total);
						logmessage += "\n";
						sprintf(rating_classification, "Low          ");
						sprintf(rating_filename_suffix, "low");
						rating = Rating_type::Low;
					}
				} else { // distance < 9999
					nb_null_impact++;
					logmessage = "No impact detected by the algorithm.";
					short_logmessage = logmessage;
					LogString((CString)logmessage.c_str(), output_log_file.c_str(), &log_counter, GUI_display, &wait_count_total);
					logmessage += "\n";
					sprintf(rating_classification, "Null         ");
					sprintf(rating_filename_suffix, "null");
					rating = Rating_type::Null;
				}
				if (rating != Rating_type::Error) logmessage += "distance = " + std::to_string(distance) + "\nconfidence = " + std::to_string(confidence) + "\nmax-mean = " + std::to_string(max_mean_stat[2] - max_mean_stat[1]) + "\n";
				if (is_image_correct && (strlen(rating_filename_suffix) > 0)) {
					init_string(tmpstring);
					init_string(tmpstring2);
					sprintf(tmpstring, "_%s", rating_filename_suffix);
					strcpy_s(rating_filename_suffix, sizeof(rating_filename_suffix),  tmpstring);
					if (is_image_correct) {
						if (opts.detail) rename_replace(dtc_full_filename(opts.ofilename, DTC_DIFF_SUFFIX, detail_folder_path_string.c_str(), tmpstring), dtc_full_filename_2suffix(opts.ofilename, rating_filename_suffix, DTC_DIFF_SUFFIX, detail_folder_path_string.c_str(), tmpstring2), "details", __func__);
						rename_replace(dtc_full_filename(opts.ofilename, DTC_MEAN_SUFFIX, detection_folder_fullpathname_string.c_str(), tmpstring), dtc_full_filename_2suffix(opts.ofilename, rating_filename_suffix, DTC_MEAN_SUFFIX, detection_folder_fullpathname_string.c_str(), tmpstring2), "detection", __func__);
						rename_replace(dtc_full_filename(opts.ofilename, DTC_MAX_MEAN_SUFFIX, detection_folder_fullpathname_string.c_str(), tmpstring), dtc_full_filename_2suffix(opts.ofilename, rating_filename_suffix, DTC_MAX_MEAN_SUFFIX, detection_folder_fullpathname_string.c_str(), tmpstring2), "detection", __func__);
					}
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

				// Refreshes the impacts classification in dialog window
				CDeTeCtMFCDlg::getimpactNull()->SetWindowText(std::to_wstring(nb_null_impact + nb_error_impact).c_str());
				CDeTeCtMFCDlg::getimpactLow()->SetWindowText(std::to_wstring(nb_low_impact).c_str());
				CDeTeCtMFCDlg::getimpactHigh()->SetWindowText(std::to_wstring(nb_high_impact).c_str());

				begin_imagedisplay_time = clock();
				if ((nframe == 0) || !is_ROI_ok) {
					LogString(_T("Computation time: ") + (CString)(std::to_string(int(end - begin) / CLOCKS_PER_SEC).c_str()) + _T(" second") + (CString)s.c_str(), output_log_file.c_str(), &log_counter, GUI_display, &wait_count_total);
					begin_imagedisplay_time += wait_imagedisplay_seconds * 1000;
				}
				else {
					LogString(_T("Computation time: ") + (CString)(std::to_string(int(end - begin) / CLOCKS_PER_SEC).c_str()) + _T(" second") + (CString)s.c_str() + _T(", showing detection image")
						+ _T(" (automatically closed in ") + (CString)(std::to_string(wait_imagedisplay_seconds).c_str()) + _T(" second") + (CString)s.c_str() + _T(")..."), output_log_file.c_str(), &log_counter, GUI_display, &wait_count_total);

					if (opts.show_detect_image) {
						//cv::destroyWindow("Detection image");
						cv::imshow("Detection image", pADUdtcImg);
						cv::waitKey(1);
					}
					if (opts.show_mean_image) {
						//cv::destroyWindow("Mean image");
						cv::imshow("Mean image", pADUavgMat);
						cv::waitKey(1);
					}
				}
				pGryMat.~Mat();
				PreviousFrameMat[0].~Mat();
				PreviousFrameMat[1].~Mat();
				PreviousFrameMat[2].~Mat();
				pADUdarkMat.~Mat();
				pADUavgMat.~Mat();
				pADUmaxMat.~UMat();
				pADUmaxImg.~Mat();
				pADUavgImg.~Mat();
				pFlatADUmaxMat.~UMat();
				pADUdtcMat.~Mat();
				pADUdtcImg.~Mat();
				pADUdtcImg2.~Mat();


				char buffer5[MAX_STRING] = { 0 };
				sprintf_s(buffer5, MAX_STRING, "detect5:				opts    : %p	opts->ignore	:	%i\n", &opts, opts.ignore);
				OutputDebugStringA(buffer5);

				if (opts.ignore)
					dtcCorrectDatation((DtcCapture*)pCapture, &start_time, &end_time, &duration, &fps_real, &timetype, comment);
				std::string location = filename_acquisition.substr(0, filename_acquisition.find_last_of("\\") + 1);

				if (extension.compare(AUTOSTAKKERT_EXT) == 0) {
					if (opts.autostakkert) strcat_s(comment, sizeof(comment), ", from AS!");
					else strcat_s(comment, sizeof(comment), ", from .as3");
				}

				LogInfo info(opts.filename, start_time, end_time, duration, fps_real, timetype, comment, nb_impact, confidence, distance, mean_stat, mean2_stat, max_mean_stat, max_mean2_stat, diff_stat, diff2_stat, temporal_density, rating_classification, croi.width, croi.height);
				dtcWriteLog2(log_consolidated_directory, info, (pCapture->CaptureInfo), &logline_tmp, &wait_count_total);
				dtcWriteLog2(log, info, (pCapture->CaptureInfo), &logline_tmp, &wait_count_total);

				/*FINAL CLEANING**************************************/
				//if (opts.viewDif) cv::destroyWindow("Initial differential photometry");
				//if (opts.viewRef) cv::destroyWindow("Reference frame");
				//if (opts.viewROI) cv::destroyWindow("ROI");
				//if (opts.viewTrk) cv::destroyWindow("Tracking");
				//if (opts.viewMsk) cv::destroyWindow("Mask");
				//if (opts.viewThr) cv::destroyWindow("Thresholded differential photometry");
				//if (opts.viewSmo) cv::destroyWindow("Smoothed differential photometry");
				//if (opts.viewRes) cv::destroyWindow("Resulting differential photometry");
				//if (opts.viewHis) cv::destroyWindow("Histogram");
				//cv::destroyAllWindows();

				if (opts.thrWithMask || opts.viewMsk || ((strlen(opts.ovfname) > 0) && (opts.ovtype == OTYPE_MSK))) {
					pMskMat.~Mat();
					pMskImg.~UMat();
				}
				if (opts.filter.type >= 0 || opts.viewSmo) {
					pSmoMat.~Mat();
				}
				if (opts.viewTrk || ((opts.ovtype == OTYPE_TRK) && (strlen(opts.ovfname) > 0))) {
					pTrkMat.~Mat();
					pTrkImg.~UMat();
				}
				if (opts.viewDif || opts.viewRes || opts.viewHis || ((strlen(opts.ovfname) > 0) && (opts.ovtype == OTYPE_DIF ||
					opts.ovtype == OTYPE_HIS))) {
					pDifMat.~Mat();
				}
				pRefMat.~Mat();
				if (opts.viewHis || ((strlen(opts.ovfname) > 0) && (opts.ovtype == OTYPE_HIS))) pHisImg.~Mat();
				pFirstFrameROIMat.~Mat();
				pROIMat.~Mat();
				tempROIMat.~Mat();
				tempGryMat.~Mat();
				dtcReleaseCapture(pCapture);
				pCapture = NULL;

				char buffer6[MAX_STRING] = { 0 };
				sprintf_s(buffer6, MAX_STRING, "detect6:				opts    : %p	opts->ignore	:	%i\n", &opts, opts.ignore);
				OutputDebugStringA(buffer6);


				// acquition has been processed, increasing counter					
				//acquisitions_processed++;
				//totalProgress_wstring = L"Total\n(" + std::to_wstring(acquisitions_processed + acquisition_index_children) + L"/" + std::to_wstring(acquisitions_to_be_processed) + L")";
//if (opts.parent_instance) LogString(_T("2: parent / children / done / tobe = ") + (CString)(std::to_string(acquisitions_processed).c_str()) + (CString)(" / ") + (CString)(std::to_string(acquisition_index_children).c_str()) + (CString)(" / ") + (CString)(std::to_string(acquisitions_processed + acquisition_index_children).c_str()) + (CString)(" / ") + (CString)(std::to_string(acquisitions_to_be_processed).c_str()), output_log_file.c_str(), &log_counter, TRUE, &wait_count_total);
				//CDeTeCtMFCDlg::gettotalProgress()->SetWindowText(totalProgress_wstring.c_str());
				UpdateProgress(acquisitions_to_be_processed, acquisitions_processed, acquisition_index_children, nframe, frame_number, opts.DeTeCtQueueFilename);
				acquisitions_processed++;
			}

// ********************************************************************
// ******************* End of frame processing **********************
// ********************************************************************

			catch (std::exception& e) {
					std::string exception_message(e.what());
					LogString(L"ERROR: ", output_log_file.c_str(), &log_counter, GUI_display, &wait_count_total);
					LogString((CString)std::string(e.what()).c_str(), output_log_file.c_str(), &log_counter, GUI_display, &wait_count_total);
					logmessage = "ERROR: " + std::string(e.what());
					short_logmessage = logmessage;
					log_messages.push_back(short_filename + ":" + "    " + logmessage);
					char msgtext[MAX_STRING] = { 0 };
					snprintf(msgtext, MAX_STRING, "exception error: %s", std::string(e.what()).c_str());
					ErrorExit(TRUE, TRUE, "wrong header size", __func__, msgtext);
					rating = Rating_type::Error;
					nb_error_impact++;
					// log_messages.push_back("    " + logmessage);
			}
				//message = "--------- " + short_filename + " analysis done ---------";
			LogString(_T(""), output_log_file.c_str(), &log_counter, GUI_display, &wait_count_total);
			if (strlen(opts.DeTeCtQueueFilename) > 0) {
				CString objectname(opts.filename);
				CString objectname_cstring;
				if (filename_autostakkert.size() > 0) {
					objectname_cstring = filename_autostakkert.c_str();
				}
				else {
					objectname_cstring = objectname;
				}
				CString tag;
				//									"file_processing"
				if (opts.parent_instance)	tag =	"file_ok        ";
				else						tag =	"file_processed ";
				if (!opts.parent_instance && !filesys::exists(CString2string((CString)opts.DeTeCtQueueFilename))) dlg.OnFileExit(); 	// exits DeTeCt if Queuefile does not exists (removed at parent exit) for a child instance. Added because of difficulty to terminate children processes when exiting parent instance
				else {
if (opts.debug) LogString(_T("!Debug info: Setting processed file from queue"), output_log_file.c_str(), &log_counter, TRUE, &wait_count_total);
					SetProcessingFileProcessedFromQueue(objectname_cstring, _T("|") + objectname + _T("|") + (CString)short_logmessage.c_str() + _T("|") + (CString)std::to_string((int)rating).c_str() + _T("|") + (CString)std::to_string(duration).c_str() + _T("|") + (CString)std::to_string(nframe).c_str() + _T("|") + (CString)std::to_string(fps_int).c_str(), tag, (CString)opts.DeTeCtQueueFilename);
					if (opts.debug) LogString(_T("File processed : ") + objectname_cstring, output_log_file.c_str(), &log_counter, GUI_display, &wait_count_total);
				}
				// Display shortened message in case of multi instances mode 
				if ((!GUI_display) || (opts.debug)) {
					CDeTeCtMFCDlg::getLog()->AddString((CString)getDateTime().str().c_str() + "----- " + objectname_cstring + " -----");
					CDeTeCtMFCDlg::getLog()->AddString((CString)getDateTime().str().c_str() + std::to_string(nframe).c_str() + (CString)" frames @ " + std::to_string(fps_int).c_str() + (CString)" fps (" + std::to_string((int)duration).c_str() + (CString)"s duration)");
					CDeTeCtMFCDlg::getLog()->AddString((CString)getDateTime().str().c_str() + short_logmessage.c_str());
					CDeTeCtMFCDlg::getLog()->AddString((CString)getDateTime().str().c_str());
					CDeTeCtMFCDlg::getLog()->SetTopIndex(CDeTeCtMFCDlg::getLog()->GetCount() - 1);
					CDeTeCtMFCDlg::getLog()->RedrawWindow();
				}
			}
			DisplayProcessingTime(&computing_threshold_time, &end, computing_refresh_duration, begin, begin_total);
			
			if ((begin_imagedisplay_time > 0) && (acquisition_index < local_acquisition_files_list.acquisition_file_list.size()) && (wait_imagedisplay_seconds > 0)) {
				clock_t delay_left = (int)(begin_imagedisplay_time)+wait_imagedisplay_seconds * 1000 - clock();
				if (delay_left> 0) cv::waitKey(delay_left);
				if ((opts.show_detect_image) && (cv::getWindowProperty("Detection image", cv::WND_PROP_VISIBLE) > 0)) 	cv::destroyWindow("Detection image");
				if ((opts.show_mean_image)		&& (cv::getWindowProperty("Mean image", cv::WND_PROP_VISIBLE)  > 0)) 	cv::destroyWindow("Mean image");
				begin_imagedisplay_time = 0;
			}
		}

		if (strlen(opts.dirname) > 0) {
			CString objectname(opts.dirname);
			RemoveFileFromQueue(objectname, (CString) opts.DeTeCtQueueFilename, NULL, TRUE);
			if (opts.debug) LogString(L"File directory removed: " + objectname, output_log_file.c_str(), &log_counter, TRUE, &wait_count_total);
		}
		local_acquisition_files_list.file_list = std::vector<std::string>();
		local_acquisition_files_list.acquisition_file_list = std::vector<std::string>();
		local_acquisition_files_list.nb_prealigned_frames = {};
		local_acquisition_files_list.acquisition_size = {};

		char buffer7[MAX_STRING] = { 0 };
		sprintf_s(buffer7, MAX_STRING, "detect7:				opts    : %p	opts->ignore	:	%i\n", &opts, opts.ignore);
		OutputDebugStringA(buffer7);

		if (!opts.parent_instance && !opts.autostakkert && (NbPossibleChildInstances_fromMemoryandCPUUsage()<0)) dlg.OnFileExit(); //exit DeTeCt child instance if memory usage too high

// ***********************************************************************
// ******************** Looks for new job in queue ***********************
// ***********************************************************************

//		if (!popts->interactive) {
		if (local_acquisition_files_list.file_list.size() == 0) {
			std::string waiting_message = "";
			if (NbItemFromQueue(_T("file"), (CString)opts.DeTeCtQueueFilename, NULL, TRUE) == 0) {
				if ((opts.parent_instance) && (opts.autostakkert) && (IsProcessRunning(opts.autostakkert_PID))) { // if no more files to compute while autostakkert still running
					waiting_message = " checking for files to process, CLOSE AUTOSTAKKERT WHEN DONE then DETECT WILL CLOSE AUTOMATICALLY!\n";
					UpdateProgress(acquisitions_to_be_processed, acquisitions_processed, acquisition_index_children, 0, 0, opts.DeTeCtQueueFilename);
					if (logmessage3.size() > 0) message_cstring = message_cstring + (CString)"\n" + (CString)logmessage3.c_str(); // ???
				} else if ((opts.parent_instance) && (ChildrenProcessesNumber() > 0)) // normal mode with instances still running
					waiting_message =	waiting_message + "checking for other DeTeCt process(es) running to finish ...\n";
			} else waiting_message =	waiting_message + "checking new files to be processed ...\n"; // still files to compute
			if (waiting_message.size() > 1) {
				LogString((CString)"PLEASE WAIT, " + (CString)waiting_message.c_str(), output_log_file.c_str(), &log_counter, TRUE, &wait_count_total);
				CDeTeCtMFCDlg::getfileName()->SetWindowText((CString)waiting_message.c_str());
			}
		}
		BOOL QueueListEmpty			= FALSE;
		int total_sleep_duration	= queue_scan_delay;	// wait (in ms)
		if (queue_scan_delay_random_max  >0 ) total_sleep_duration += rand() % queue_scan_delay_random_max; 	
		int sleep_duration			= 0;

		if (opts.parent_instance) { 
			nb_instances = 0;
			DisplayInstanceType(&nb_instances); // Display number of instances only if files processed (Forks does the display) and if not child instance
		}
		// looks for new job for a child only if not too many children
		int nbChildren = ParentChildrenProcessesNumber(MAX(opts.detect_PID, opts.autostakkert_PID));
		maxinstances_previous = opts.maxinstances;
		if (filesys::exists(CString2string((CString)opts.DeTeCtQueueFilename))) opts.maxinstances = GetIntParamFromQueue(_T("max_instances"), (CString)opts.DeTeCtQueueFilename);
if (opts.debug) LogString(_T("!Debug info: Check queue: parent=") + (CString)std::to_string(opts.parent_instance).c_str() + _T(" \nmax=") + (CString)std::to_string(opts.maxinstances).c_str() + _T(" \nnb children=") + (CString)std::to_string(nbChildren).c_str() + _T(" \nDeTeCt PID=") + (CString)std::to_string(opts.detect_PID).c_str() + _T(" \nAS PID=") + (CString)std::to_string(opts.autostakkert).c_str() + " " + (CString)std::to_string(opts.autostakkert_PID).c_str(), output_log_file.c_str(), &log_counter, GUI_display, &wait_count_total);


// ****** MAIN WAITING LOOP *** //
// **************************** //
		if ((!((!opts.parent_instance) && ((opts.maxinstances - nbChildren - 1) < 0))) && (local_acquisition_files_list.file_list.size() == 0)) do {
			if ((opts.parent_instance) && (strlen(opts.DeTeCtQueueFilename) > 0)) { // get other processed files by other instances

			// ******************** Looks for other processed files*****************//
			// *********************************************************************//
				double duration_total_others = 0;
				if (opts.maxinstances > 1) {
//if (opts.parent_instance) LogString(_T("7a: parent / children / done / tobe = ") + (CString)(std::to_string(acquisitions_processed).c_str()) + (CString)(" / ") + (CString)(std::to_string(acquisition_index_children).c_str()) + (CString)(" / ") + (CString)(std::to_string(acquisitions_processed + acquisition_index_children).c_str()) + (CString)(" / ") + (CString)(std::to_string(acquisitions_to_be_processed).c_str()), output_log_file.c_str(), &log_counter, TRUE, &wait_count_total);
					int nb_processed_files = (GetOtherProcessedFiles(acquisitions_processed, &acquisition_index_children, &acquisitions_to_be_processed, &nb_error_impact, &nb_null_impact, &nb_low_impact, &nb_high_impact, &duration_total_others, &log_messages, opts.DeTeCtQueueFilename, &computing_threshold_time, &end, computing_refresh_duration, begin, begin_total, 0, 1));
if ((opts.parent_instance) && (strlen(opts.DeTeCtQueueFilename) > 0)) {  //Update correctly figure if children instances ignore some files
	acquisitions_to_be_processed = NbFilesFromQueue((CString)opts.DeTeCtQueueFilename) - NbItemFromQueue(_T("file_ko"), (CString)opts.DeTeCtQueueFilename, NULL, TRUE);
	//totalProgress_wstring = L"Total\n(" + std::to_wstring(acquisitions_processed + acquisition_index_children) + L"/" + std::to_wstring(acquisitions_to_be_processed) + L")";
	//CDeTeCtMFCDlg::gettotalProgress()->SetWindowText(totalProgress_wstring.c_str());
	UpdateProgress(acquisitions_to_be_processed, acquisitions_processed, acquisition_index_children, 0, 0, opts.DeTeCtQueueFilename);

}

//if (opts.parent_instance) LogString(_T("7b: parent / children / done / tobe = ") + (CString)(std::to_string(acquisitions_processed).c_str()) + (CString)(" / ") + (CString)(std::to_string(acquisition_index_children).c_str()) + (CString)(" / ") + (CString)(std::to_string(acquisitions_processed + acquisition_index_children).c_str()) + (CString)(" / ") + (CString)(std::to_string(acquisitions_to_be_processed).c_str()), output_log_file.c_str(), &log_counter, TRUE, &wait_count_total);

					if (nb_processed_files > 0) {
						if (opts.debug) LogString(L"File(s) processed fetched: " + (CString)std::to_string(nb_processed_files).c_str(), output_log_file.c_str(), &log_counter, TRUE, &wait_count_total);
						duration_total += duration_total_others;
						CDeTeCtMFCDlg::getduration()->SetWindowText((CString)"Duration processed (total): " + std::to_wstring((int)duration_total).c_str() + (CString)"s");
						if ((opts.maxinstances > 1) && (!filesys::exists(CString2string((CString)opts.DeTeCtQueueFilename))))  AcquisitionFileListToQueue(&local_acquisition_files_list, _T("file_ok"), acquisition_index - 1, (CString)log.c_str(), &acquisitions_to_be_processed);
						float CPULoad = GetCPULoad(FALSE);
						nb_new_instances = ForksInstances(opts.maxinstances, ASorDeTeCtPID(opts.autostakkert_PID, opts.detect_PID), (CString)opts.DeTeCtQueueFilename, queue_scan_delay, queue_scan_delay_random_max, &nb_instances);
						if (nb_new_instances > 1)		LogString((CString)std::to_string(nb_new_instances).c_str() + _T(" new instances launched (") + (CString)std::to_string(nb_instances).c_str() + _T(" in total)") + _T(" (") + (CString)std::to_string((int)(100 - CPULoad * 100)).c_str() + _T("% CPU available)"), output_log_file.c_str(), &log_counter, TRUE, &wait_count_total);
						else if (nb_new_instances == 1) LogString((CString)std::to_string(nb_new_instances).c_str() + _T(" new instance launched (") + (CString)std::to_string(nb_instances).c_str() + _T(" in total)") + _T(" (") + (CString)std::to_string((int)(100 - CPULoad * 100)).c_str() + _T("% CPU available)"), output_log_file.c_str(), &log_counter, TRUE, &wait_count_total);
						else if (opts.parent_instance) { //&&((popts->maxinstances > maxinstances_previous) || (nb_instances < popts->maxinstances))) {
							nb_instances = 0;
							DisplayInstanceType(&nb_instances);
						}
						if ((opts.parent_instance) && (opts.autostakkert) && (IsProcessRunning(opts.autostakkert_PID))) {		// if no more files to compute while autostakkert still running, message and progress bar update
							acquisitions_to_be_processed += nb_processed_files;
							//progress_all_status = MAX_RANGE_PROGRESS * ((float) (acquisitions_processed + acquisition_index_children) / (float) acquisitions_to_be_processed);
							//CDeTeCtMFCDlg::getProgress_all()->SetPos((short)(progress_all_status));
							//CDeTeCtMFCDlg::getProgress_all()->UpdateWindow();
															
							std::string waiting_message = "DO NOT CLOSE this window - checking for files to process,\n\n CLOSE AUTOSTAKKERT WHEN DONE";
							LogString((CString)"PLEASE WAIT, " + (CString)waiting_message.c_str(), output_log_file.c_str(), &log_counter, TRUE, &wait_count_total);
							UpdateProgress(acquisitions_to_be_processed, acquisitions_processed, acquisition_index_children, 0, 0, opts.DeTeCtQueueFilename);
						}
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
			if ((begin_imagedisplay_time > 0) && (wait_imagedisplay_seconds > 0)){
				int wait_time_rest = (int)(begin_imagedisplay_time)+wait_imagedisplay_seconds * 1000 - clock();
				if (wait_time_rest < 0) { // stop displaying image as wait time is over
					begin_imagedisplay_time = 0;
					//if (opts.show_detect_image) cv::destroyWindow("Detection image"); 
					//if (opts.show_mean_image)	cv::destroyWindow("Mean image");
					cv::destroyAllWindows();
				}
			}
			CString objectname;
			QueueListEmpty = FALSE;
			if (!opts.parent_instance && (!filesys::exists(CString2string((CString)opts.DeTeCtQueueFilename)) || !IsProcessRunning(opts.detect_PID))) dlg.OnFileExit(); 	// exits DeTeCt child instance if Queuefile does not exists or parent not running. Added because of difficulty to terminate children processes when exiting parent instance
			
			// ******************** Looks for new file to process*******************//
			// *********************************************************************//
if (opts.debug) LogString(_T("!Debug info: Getting file from queue"), output_log_file.c_str(), &log_counter, TRUE, &wait_count_total);
			if (GetFileFromQueue(&objectname, (CString) opts.DeTeCtQueueFilename)) {
				LogString(_T("File fetched : ") + objectname, output_log_file.c_str(), &log_counter, TRUE, &wait_count_total);
				std::ifstream file_stream(objectname);
				if (file_stream) {
					CT2A objectnamechar(objectname);
					strcpy_s(opts.filename, objectnamechar);
					file_stream.close();
					std::wstringstream ss;
					std::string file;
					std::string filename_acquisition;
					int nframe = -1;
					PIPPInfo pipp_info;

					file = std::string(opts.filename);
					std::string extension = file.substr(file.find_last_of(".") + 1, file.size() - file.find_last_of(".") - 1);

					if ((Is_Capture_OK_from_File(file, &filename_acquisition, &nframe, &ss)) &&
						// ********* Error if acquisition has not enough frames
						(Is_Capture_Long_Enough(file, nframe, &ss)) &&
							// ********* Ignores dark, pipp, winjupos derotated files
							(!Is_Capture_Special_Type(file, &ss)) &&
								// ********* Ignores PIPP with no integrity
								(!Is_PIPP(file) || ((Is_PIPP(file) && Is_PIPP_OK(file, &pipp_info, &ss)))) &&
									// ***** if option noreprocessing on, check in detect log file if file already processed or processed with in datation only mode
									(Is_CaptureFile_To_Be_Processed(filename_acquisition, log_consolidated_directory, &ss))) {
										// ********* Finally adds file to the list !
										// Set-up global variable
										scan_folder_path = file.substr(0, file.find_last_of("\\"));
										local_acquisition_files_list.file_list.push_back(std::string(opts.filename));
										file = file.substr(file.find_last_of("\\") + 1, file.length());
										if (extension.compare(AUTOSTAKKERT_EXT) != 0) {
											local_acquisition_files_list.acquisition_file_list.push_back(std::string(opts.filename));
											local_acquisition_files_list.acquisition_size.push_back(filesize(opts.filename));
											//ss << "Adding " << file.c_str() << " (" << local_acquisition_files_list.acquisition_size.at(0) / MEGABYTES << "MB in " << scan_folder_path.c_str() << ") for analysis\n";
											ss << "Adding " << file.c_str() << " (in " << scan_folder_path.c_str() << ") for analysis\n";
										}
										else {
											local_acquisition_files_list.acquisition_file_list.push_back(std::string(filename_acquisition));
											local_acquisition_files_list.acquisition_size.push_back(filesize(filename_acquisition.c_str()));
											//ss << "Adding " << file.c_str() << " (" << local_acquisition_files_list.acquisition_size.at(0) / MEGABYTES << "MB acquisition file " << filename_acquisition.c_str() << " in " << scan_folder_path.c_str() << ") for analysis\n";
											ss << "Adding " << file.c_str() << " (acquisition file " << filename_acquisition.c_str() << " in " << scan_folder_path.c_str() << ") for analysis\n";
										}
										local_acquisition_files_list.nb_prealigned_frames.push_back(nframe);
										local_acquisition_files_list.acquisition_size.push_back(filesize(opts.filename));
										CDeTeCtMFCDlg::getfileName()->SetWindowText(L"");
										acquisition_index = 0;
					}
					else
						RemoveItemsFromQueue(objectname, (CString)"file_processing", (CString)opts.DeTeCtQueueFilename, NULL, TRUE);
					// Prints message
					LogString((CString)ss.str().c_str(), output_log_file.c_str(), &log_counter, TRUE, &wait_count_total);
				}
				else {
					DIR *folder_object;
					CT2A objectnamechar(objectname);
					if (folder_object = opendir(objectnamechar)) {
						strcpy_s(opts.dirname, objectnamechar);
						std::string path = std::string(opts.dirname);
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
								PIPPInfo pipp_info;

								if (
									(Is_Capture_OK_from_File(filename_folder, &filename_acquisition, &nframe, &ss3)) &&
									// ********* Error if acquisition has not enough frames
									(Is_Capture_Long_Enough(filename_folder, nframe, &ss3)) &&
									// ********* Ignores dark, pipp, winjupos derotated files
									(!Is_Capture_Special_Type(filename_folder, &ss3)) &&
									// ********* Ignores PIPP with no integrity
									(!Is_PIPP(filename_folder) || ((Is_PIPP(filename_folder) && Is_PIPP_OK(filename_folder, &pipp_info, &ss3)))) &&
									// ***** if option noreprocessing on, check in detect log file if file already processed or processed with in datation only mode
									(Is_CaptureFile_To_Be_Processed(filename_acquisition, log_consolidated_directory, &ss3))
									) {
												// ********* Finally adds file to the list !
									std::string extension		= filename_folder.substr(filename_folder.find_last_of(".") + 1, filename_folder.size() - filename_folder.find_last_of(".") - 1);
									std::string filename_path	= filename_folder.substr(0, filename_folder.find_last_of("\\"));
									std::string file			= filename_folder.substr(filename_folder.find_last_of("\\") + 1, filename_folder.length());
									if (extension.compare(AUTOSTAKKERT_EXT) != 0) {
										//ss3 << "Adding " << file.c_str() << " (" << local_acquisition_files_list.acquisition_size.at(index) / MEGABYTES << "MB in " << filename_path.c_str() << ") for analysis\n";
										ss3 << "Adding " << file.c_str() << " (in " << filename_path.c_str() << ") for analysis\n";
									}
									else {
										//ss3 << "Adding " << file.c_str() << " (" << local_acquisition_files_list.acquisition_size.at(index) / MEGABYTES << "MB acquisition file " << filename_acquisition.c_str() << " in " << filename_path.c_str() << ") for analysis\n";
										ss3 << "Adding " << file.c_str() << " (acquisition file " << filename_acquisition.c_str() << " in " << filename_path.c_str() << ") for analysis\n";
									}
									index++;
								}
								else {
									local_acquisition_files_list.file_list.erase(local_acquisition_files_list.file_list.begin() + index);
									local_acquisition_files_list.acquisition_file_list.erase(local_acquisition_files_list.acquisition_file_list.begin() + index);
									local_acquisition_files_list.nb_prealigned_frames.erase(local_acquisition_files_list.nb_prealigned_frames.begin() + index); // WARNING in debug, error in .begin()
									local_acquisition_files_list.acquisition_size.erase(local_acquisition_files_list.acquisition_size.begin() + index);
								}
								LogString((CString)ss3.str().c_str(), output_log_file.c_str(), &log_counter, TRUE, &wait_count_total);
							}
						}

					}
				}
				file_stream.close();
			}
			else QueueListEmpty = TRUE;
if (opts.debug) LogString(
				_T("File list size=") + (CString)std::to_string(local_acquisition_files_list.file_list.size()).c_str() +
				_T(", QueueListEmpty=") + (CString)std::to_string(QueueListEmpty).c_str() + 
				_T(", parent=") + (CString)std::to_string(opts.parent_instance).c_str() +
				_T(", AS_PID=") + (CString)std::to_string(opts.autostakkert_PID).c_str() +
				_T(", AS=") + (CString)std::to_string(opts.autostakkert).c_str() +
				_T(", Is AS Running=") + (CString)std::to_string(IsProcessRunning(opts.autostakkert_PID)).c_str() +
				_T(", # of Children=") + (CString)std::to_string(ChildrenProcessesNumber()).c_str() +
				_T(", exit=") + (CString)std::to_string(opts.autoexit).c_str(),
				output_log_file.c_str(), &log_counter, TRUE, &wait_count_total);
		} while		((local_acquisition_files_list.file_list.size() == 0) &&																		// Waiting mode if no file in the list
						((!QueueListEmpty) ||																										//		and queue still not empty
							((!((!opts.parent_instance) && (opts.autostakkert) && (opts.autostakkert_PID > 0))) &&									//		or not AutoStakkert child 
								((opts.parent_instance)	&& (((opts.autostakkert) && (IsProcessRunning(opts.autostakkert_PID)))						//			and parent instance and AutoStakkert parent still running
															|| ((!opts.autostakkert) && (ChildrenProcessesNumber() > 0)))							//				or normal parent instance with other instances of DeTeCt running
								)
							)
						)
					);
//		}
		if ((begin_imagedisplay_time > 0) && (wait_imagedisplay_seconds > 0)) {
			int wait_time_rest = (int)(begin_imagedisplay_time)+wait_imagedisplay_seconds * 1000 - clock();
			if (wait_time_rest > 0) {
				if ((opts.show_detect_image) || (opts.show_mean_image))	cv::waitKey(wait_time_rest);  // wait time left before wait time defined, before stopping displaying image
				begin_imagedisplay_time = 0;
				if ((opts.show_detect_image)	&& (cv::getWindowProperty("Detection image", cv::WND_PROP_VISIBLE) > 0)) 	cv::destroyWindow("Detection image");
				if ((opts.show_mean_image)		&& (cv::getWindowProperty("Mean image", cv::WND_PROP_VISIBLE) > 0)) 		cv::destroyWindow("Mean image");
			}
		}
	} while (local_acquisition_files_list.file_list.size() > 0);


// **************************************************************************
// ******************** End of acquisitions processing **********************
// **************************************************************************

	if (opts.debug) LogString(_T("!Debug info: Ends"), output_log_file.c_str(), &log_counter, TRUE, &wait_count_total);

	std::string message = "Processing done, now finishing session, please wait."; 
	LogString((CString)message.c_str(), output_log_file.c_str(), &log_counter, TRUE, &wait_count_total);

	// Last update of file counts with actual figures
	UpdateProgress(acquisitions_to_be_processed, acquisitions_processed, acquisition_index_children, 0, 0, opts.DeTeCtQueueFilename);
	
	//if (opts.parent_instance) LogString(_T("3: parent / children / done / tobe = ") + (CString)(std::to_string(acquisitions_processed).c_str()) + (CString)(" / ") + (CString)(std::to_string(acquisition_index_children).c_str()) + (CString)(" / ") + (CString)(std::to_string(acquisitions_processed + acquisition_index_children).c_str()) + (CString)(" / ") + (CString)(std::to_string(acquisitions_to_be_processed).c_str()), output_log_file.c_str(), &log_counter, TRUE, &wait_count_total);
	if (opts.parent_instance) {
		nb_instances = 0;
		DisplayInstanceType(&nb_instances); // Display number of instances only if files processed (Forks does the display) and if not child instance
/*		LogString(_T("wait_imagedisplay_seconds   (s) = ") + (CString)std::to_string(wait_imagedisplay_seconds).c_str(), output_log_file.c_str(), &log_counter, TRUE, &wait_count_total);
		LogString(_T("check_children_time_factor      = ") + (CString)std::to_string(check_children_time_factor).c_str(), output_log_file.c_str(), &log_counter, TRUE, &wait_count_total);
		LogString(_T("update_count               (ms) = ") + (CString)std::to_string(update_count).c_str(), output_log_file.c_str(), &log_counter, TRUE, &wait_count_total);
		LogString(_T("display_update_duration    (ms) = ") + (CString)std::to_string(display_update_duration).c_str(), output_log_file.c_str(), &log_counter, TRUE, &wait_count_total);
		LogString(_T("processing_update_duration (ms) = ") + (CString)std::to_string(processing_update_duration).c_str(), output_log_file.c_str(), &log_counter, TRUE, &wait_count_total);
		LogString(_T("instances_update_duration  (ms) = ") + (CString)std::to_string(instances_update_duration).c_str(), output_log_file.c_str(), &log_counter, TRUE, &wait_count_total);
		*/
	}
	DisplayProcessingTime(&computing_threshold_time, &end, computing_refresh_duration, begin, begin_total);
	message = "Total processing time:   " + std::to_string((int)((end - begin_total) / (double)CLOCKS_PER_SEC)) + "s";
	LogString((CString)message.c_str(), output_log_file.c_str(), &log_counter, TRUE, &wait_count_total);

	///processing_time_str.Format(L"Processing time: %.*fs (file)  %.*fs (%s)"
	///message = "Processing done, now finishing session, please wait.";
	//LogString((CString)message.c_str(), output_log_file.c_str(), &log_counter, TRUE, &wait_count_total);

	begin_imagedisplay_time = 0;
	if (wait_imagedisplay_seconds > 0) {
		cv::destroyAllWindows();
		//if (opts.show_detect_image)	cv::destroyWindow("Detection image");
		//if (opts.show_mean_image)	cv::destroyWindow("Mean image");
	}
	// ******************* end of processing configuration
	//move to the end
	/*	if ((opts.parent_instance) && (opts.autostakkert) && (!IsProcessRunning(opts.autostakkert_PID))) {
		opts.autostakkert = FALSE;
		opts.autostakkert_PID = 0;
		dlg.execAS.SetCheck(false);
		LogString(L"Automatic execution from parent AutoStakkert terminated", output_log_file.c_str(), &log_counter, TRUE, &wait_count_total);
	}*/

	// * delete process queue if parent instance *
	if ((opts.parent_instance) && (strlen(opts.DeTeCtQueueFilename) > 0)) {
		//UnlockQueue((CString)opts.DeTeCtQueueFilename); //new queue method
		remove(opts.DeTeCtQueueFilename);
		strcpy_s(opts.DeTeCtQueueFilename, sizeof(opts.DeTeCtQueueFilename), "");
	}
	//LogString(_T(""), output_log_file.c_str(), &log_counter, TRUE, &wait_count_total);

	if (acquisitions_processed > 0) {
		// * defines new name for logfile in impact_detection directory 
		double second_min, second_max;
		int minute_min, minute_max;
		int hour_min, hour_max;
		int day_min, day_max;
		int month_min, month_max;
		int year_min, year_max;
		char suffix_char[MAX_STRING]				= { 0 };
		char consolidated_suffix_char[MAX_STRING]	= { 0 };
		char planet_char[MAX_STRING]				= { 0 };

		if (planet_jupiter > 0)	strcat_s(planet_char, sizeof(planet_char), "_jupiter");
		if (planet_saturn > 0)	strcat_s(planet_char, sizeof(planet_char), "_saturn");

		jd_to_date(start_time_min, &second_min, &minute_min, &hour_min, &day_min, &month_min, &year_min);
		jd_to_date(start_time_max, &second_max, &minute_max, &hour_max, &day_max, &month_max, &year_max);
		sprintf(suffix_char, "%s_%04d%02d%02d_%02d%02d", planet_char, year_min, month_min, day_min, hour_min, minute_min);
		if ((day_min != day_max) || (month_min != month_max) || (year_min != year_max)) sprintf(suffix_char, "%s-%04d%02d%02d_%02d%02d", suffix_char, year_max, month_max, day_max, hour_max, minute_max);
		else if ((hour_min != hour_max) || (minute_min != minute_max) || (second_min != second_max)) sprintf(suffix_char, "%s-%02d%02d", suffix_char, hour_max, minute_max);
		//CT2A detection_folder_name_char(CString(detection_folder_name_string.c_str()));
		sprintf(consolidated_suffix_char, "%s_%s.log", suffix_char, detection_folder_name_string.c_str());
		sprintf(suffix_char, "%s.log", suffix_char);

		message = "Total duration analyzed: ";
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

		char tmpchar[MAX_STRING] = { 0 };
		LogString((CString)message.c_str(), output_log_file.c_str(), &log_counter, TRUE, &wait_count_total);
		//LogString(L"In " + (CString)(log_consolidated_directory.c_str()) + L", please find:", output_log_file.c_str());
		//LogString(L" * log file " + (CString)(left(DeTeCtFileName(tmpchar), InRstr(DeTeCtFileName(tmpchar), "."), tmpchar)) + DTC_LOG_SUFFIX, output_log_file.c_str());
		//LogString(L" * folder " + (CString)(right(detection_folder_fullpathname_string.c_str(), strlen(detection_folder_fullpathname_string.c_str()) - InRstr(detection_folder_fullpathname_string.c_str(), "\\") - 1, tmpchar)) + L" for checking images", output_log_file.c_str());

		if (opts.dateonly) LogString(L"WARNING, datation info only, no detection analysis was performed\n", output_log_file.c_str(), &log_counter, TRUE, &wait_count_total);

		CT2A LogOrgFilename(DeTeCt_additional_filename_from_folder(CString(log.c_str()), DTC_LOG_SUFFIX));
		CT2A LogNewFilename(DeTeCt_additional_filename_from_folder(CString(log.c_str()), (CString)suffix_char));
		CT2A LogConsolidatedNewFilename(DeTeCt_additional_filename_from_folder(CString(log_consolidated_directory.c_str()), (CString)consolidated_suffix_char));

		CT2A tmp_log_detection_dirname(DeTeCt_additional_filename_from_folder(_T(""), (CString)suffix_char));
		strcpy_s(log_detection_dirname, sizeof(log_detection_dirname),  tmp_log_detection_dirname);

		CT2A OutOrgFilename2(CString(log.c_str()) + L"\\" + OUTPUT_FILENAME + DTC_LOG_SUFFIX);
		CT2A OutNewFilename2(CString(log.c_str()) + L"\\" + OUTPUT_FILENAME + (CString)suffix_char);

		if (opts.dateonly) LogString(L"WARNING, datation info only, no detection analysis was performed\n", output_log_file.c_str(), &log_counter, TRUE, &wait_count_total);
		LogString(_T(""), output_log_file.c_str(), &log_counter, TRUE, &wait_count_total);

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
		if (wait_count_total>0) LogString(L"Log files waiting times: " + (CString)std::to_string(wait_count_total).c_str() + L" times = " + (CString)std::to_string(wait_count_total * FILEACCESS_WAIT_MS).c_str() + L" ms", output_log_file.c_str(), &log_counter, TRUE, &wait_count_total);
		message = message + "\n" + "CHECK and SEND the RESULTS to:   delcroix.marc@free.fr   NO DETECTION also MATTERS!\n";
		CDeTeCtMFCDlg::getfileName()->SetWindowText((CString)message.c_str());

		for (std::string msg : log_messages) {
			std::wstring wmsg = std::wstring(msg.begin(), msg.end());
			CString Cmsg = CString(wmsg.c_str(), (int)wmsg.length());
			LogString(Cmsg, output_log_file.c_str(), &log_counter, TRUE, &wait_count_total);
		}
		log_messages.clear();

		// renames logs and shows links, creates zip if parent instance
		char item_to_be_zipped_shortname[MAX_STRING]	= { 0 };
		if ((opts.parent_instance) || (instance_type == Instance_type::single) ||(instance_type == Instance_type::autostakkert_single)) {
			dtcSortLog(LogOrgFilename, LogNewFilename);
			if (filesys::exists(CString2string((CString)LogNewFilename)) && filesys::exists(CString2string((CString)LogNewFilename))) remove(LogOrgFilename);

			CString LogNewFilename_cstring;
			CString LogConsolidatedNewFilename_cstring;
			char2CString(LogNewFilename, &LogNewFilename_cstring);
			char2CString(LogConsolidatedNewFilename, &LogConsolidatedNewFilename_cstring);
			duplicate_txtfile(LogNewFilename_cstring, LogConsolidatedNewFilename_cstring);

//			CDeTeCtMFCDlg::getdetectLoglink()->SetURL((CString)log_consolidated_directory.c_str() + _T("\\") + DeTeCt_additional_filename_from_folder(_T(""), DTC_LOG_SUFFIX));
			CDeTeCtMFCDlg::getdetectLoglink()->SetURL((CString)LogNewFilename);
			dlg.EnableLogLink(TRUE);
			dlg.GetDlgItem(IDC_MFCLINK_DETECTLOG)->SetWindowTextW(_T("Detection log"));

			CDeTeCtMFCDlg::getdetectImageslink()->SetURL((CString)detection_folder_fullpathname.c_str());
			dlg.EnableImagesLink(TRUE);
			dlg.GetDlgItem(IDC_MFCLINK_DETECTIMAGES)->SetWindowTextW(_T("Detection images to check"));

			// *************** creates zip file ***********/
			strcpy_s(zipfile, sizeof(zipfile), "\0");
			if ((opts.zip) && (!opts.dateonly) && (!dev_mode)) {
				strcpy_s(zip_detection_location, sizeof(zip_detection_location), zipfile);
				strcpy_s(opts.zipname, sizeof(opts.zipname), zipfile);
				char item_to_be_zipped[MAX_STRING] = { 0 };

				strcat_s(item_to_be_zipped, sizeof(item_to_be_zipped), detection_folder_fullpathname_string.c_str());		// eg "G:\\work\\Impact\\tests\\autostakkert3.2_detect\\Impact_detection_run@2020-05-08_00-39-34"
				strcat_s(item_to_be_zipped, sizeof(item_to_be_zipped), "\0");

				strcat_s(item_to_be_zipped_shortname, sizeof(item_to_be_zipped_shortname), detection_folder_name_string.c_str());
				strcat_s(item_to_be_zipped_shortname, sizeof(item_to_be_zipped_shortname), ".zip");
				strcat_s(item_to_be_zipped_shortname, sizeof(item_to_be_zipped_shortname), "\0");
				strcpy_s(opts.zipname, sizeof(opts.zipname), item_to_be_zipped_shortname);
				strcat_s(zipfile, sizeof(zipfile), detection_folder_fullpathname_string.c_str());					// eg "G:\\work\\Impact\\tests\\autostakkert3.2_detect\\Impact_detection_run@2020-05-08_00-39-34"
				if (strlen(planet_char) > 0) strcat_s(zipfile, sizeof(zipfile), planet_char);						// eg "G:\\work\\Impact\\tests\\autostakkert3.2_detect\\Impact_detection_run@2020-05-08_00-39-34_jupiter"
				strcat_s(zipfile, sizeof(zipfile), ".zip");
				strcat_s(zipfile, sizeof(zipfile), "\0");															// eg "G:\\work\\Impact\\tests\\autostakkert3.2_detect\\Impact_detection_run@2020-05-08_00-39-34_jupiter.zip"
				strcpy_s(zip_detection_location, sizeof(zip_detection_location), log_consolidated_directory.c_str());				// eg "G:\\work\\Impact\\tests\\autostakkert3.2_detect"

				CDeTeCtMFCDlg::getfileName()->SetWindowText(L"Creating zip file, please wait ...");
				LogString(_T(""), output_log_file.c_str(), &log_counter, TRUE, &wait_count_total);
				LogString(L"Creating zip file, please wait ...", output_log_file.c_str(), &log_counter, TRUE, &wait_count_total);
				LogString(L"(last line of output log in Impact_detection Zip file)", output_log_file.c_str(), &log_counter, FALSE, &wait_count_total);
				zip(zipfile, item_to_be_zipped, output_log_file.c_str(), &log_counter);
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

					LogString(L"Zip file " + (CString)(right(zipfile, strlen(zipfile) - InRstr(zipfile, "\\") - 1, tmpchar)) + L" created", output_log_file.c_str(), &log_counter, TRUE, &wait_count_total);
					message = "(Click) check detection images and send email with zip file!\n";
					CDeTeCtMFCDlg::getfileName()->SetWindowText((CString) message.c_str());
				}
				else if (!opts.dateonly) {
					dlg.EnableZipLink(FALSE);
					LogString(L"ERROR: zip file " + (CString)(right(zipfile, strlen(zipfile) - InRstr(zipfile, "\\") - 1, tmpchar)) + L" not created!", output_log_file.c_str(), &log_counter, TRUE, &wait_count_total);
					message = "(Click) Check detection images and send email with log!\n";
					CDeTeCtMFCDlg::getfileName()->SetWindowText((CString)message.c_str());
				}
				filetest.close();
			} //end of zipfile creation
			CWnd *resultsbtn = dlg.GetDlgItem(IDC_BUTTON_CHECKRESULTS);
			if (resultsbtn) {
				resultsbtn->EnableWindow(TRUE);
			}
		}

		if (opts.parent_instance) {
			log_messages.push_back("");
			log_messages.push_back("Click \"Check detection images...\" button to open in \"" + log_consolidated_directory + "\" :");
			log_messages.push_back(" - an explorer in \"" + detection_folder_name_string + "\" to check the detection images");
			std::ifstream filetest(zipfile);
			if (filetest) {
				std::string stritem_to_be_zipped_shortname(item_to_be_zipped_shortname);
				log_messages.push_back(" - an explorer where \"" + stritem_to_be_zipped_shortname + "\" to be sent by email is  (along with " + left(DeTeCtFileName(tmpchar), InRstr(DeTeCtFileName(tmpchar), "."), tmpchar) + " log)");
				if (!opts.dateonly)  
					if (opts.email) log_messages.push_back(" - an email to send the results by attaching \"" + stritem_to_be_zipped_shortname + "\" file");
					else log_messages.push_back("Please send an email with the results by attaching \"" + stritem_to_be_zipped_shortname + "\" file");
			}
			else {
				std::string strlog_detection_dirname(log_detection_dirname);
				if (!opts.dateonly)
					if (opts.email) log_messages.push_back(" - an email to send the results by attaching the detection images and \"" + strlog_detection_dirname + "\" from the \"" + detection_folder_name_string + "\" folder");
					else log_messages.push_back("Please send an email with the results by attaching the detection images and \"" + strlog_detection_dirname + "\" from the \"" + detection_folder_name_string + "\" folder");
			}
			filetest.close();
			log_messages.push_back("");
			log_messages.push_back("CHECK the DETECTION IMAGES for impacts and SEND the RESULTS (delcroix.marc@free.fr), NO DETECTION also MATTERS!");

			char email_subject_link[MAX_STRING] = { 0 };
			strcpy_s(email_subject_link, sizeof(email_subject_link), "");
			strcpy_s(email_subject_probabilities, sizeof(email_subject_probabilities), " (");
			strcpy_s(email_body_probabilities, sizeof(email_body_probabilities), "");

			if (nb_high_impact > 0) {
				strcat_s(email_subject_probabilities, sizeof(email_subject_probabilities), std::to_string(nb_high_impact).c_str());
				strcat_s(email_subject_probabilities, sizeof(email_subject_probabilities), " high");
				strcat_s(email_body_probabilities, sizeof(email_body_probabilities), "High probability= ");
				strcat_s(email_body_probabilities, sizeof(email_body_probabilities), std::to_string(nb_high_impact).c_str());
				strcat_s(email_body_probabilities, sizeof(email_body_probabilities), "%0A");
				strcpy_s(email_subject_link, sizeof(email_subject_link), ", ");
			}
			if (nb_low_impact > 0) {
				strcat_s(email_subject_probabilities, sizeof(email_subject_probabilities), email_subject_link);
				strcat_s(email_subject_probabilities, sizeof(email_subject_probabilities), std::to_string(nb_low_impact).c_str());
				strcat_s(email_subject_probabilities, sizeof(email_subject_probabilities), " low");
				strcat_s(email_body_probabilities, sizeof(email_body_probabilities), "Low  probability= ");
				strcat_s(email_body_probabilities, sizeof(email_body_probabilities), std::to_string(nb_low_impact).c_str());
				strcat_s(email_body_probabilities, sizeof(email_body_probabilities), "%0A");
				strcpy_s(email_subject_link, sizeof(email_subject_link), ", ");
			}
			if (nb_null_impact > 0) {
				strcat_s(email_body_probabilities, sizeof(email_body_probabilities), "Null probability= ");
				strcat_s(email_body_probabilities, sizeof(email_body_probabilities), std::to_string(nb_null_impact).c_str());
				strcat_s(email_body_probabilities, sizeof(email_body_probabilities), "%0A");
			}
			if (nb_error_impact > 0) {
				strcat_s(email_body_probabilities, sizeof(email_body_probabilities), "Error           = ");
				strcat_s(email_body_probabilities, sizeof(email_body_probabilities), std::to_string(nb_error_impact).c_str());
				strcat_s(email_body_probabilities, sizeof(email_body_probabilities), "%0A");
			}
			strcat_s(email_subject_probabilities, sizeof(email_subject_probabilities), email_subject_link);
			strcat_s(email_subject_probabilities, sizeof(email_subject_probabilities), std::to_string(nb_null_impact + nb_error_impact + nb_low_impact + nb_high_impact).c_str());
			strcat_s(email_subject_probabilities, sizeof(email_subject_probabilities), " total)");
			strcat_s(email_body_probabilities, sizeof(email_body_probabilities), "Total                = ");
			strcat_s(email_body_probabilities, sizeof(email_body_probabilities), std::to_string(nb_null_impact + nb_error_impact + nb_low_impact + nb_high_impact).c_str());
			strcat_s(email_body_probabilities, sizeof(email_body_probabilities), "%0A");
		}
//		if (popts->autostakkert) {
		//if (popts->parent_instance) {
		if (!opts.autoexit) {
			log_messages.push_back("");
			log_messages.push_back("You can SAFELY CLOSE this window.");
			log_messages.push_back("================================================================================");
		}
		if ((opts.parent_instance) && (opts.autostakkert)) { // restore imposed options by autostakkert mode
			//log_messages.push_back(std::to_string(opts.interactive_bak).c_str());
			//log_messages.push_back(std::to_string(opts.maxinstances_bak).c_str());
			//log_messages.push_back(std::to_string(opts.reprocessing_bak).c_str());
			opts.interactive = opts.interactive_bak;
			opts.reprocessing = opts.reprocessing_bak;
			opts.maxinstances = opts.maxinstances_bak;
		}
		WriteIni();

		for (std::string msg : log_messages) {
			std::wstring wmsg = std::wstring(msg.begin(), msg.end());
			CString Cmsg = CString(wmsg.c_str(), (int)wmsg.length());
			LogString(Cmsg, output_log_file.c_str(), &log_counter, TRUE, &wait_count_total);
		}
		log_messages.clear();

		if (opts.parent_instance) {
			// copies output log to central directory
			std::wstring output_log_file2(log_directory.begin(), log_directory.end());
			output_log_file2 = output_log_file2.append(L"\\").append(OUTPUT_FILENAME).append(DTC_LOG_SUFFIX);
			std::wofstream output_log2(output_log_file2.c_str(), std::ios_base::app);
			if (opts.dateonly) output_log2 << "WARNING, datation info only, no detection analysis was performed\n";
			std::wifstream output_log_in(output_log_file.c_str());
			output_log2 << output_log_in.rdbuf();
			output_log_in.close();
			//		output_log2 << getDateTime().str().c_str() << "\n";
			//		output_log2 << getDateTime().str().c_str() << message.c_str() << "\n";
			output_log2 << "======================================================================================================\n\n";
			

			output_log2.flush();
			output_log2.close();
			if (rename(OutOrgFilename2, OutNewFilename2)!= 0) {
				char msgtext[MAX_STRING] = {0};
				snprintf(msgtext,MAX_STRING, "cannot rename output file %s\n", OutOrgFilename2.m_psz);
				Warning(FALSE, "cannot rename output file", __func__, msgtext);
			}
			output_log_in.close();
			cv::destroyAllWindows();
			//if (opts.show_detect_image) cv::destroyWindow("Detection image");
			//if (opts.show_mean_image)	cv::destroyWindow("Mean image");
		}
} // end if acquisition > 0
	
	else {
		// TODO: Message nothing has been done
		LogString(L"WARNING, no file has been processed\n", output_log_file.c_str(), &log_counter, TRUE, &wait_count_total);
		LogString(_T(""), output_log_file.c_str(), &log_counter, TRUE, &wait_count_total);
		
		// TODO: Delete log if empty
	}
	//if (!popts->debug) {
		if (opts.autoexit) {
			if (opts.shutdown) {
				opts.shutdown = FALSE;
				WriteIni();
				ShellExecute(NULL, L"open", _T("cmd"), _T("/c shutdown /s /t 30 /d u:0:0"), NULL, SW_NORMAL);
			}
			dlg.OnFileExit();
		}
		if (opts.shutdown) {
			opts.shutdown = FALSE;
			WriteIni();
			ShellExecute(NULL, L"open", _T("cmd"), _T("/c shutdown /s /t 30 /d u:0:0"), NULL, SW_NORMAL);
			dlg.OnFileExit();
		}
		//}

	if (((opts.autostakkert_PID > 0) && (!opts.parent_instance)) ||  ((!opts.interactive) && (!opts.parent_instance) && (!opts.autostakkert))) dlg.OnFileExit();	// Automatically exit for child instances in autostakkert mode or automatic mode
	if ((!opts.interactive) && (!opts.parent_instance))	dlg.OnFileExit();																							// Automatically exit of parent non autostakkert instance in automatic mode
	
	//New moved from above - needed?
	if ((opts.parent_instance) && (opts.autostakkert) && (!IsProcessRunning(opts.autostakkert_PID))) {
		//MessageBox(NULL, _T("Warning: please check links for detection images to check, detection log, and folder with zip files to send before DeTeCt checks for an update"), _T("DeTeCt from AutoStakkert done, check for update will start"), MB_OK + MB_ICONWARNING + MB_SETFOREGROUND + MB_TOPMOST); 
		opts.autostakkert = FALSE;
		opts.autostakkert_PID = 0;
		dlg.execAS.SetCheck(false);
		dlg.OnCheckUpdate();
		CDeTeCtMFCDlg::getLog()->SetTopIndex(CDeTeCtMFCDlg::getLog()->GetCount() - 1);
		CDeTeCtMFCDlg::getLog()->RedrawWindow();
		//LogString(L"Automatic execution from parent AutoStakkert terminated", output_log_file.c_str(), &log_counter, TRUE, &wait_count_total);
	}

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

	init_string(opts.filename);
	init_string(opts.dirname);

	return TRUE;
}


/******************************************************************************************************************************************************************************************************************************************************************************************
/******************************************************************************************************************************************************************************************************************************************************************************************

									INTERNAL FUNCTIONS

/******************************************************************************************************************************************************************************************************************************************************************************************
/******************************************************************************************************************************************************************************************************************************************************************************************

/**********************************************************************************************//**
* @fn	int item_frame_rank_cmp(const void *a, const void *b)
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
int item_frame_rank_cmp(const void *a, const void *b)
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
	char tmpstring[MAX_STRING] = { 0 };
	char filename[MAX_STRING]	= { 0 };
	
	snprintf(filename, strlen(acquisition_filename) - 4, "%s", acquisition_filename);
	strcat_s(filename, suffix);
	filename[std::strlen(filename)] = '\0';
	//strcpy_s(full_filename, sizeof(full_filename), path_name); // CRASHES
	//strcat_s(full_filename, MAX_STRING, right(filename, strlen(filename) - InRstr(filename, "\\"), tmpstring)); // CRASHES
	std::strcpy(full_filename, path_name);
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
	char tmpstring[MAX_STRING] = { 0 };
	char filename[MAX_STRING] = { 0 };

	snprintf(filename, strlen(acquisition_filename) - 4, "%s", acquisition_filename);
	strcat_s(filename, suffix);
	strcat_s(filename, suffix2);
	filename[std::strlen(filename)] = '\0';
	//strcpy_s(full_filename, sizeof(full_filename), path_name); // CRASHES
	//strcat_s(full_filename, MAX_STRING, right(filename, strlen(filename) - InRstr(filename, "\\"), tmpstring)); // CRASHES
	std::strcpy(full_filename, path_name);
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

void zip(char *zipfilename, char *item_to_be_zipped, std::wstring output_filename, int* log_counter)
{
	#define MAX_THREADS 5000
	int waitms = 0;

	// Create zip file
	FILE* f = fopen(zipfilename, "wb");
	fwrite("\x50\x4B\x05\x06\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0", 22, 1, f);
	fclose(f);
	if (opts.debug) LogString(L"zip: heartbeat", output_filename.c_str(), log_counter, FALSE, &waitms);

	DWORD strlen			= 0;
	HRESULT hResult;
	IShellDispatch *pISD	= NULL;
	Folder *pToFolder;
	VARIANT	vDir			= {};
	VARIANT vFile			= {};
	VARIANT vOpt			= {};
	BSTR strptr1, strptr2;

	if (CoInitialize(NULL) != S_OK) {
if (opts.debug) LogString(L"zip: ERROR: Impossible to initialize COM library", output_filename.c_str(), log_counter, FALSE, &waitms);
		 char msgtext[MAX_STRING] = { 0 };
		snprintf(msgtext, MAX_STRING, "cannot initialize COM library");
		ErrorExit(TRUE, TRUE, "cannot initialize COM library", __func__, msgtext);
	}
	hResult = CoCreateInstance(CLSID_Shell, NULL, CLSCTX_INPROC_SERVER, IID_IShellDispatch, (void **)&pISD);
if (opts.debug) LogString(L"zip: heartbeat", output_filename.c_str(), log_counter, FALSE, &waitms);

	if (SUCCEEDED(hResult) && pISD != NULL)
	{
		strlen	= MultiByteToWideChar(CP_ACP, 0, zipfilename, -1, 0, 0);
		strptr1	= SysAllocStringLen(0, strlen);
		MultiByteToWideChar(CP_ACP, 0, zipfilename, -1, strptr1, strlen);

		VariantInit(&vDir);
		vDir.vt			= VT_BSTR;
		vDir.bstrVal	= strptr1;
		hResult			= pISD->NameSpace(vDir, &pToFolder);
if (opts.debug) LogString(L"zip: heartbeat", output_filename.c_str(), log_counter, FALSE, &waitms);

		if (SUCCEEDED(hResult))
		{
			strlen		= MultiByteToWideChar(CP_ACP, 0, item_to_be_zipped, -1, 0, 0);
			strptr2		= SysAllocStringLen(0, strlen);
			MultiByteToWideChar(CP_ACP, 0, item_to_be_zipped, -1, strptr2, strlen);

			VariantInit(&vFile);
			vFile.vt		= VT_BSTR;
			vFile.bstrVal	= strptr2;

			VariantInit(&vOpt);
			vOpt.vt			= VT_I4;
			vOpt.lVal		= 4;          // Do not display a progress dialog box
if (opts.debug) LogString(L"zip: heartbeat", output_filename.c_str(), log_counter, FALSE, &waitms);

	/* Attempt to log current existing threads - failed */
	
	//HANDLE hThrd0[MAX_THREADS];
			DWORD ThreadID0[MAX_THREADS]	= {};
			HANDLE h0 = CreateToolhelp32Snapshot(TH32CS_SNAPALL, 0);  //TH32CS_SNAPMODULE, 0);
if (opts.debug) LogString(L"zip: heartbeat", output_filename.c_str(), log_counter, FALSE, &waitms);
			DWORD NUM_THREADS0				= 0;
if (opts.debug) LogString(L"zip: initialization done", output_filename.c_str(), log_counter, FALSE, &waitms);
			if (h0 != INVALID_HANDLE_VALUE) {
				THREADENTRY32 te			= {};
				te.dwSize = sizeof(te);
				if (Thread32First(h0, &te)) {
if (opts.debug) LogString(L"zip: heartbeat Thread32First", output_filename.c_str(), log_counter, FALSE, &waitms);
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
if (opts.debug) LogString(L"zip: heartbeat end while loop", output_filename.c_str(), log_counter, FALSE, &waitms);
				}
				CloseHandle(h0);
if (opts.debug) LogString(L"zip: Thread enumeration done", output_filename.c_str(), log_counter, FALSE, &waitms);
			}

			hResult = NULL;
			//Copying
			hResult = pToFolder->CopyHere(vFile, vOpt); //NOTE: this appears to always return S_OK even on error
if (opts.debug) LogString(L"zip: heartbeat", output_filename.c_str(), log_counter, FALSE, &waitms);
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
				HANDLE hThrd[MAX_THREADS]		= {};
				DWORD ThreadID[MAX_THREADS]		= {};
				HANDLE h = CreateToolhelp32Snapshot(TH32CS_SNAPALL, 0);  //TH32CS_SNAPMODULE, 0);
				DWORD NUM_THREADS				= 0;
				if (h != INVALID_HANDLE_VALUE) {
					THREADENTRY32 te	= {};
					int Threads_all_nb	= 0;
					te.dwSize = sizeof(te);
					if (Thread32First(h, &te)) {
if (opts.debug) LogString(L"zip: Enumerate current threads", output_filename.c_str(), log_counter, FALSE, &waitms);
						do {
							if (te.dwSize >= (FIELD_OFFSET(THREADENTRY32, th32OwnerProcessID) + sizeof(te.th32OwnerProcessID))) {
								//only enumerate threads that are called by this process and not the main thread
								if ((te.th32OwnerProcessID == GetCurrentProcessId()) && (te.th32ThreadID != GetCurrentThreadId())) {
									DWORD ThreadID_index	= 0;
									BOOL Is_ZipThread		= TRUE;
									Threads_all_nb++;
									while (Is_ZipThread && ThreadID_index < NUM_THREADS0) if (te.th32ThreadID == ThreadID0[ThreadID_index]) Is_ZipThread = FALSE; else ThreadID_index++;

									if (Is_ZipThread) {
if (opts.debug) LogString(L"zip: Start the operation", output_filename.c_str(), log_counter, FALSE, &waitms);
										//printf("Process 0x%04x Thread 0x%04x\n", te.th32OwnerProcessID, te.th32ThreadID);
										ThreadID[NUM_THREADS] = te.th32ThreadID;
										//hThrd[NUM_THREADS] = OpenThread(THREAD_ALL_ACCESS, FALSE, te.th32ThreadID);
										hThrd[NUM_THREADS] = OpenThread(SYNCHRONIZE, FALSE, te.th32ThreadID);			// CORRECTED ACCORDING TO STACKOVERFLOW
										NUM_THREADS++;
									}
								}
							}
							te.dwSize = sizeof(te);
						} while (Thread32Next(h, &te));
if (opts.debug) LogString(L"zip: heartbeat end of while loop 2", output_filename.c_str(), log_counter, FALSE, &waitms);
					}
					CloseHandle(h);

					//Wait for all threads to exit
if (opts.debug) LogString(L"zip: Wait for any new threads", output_filename.c_str(), log_counter, FALSE, &waitms);
					WaitForMultipleObjects(NUM_THREADS, hThrd, TRUE, INFINITE);
					//(Usually object/thread closed is the last one)
					//DWORD object_closed = WaitForMultipleObjects(NUM_THREADS, hThrd, TRUE, INFINITE) - WAIT_OBJECT_0;
					//WaitForSingleObject(hThrd[NUM_THREADS - 1], INFINITE);

					//Close All handles
if (opts.debug) LogString(L"zip: Close handles", output_filename.c_str(), log_counter, FALSE, &waitms);
					for (DWORD i = 0; i < NUM_THREADS; i++) {
						if (hThrd[i] != 0) CloseHandle(hThrd[i]); // warning C6387 disabling
					}
				} //if invalid handle
			} //if CopyHere() hResult is S_OK
if (opts.debug) LogString(L"zip: Cleaning", output_filename.c_str(), log_counter, FALSE, &waitms);
			SysFreeString(strptr2);
			pToFolder->Release();
		}
		SysFreeString(strptr1);
		pISD->Release();
	}
	CoUninitialize();
if (opts.debug) LogString(L"zip: heartbeat end of cleaning", output_filename.c_str(), log_counter, FALSE, &waitms);
}

void LogString(CString log_cstring,  CString output_filename, int *log_counter, BOOL GUI_display, int* pwaitms)
{
	if ((GUI_display) || (opts.debug)) {
		CDeTeCtMFCDlg::getLog()->AddString((CString)getDateTime().str().c_str() + log_cstring);
		CDeTeCtMFCDlg::getLog()->SetTopIndex(CDeTeCtMFCDlg::getLog()->GetCount() - 1);
		CDeTeCtMFCDlg::getLog()->RedrawWindow();
	}
	
	//GetLockQueue((CString)__FUNCTION__ + _T(" ") + log_cstring + _T(" ") + output_filename, output_filename); //new queue method
	//std::ofstream output_log_stream(output_filename, std::ios_base::app, _SH_DENYRW);
	//std::ofstream output_log_stream(output_filename, std::ios_base::app);

	*pwaitms += NbWaitedUnlockedFile(output_filename, FILEACCESS_WAIT_MS);
	std::ofstream output_log_stream(output_filename, std::ios_base::app);

	std::string log_string = CString2string(log_cstring); 	// do not know why, but need to convert to string then back to cstring to have it work
	CString log_counter_cstr;
	log_counter_cstr.Format(L"%05i", (*log_counter));
	std::string log_counter_string = CString2string(log_counter_cstr);
	std::string debug_string = "";
	if (opts.debug) {
		debug_string.append(" (");
		debug_string.append(std::to_string(NbFilesFromQueue((CString)opts.DeTeCtQueueFilename)).c_str());
		debug_string.append(" files)");
	}
	output_log_stream  << "PID " << std::setfill('0') << std::setw(5) << std::to_string(GetCurrentProcessId()).c_str() << "-" << log_counter_string.c_str() << debug_string << ": " << getDateTimeMillis().str().c_str() << /*_T("CPU ") + (CString)std::to_string((int)(GetCPULoad() * 100)).c_str() + _T("% - ") <<*/ log_string.c_str() << "\n";
	output_log_stream.flush();
	output_log_stream.close();
	//UnlockQueue(output_filename); //new queue method
	(*log_counter) = (*log_counter) + 1;
}

int GetOtherProcessedFiles(const size_t acquisition_index, size_t* pacquisition_index_children, size_t* pacquisitions_to_be_processed, int* pnb_error_impact, int* pnb_null_impact, int* pnb_low_impact, int* pnb_high_impact, double* pduration_total, std::vector<std::string>* plog_messages, char* DeTeCtQueueFilename, clock_t* pcomputing_threshold_time, clock_t* plast_time, const clock_t refresh_duration, const clock_t single_time, const clock_t total_time, const int nframe, const int frame_number) {
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
	while (GetProcessedFileFromQueue(&processed_filename, &processed_filename_acquisition, &processed_message, &processed_rating, &duration, &nframe_child, &fps_int_child, (CString)DeTeCtQueueFilename)) {

		totalProgress_wstring_tmp = L"Total\n(" + std::to_wstring(acquisition_index + (*pacquisition_index_children)) + L"/" + std::to_wstring(MAX(*pacquisitions_to_be_processed, acquisition_index + (*pacquisition_index_children)+1)) + L")";
//	LogString(_T("4: parent / children / done / tobe = ") + (CString)(std::to_string(acquisitions_processed).c_str()) + (CString)(" / ") + (CString)(std::to_string(acquisition_index_children).c_str()) + (CString)(" / ") + (CString)(std::to_string(acquisitions_processed + acquisition_index_children).c_str()) + (CString)(" / ") + (CString)(std::to_string(acquisitions_to_be_processed).c_str()), output_log_file.c_str(), &log_counter, TRUE, &wait_count_total);
		//CDeTeCtMFCDlg::gettotalProgress()->SetWindowText(totalProgress_wstring_tmp.c_str());
		//CDeTeCtMFCDlg::getProgress_all()->SetPos((short)(MAX_RANGE_PROGRESS * (float)(acquisition_index + (*pacquisition_index_children)) / MAX(*pacquisitions_to_be_processed, acquisition_index + (*pacquisition_index_children)+1)));
		//CDeTeCtMFCDlg::getProgress_all()->UpdateWindow();
		UpdateProgress((*pacquisitions_to_be_processed), acquisition_index, (*pacquisition_index_children), nframe, frame_number, DeTeCtQueueFilename);

		if (clock() > *pcomputing_threshold_time) DisplayProcessingTime(pcomputing_threshold_time, plast_time, refresh_duration, single_time, total_time);
		switch (processed_rating) {
		case Rating_type::Error:
			(*pnb_error_impact)++;
			CDeTeCtMFCDlg::getimpactNull()->SetWindowText(std::to_wstring((*pnb_null_impact) + (*pnb_error_impact)).c_str());
			break;
		case Rating_type::Null:
			(*pnb_null_impact)++;
			CDeTeCtMFCDlg::getimpactNull()->SetWindowText(std::to_wstring((*pnb_null_impact) + (*pnb_error_impact)).c_str());
			break;
		case Rating_type::Low:
			(*pnb_low_impact)++;
			CDeTeCtMFCDlg::getimpactLow()->SetWindowText(std::to_wstring((*pnb_low_impact)).c_str());
			break;
		case Rating_type::High:
			(*pnb_high_impact)++;
			CDeTeCtMFCDlg::getimpactHigh()->SetWindowText(std::to_wstring((*pnb_high_impact)).c_str());
			break;
		}

		(*pacquisition_index_children)++;
//(*pacquisitions_to_be_processed) = NbFilesFromQueue(char2CString(DeTeCtQueueFilename, &tmp));
		(*pduration_total) += duration;
		std::string processed_filename_acquisition_string = CString2string(processed_filename_acquisition);
		std::string processed_short_filename = processed_filename_acquisition_string.substr(processed_filename_acquisition_string.find_last_of("\\") + 1, processed_filename_acquisition_string.length());
		plog_messages->push_back(processed_short_filename + ":" + "    " + CString2string(processed_message));

		CDeTeCtMFCDlg::getLog()->AddString((CString)getDateTime().str().c_str() + "----- " + processed_short_filename.c_str() + " -----");
		CDeTeCtMFCDlg::getLog()->AddString((CString)getDateTime().str().c_str() + std::to_string(nframe_child).c_str() + (CString)" frames @ " + std::to_string(fps_int_child).c_str() + (CString)" fps (" + std::to_wstring((int)duration).c_str() + "s duration)");
		CDeTeCtMFCDlg::getLog()->AddString((CString)getDateTime().str().c_str() + processed_message);
		CDeTeCtMFCDlg::getLog()->AddString((CString)getDateTime().str().c_str());
		CDeTeCtMFCDlg::getLog()->SetTopIndex(CDeTeCtMFCDlg::getLog()->GetCount() - 1);
		CDeTeCtMFCDlg::getLog()->RedrawWindow();

		duration = 0;
		nb_otherprocessedfiles++;

//		if ((opts.parent_instance) && (strlen(opts.DeTeCtQueueFilename) > 0)) (*pacquisitions_to_be_processed) = NbFilesFromQueue((CString)opts.DeTeCtQueueFilename);
		totalProgress_wstring_tmp = L"Total\n(" + std::to_wstring(acquisition_index + (*pacquisition_index_children)) + L"/" + std::to_wstring(MAX(*pacquisitions_to_be_processed, acquisition_index + (*pacquisition_index_children))) + L")";
//		totalProgress_wstring_tmp = L"Total\n(" + std::to_wstring(acquisition_index + (*pacquisition_index_children)) + L"/" + std::to_wstring(acquisition_index + (*pacquisition_index_children)) + L")";
//LogString(_T("5: parent / children / done / tobe = ") + (CString)(std::to_string(acquisitions_processed).c_str()) + (CString)(" / ") + (CString)(std::to_string(acquisition_index_children).c_str()) + (CString)(" / ") + (CString)(std::to_string(acquisitions_processed + acquisition_index_children).c_str()) + (CString)(" / ") + (CString)(std::to_string(acquisitions_to_be_processed).c_str()), output_log_file.c_str(), &log_counter, TRUE, &wait_count_total);
		//CDeTeCtMFCDlg::gettotalProgress()->SetWindowText(totalProgress_wstring_tmp.c_str());
		//CDeTeCtMFCDlg::getProgress_all()->SetPos((short)(MAX_RANGE_PROGRESS * (float)(acquisition_index + (*pacquisition_index_children)) / MAX(*pacquisitions_to_be_processed, acquisition_index + (*pacquisition_index_children))));
		//CDeTeCtMFCDlg::getProgress_all()->UpdateWindow();
		UpdateProgress((*pacquisitions_to_be_processed), acquisition_index, (*pacquisition_index_children), nframe, frame_number, DeTeCtQueueFilename);

	}
	(*pacquisitions_to_be_processed) = NbFilesFromQueue(char2CString(DeTeCtQueueFilename, &tmp)); //BUGFIX if files ignored during processing by other processes
	return nb_otherprocessedfiles;
}

//!!!!!!!!!!!!!!!!!!!!!!WARNING!!!!!!!!!!!!!!!! Integrated version, much quicker but misses some processed files displayed for unknown reason
int GetOtherProcessedFiles2(const int acquisitions_processed, int* pacquisition_index_children, int* pacquisitions_to_be_processed, int* pnb_error_impact, int* pnb_null_impact, int* pnb_low_impact, int* pnb_high_impact, double* pduration_total, std::vector<std::string>* plog_messages, char* DeTeCtQueueFilename, clock_t* pcomputing_threshold_time, clock_t* plast_time, const clock_t refresh_duration, const clock_t single_time, const clock_t total_time, const int nframe, const int frame_number) {
	CString processed_filename;
	CString processed_filename_acquisition;
	CString processed_message;
	CString tmp;
	Rating_type processed_rating = Rating_type::Error;
	std::wstring totalProgress_wstring_tmp;
	int		nb_otherprocessedfiles = 0;
	int nframe_child = 0;
	int fps_int_child = 0;

	double duration = 0;
	if (!filesys::exists(CString2string(char2CString(DeTeCtQueueFilename, &tmp))))
	{
		char msgtext[MAX_STRING] = { 0 };
		snprintf(msgtext, MAX_STRING, "cannot find acquisition queue file %s", DeTeCtQueueFilename);
		ErrorExit(TRUE, TRUE, "queue file not found", __func__, msgtext);  	// exits DeTeCt if Queuefile does not exists
	}
	//CString	processed_line;
	BOOL	status;
	HANDLE	QueueFileHandle = INVALID_HANDLE_VALUE;
	CString line = L"";
	std::vector<CString> cstring_lines;
	BOOL file_to_be_updated = FALSE;

	if (OpenRWQueueFile(char2CString(DeTeCtQueueFilename, &tmp), &QueueFileHandle)) {
		do {
			line = GetLine(QueueFileHandle);
			if (line.GetLength() > 1) {
				if (line.Left(17) == (_T("file_processed : "))) {
					//if (line.Find(_T("file_processed : "), 0) == 0) {
					file_to_be_updated = TRUE;

					std::string tmp_line(CString2string(line));
					status = TRUE;

					while (tmp_line.substr(tmp_line.size() - 1, 1) == " ") tmp_line.erase(tmp_line.size() - 1, 1);
					if (tmp_line.find("|")) processed_filename = tmp_line.substr(0, tmp_line.find("|")).c_str(); //
					else status = FALSE;
					tmp_line.erase(0, tmp_line.find("|") + 1);

					while (tmp_line.substr(tmp_line.size() - 1, 1) == " ") tmp_line.erase(tmp_line.size() - 1, 1);
					if (tmp_line.find("|")) processed_filename_acquisition = tmp_line.substr(0, tmp_line.find("|")).c_str();
					else status = FALSE;
					tmp_line.erase(0, tmp_line.find("|") + 1);

					while (tmp_line.substr(tmp_line.size() - 1, 1) == " ") tmp_line.erase(tmp_line.size() - 1, 1);
					if (tmp_line.find("|")) processed_message = tmp_line.substr(0, tmp_line.find("|")).c_str();
					else status = FALSE;
					tmp_line.erase(0, tmp_line.find("|") + 1);

					while (tmp_line.substr(tmp_line.size() - 1, 1) == " ") tmp_line.erase(tmp_line.size() - 1, 1);
					if (tmp_line.find("|")) processed_rating = (Rating_type)(atoi(tmp_line.substr(0, tmp_line.find("|")).c_str()));
					else status = FALSE;
					tmp_line.erase(0, tmp_line.find("|") + 1);

					while (tmp_line.substr(tmp_line.size() - 1, 1) == " ") tmp_line.erase(tmp_line.size() - 1, 1);
					if (tmp_line.find("|")) duration = atoi(tmp_line.c_str());
					else status = FALSE;
					tmp_line.erase(0, tmp_line.find("|") + 1);

					while (tmp_line.substr(tmp_line.size() - 1, 1) == " ") tmp_line.erase(tmp_line.size() - 1, 1);
					if (tmp_line.find("|")) nframe_child = atoi(tmp_line.c_str());
					else status = FALSE;
					tmp_line.erase(0, tmp_line.find("|") + 1);

					while (tmp_line.substr(tmp_line.size() - 1, 1) == " ") tmp_line.erase(tmp_line.size() - 1, 1);
					if (tmp_line.find("|")) fps_int_child = atoi(tmp_line.c_str());
					else status = FALSE;

					if (status)	line.Replace(_T("file_processed "), _T("file_ok        "));
					else		line.Replace(_T("file_processed "), _T("file_ko        "));

/*					totalProgress_wstring_tmp = L"Total\n(" + std::to_wstring(acquisition_index + (*pacquisition_index_children)) + L"/" + std::to_wstring(MAX(*pacquisitions_to_be_processed, acquisition_index + (*pacquisition_index_children) + 1)) + L")";
					CDeTeCtMFCDlg::gettotalProgress()->SetWindowText(totalProgress_wstring_tmp.c_str());
					CDeTeCtMFCDlg::getProgress_all()->SetPos((short)(MAX_RANGE_PROGRESS * (float)(acquisition_index + (*pacquisition_index_children)) / MAX(*pacquisitions_to_be_processed, acquisition_index + (*pacquisition_index_children) + 1)));
					CDeTeCtMFCDlg::getProgress_all()->UpdateWindow();
					if (clock() > *pcomputing_threshold_time) DisplayProcessingTime(pcomputing_threshold_time, plast_time, refresh_duration, single_time, total_time);*/
					switch (processed_rating) {
					case Rating_type::Error:
						(*pnb_error_impact)++;
						break;
					case Rating_type::Null:
						(*pnb_null_impact)++;
						break;
					case Rating_type::Low:
						(*pnb_low_impact)++;
						break;
					case Rating_type::High:
						(*pnb_high_impact)++;
						break;
					}
					(*pacquisition_index_children)++;
					//(*pacquisitions_to_be_processed) = NbFilesFromQueue(char2CString(DeTeCtQueueFilename, &tmp));
					(*pduration_total) += duration;
					std::string processed_filename_acquisition_string = CString2string(processed_filename_acquisition);
					std::string processed_short_filename = processed_filename_acquisition_string.substr(processed_filename_acquisition_string.find_last_of("\\") + 1, processed_filename_acquisition_string.length());
					plog_messages->push_back(processed_short_filename + ":" + "    " + CString2string(processed_message));

					CDeTeCtMFCDlg::getLog()->AddString((CString)getDateTime().str().c_str() + "----- " + processed_short_filename.c_str() + " -----");
					CDeTeCtMFCDlg::getLog()->AddString((CString)getDateTime().str().c_str() + std::to_string(nframe_child).c_str() + (CString)" frames @ " + std::to_string(fps_int_child).c_str() + (CString)" fps (" + std::to_wstring((int)duration).c_str() + "s duration)");
					CDeTeCtMFCDlg::getLog()->AddString((CString)getDateTime().str().c_str() + processed_message);
					CDeTeCtMFCDlg::getLog()->AddString((CString)getDateTime().str().c_str());

					duration = 0;
					nb_otherprocessedfiles++;

/*					totalProgress_wstring_tmp = L"Total\n(" + std::to_wstring(acquisition_index + (*pacquisition_index_children)) + L"/" + std::to_wstring(MAX(*pacquisitions_to_be_processed, acquisition_index + (*pacquisition_index_children))) + L")";
					CDeTeCtMFCDlg::gettotalProgress()->SetWindowText(totalProgress_wstring_tmp.c_str());
					CDeTeCtMFCDlg::getProgress_all()->SetPos((short)(MAX_RANGE_PROGRESS * (float)(acquisition_index + (*pacquisition_index_children)) / MAX(*pacquisitions_to_be_processed, acquisition_index + (*pacquisition_index_children))));
					CDeTeCtMFCDlg::getProgress_all()->UpdateWindow();*/
				}
			}
			cstring_lines.push_back(line);
		} while (line.GetLength() > 1);
		if (file_to_be_updated) {
			DWORD	dwBytesWritten = 0;

			SetFilePointerEx(QueueFileHandle, { 0 }, NULL, FILE_BEGIN);
			SetEndOfFile(QueueFileHandle);
			std::for_each(cstring_lines.begin(), cstring_lines.end(), [&](const CString cstring_line) {
				CT2A line(cstring_line + _T("\n"));
				WriteFile(QueueFileHandle, line, cstring_line.GetLength() + 1, &dwBytesWritten, NULL);
			});
			CloseHandle(QueueFileHandle);

			//Refresh display after saving file
			CDeTeCtMFCDlg::getimpactNull()->SetWindowText(std::to_wstring((*pnb_null_impact) + (*pnb_error_impact)).c_str());
			CDeTeCtMFCDlg::getimpactLow()->SetWindowText(std::to_wstring((*pnb_low_impact)).c_str());
			CDeTeCtMFCDlg::getimpactHigh()->SetWindowText(std::to_wstring((*pnb_high_impact)).c_str());

			CDeTeCtMFCDlg::getLog()->SetTopIndex(CDeTeCtMFCDlg::getLog()->GetCount() - 1);
			CDeTeCtMFCDlg::getLog()->RedrawWindow();

			if (clock() > *pcomputing_threshold_time) DisplayProcessingTime(pcomputing_threshold_time, plast_time, refresh_duration, single_time, total_time);
//if ((opts.parent_instance) && (strlen(opts.DeTeCtQueueFilename) > 0)) acquisitions_to_be_processed = NbFilesFromQueue((CString)opts.DeTeCtQueueFilename);
			//totalProgress_wstring_tmp = L"Total\n(" + std::to_wstring(acquisitions_processed + (*pacquisition_index_children)) + L"/" + std::to_wstring(MAX(*pacquisitions_to_be_processed, acquisitions_processed + (*pacquisition_index_children))) + L")";
			//CDeTeCtMFCDlg::gettotalProgress()->SetWindowText(totalProgress_wstring_tmp.c_str());
			//CDeTeCtMFCDlg::getProgress_all()->SetPos((short)(MAX_RANGE_PROGRESS* (float)(acquisitions_processed + (*pacquisition_index_children)) / MAX(*pacquisitions_to_be_processed, acquisitions_processed + (*pacquisition_index_children))));
			//CDeTeCtMFCDlg::getProgress_all()->UpdateWindow();
			UpdateProgress((*pacquisitions_to_be_processed), acquisitions_processed, (*pacquisition_index_children), nframe, frame_number, DeTeCtQueueFilename);

		} else CloseHandle(QueueFileHandle);
	}
	return nb_otherprocessedfiles;
}

int	ForksInstances(const int maxinstances, const int PID, const CString DeTeCtQueueFilename, const int scan_time, const int scan_time_random_max, int *pnbinstances)
{
	int nb_forked_instances = 0;
	int files_to_be_processed = 0;
	// Forks other DeTeCt instances if work in queue

	if ((opts.parent_instance) && (maxinstances > 1)) {
		files_to_be_processed = NbItemFromQueue(L"file", DeTeCtQueueFilename, NULL, TRUE);
		if (files_to_be_processed > 0) {
			(*pnbinstances) = ChildrenProcessesNumber() + 1;
			if ((*pnbinstances) < maxinstances) { //No instance to fork if maximum is reached!
				int nb_instances_to_be_forked = MIN(MIN(files_to_be_processed, maxinstances - (*pnbinstances)), NbPossibleChildInstances_fromMemoryandCPUUsage());	// NBItemQueue files to be processed
				// Forks DeTeCt.exe -auto
				nb_forked_instances = 0;
				CString options = _T(" -auto  -maxinst ") + (CString)std::to_string(maxinstances).c_str();
				if (opts.debug) options = options + _T(" -debug");
				while (nb_forked_instances < nb_instances_to_be_forked) { // no variable there: we do not know at which speed DeTeCt child instances will be launched
					if (nb_forked_instances > 0) {
						if (scan_time_random_max > 0) Sleep(scan_time + rand() % scan_time_random_max);	// in ms
						else Sleep(scan_time);
					}
					HINSTANCE status;
					int child_window_state = SW_HIDE;
					if (opts.debug) child_window_state = SW_NORMAL;
					if (PID > 0) { // Should not happen, as stores parent DeTeCt PID
						if (!opts.autostakkert)	status = ShellExecute(NULL, L"open", DeTeCt_additional_filename_exe_fullpath(_T(".exe")), options + _T(" -dtcpid ")	+ (CString)std::to_string(PID).c_str(), NULL, child_window_state);
						else					status = ShellExecute(NULL, L"open", DeTeCt_additional_filename_exe_fullpath(_T(".exe")), options + _T(" -aspid ")	+ (CString)std::to_string(PID).c_str(), NULL, child_window_state);
					}
					else {
												status = ShellExecute(NULL, L"open", DeTeCt_additional_filename_exe_fullpath(_T(".exe")), options, NULL, child_window_state);
					}
					if ((uintptr_t)status <= 32) AfxMessageBox(_T("Error ShellExecute, code ") + (CString)std::to_string((uintptr_t)(status)).c_str());
					nb_forked_instances++;
				}
			}
			(*pnbinstances) += nb_forked_instances;
			DisplayInstanceType(pnbinstances);
		}
	}
	return nb_forked_instances;
}

int		ASorDeTeCtPID(const int AutoStakkert_ID, const int DeTeCt_ID) {
	if (AutoStakkert_ID > 0) return AutoStakkert_ID;
	else return DeTeCt_ID;
}

void	DisplayProcessingTime(clock_t *pcomputing_threshold_time, clock_t *plast_time, const clock_t refresh_duration, const clock_t single_time, const clock_t total_time) {
	CString processing_time_str;
	CString total_type;

	(*plast_time) = clock();
	(*pcomputing_threshold_time) = (*plast_time) + refresh_duration;

	processing_time_str.Format(L"Processing time: %.*fs (file)  %.*fs (%s)", 0, ((double)((*plast_time) - single_time) / (double)CLOCKS_PER_SEC), 0, ((double)((*plast_time) - total_time) / (double)CLOCKS_PER_SEC), static_cast<LPCTSTR>(TotalType()));
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
				if (opts.autostakkert)	instance_type = Instance_type::autostakkert_parent;
				else					instance_type = Instance_type::parent;
		else if (opts.autostakkert)		instance_type = Instance_type::autostakkert_single;
		else							instance_type = Instance_type::single;
	else if (opts.autostakkert)			instance_type = Instance_type::autostakkert_child;
	else								instance_type = Instance_type::child;

	switch (instance_type) {
		case Instance_type::autostakkert_parent:
			(*pinstance_text) = "AS! PARENT";
			break;
		case Instance_type::parent:
			(*pinstance_text) = "PARENT";
			break;
		case Instance_type::autostakkert_single:
			(*pinstance_text) = "AS! single";
			break;
		case Instance_type::single:
			(*pinstance_text) = "Single";
			break;
		case Instance_type::autostakkert_child:
			(*pinstance_text) = "AS! child";
			break;
		case Instance_type::child:
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
		case Instance_type::autostakkert_parent:
		case Instance_type::parent:
			if ((*nbinstances) == 0) {
				(*nbinstances) = ChildrenProcessesNumber() + 1;	// heavy computing, does not compute and uses value if not nul
			}
//			if (nbinstances > 1) {
				if (opts.autostakkert) instance_cstring = _T("Executed from AS!, ") +  instance_cstring;
				nbinstances_cstr.Format(L"%d", (*nbinstances));
				max_nbinstances_cstr.Format(L"%d", opts.maxinstances);
				if (opts.debug) instance_cstring = instance_type_cstring + _T(" (") + nbinstances_cstr + _T("/") + max_nbinstances_cstr + _T(")");
				else {
					instance_cstring = nbinstances_cstr + _T("/") + max_nbinstances_cstr + _T(" instance");
					if ((*nbinstances) > 1) instance_cstring += _T("s");
					instance_cstring += _T(" running");
				}
				/*if (opts.debug) instance_cstring = instance_type_cstring + _T(" (") + nbinstances_cstr + _T("/") + max_nbinstances_cstr + _T(")");
				else {
					instance_cstring = nbinstances_cstr + _T(" instance");
					if (nbinstances > 1) instance_cstring = instance_cstring + _T("s");
					instance_cstring = instance_cstring + _T(" running");
				}*/
	//		}
			break;
// No display in child mode, so no calculation of nbinstances
		case Instance_type::autostakkert_child:
		case Instance_type::child:
		case Instance_type::autostakkert_single:
		case Instance_type::single:
			break;
	}
	CDeTeCtMFCDlg::getInstance()->SetWindowText(instance_cstring);
	
	return instance_type;
}

void	WriteIni() {
	CString str;
	CString DeTeCtIniFilename = DeTeCt_additional_filename_exe_fullpath(DTC_INI_SUFFIX);

	::WritePrivateProfileString(L"general", L"version", CA2T(VERSION_NB), DeTeCtIniFilename);
	str.Format(L"%.2f", opts.impact_brightness_increase_min_factor);
	::WritePrivateProfileString(L"impact", L"brightness_increase_min_factor", str, DeTeCtIniFilename);
	str.Format(L"%d", opts.nframesRef);
	::WritePrivateProfileString(L"other", L"refmin", str, DeTeCtIniFilename);
	str.Format(L"%d", opts.incrFrameImpact);
	::WritePrivateProfileString(L"impact", L"frames", str, DeTeCtIniFilename);
	str.Format(L"%.2f", opts.impact_duration_min);
	::WritePrivateProfileString(L"impact", L"impact_duration_min", str, DeTeCtIniFilename);
	str.Format(L"%.1f", opts.impact_radius_min);
	::WritePrivateProfileString(L"impact", L"impact_radius_min", str, DeTeCtIniFilename);
	str.Format(L"%.1f", opts.impact_radius_max);
	::WritePrivateProfileString(L"impact", L"impact_radius_max", str, DeTeCtIniFilename);
	str.Format(L"%.1f", opts.impact_radius_ratio);
	::WritePrivateProfileString(L"impact", L"impact_radius_ratio", str, DeTeCtIniFilename);
	str.Format(L"%.1f", opts.impact_radius_shared_candidates_factor_min);
	::WritePrivateProfileString(L"impact", L"impact_radius_shared_candidates_factor_min", str, DeTeCtIniFilename);
	str.Format(L"%.0f", opts.threshold);
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
	str.Format(L"%.2f", opts.bg_detection_peak_factor);
	::WritePrivateProfileString(L"background", L"bg_detection_peak_factor", str, DeTeCtIniFilename);
	str.Format(L"%d", opts.bg_detection_consecutive_values);
	::WritePrivateProfileString(L"background", L"bg_detection_consecutive_values", str, DeTeCtIniFilename);
	
	str.Format(L"%d", opts.use_one_algo_to_reject_frame);
	::WritePrivateProfileString(L"rejection", L"use_one_algo_to_reject_frame", str, DeTeCtIniFilename);
	str.Format(L"%d", opts.use_reference_similarity[SSIM]);
	::WritePrivateProfileString(L"rejection", L"use_reference_similarity[SSIM]", str, DeTeCtIniFilename);
	str.Format(L"%d", opts.use_reference_similarity[MSE]);
	::WritePrivateProfileString(L"rejection", L"use_reference_similarity[MSE]", str, DeTeCtIniFilename);
	str.Format(L"%d", opts.use_reference_similarity[NCC]);
	::WritePrivateProfileString(L"rejection", L"use_reference_similarity[NCC]", str, DeTeCtIniFilename);
	str.Format(L"%d", opts.use_previous_frame_similarity[SSIM]);
	::WritePrivateProfileString(L"rejection", L"use_previous_frame_similarity[SSIM]", str, DeTeCtIniFilename);
	str.Format(L"%d", opts.use_previous_frame_similarity[MSE]);
	::WritePrivateProfileString(L"rejection", L"use_previous_frame_similarity[MSE]", str, DeTeCtIniFilename);
	str.Format(L"%d", opts.use_previous_frame_similarity[NCC]);
	::WritePrivateProfileString(L"rejection", L"use_previous_frame_similarity[NCC]", str, DeTeCtIniFilename);

	str.Format(L"%.2f", opts.similarity_reference_decrease_min_pc[SSIM]);
	::WritePrivateProfileString(L"rejection", L"similarity_reference_decrease_min_pc[SSIM]", str, DeTeCtIniFilename);
	str.Format(L"%.2f", opts.similarity_reference_decrease_min_pc[MSE]);
	::WritePrivateProfileString(L"rejection", L"similarity_reference_decrease_min_pc[MSE]", str, DeTeCtIniFilename);
	str.Format(L"%.2f", opts.similarity_reference_decrease_min_pc[NCC]);
	::WritePrivateProfileString(L"rejection", L"similarity_reference_decrease_min_pc[NCC]", str, DeTeCtIniFilename);
	str.Format(L"%.2f", opts.similarity_previous_frame_decrease_min_pc[SSIM]);
	::WritePrivateProfileString(L"rejection", L"similarity_previous_frame_decrease_min_pc[SSIM]", str, DeTeCtIniFilename);
	str.Format(L"%.2f", opts.similarity_previous_frame_decrease_min_pc[MSE]);
	::WritePrivateProfileString(L"rejection", L"similarity_previous_frame_decrease_min_pc[MSE]", str, DeTeCtIniFilename);
	str.Format(L"%.2f", opts.similarity_previous_frame_decrease_min_pc[NCC]);
	::WritePrivateProfileString(L"rejection", L"similarity_previous_frame_decrease_min_pc[NCC]", str, DeTeCtIniFilename);

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
	str.Format(L"%.3f", opts.impact_distance_max);
	::WritePrivateProfileString(L"impact", L"impact_distance_max", str, DeTeCtIniFilename);
	str.Format(L"%.2f", opts.impact_max_avg_min);
	::WritePrivateProfileString(L"impact", L"impact_max_avg_min", str, DeTeCtIniFilename);
	str.Format(L"%.2f", opts.impact_confidence_min);
	::WritePrivateProfileString(L"impact", L"impact_confidence_min", str, DeTeCtIniFilename);

	str.Format(L"%d", opts.show_detect_image);
	::WritePrivateProfileString(L"view", L"detect",	str, DeTeCtIniFilename);
	str.Format(L"%d", opts.show_mean_image);
	::WritePrivateProfileString(L"view", L"mean",	str, DeTeCtIniFilename);
	str.Format(L"%d", opts.viewROI);
	::WritePrivateProfileString(L"view", L"roi",	str, DeTeCtIniFilename);
	str.Format(L"%d", opts.viewTrk);
	::WritePrivateProfileString(L"view", L"trk",	str, DeTeCtIniFilename);
	str.Format(L"%d", opts.viewRef);
	::WritePrivateProfileString(L"view", L"ref",	str, DeTeCtIniFilename);
	str.Format(L"%d", opts.viewMsk);
	::WritePrivateProfileString(L"view", L"msk",	str, DeTeCtIniFilename);
	str.Format(L"%d", opts.viewThr);
	::WritePrivateProfileString(L"view", L"thr",	str, DeTeCtIniFilename);
	str.Format(L"%d", opts.viewSmo);
	::WritePrivateProfileString(L"view", L"smo",	str, DeTeCtIniFilename);
	str.Format(L"%d", opts.viewRes);
	::WritePrivateProfileString(L"view", L"res",	str, DeTeCtIniFilename);
	str.Format(L"%d", opts.viewDif);
	::WritePrivateProfileString(L"view", L"dif",	str, DeTeCtIniFilename);
	str.Format(L"%d", opts.viewHis);
	::WritePrivateProfileString(L"view", L"his",	str, DeTeCtIniFilename);
	str.Format(L"%d", opts.ignore);
	::WritePrivateProfileString(L"other", L"ignore", str, DeTeCtIniFilename);
	//int bayerCodes[] = { 0, cv::COLOR_BayerBG2RGB, cv::COLOR_BayerGB2RGB, cv::COLOR_BayerRG2RGB, cv::COLOR_BayerGR2RGB };
	str.Format(L"%d", opts.bayer);
	::WritePrivateProfileString(L"other", L"debayer", str, DeTeCtIniFilename);
	str.Format(L"%d", opts.filter.type);
	::WritePrivateProfileString(L"other", L"filter", str, DeTeCtIniFilename);
	::WritePrivateProfileString(L"other", L"darkfile", CA2T(opts.darkfilename), DeTeCtIniFilename);

	//str.Format(L"%d", opts.debug);
	//::WritePrivateProfileString(L"processing", L"debug", str, DeTeCtIniFilename);
	//str.Format(L"%d", opts.dateonly);
	//::WritePrivateProfileString(L"processing", L"dateonly", str, DeTeCtIniFilename);
	str.Format(L"%d", opts.zip);
	::WritePrivateProfileString(L"processing", L"zip", str, DeTeCtIniFilename);
	str.Format(L"%d", opts.email);
	::WritePrivateProfileString(L"processing", L"email", str, DeTeCtIniFilename);
	// From main window checkboxes
	str.Format(L"%d", !opts.interactive);
	::WritePrivateProfileString(L"processing", L"autoprocessing", str, DeTeCtIniFilename);
	str.Format(L"%d", opts.autoexit);
	::WritePrivateProfileString(L"processing", L"autoexit", str, DeTeCtIniFilename);
	str.Format(L"%d", opts.shutdown);
	::WritePrivateProfileString(L"processing", L"autoshutdown", str, DeTeCtIniFilename);
	str.Format(L"%d", opts.maxinstances);
	::WritePrivateProfileString(L"processing", L"maxinstances", str, DeTeCtIniFilename);
	str.Format(L"%d", opts.reprocessing);
	::WritePrivateProfileString(L"processing", L"reprocessing", str, DeTeCtIniFilename);
	str.Format(L"%d", opts.resources_usage);
	::WritePrivateProfileString(L"processing", L"resources", str, DeTeCtIniFilename);
	// OpenCL deactivation not saved
	//str.Format(L"%d", opts.OpenCL);
	//::WritePrivateProfileString(L"processing", L"OpenCL", str, DeTeCtIniFilename);
}

void	AcquisitionFileListToQueue(AcquisitionFilesList *pacquisition_files, const CString tag_current, const size_t index_current, const CString out_directory, size_t *pacquisitions_to_be_processed) {
	CString tmp, tmp2;
	if (!filesys::exists(CString2string((CString)opts.DeTeCtQueueFilename))) {
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
		if ((out_directory.GetLength() > 0) && (!GetItemFromQueue(&log_cstring, _T("output_dir: "), (CString)opts.DeTeCtQueueFilename, NULL, TRUE))) PushItemToQueue(out_directory, _T("output_dir"), char2CString(opts.DeTeCtQueueFilename, &tmp2), NULL, TRUE);
		SetIntParamToQueue(opts.maxinstances, _T("max_instances"), (CString)opts.DeTeCtQueueFilename);
	}

	if (filesys::exists(CString2string((CString)opts.DeTeCtQueueFilename))) {
//		std::string filename;
		int index = 0;
		//int initial_file_list_size = pacquisition_files->file_list.size();
		(*pacquisitions_to_be_processed) = (int) pacquisition_files->file_list.size();
		while (index < pacquisition_files->file_list.size()) {
//			filename = pacquisition_files->file_list.at(index);
			if (index_current >= pacquisition_files->file_list.size())	PushItemToQueue(char2CString(pacquisition_files->file_list.at(index++).c_str(), &tmp), _T("file"), char2CString(opts.DeTeCtQueueFilename, &tmp2), NULL, TRUE);
			else if (index < index_current)								PushItemToQueue(char2CString(pacquisition_files->file_list.at(index).c_str(), &tmp), _T("file_ok"), char2CString(opts.DeTeCtQueueFilename, &tmp2), NULL, TRUE);
			else if (index == index_current)							PushItemToQueue(char2CString(pacquisition_files->file_list.at(index).c_str(), &tmp), tag_current, char2CString(opts.DeTeCtQueueFilename, &tmp2), NULL, TRUE);
			else {
																		PushFileToQueue(char2CString(pacquisition_files->file_list.at(index).c_str(), &tmp), char2CString(opts.DeTeCtQueueFilename, &tmp2));
				if ((index_current >= 0) && (index > index_current)) {
					pacquisition_files->file_list.erase(pacquisition_files->file_list.begin() + index);
					pacquisition_files->acquisition_file_list.erase(pacquisition_files->acquisition_file_list.begin() + index);
					pacquisition_files->nb_prealigned_frames.erase(pacquisition_files->nb_prealigned_frames.begin() + index); // WARNING in debug, error in .begin()
					pacquisition_files->acquisition_size.erase(pacquisition_files->acquisition_size.begin() + index);
				}
				else index++;
			}
			//index++;
		}
	}
}

int		rename_replace(const char *src, const char *dest, const char *foldername, const char* function) {
	char				errnostring[MAX_STRING] = { 0 };
	int					return_value = 0;
		
	//bool same_file = strcmp(src, dest);
	if (strcmp(src, dest) != 0) {							//strcmp(src, dest)!=0 does not work ???
		if (file_exists(dest)) remove(dest);	//if (filesys::exists(CString2string((CString)dest))) does not work
		if (rename(src, dest) != 0) {
			//Sleep(FILEACCESS_WAIT_MS * 10);
			if (!file_exists(dest)) {
				return_value = errno;
				strcpy_s(errnostring, sizeof(errnostring), strerror(return_value));
				char msgtext[MAX_STRING] = { 0 };
				char shorttext[MAX_STRING] = { 0 };
				snprintf(shorttext, MAX_STRING, "cannot rename file in %s folder", foldername);
				snprintf(msgtext, MAX_STRING, "cannot rename file %s to %s in %s folder (error %s)\n", src, dest, foldername, errnostring);
				//Warning(WARNING_MESSAGE_BOX, shorttext, function, msgtext);
				Warning(FALSE, shorttext, function, msgtext);
			}
		}
	}
	return return_value;
}

void	UpdateProgress(const size_t acquisitions_to_be_processed, const size_t processed, const size_t children, const int nframe, const int frame_number, const char *QueueFilename) {
	size_t acquisitions_to_be_processed_local = 0;

	if (strlen(opts.DeTeCtQueueFilename)>0)			acquisitions_to_be_processed_local = NbFilesFromQueue((CString)QueueFilename) - NbItemFromQueue(_T("file_ko"), (CString)opts.DeTeCtQueueFilename, NULL, TRUE);
	if (acquisitions_to_be_processed_local == 0)	acquisitions_to_be_processed_local = acquisitions_to_be_processed;						//if single instance no QueueFilename anymore
	if ((nframe == 0) && (frame_number == 0))	CDeTeCtMFCDlg::getProgress()->SetPos((short)(MAX_RANGE_PROGRESS));
	else										CDeTeCtMFCDlg::getProgress()->SetPos((short)(MAX_RANGE_PROGRESS * ((float)nframe / (float)frame_number)));
												CDeTeCtMFCDlg::getProgress()->UpdateWindow();

	std::wstring totalProgress = L"Total\n(" + std::to_wstring(processed + children) + L"/" + std::to_wstring(acquisitions_to_be_processed_local) + L")";
												CDeTeCtMFCDlg::gettotalProgress()->SetWindowText(totalProgress.c_str());

	if ((nframe == 0) && (frame_number == 0))	CDeTeCtMFCDlg::getProgress_all()->SetPos((short)(MAX_RANGE_PROGRESS * ((float)(processed + children)) / ((float)acquisitions_to_be_processed_local)));
	else if (acquisitions_to_be_processed_local == processed + children) CDeTeCtMFCDlg::getProgress_all()->SetPos((short)(MAX_RANGE_PROGRESS));
	else 										CDeTeCtMFCDlg::getProgress_all()->SetPos((short)(MAX_RANGE_PROGRESS * ((float)(processed + children) + ((float)nframe / frame_number)) / ((float)acquisitions_to_be_processed_local)));
												CDeTeCtMFCDlg::getProgress_all()->UpdateWindow();
}

void	Show_matrix(const cv::Mat matrix, const char *title, const bool normalize_image, const int wait_ms) {
cv::UMat image_window;

if (normalize_image) {
	double minLum, maxLum;
	cv::minMaxLoc(matrix, &minLum, &maxLum, NULL, NULL);
	matrix.convertTo(image_window, CV_8U, 255.0 / maxLum, 0);
} else matrix.convertTo(image_window, CV_8U);
cv::imshow(title, image_window);
cv::waitKey(wait_ms);
image_window.~UMat();
}

/******************************************************************************************************************************************************************************************************************************************************************************************
									ALGORITHM FUNCTIONS
/******************************************************************************************************************************************************************************************************************************************************************************************/

bool is_point_black_in_frame(cv::Mat FrameMat, const cv::Point Point_to_check, const int delta_pixels, const float background_level) {
	// True if any of the + is black in frame
	//     +
	//   + + +
	//     +
	if (FrameMat.at<uchar>(Point_to_check.y, Point_to_check.x) <= background_level) return true;
	if (FrameMat.at<uchar>(MAX(Point_to_check.y - delta_pixels, 0), Point_to_check.x) <= background_level) return true;
	if (FrameMat.at<uchar>(Point_to_check.y, MAX(Point_to_check.x - delta_pixels, 0)) <= background_level) return true;
	if (FrameMat.at<uchar>(MIN(Point_to_check.y + delta_pixels, FrameMat.rows - 1), Point_to_check.x) <= background_level) return true;
	if (FrameMat.at<uchar>(Point_to_check.y, MIN(Point_to_check.x + delta_pixels, FrameMat.cols - 1)) <= background_level) return true;
	return false;
}

bool is_zone_black_around_point(const cv::Mat FrameMat, const cv::Point Point_to_check, const int delta_pixels, const float background_level) {
	// False if any of the + is not black or out of frame
	//                     +
	//     +           +       +
	//   + O +       +     O    +
	//     +          +        +  
	//                     +
	if ((Point_to_check.x + delta_pixels <= FrameMat.cols - 1)		&& (FrameMat.at<uchar>(Point_to_check.y, Point_to_check.x + delta_pixels)) > background_level)	return false;
	if ((Point_to_check.x - delta_pixels >= 0)						&& (FrameMat.at<uchar>(Point_to_check.y, Point_to_check.x - delta_pixels)) > background_level)	return false;
	if ((Point_to_check.y + delta_pixels <= FrameMat.rows - 1)		&& (FrameMat.at<uchar>(Point_to_check.y + delta_pixels, Point_to_check.x)) > background_level)	return false;
	if ((Point_to_check.y - delta_pixels >= 0)						&& (FrameMat.at<uchar>(Point_to_check.y - delta_pixels, Point_to_check.x)) > background_level)	return false;
	int	delta_diag_pixels = (int)((float)delta_pixels * cos(3.14159265 / 4.0));
	if ((Point_to_check.x + delta_diag_pixels <= FrameMat.cols - 1)	&& (Point_to_check.y + delta_diag_pixels <= FrameMat.rows - 1)	&& (FrameMat.at<uchar>(Point_to_check.y + delta_diag_pixels, Point_to_check.x + delta_diag_pixels)) > background_level)	return false;
	if ((Point_to_check.x - delta_diag_pixels >= 0)					&& (Point_to_check.y - delta_diag_pixels >= 0)					&& (FrameMat.at<uchar>(Point_to_check.y - delta_diag_pixels, Point_to_check.x - delta_diag_pixels)) > background_level) return false;
	if ((Point_to_check.x + delta_diag_pixels <= FrameMat.cols - 1)	&& (Point_to_check.y - delta_diag_pixels >= 0)					&& (FrameMat.at<uchar>(Point_to_check.y - delta_diag_pixels, Point_to_check.x + delta_diag_pixels)) > background_level) return false;
	if ((Point_to_check.x - delta_diag_pixels >= 0)					&& (Point_to_check.y + delta_diag_pixels <= FrameMat.rows - 1)	&& (FrameMat.at<uchar>(Point_to_check.y + delta_diag_pixels, Point_to_check.x - delta_diag_pixels)) > background_level)	return false;
	return true;
}


double planet_radius(const cv::Mat FrameMat, const double background_level) {
	std::array<double, 8> planet_radius{ 0,0,0,0,0,0,0,0 };

	planet_radius[0] = planet_radius_single_estimation(FrameMat, background_level, 0, 1, (FrameMat.cols - 1) / 2, (FrameMat.rows - 1) / 2, 0, (FrameMat.rows - 1) / 2);
	planet_radius[1] = planet_radius_single_estimation(FrameMat, background_level, (FrameMat.cols - 1), -1, (FrameMat.cols - 1) / 2, (FrameMat.rows - 1) / 2, 0, (FrameMat.rows - 1) / 2);
	planet_radius[2] = planet_radius_single_estimation(FrameMat, background_level, (FrameMat.cols - 1) / 2, 0, (FrameMat.cols - 1) / 2, 0, 1, (FrameMat.rows - 1) / 2);
	planet_radius[3] = planet_radius_single_estimation(FrameMat, background_level, (FrameMat.cols - 1) / 2, 0, (FrameMat.cols - 1) / 2, (FrameMat.rows - 1), -1, (FrameMat.rows - 1) / 2);

	int xmin;
	int xmax;
	int ymin;
	int ymax;
	int xcenter = (FrameMat.cols - 1) / 2;
	int ycenter = (FrameMat.rows - 1) / 2;
	if (FrameMat.cols > FrameMat.rows) {
		xmin = xcenter - ycenter;
		xmax = xcenter + ycenter;
		ymin = 0;
		ymax = (FrameMat.rows -1);
	}
	else {
		xmin = 0;
		xmax = (FrameMat.cols - 1);
		ymin = ycenter - xcenter;
		ymax = ycenter + xcenter;
	}
	planet_radius[4] = planet_radius_single_estimation(FrameMat, background_level, xmin, 1, xcenter, ymin, 1, ycenter);
	planet_radius[5] = planet_radius_single_estimation(FrameMat, background_level, xmax, -1, xcenter, ymax, -1, ycenter);
	planet_radius[6] = planet_radius_single_estimation(FrameMat, background_level, xmin, 1, xcenter, ymax, -1, ycenter);
	planet_radius[7] = planet_radius_single_estimation(FrameMat, background_level, xmax, -1, xcenter, ymin, 1, ycenter);
	//return ((planet_radius[0] + planet_radius[1]) / 2.0 + (planet_radius[2] + planet_radius[3]) / 2.0 + (planet_radius[4] + planet_radius[5]) / 2.0 + (planet_radius[6] + planet_radius[7]) / 2.0) / 4.0;

	//excluding 5 smallest and 2 biggest values - should cope with 2 jovian satellites, or Saturn's ring being in.
	std::sort(planet_radius.begin(), planet_radius.end());
	return (planet_radius[5]);
}

bool is_bright_point_valid(cv::Mat pGryMat, const cv::Point Point_to_check, const float background, double* pplanet_radius_estimation, double* pdistance_to_planet_center, bool *pIs_black, bool* pIs_outside_of_planet, bool* pIs_not_on_planet) {
	(*pIs_not_on_planet)		= false;
	(*pIs_outside_of_planet)	= false;
	(*pplanet_radius_estimation) = 0.0;
	(*pdistance_to_planet_center) = 0.0;

	// Checks if maxpoint value in frame is black (case: satellite shadow transit (bright in differential matrix, black in mean/current frame))
	(*pIs_black) = is_point_black_in_frame(pGryMat, Point_to_check, 1, background);
	if (*pIs_black) return false;

	// Checks if outside of planet disk checking black around (case satellite aside planet) and exclude solution
	(*pIs_outside_of_planet) = is_zone_black_around_point(pGryMat, Point_to_check, CROSS_DIFFERENTIAL_PHOTOMETRY_LMAX / 2, background);
	if ((*pIs_outside_of_planet)) return false;			// invalid if outside_of_planet or not not on planet

	// Checks if outside of planet disk checking distance from planet disk (case satellite aside planet) and exclude solution by setting maxLum to zero(other points will be used then)
	/*double planet_radius2 = MAX(pGryMat.cols, pGryMat.rows) / 2.0;*/
	(*pplanet_radius_estimation) = planet_radius(pGryMat, background);
	(*pdistance_to_planet_center) = sqrt(pow(Point_to_check.x - pGryMat.cols / 2.0, 2) + pow(Point_to_check.y - pGryMat.rows / 2.0, 2));
	(*pIs_not_on_planet) = ((*pdistance_to_planet_center) > (*pplanet_radius_estimation));
	return (!(*pIs_not_on_planet));		// invalid if outside_of_planet or not not on planet
//or//	
}

double planet_radius_single_estimation(const cv::Mat FrameMat, const double background_level, const unsigned int init_x, const int delta_x, const unsigned int last_x, const unsigned int init_y, const int delta_y, const unsigned int last_y) {
	//Checks if outside of planet disk checking distance from planet disk(case satellite aside planet) and exclude solution by setting maxLum to zero(other points will be used then)
	int x = init_x;
	int y = init_y;
	//while ((x >= 0) && (y >= 0) && !((x == last_x) || (y == last_y)) && (FrameMat.at<float>(y, x) <= background_level)) {
	//while ((x >= 0) && (y >= 0) && (x < (FrameMat.cols - 1)) && (y < (FrameMat.rows - 1)) && (FrameMat.at<float>(y, x) <= background_level)) {
	while ((x >= 0) && (y >= 0) && ((unsigned int) x != last_x) && ((unsigned int)y != last_y) && (x < FrameMat.cols) && (y < FrameMat.rows) && (FrameMat.at<uchar>(y, x) <= background_level)) {
		x += delta_x;
		y += delta_y;
	}
	return sqrt(pow(x - FrameMat.cols / 2.0, 2) + pow(y - FrameMat.rows / 2.0, 2));
}

bool check_bright_point_on_bright_line_column(cv::Mat pADUdtcMat, cv::Mat pADUdtcMat_invalid_points_corrected, const cv::Point brightestDtcImgPoint, bool* pIs_bright_line, bool* pIs_bright_column, const double distance_factor_from_edge, const double line_column_avg_min, const bool correct_line, const int correction_value) {
	// calculate average brightness of line in image for brightest point
	
	double distance_from_edge = distance_factor_from_edge * pADUdtcMat.rows;
	if ((brightestDtcImgPoint.y < distance_from_edge) || (brightestDtcImgPoint.y > (pADUdtcMat.rows - distance_from_edge))) {
		cv::Scalar line_average = cv::mean(pADUdtcMat.row(brightestDtcImgPoint.y));
		(*pIs_bright_line) = (line_average[0] > line_column_avg_min);
		if ((*pIs_bright_line) && (correct_line)) {
			cv::line(pADUdtcMat, cv::Point(0, brightestDtcImgPoint.y), cv::Point(pADUdtcMat.cols - 1, brightestDtcImgPoint.y), (correction_value, correction_value, correction_value), 1, 8, 0);
			cv::line(pADUdtcMat_invalid_points_corrected, cv::Point(0, brightestDtcImgPoint.y), cv::Point(pADUdtcMat_invalid_points_corrected.cols - 1, brightestDtcImgPoint.y), (correction_value, correction_value, correction_value), 1, 8, 0);
		}
	}
	else (*pIs_bright_line) = false;

	distance_from_edge = distance_factor_from_edge * pADUdtcMat.cols;
	if ((brightestDtcImgPoint.x < distance_from_edge) || (brightestDtcImgPoint.x > (pADUdtcMat.cols - distance_from_edge))) {
		cv::Scalar column_average = cv::mean(pADUdtcMat.col(brightestDtcImgPoint.x));
		(*pIs_bright_column) = (column_average[0] > line_column_avg_min);
		if ((*pIs_bright_column) && (correct_line)) {
			cv::line(pADUdtcMat, cv::Point(brightestDtcImgPoint.x, 0), cv::Point(brightestDtcImgPoint.x, pADUdtcMat.rows - 1), (correction_value, correction_value, correction_value), 1, 8, 0);
			cv::line(pADUdtcMat_invalid_points_corrected, cv::Point(brightestDtcImgPoint.x, 0), cv::Point(brightestDtcImgPoint.x, pADUdtcMat_invalid_points_corrected.rows - 1), (correction_value, correction_value, correction_value), 1, 8, 0);
		}
	} else (*pIs_bright_column) = false;

	return ((*pIs_bright_line) || (*pIs_bright_column));
}

/**********************************************************************************************//**
 * @fn	void dtcDrawImpact(cv::Mat frame, cv::Point point)
 *
 * @brief	Draw the point where the impact ocurrs as a "crosshair".
 *
 * @author	Jon
 * @date	2017-05-12
 *
 * @param	frame	The frame.
 * @param	point	The point of impact.
 **************************************************************************************************/

void dtcDrawImpact(const cv::Mat frame, const cv::Point point, const cv::Scalar colour, const double length, const bool variable_thickness, const unsigned int ROI_size) {

	//						longmin				longmax
	//		ROImax						lmax/2			lmax
	//		ROImin		lmax/4			lmax/2

	double lmax_calc;
	if	(ROI_size <= (const unsigned) CROSS_ROI_MIN)		lmax_calc = length / 2.0 ;
	else if (ROI_size >= (const unsigned) CROSS_ROI_MAX)	lmax_calc = length;
	else lmax_calc =										length / 2.0 + length * (1.0 / 4.0) * (ROI_size - CROSS_ROI_MIN) / (CROSS_ROI_MAX - CROSS_ROI_MIN);
	double lmin_calc = lmax_calc / 2;
	int lmin = (int) lmin_calc; 
	int lmax = (int)lmax_calc;

	int thickness = CROSS_SIZEMAX;
	if	((variable_thickness) && (ROI_size < (unsigned int) (CROSS_ROI_MIN + CROSS_ROI_MAX)/2)) thickness = CROSS_SIZEMIN;

	cv::line(frame, cv::Point(point.x + lmin, point.y), cv::Point(point.x + lmax, point.y), colour, thickness, 8, 0);
	cv::line(frame, cv::Point(point.x - lmax, point.y), cv::Point(point.x - lmin, point.y), colour, thickness, 8, 0);
	cv::line(frame, cv::Point(point.x, point.y - lmax), cv::Point(point.x, point.y - lmin), colour, thickness, 8, 0);
	cv::line(frame, cv::Point(point.x, point.y + lmin), cv::Point(point.x, point.y + lmax), colour, thickness, 8, 0);
}



bool	Is_frame_similarity_valid(const Similarity_type similarity_method, const Comparison_type comparison_frame_type, const int nframe, const int frame_errors, bool* pis_frame_errors, bool* pis_frame_duplicate, bool* pis_frame_errors_too_different,
			std::array<double, max_similarity>* psimilarity_reference, std::array<double, max_similarity>* psimilarity_reference_last_valid, std::array<double, max_similarity>* pdelta_similarity_reference, const cv::Mat pReferenceMat,
			std::array<double, max_similarity>* psimilarity_previous_frame, std::array<double, max_similarity>* psimilarity_previous_frame_last_valid, std::array<double, max_similarity>* pdelta_similarity_previous_frame, std::array<cv::Mat, max_similarity>* pPreviousFrameMat,
			const cv::Mat pGryMat) {
	
	if ((!opts.use_reference_similarity[SSIM] && !opts.use_previous_frame_similarity[SSIM])
		&& (!opts.use_reference_similarity[MSE] && !opts.use_previous_frame_similarity[MSE])
		&& (!opts.use_reference_similarity[NCC] && !opts.use_previous_frame_similarity[NCC])) return true;

	(*pis_frame_duplicate)				= false;
	(*pis_frame_errors)					= false;
	(*pis_frame_errors_too_different)	= false;
//
// checks similarity of the current frame with reference frame, or last valid frame
//
	//std::array<double, max_similarity> similarity_reference_decrease_min_pc = { opts.similarity_reference_decrease_min_pc[SSIM], 0.03, 0.08 }; //opts.similarity_reference_decrease_min_pc
	bool	is_frame_valid = true;
	double duplicate_detection_threshold = 0.999999;
	
	std::array<double, max_similarity> similarity_reference_last_valid_save			= { 0.0 };				// [SSIM, ME, NCC] to save last reference valid similarity for historic
	std::array<double, max_similarity> similarity_previous_frame_last_valid_save	= { 0.0 };				// [SSIM, ME, NCC] to save last reference valid similarity for historic

	// no similarity to compute
	if (((nframe - frame_errors) == 1) && (comparison_frame_type == Comparison_type::previous_frame)) (*pPreviousFrameMat)[similarity_method] = pGryMat.clone();
	
	// compute similarities needed
	else {
		if (comparison_frame_type == Comparison_type::reference)			similarity_reference_last_valid_save[similarity_method]		=	(*psimilarity_reference_last_valid)[similarity_method];
		if (comparison_frame_type == Comparison_type::previous_frame)		similarity_previous_frame_last_valid_save[similarity_method] =	(*psimilarity_previous_frame_last_valid)[similarity_method];

		if (similarity_method == SSIM) {
			if (comparison_frame_type == Comparison_type::reference)		(*psimilarity_reference)[similarity_method]			= dtcGetSimilarity_SSIM(pReferenceMat, pGryMat)[0];				//7ms
			if (comparison_frame_type == Comparison_type::previous_frame)	(*psimilarity_previous_frame)[similarity_method]	= dtcGetSimilarity_SSIM((*pPreviousFrameMat)[similarity_method], pGryMat)[0];
		}
		else if (similarity_method == MSE) {
			if (comparison_frame_type == Comparison_type::reference)		(*psimilarity_reference)[similarity_method]			= dtcGetSimilarity_MSE(pReferenceMat, pGryMat)[0];					//1ms
			if (comparison_frame_type == Comparison_type::previous_frame)	(*psimilarity_previous_frame)[similarity_method]	= dtcGetSimilarity_MSE((*pPreviousFrameMat)[similarity_method], pGryMat)[0];
		}
		else if (similarity_method == NCC) {
			if (comparison_frame_type == Comparison_type::reference)		(*psimilarity_reference)[similarity_method]			= dtcGetSimilarity_NCC(pReferenceMat, pGryMat)[0];					//5ms
			if (comparison_frame_type == Comparison_type::previous_frame)	(*psimilarity_previous_frame)[similarity_method]	= dtcGetSimilarity_NCC((*pPreviousFrameMat)[similarity_method], pGryMat)[0];
		}
	}

	// no or first similarity computed
	if ((nframe - frame_errors) <= 2) {
		if (comparison_frame_type == Comparison_type::reference)		(*psimilarity_reference_last_valid)[similarity_method]		= (*psimilarity_reference)[similarity_method];
		if (comparison_frame_type == Comparison_type::previous_frame)	(*psimilarity_previous_frame_last_valid)[similarity_method] = (*psimilarity_previous_frame_last_valid)[similarity_method];
	}

	// detection of duplicated frame
	if ((nframe - frame_errors) != 1) {
		if ((comparison_frame_type == Comparison_type::reference) && ((*psimilarity_reference)[similarity_method] >= duplicate_detection_threshold)) {
//			LogString(L"Duplicate of 1st frame frame #" + (CString)std::to_string(nframe).c_str() + L" (reference similarity(" + (CString)std::to_string(similarity_method).c_str() + "): " + (CString)std::to_string((*psimilarity_reference)[similarity_method]).c_str() + L")", output_filename, plog_counter, FALSE, pwait_count_total);
			(*psimilarity_reference)[similarity_method] = (*psimilarity_reference_last_valid)[similarity_method];
			(*pis_frame_duplicate) = true;
			is_frame_valid = true;
		}
		if ((comparison_frame_type == Comparison_type::previous_frame) && ((*psimilarity_previous_frame)[similarity_method] >= duplicate_detection_threshold)) {
//			LogString(L"Duplicate of 1st frame frame #" + (CString)std::to_string(nframe).c_str() + L" (previous frame similarity(" + (CString)std::to_string(similarity_method).c_str() + "): " + (CString)std::to_string((*psimilarity_previous_frame)[similarity_method]).c_str() + L")", output_filename, plog_counter, FALSE, pwait_count_total);
			(*psimilarity_previous_frame)[similarity_method] = (*psimilarity_previous_frame_last_valid)[similarity_method];
			(*pis_frame_duplicate) = true;
			is_frame_valid = true;
		}
	}
	// test decrease of similarity threshold to determine if frame is invalid
	if (comparison_frame_type == Comparison_type::reference) {
		(*pdelta_similarity_reference)[similarity_method] = (*psimilarity_reference)[similarity_method] - (*psimilarity_reference_last_valid)[similarity_method];
		if (!opts.use_all_algo_for_test && (*pdelta_similarity_reference)[similarity_method] <= -(opts.similarity_reference_decrease_min_pc[similarity_method] / 100.0)) {		// ignore frame when similarity is too much decreasing
			//(similarity_reference[similarity_method] > 0.0) && (similarity_reference_last_valid[similarity_method] > 0.0) &&*/ (/*(similarity_reference[similarity_method] <= 0.5) ||
			//LogString(L"Ignoring different frame #" + (CString)std::to_string(nframe).c_str() + L" (reference similarity(" + (CString)std::to_string(similarity_method).c_str() + "): " + (CString)std::to_string((*psimilarity_reference)[similarity_method]).c_str() + L" vs " + (CString)std::to_string((*psimilarity_reference_last_valid)[similarity_method]).c_str() + L" (" + (CString)std::to_string((*psimilarity_reference)[similarity_method] - (*psimilarity_reference_last_valid)[similarity_method]).c_str() + L")", output_filename, plog_counter, FALSE, pwait_count_total); (*psimilarity_reference_last_valid)[similarity_method] = similarity_reference_last_valid_save[similarity_method];		// do not take ignored frame as reference!
			(*pis_frame_errors) = true;
			(*pis_frame_errors_too_different) = true;
			is_frame_valid = false;
		}
		else {
			(*psimilarity_reference_last_valid)[similarity_method] = (*psimilarity_reference)[similarity_method];
		}
	}
	if (comparison_frame_type == Comparison_type::previous_frame) {
		(*pdelta_similarity_previous_frame)[similarity_method] = (*psimilarity_previous_frame)[similarity_method] - (*psimilarity_previous_frame_last_valid)[similarity_method];
		if (!opts.use_all_algo_for_test && (*pdelta_similarity_previous_frame)[similarity_method] <= -(opts.similarity_previous_frame_decrease_min_pc[similarity_method] / 100.0)) { // ignore frame when similarity is too much decreasing
			(*psimilarity_previous_frame_last_valid)[similarity_method] = similarity_previous_frame_last_valid_save[similarity_method];		// do not take ignored frame as reference!
//				LogString(L"Ignoring different frame #" + (CString)std::to_string(nframe).c_str() + L" (previous frame similarity(" + (CString)std::to_string(similarity_method).c_str() + "): " + (CString)std::to_string((*psimilarity_reference)[similarity_method]).c_str() + L" vs " + (CString)std::to_string((*psimilarity_reference_last_valid)[similarity_method]).c_str() + L" (" + (CString)std::to_string((*psimilarity_reference)[similarity_method] - (*psimilarity_reference_last_valid)[similarity_method]).c_str() + L")", output_filename, plog_counter, FALSE, pwait_count_total);
			(*pis_frame_errors) = true;
			(*pis_frame_errors_too_different) = true;
			is_frame_valid = false;
		}
		else {
			(*psimilarity_previous_frame_last_valid)[similarity_method] = (*psimilarity_previous_frame)[similarity_method];
			(*pPreviousFrameMat)[similarity_method] = pGryMat.clone();
		}
	}
	return is_frame_valid;
}