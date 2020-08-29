/**********************************************************************************************//**
 * @file	datation2.cpp.
 *
 * @brief	Implements the datation 2 class.
 **************************************************************************************************/

#include <chrono>
#include <ctime>
#include <sstream> 
#include <string>
#include <iomanip>

#include "processes_queue.h"
#include "dtcgui.hpp"

#include <Windows.h> //after processes_queue.h

#include "datation2.h"
#include "dtc.h"
#include "common2.h"


/**********************************************************************************************//**
 * @fn	void fprint_jd_wj(std::ofstream *stream, const double jd)
 *
 * @brief	Prints the julian date into the file stream.
 *
 * @author	Jon
 * @date	2017-05-12
 *
 * @param [in,out]	stream	If non-null, the stream.
 * @param 		  	jd	  	The julian date.
 **************************************************************************************************/

void fprint_jd_wj(std::ofstream *stream, const double jd)
{
	double sec;
	int min;
	int hour;
	int day;
	int month;
	int year;
	long min_frac;
	double jd_frac;
	double jd_wj;
	long precision = 10 * 10 * 10 * 10 * 10 * 10;


	jd_frac = jd - ceil(jd);
	jd_wj = ceil(jd) + floor(0.5 + jd_frac * 24 * 60 * precision) / (24 * 60 * precision) + 0.0000000001;
	jd_to_date(jd_wj, &sec, &min, &hour, &day, &month, &year);

	min_frac = (long)ceil(floor(0.5 + sec*precision / 60.0));
	if (min_frac >= precision) {
		min_frac = (long)(precision - 1);
	}

	*stream << year << "/" << std::setfill('0') << std::setw(2) << month << "/" << std::setfill('0') << std::setw(2) << 
		day << " " << std::setfill('0') << std::setw(2) << hour << ":" << std::setfill('0') << std::setw(2) << min;
	*stream << "," << std::setfill('0') << std::setw(6) << (int)min_frac;
}

void fprint_jd_wj_string(std::stringstream *stream, const double jd)
{
	double sec;
	int min;
	int hour;
	int day;
	int month;
	int year;
	long min_frac;
	double jd_frac;
	double jd_wj;
	long precision = 10 * 10 * 10 * 10 * 10 * 10;


	jd_frac = jd - ceil(jd);
	jd_wj = ceil(jd) + floor(0.5 + jd_frac * 24 * 60 * precision) / (24 * 60 * precision) + 0.0000000001;
	jd_to_date(jd_wj, &sec, &min, &hour, &day, &month, &year);

	min_frac = (long)ceil(floor(0.5 + sec * precision / 60.0));
	if (min_frac >= precision) {
		min_frac = (long)(precision - 1);
	}

	*stream << year << "/" << std::setfill('0') << std::setw(2) << month << "/" << std::setfill('0') << std::setw(2) <<
		day << " " << std::setfill('0') << std::setw(2) << hour << ":" << std::setfill('0') << std::setw(2) << min;
	*stream << "," << std::setfill('0') << std::setw(6) << (int)min_frac;
}

/**********************************************************************************************//**
 * @fn	void dtcWriteWholeLog(std::string location, const char *dtcexename, const double start_time, 
 * 		const double end_time, const double duration, const double fps, const TIME_TYPE timetype, 
 * 		const char *filename, const char *comment, const int nb_impact, const int print)
 *
 * @brief	Dtc write whole log (only one item -- LOOK the function below).
 *
 * @author	Jon
 * @date	2017-05-12
 *
 * @param	location  	The location.
 * @param	dtcexename	The dtcexename.
 * @param	start_time	The start time.
 * @param	end_time  	The end time.
 * @param	duration  	The duration.
 * @param	fps		  	The FPS.
 * @param	timetype  	The timetype.
 * @param	filename  	Filename of the file.
 * @param	comment   	The comment.
 * @param	nb_impact 	The nb impact.
 * @param	print	  	The print.
 **************************************************************************************************/
