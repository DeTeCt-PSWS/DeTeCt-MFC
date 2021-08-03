#pragma once
#ifndef __DATATION2_H_
#define __DATATION2_H_

#include "datation.h"
#include <iostream>
#include <fstream>

/**********************************************************************************************//**
 * @struct	_LogInfo
 *
 * @brief	Information for the log corresponding to each file analysed. 
 * 			Same as the old file with the confidence field added.
 *
 * @author	Jon
 * @date	2017-05-12
 **************************************************************************************************/

//#define DETECTLOGNAME "DeTeCt.log"

struct _LogInfo {
	char *filename;
	double start_time;
	double end_time;
	double duration;
	double fps;
	TIME_TYPE timetype;
	char *comment;
	int nb_impact;
	double confidence;
	double distance;
	double mean_stat[3];
	double mean2_stat[3];
	double max_mean_stat[3];
	double max_mean2_stat[3];
	double diff_stat[3];
	double diff2_stat[3];
	char *rating_classification;
	//double noise;

	_LogInfo(const char *fn, const double st, const double et, const double d,
		const double fs, const TIME_TYPE tt, const char *com, const int ni, double c, double dist, double mean_m[3], double mean2_m[3], double max_mean_m[3], double max_mean2_m[3], double diff_m[3], double diff2_m[3], const char *classification) {
		filename = (char*)fn;
		start_time = st;
		end_time = et;
		duration = d;
		fps = fs;
		timetype = tt;
		comment = (char*)com;
		nb_impact = ni;
		confidence = c;
		distance = dist;
		for (int i = 0; i < 3; i++) {
			mean_stat[i] = mean_m[i];
			mean2_stat[i] = mean2_m[i];
			max_mean_stat[i] = max_mean_m[i];
			max_mean2_stat[i] = max_mean2_m[i];
			diff_stat[i] = diff_m[i];
			diff2_stat[i] = diff2_m[i];
		}
		rating_classification = (char*)classification;
		//noise = CaptureInfo.noise;
	}
};

typedef struct _LogInfo LogInfo;

//void dtcWriteWholeLog(std::string location, const char *dtcexename, const double start_time, const double end_time, const double duration, const double fps, const TIME_TYPE timetype, const char *filename, const char *comment, const int nb_impact, const int print);

void dtcWriteLogHeader(std::string location);

//void dtcCloseLog(std::string location);

void dtcWriteLog2(const std::string location, const LogInfo video_info, DtcCaptureInfo CaptureInfo, std::stringstream *logline, int* pwaitms);

//void dtcWriteWholeLog(std::string location, std::vector<LogInfo> videos_info);

void fprint_jd_wj(std::ofstream *stream, const double jd);

void fprint_jd_wj_string(std::stringstream *stream, const double jd);

std::stringstream getRunTime();
std::stringstream getDateTime();
std::stringstream getDateTimeMillis();

#endif