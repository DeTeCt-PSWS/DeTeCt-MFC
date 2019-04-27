#pragma once
#ifndef __DATATION_H_
#define __DATATION_H_

#include "datation.h"
#include <iostream>
#include <fstream>

/**********************************************************************************************//**
 * @struct	_LogInfo
 *
 * @brief	Information for the log corresponding to each file analysed. 
 * 			Same as the old file with the certainty field added.
 *
 * @author	Jon
 * @date	2017-05-12
 **************************************************************************************************/

#define DETECTLOGNAME "DeTeCt.log"

struct _LogInfo {
	char *filename;
	double start_time;
	double end_time;
	double duration;
	double fps;
	TIME_TYPE timetype;
	char *comment;
	int nb_impact;
	double certainty;

	_LogInfo(const char *fn, const double st, const double et, const double d,
		const double fs, const TIME_TYPE tt, const char *com, const int ni, double c) {
		filename = (char*)fn;
		start_time = st;
		end_time = et;
		duration = d;
		fps = fs;
		timetype = tt;
		comment = (char*)com;
		nb_impact = ni;
		certainty = c;
	}
};

typedef struct _LogInfo LogInfo;

//void dtcWriteWholeLog(std::string location, const char *dtcexename, const double start_time, const double end_time, const double duration, const double fps, const TIME_TYPE timetype, const char *filename, const char *comment, const int nb_impact, const int print);

void dtcWriteLogHeader(std::string location);

void dtcCloseLog(std::string location);

void dtcWriteLog2(std::string location, LogInfo video_info);

void dtcWriteWholeLog(std::string location, std::vector<LogInfo> videos_info);

void fprint_jd_wj(std::ofstream *stream, const double jd);

std::stringstream getRunTime();

std::stringstream getDateTime();

#endif