/*
void dtcWriteWholeLog(std::string location, const char *dtcexename, const double start_time, const double end_time, const double duration, const double fps, const TIME_TYPE timetype, const char *filename, const char *comment, const int nb_impact, const int print)
{
	std::string dtclogfilename(dtcexename);

	location.append("DeTeCt.log");

	std::ofstream output_file(location);

	output_file << "DeTeCt; jovian impact detection software " << full_version.c_str() << "\n";
	output_file << "PLEASE SEND THIS FILE to Marc Delcroix - delcroix.marc@free.fr - for work on impact frequency (participants will be named if work is published) - NO DETECTION MATTERS!\n";
	output_file << "Rating;    Start;                      End;                        Mid;                       Duration (s);  fps (fr/s);  File;                        DeTeCt version and comment\n";

	if (nb_impact >= 0) 
		output_file << " " << nb_impact << "       ; ";
	else
		output_file << "Not known ";

	fprint_jd_wj(&output_file, start_time);
	switch (timetype) {
	case UT:
		output_file << " UT; ";
		break;
	case LT:
		output_file << " LT; ";
		break;
	default:
		output_file << " xx; ";
	}
	fprint_jd_wj(&output_file, end_time);
	switch (timetype) {
	case UT:
		output_file << " UT; ";
		break;
	case LT:
		output_file << " LT; ";
		break;
	default:
		output_file << " xx; ";
	}
	fprint_jd_wj(&output_file, (end_time + start_time) / 2);
	switch (timetype) {
	case UT:
		output_file << " UT; ";
		break;
	case LT:
		output_file << " LT; ";
		break;
	default:
		output_file << " xx; ";
	}
	output_file << std::setprecision(4) << std::fixed << duration << " s;\\t\\t";
	output_file << std::setprecision(3) << std::fixed << fps << " fr/s; ";
	output_file << filename << "; ";
	output_file << full_version.c_str() << " (" << comment << "); ";
	std::string os_version = "";
	GetOSversion(&os_version);
	output_file.close();
	if (output_file.is_open() != 0) {
		fprintf(stderr, "ERROR in dtcWriteLog closing file %s\n", dtclogfilename);
		exit(EXIT_FAILURE);
	}

	if (print) {
		printf("[");
		fprint_jd_wj(stdout, start_time);
		printf("-");
		fprint_jd_wj(stdout, end_time);
		printf(" ");
		switch (timetype) {
		case UT:
			printf("UT");
			break;
		case LT:
			printf("LT");
			break;
		default:
			printf("xx");
		}
		printf("], ");
		if (((end_time - start_time) >= 0.0) && ((end_time - start_time)<12.0 / 24.0)) {
			printf("mid ");
			fprint_jd_wj(stdout, (start_time + end_time) / 2);
			printf(" ");
			switch (timetype) {
			case UT:
				printf("UT, ");
				break;
			case LT:
				printf("LT, ");
				break;
			default:
				printf("xx, ");
			}
		}
		printf("%9.4lf s, ", duration);
		printf("%7.3lf fr/s, ", fps);
		printf("(%s)\n", comment);
	}
}
*/

/**********************************************************************************************//**
 * @fn	void dtcWriteWholeLog2(std::string location, std::vector<LogInfo> videos_info)
 *
 * @brief	Writes the whole log into a dtc.log file located in the root folder of the list (tree, in
 * 			other words), by iterating the list of the log information retrieved from the analysed files
 * 			themselves. 
 * 			Follows the same schema of the previous version, using std functions. 
 *
 * @author	Jon
 * @date	2017-05-12
 *
 * @param	location   	The location of the folder where dtc.log will be stored.
 * @param	videos_info	List of the information describing the algorithm output.
 **************************************************************************************************/
/*
void dtcWriteWholeLog(std::string location, std::vector<LogInfo> videos_info) {
	
	CT2A DeTeCtLogFilename(DeTeCt_additional_filename_from_folder(CString(location.c_str()), DTC_LOG_SUFFIX));

	std::ofstream output_file(DeTeCtLogFilename);

	output_file << "DeTeCt; jovian impact detection software " << full_version.c_str() << "\n";
	output_file << "PLEASE SEND THIS FILE to Marc Delcroix - delcroix.marc@free.fr - for work on impact frequency (participants will be named if work is published) - NO DETECTION MATTERS!\n";
	output_file << "confidence;		Rating;    Start;                      End;                        Mid;                        Duration (s);	fps (fr/s);  File;                        DeTeCt version and comment;\n";

	for (LogInfo video_info : videos_info) {
		output_file << std::setprecision(4) << std::fixed << video_info.confidence << "		";
		if (video_info.nb_impact >= 0)
			output_file << " " << video_info.nb_impact << "       ; ";
		else
			output_file << "Not known ";

		fprint_jd_wj(&output_file, video_info.start_time);
		switch (video_info.timetype) {
		case UT:
			output_file << " UT; ";
			break;
		case LT:
			output_file << " LT; ";
			break;
		default:
			output_file << " xx; ";
		}
		fprint_jd_wj(&output_file, video_info.end_time);
		switch (video_info.timetype) {
		case UT:
			output_file << " UT; ";
			break;
		case LT:
			output_file << " LT; ";
			break;
		default:
			output_file << " xx; ";
		}
		fprint_jd_wj(&output_file, (video_info.end_time + video_info.start_time) / 2);
		switch (video_info.timetype) {
		case UT:
			output_file << " UT; ";
			break;
		case LT:
			output_file << " LT; ";
			break;
		default:
			output_file << " xx; ";
		}
		output_file << std::setprecision(4) << std::fixed << video_info.duration << " s; ";
		output_file << std::setprecision(3) << std::fixed << video_info.fps << " fr/s; ";
		output_file << video_info.filename << "; ";
		output_file << full_version.c_str() << " (" << video_info.comment << "); ";
		std::string os_version = "";
		GetOSversion(&os_version);
		output_file << os_version.c_str() << "\n";
	}
	output_file.close();
}*/

void dtcWriteLogHeader(std::string location) {
	CT2A DeTeCtLogFilename(DeTeCt_additional_filename_from_folder(CString(location.c_str()), DTC_LOG_SUFFIX));

	std::ifstream filetest(DeTeCtLogFilename);
	if (!filetest) {
		std::ofstream output_file;

		output_file.open(DeTeCtLogFilename);

		output_file << "DeTeCt; jovian impact detection software " << full_version.c_str() << "\n";
		output_file << "PLEASE SEND THIS FILE to Marc Delcroix - delcroix.marc@free.fr - for work on impact frequency (participants will be named if work is published) - NO DETECTION MATTERS!\n";
		output_file << "confidence Rating    ;     Start                 ;     End                   ;     Mid                   ; Duration; fps (fr/s) ; File;                        DeTeCt version and comment; os_version; mean min;avg;max; mean2 min;avg;max; max-mean mean;avg;max; max-mean2 min;avg;max; diff min;avg;max; diff2 min;avg max; distance\n";
		//              0.0000	Null         ; 2011/07/01 15:56,595000 LT; 2011/07/01 15:56,650000 LT; 2011/07/01 15:56,622500 LT; 3.3000 s; 30.000 fr/s; G:\work\Impact\tests\...
		output_file.close();
	}
	else filetest.close();
}

/*
void dtcCloseLog(std::string location) {
	CT2A DeTeCtLogFilename(DeTeCt_additional_filename_from_folder(CString(location.c_str()), DTC_LOG_SUFFIX));

	std::ofstream output_file(DeTeCtLogFilename);

	output_file.close();
}*/

/**********************************************************************************************//**
* @fn	void dtcWriteWholeLog2(std::string location, std::vector<LogInfo> videos_info)
*
* @brief	Writes the whole log into a dtc.log file located in the root folder of the list (tree, in
* 			other words), by iterating the list of the log information retrieved from the analysed files
* 			themselves.
* 			Follows the same schema of the previous version, using std functions.
*
* @author	Jon
* @date		2018-02-22
*
* @param	location   	The location of the folder where dtc.log will be stored.
* @param	videos_info	List of the information describing the algorithm output.
																								**************************************************************************************************/
void dtcWriteLog2(std::string location, LogInfo video_info, std::stringstream *logline) {
	CT2A DeTeCtLogFilename(DeTeCt_additional_filename_from_folder(CString (location.c_str()), DTC_LOG_SUFFIX)); 
	std::ofstream output_file(DeTeCtLogFilename, std::ios_base::app);

	std::string os_version = "";
	GetOSversion(&os_version);

	if ((video_info.confidence < 0) && (video_info.nb_impact < 0)) {
		output_file << "Error	";
	} else if (!opts.dateonly) {
		output_file << std::setprecision(4) << std::fixed << video_info.confidence << "	";
	} else {
		output_file << "N/A		";
	}
	output_file << video_info.rating_classification << "; ";
	fprint_jd_wj(&output_file, video_info.start_time);
	switch (video_info.timetype) {
	case UT:
		output_file << " UT; ";
		break;
	case LT:
		output_file << " LT; ";
		break;
	default:
		output_file << " xx; ";
	}
	fprint_jd_wj(&output_file, video_info.end_time);
	switch (video_info.timetype) {
	case UT:
		output_file << " UT; ";
		break;
	case LT:
		output_file << " LT; ";
		break;
	default:
		output_file << " xx; ";
	}
	fprint_jd_wj(&output_file, (video_info.end_time + video_info.start_time) / 2);
	switch (video_info.timetype) {
	case UT:
		output_file << " UT; ";
		break;
	case LT:
		output_file << " LT; ";
		break;
	default:
		output_file << " xx; ";
	}
	output_file.setf(std::ios::right, std::ios::adjustfield);
	output_file << std::setfill(' ') << std::setw(8) << std::setprecision(4) << std::fixed << video_info.duration << " s; ";
	output_file << std::setfill(' ') << std::setw(7) << std::setprecision(3) << std::fixed << video_info.fps << " fr/s; ";
	output_file << video_info.filename << "; ";
	output_file << full_version.c_str() << " (" << video_info.comment << "); ";
	output_file << os_version.c_str() << "; ";
	if (!opts.dateonly) {
		output_file << std::setfill(' ') << std::setw(7) << std::setprecision(3) << video_info.mean_stat[0] << ";";
		output_file << std::setfill(' ') << std::setw(7) << std::setprecision(3) << video_info.mean_stat[1] << ";";
		output_file << std::setfill(' ') << std::setw(7) << std::setprecision(3) << video_info.mean_stat[2] << "; ";
		output_file << std::setfill(' ') << std::setw(7) << std::setprecision(3) << video_info.mean2_stat[0] << ";";
		output_file << std::setfill(' ') << std::setw(7) << std::setprecision(3) << video_info.mean2_stat[1] << ";";
		output_file << std::setfill(' ') << std::setw(7) << std::setprecision(3) << video_info.mean2_stat[2] << "; ";
		output_file << std::setfill(' ') << std::setw(7) << std::setprecision(3) << video_info.max_mean_stat[0] << ";";
		output_file << std::setfill(' ') << std::setw(7) << std::setprecision(3) << video_info.max_mean_stat[1] << ";";
		output_file << std::setfill(' ') << std::setw(7) << std::setprecision(3) << video_info.max_mean_stat[2] << "; ";
		output_file << std::setfill(' ') << std::setw(7) << std::setprecision(3) << video_info.max_mean2_stat[0] << ";";
		output_file << std::setfill(' ') << std::setw(7) << std::setprecision(3) << video_info.max_mean2_stat[1] << ";";
		output_file << std::setfill(' ') << std::setw(7) << std::setprecision(3) << video_info.max_mean2_stat[2] << "; ";
		output_file << std::setfill(' ') << std::setw(7) << std::setprecision(3) << video_info.diff_stat[0] << ";";
		output_file << std::setfill(' ') << std::setw(7) << std::setprecision(3) << video_info.diff_stat[1] << ";";
		output_file << std::setfill(' ') << std::setw(7) << std::setprecision(3) << video_info.diff_stat[2] << "; ";
		output_file << std::setfill(' ') << std::setw(7) << std::setprecision(3) << video_info.diff2_stat[0] << ";";
		output_file << std::setfill(' ') << std::setw(7) << std::setprecision(3) << video_info.diff2_stat[1] << ";";
		output_file << std::setfill(' ') << std::setw(7) << std::setprecision(3) << video_info.diff2_stat[2] << "; ";
		output_file << std::setfill(' ') << std::setw(8) << std::setprecision(3) << video_info.distance;
	} else output_file << ";;;;;;;;;;;;;;;;;";
	output_file << "\n";
	output_file.close();

	if ((video_info.confidence < 0) && (video_info.nb_impact < 0)) {
		(*logline) << "Error	";
	} else if (!opts.dateonly) {
		(*logline) << std::setprecision(4) << std::fixed << video_info.confidence << "	";
	}
	else {
		(*logline) << "N/A		";
	}
	(*logline) << video_info.rating_classification << "; ";
	fprint_jd_wj_string(logline, video_info.start_time);
	switch (video_info.timetype) {
	case UT:
		(*logline) << " UT; ";
		break;
	case LT:
		(*logline) << " LT; ";
		break;
	default:
		(*logline) << " xx; ";
	}
	fprint_jd_wj_string(logline, video_info.end_time);
	switch (video_info.timetype) {
	case UT:
		(*logline) << " UT; ";
		break;
	case LT:
		(*logline) << " LT; ";
		break;
	default:
		(*logline) << " xx; ";
	}
	fprint_jd_wj_string(logline, (video_info.end_time + video_info.start_time) / 2);
	switch (video_info.timetype) {
	case UT:
		(*logline) << " UT; ";
		break;
	case LT:
		(*logline) << " LT; ";
		break;
	default:
		(*logline) << " xx; ";
	}
	(*logline).setf(std::ios::right, std::ios::adjustfield);
	(*logline) << std::setfill(' ') << std::setw(8) << std::setprecision(4) << std::fixed << video_info.duration << " s; ";
	(*logline) << std::setfill(' ') << std::setw(7) << std::setprecision(3) << std::fixed << video_info.fps << " fr/s; ";
	(*logline) << video_info.filename << "; ";
	(*logline) << full_version.c_str() << " (" << video_info.comment << "); ";
	(*logline) << os_version.c_str() << "; ";
	if (!opts.dateonly) {
		(*logline) << std::setfill(' ') << std::setw(7) << std::setprecision(3) << video_info.mean_stat[0] << ";";
		(*logline) << std::setfill(' ') << std::setw(7) << std::setprecision(3) << video_info.mean_stat[1] << ";";
		(*logline) << std::setfill(' ') << std::setw(7) << std::setprecision(3) << video_info.mean_stat[2] << "; ";
		(*logline) << std::setfill(' ') << std::setw(7) << std::setprecision(3) << video_info.mean2_stat[0] << ";";
		(*logline) << std::setfill(' ') << std::setw(7) << std::setprecision(3) << video_info.mean2_stat[1] << ";";
		(*logline) << std::setfill(' ') << std::setw(7) << std::setprecision(3) << video_info.mean2_stat[2] << "; ";
		(*logline) << std::setfill(' ') << std::setw(7) << std::setprecision(3) << video_info.max_mean_stat[0] << ";";
		(*logline) << std::setfill(' ') << std::setw(7) << std::setprecision(3) << video_info.max_mean_stat[1] << ";";
		(*logline) << std::setfill(' ') << std::setw(7) << std::setprecision(3) << video_info.max_mean_stat[2] << "; ";
		(*logline) << std::setfill(' ') << std::setw(7) << std::setprecision(3) << video_info.max_mean2_stat[0] << ";";
		(*logline) << std::setfill(' ') << std::setw(7) << std::setprecision(3) << video_info.max_mean2_stat[1] << ";";
		(*logline) << std::setfill(' ') << std::setw(7) << std::setprecision(3) << video_info.max_mean2_stat[2] << "; ";
		(*logline) << std::setfill(' ') << std::setw(7) << std::setprecision(3) << video_info.diff_stat[0] << ";";
		(*logline) << std::setfill(' ') << std::setw(7) << std::setprecision(3) << video_info.diff_stat[1] << ";";
		(*logline) << std::setfill(' ') << std::setw(7) << std::setprecision(3) << video_info.diff_stat[2] << "; ";
		(*logline) << std::setfill(' ') << std::setw(7) << std::setprecision(3) << video_info.diff2_stat[0] << ";";
		(*logline) << std::setfill(' ') << std::setw(7) << std::setprecision(3) << video_info.diff2_stat[1] << ";";
		(*logline) << std::setfill(' ') << std::setw(7) << std::setprecision(3) << video_info.diff2_stat[2] << "; ";
		(*logline) << std::setfill(' ') << std::setw(8) << std::setprecision(3) << video_info.distance;
	}
	else (*logline) << ";;;;;;;;;;;;;;;;;";
	(*logline) << "\n";
}

/**********************************************************************************************//**
 * @fn	std::stringstream getRunTime()
 *
 * @brief	Gets the date and time for the interface messages.
 *
 * @author	Jon
 * @date	2017-05-12
 *
 * @return	The date and time in ISO format.
 **************************************************************************************************/
std::stringstream getRunTime() {
	auto now = std::chrono::system_clock::now();
	auto now_time_t = std::chrono::system_clock::to_time_t(now);

	std::stringstream ss;
	ss << std::put_time(std::localtime(&now_time_t), "%Y-%m-%d_%H-%M-%S");
	return ss;
}

/**********************************************************************************************//**
* @fn	std::stringstream getDateTime()
*
* @brief	Gets the date and time for the interface messages.
*
* @author	Jon/Marc
* @date	2017-05-12
*
* @return	The date and time for logging
**************************************************************************************************/
std::stringstream getDateTime() {
	auto now = std::chrono::system_clock::now();
	auto now_time_t = std::chrono::system_clock::to_time_t(now);

	std::stringstream ss;
	ss << std::put_time(std::localtime(&now_time_t), "%Y-%m-%d %X - ");
	return ss;
}

std::stringstream getDateTimeMillis() {
	auto now = std::chrono::system_clock::now();
	auto now_time_t = std::chrono::system_clock::to_time_t(now);

	std::stringstream ss;

	char buffer[80];

	auto transformed = now.time_since_epoch().count() / 1000000;
	auto millis = transformed % 1000;
	sprintf(buffer, ".%03d - ", (int)millis);
	CString tmp_cstring;
	char2CString(buffer, &tmp_cstring);
	std::string tmp_string = CString2string(tmp_cstring);


	ss << std::put_time(std::localtime(&now_time_t), "%Y-%m-%d %X") << tmp_string.c_str();
	return ss;

	//return std::string(buffer);
}