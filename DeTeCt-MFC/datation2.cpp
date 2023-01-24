/**********************************************************************************************//**
 * @file	datation2.cpp.
 *
 * @brief	Implements the datation 2 class.
 **************************************************************************************************/

//#include <chrono>	//not used
//#include <ctime>	//not used
//#include <sstream>	//not used
//#include <string>	//not used
//#include <iomanip>	//not used

#include "processes_queue.hpp"
#include "dtcgui.hpp"

//#include <Windows.h> //after processes_queue.h  //not used

#include "datation2.hpp"
//#include "dtc.h"	//not used
#include "common2.hpp"
#include <numeric>      // std::iota
#include <iomanip>		// test OpenCV 4.7.0


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

	//*stream << year << "/" << std::setfill('0') << std::setw(2) << month << "/" << std::setfill('0') << std::setw(2) <<
	*stream << std::setfill('0') << std::setw(4) << year << "/" << std::setfill('0') << std::setw(2) << month << "/" << std::setfill('0') << std::setw(2) <<
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
//		fprintf(stderr, "ERROR in dtcWriteLog closing file %s\n", dtclogfilename);
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

	//std::ifstream filetest(DeTeCtLogFilename);
	//if (!filetest) {
	if (!filesys::exists(CString2string((CString)DeTeCtLogFilename))) {
		std::ofstream output_file;

		output_file.open(DeTeCtLogFilename);

		output_file << "DeTeCt; jovian impact detection software " << full_version.c_str() << "\n";
		output_file << "PLEASE SEND THIS FILE to Marc Delcroix - delcroix.marc@free.fr - for work on impact frequency (participants will be named if work is published) - NO DETECTION MATTERS!\n";
		//output_file << "confidence Rating   ; Start                     ; End                       ; Mid                       ; Duration  ; fps (fr/s)  ; File;                        DeTeCt version and comment; os_version; mean min;avg;max; mean2 min;avg;max; max-mean mean;avg;max; max-mean2 min;avg;max; diff min;avg;max; diff2 min;avg max; distance; Observer ; Location ; Scope ; Camera ; Filter ; Profile ; Diameter (arcsec) ; Magnitude ; Central Meridian (°) ;  Focal length (mm) ; Resolution (arcsec) ; Binning ; Bit depth ; Debayer ; Exposure (ms) ; Gain ; Gamma ; Auto exposure ; Software gain ; Auto histogram ; Brightness ; Auto gain ; Histogram min ; Histogram max ; Histogram (%) ; Noise ; Prefilter ; Sensor temperature (°C) ; Target                                                                                                                                                                                                                                                                                                      ; width ; height \n";
		output_file << "confidence Rating    ; Start                     ; End                       ; Mid                       ; Duration   ; fps (fr/s)  ; File                                                                                                                                                                                                                                                           ; DeTeCt version and comment                                                                      ; os_version        ;mean min; avg   ; max   ;mean2 min; avg   ; max   ;max-mean mean; avg   ; max   ;max-mean2 min; avg   ; max   ;diff min; avg   ; max   ;diff2 min; avg   ; max   ; distance  ; Observer                        ; Location                        ; Scope                           ; Camera                          ; Filter          ; Profile         ; Diameter (arcsec); Magnitude; Central Meridian (°)        ; Focal length (mm); Resolution (arcsec); Binning; Bit depth; Debayer; Exposure (ms); Gain; Gamma; Auto exposure; Software gain; Auto histogram; Brightness; Auto gain; Histogram min; Histogram max; Histogram (%);    Noise; Prefilter                       ; Sensor temperature (°C); Target                                                                                                                                                                                                                                                                                                      ; width; height; ";
		output_file << "\n";
		//              0.0000	Null         ; 2011/07/01 15:56,595000 LT; 2011/07/01 15:56,650000 LT; 2011/07/01 15:56,622500 LT;   3.3000 s; 3 0.000 fr/s; G:\work\Impact\tests\...
		output_file.close();
	}
//	else filetest.close();
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

void dtcWriteLog2(const std::string location, const LogInfo video_info, const DtcCaptureInfo CaptureInfo, std::stringstream *logline, int* pwaitms) {
	//CT2A DeTeCtLogFilename(DeTeCt_additional_filename_from_folder(CString (location.c_str()), DTC_LOG_SUFFIX));

	CString output_filename = DeTeCt_additional_filename_from_folder(CString(location.c_str()), DTC_LOG_SUFFIX);
	
	if (!filesys::exists(CString2string(output_filename))) {
		 char msgtext[MAX_STRING] = { 0 };
		char tmpline[MAX_STRING];
										
		snprintf(msgtext, MAX_STRING, "cannot find log file %s", CString2char(output_filename, tmpline));
		ErrorExit(TRUE, "cannot find log file", __func__, msgtext);
	}
	*pwaitms += NbWaitedUnlockedFile(output_filename, FILEACCESS_WAIT_MS);
	std::ofstream output_file(output_filename, std::ios_base::app);

	std::string os_version = "";
	GetOSversion(&os_version);

	char str_tmp[MAX_STRING]	= { 0 };
	char str_tmp2[MAX_STRING]	= { 0 };

	if ((video_info.confidence < 0) && (video_info.nb_impact < 0)) {
		output_file << "Error   ";
	} else if (!opts.dateonly) {
		output_file << std::setfill(' ') << std::setw(7) << std::setprecision(4) << std::fixed << video_info.confidence << " ";
	} else {
		output_file << "N/A    ";
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
	output_file << std::setfill(' ') << std::setw(9) << std::setprecision(4) << std::fixed << video_info.duration << " s;";
	output_file << std::setfill(' ') << std::setw(8) << std::setprecision(3) << std::fixed << video_info.fps << " fr/s; ";
	//output_file << video_info.filename << std::setfill(' ') << std::setw(255) << std::left << " " << "; ";
	output_file << str_trail_fill(video_info.filename, " ", 255, str_tmp) << "; ";
	strcpy(str_tmp2, full_version.c_str());
	strcat(str_tmp2, " (");
	strcat(str_tmp2, video_info.comment);
	strcat(str_tmp2, ")");
	//output_file << full_version.c_str() << " (" << video_info.comment << ")" <<std::setfill(' ') << std::setw(64) << "; ";
	output_file << str_trail_fill(str_tmp2, " ", 96, str_tmp) << "; ";
	//output_file << std::setfill(' ') << std::setw(18) << os_version.c_str() << "; ";
	output_file << str_trail_fill(os_version.c_str(), " ", 18, str_tmp) << "; ";
	//if (!opts.dateonly) {
	if ((!opts.dateonly) && (&CaptureInfo != NULL)) {
		output_file << std::setfill(' ') << std::setw(7) << std::setprecision(3) << video_info.mean_stat[0] << ";";
		output_file << std::setfill(' ') << std::setw(7) << std::setprecision(3) << video_info.mean_stat[1] << ";";
		output_file << std::setfill(' ') << std::setw(7) << std::setprecision(3) << video_info.mean_stat[2] << "; ";
		output_file << std::setfill(' ') << std::setw(8) << std::setprecision(3) << video_info.mean2_stat[0] << ";";
		output_file << std::setfill(' ') << std::setw(7) << std::setprecision(3) << video_info.mean2_stat[1] << ";";
		output_file << std::setfill(' ') << std::setw(7) << std::setprecision(3) << video_info.mean2_stat[2] << "; ";
		output_file << std::setfill(' ') << std::setw(12) << std::setprecision(3) << video_info.max_mean_stat[0] << ";";
		output_file << std::setfill(' ') << std::setw(7) << std::setprecision(3) << video_info.max_mean_stat[1] << ";";
		output_file << std::setfill(' ') << std::setw(7) << std::setprecision(3) << video_info.max_mean_stat[2] << "; ";
		output_file << std::setfill(' ') << std::setw(12) << std::setprecision(3) << video_info.max_mean2_stat[0] << ";";
		output_file << std::setfill(' ') << std::setw(7) << std::setprecision(3) << video_info.max_mean2_stat[1] << ";";
		output_file << std::setfill(' ') << std::setw(7) << std::setprecision(3) << video_info.max_mean2_stat[2] << "; ";
		output_file << std::setfill(' ') << std::setw(7) << std::setprecision(3) << video_info.diff_stat[0] << ";";
		output_file << std::setfill(' ') << std::setw(7) << std::setprecision(3) << video_info.diff_stat[1] << ";";
		output_file << std::setfill(' ') << std::setw(7) << std::setprecision(3) << video_info.diff_stat[2] << "; ";
		output_file << std::setfill(' ') << std::setw(8) << std::setprecision(3) << video_info.diff2_stat[0] << ";";
		output_file << std::setfill(' ') << std::setw(7) << std::setprecision(3) << video_info.diff2_stat[1] << ";";
		output_file << std::setfill(' ') << std::setw(7) << std::setprecision(3) << video_info.diff2_stat[2] << "; ";
		output_file << std::setfill(' ') << std::setw(10) << std::setprecision(3) << video_info.distance << "; ";
	}
	else {
		output_file << std::setfill(' ') << std::setw(7) << ";";
		output_file << std::setfill(' ') << std::setw(7) << ";";
		output_file << std::setfill(' ') << std::setw(7) << "; ";
		output_file << std::setfill(' ') << std::setw(8) << ";";
		output_file << std::setfill(' ') << std::setw(7) << ";";
		output_file << std::setfill(' ') << std::setw(7) << "; ";
		output_file << std::setfill(' ') << std::setw(12) << ";";
		output_file << std::setfill(' ') << std::setw(7) << ";";
		output_file << std::setfill(' ') << std::setw(7) << "; ";
		output_file << std::setfill(' ') << std::setw(12) << ";";
		output_file << std::setfill(' ') << std::setw(7) << ";";
		output_file << std::setfill(' ') << std::setw(7) << "; ";
		output_file << std::setfill(' ') << std::setw(7) << ";";
		output_file << std::setfill(' ') << std::setw(7) << ";";
		output_file << std::setfill(' ') << std::setw(7) << "; ";
		output_file << std::setfill(' ') << std::setw(8) << ";";
		output_file << std::setfill(' ') << std::setw(7) << ";";
		output_file << std::setfill(' ') << std::setw(7) << "; ";
		output_file << std::setfill(' ') << std::setw(10) << "; ";
	}
	//output_file << std::setfill(' ') << std::setw(32) << CaptureInfo.observer << "; ";
	output_file << str_trail_fill(CaptureInfo.observer, " ", 32, str_tmp) << "; ";
	//output_file << std::setfill(' ') << std::setw(32) << CaptureInfo.location << "; ";
	output_file << str_trail_fill(CaptureInfo.location, " ", 32, str_tmp) << "; ";
	//output_file << std::setfill(' ') << std::setw(32) << CaptureInfo.scope << "; ";
	output_file << str_trail_fill(CaptureInfo.scope, " ", 32, str_tmp) << "; ";
	//output_file << std::setfill(' ') << std::setw(32) << CaptureInfo.camera << "; ";
	output_file << str_trail_fill(CaptureInfo.camera, " ", 32, str_tmp) << "; ";
	//output_file << std::setfill(' ') << std::setw(16) << CaptureInfo.filter << "; ";
	output_file << str_trail_fill(CaptureInfo.filter, " ", 16, str_tmp) << "; ";
	//output_file << std::setfill(' ') << std::setw(16) << CaptureInfo.profile << "; ";
	output_file << str_trail_fill(CaptureInfo.profile, " ", 16, str_tmp) << "; ";
	if (CaptureInfo.diameter_arcsec >= 0) 		output_file << std::setfill(' ') << std::setw(17) << std::setprecision(2) << CaptureInfo.diameter_arcsec << "; ";
	else										output_file << str_trail_fill("", " ", 17, str_tmp) << "; ";
	if (CaptureInfo.magnitude >= -10000) 		output_file << std::setfill(' ') << std::setw(9) << std::setprecision(2) << CaptureInfo.magnitude << "; ";
	else								 		output_file << str_trail_fill("", " ", 9, str_tmp) << "; ";
	output_file << str_trail_fill(CaptureInfo.centralmeridian, " ", 28, str_tmp) << "; ";
	if (CaptureInfo.focallength_mm >= 0) 		output_file << std::setfill(' ') << std::setw(17) << std::setprecision(0) << CaptureInfo.focallength_mm << "; ";
	else								 		output_file << str_trail_fill("", " ", 17, str_tmp) << "; ";
	if (CaptureInfo.resolution >= 0) 			output_file << std::setfill(' ') << std::setw(19) << std::setprecision(2) << CaptureInfo.resolution << "; ";
	else								 		output_file << str_trail_fill("", " ", 19, str_tmp) << "; ";
	output_file << str_trail_fill(CaptureInfo.binning, " ", 7, str_tmp) << "; ";
	if (CaptureInfo.bitdepth >= 0) 				output_file << std::setfill(' ') << std::setw(9) << std::setprecision(0) << CaptureInfo.bitdepth << "; ";
	else								 		output_file << str_trail_fill("", " ", 9, str_tmp) << "; ";
	if (CaptureInfo.debayer == False)			output_file << str_trail_fill("off", " ", 7, str_tmp) << "; ";
	else if (CaptureInfo.debayer == True)		output_file << str_trail_fill("on", " ", 7, str_tmp) << "; ";
	else								 		output_file << str_trail_fill("", " ", 7, str_tmp) << "; ";
	if (CaptureInfo.exposure_ms >= 0) 			output_file << std::setfill(' ') << std::setw(13) << std::setprecision(3) << CaptureInfo.exposure_ms << "; ";
	else								 		output_file << str_trail_fill("", " ", 13, str_tmp) << "; ";
	if (CaptureInfo.gain >= 0) 					output_file << std::setfill(' ') << std::setw(4) << std::setprecision(0) << CaptureInfo.gain << "; ";
	else								 		output_file << str_trail_fill("", " ", 4, str_tmp) << "; ";
	if (CaptureInfo.gamma >= 0) 				output_file << std::setfill(' ') << std::setw(5) << std::setprecision(0) << CaptureInfo.gamma << "; ";
	else								 		output_file << str_trail_fill("", " ", 5, str_tmp) << "; ";
	if (CaptureInfo.autoexposure == False)		output_file << str_trail_fill("off", " ", 13, str_tmp) << "; ";
	else if (CaptureInfo.autoexposure == True)	output_file << str_trail_fill("on", " ", 13, str_tmp) << "; ";
	else								 		output_file << str_trail_fill("", " ", 13, str_tmp) << "; ";
	if (CaptureInfo.softwaregain >= 0) 			output_file << std::setfill(' ') << std::setw(13) << std::setprecision(0) << CaptureInfo.softwaregain << "; ";
	else								 		output_file << str_trail_fill("", " ", 13, str_tmp) << "; ";
	if (CaptureInfo.autohisto >= 0) 			output_file << std::setfill(' ') << std::setw(14) << std::setprecision(0) << CaptureInfo.autohisto << "; ";
	else								 		output_file << str_trail_fill("", " ", 14, str_tmp) << "; ";
	if (CaptureInfo.brightness >= 0) 			output_file << std::setfill(' ') << std::setw(10) << std::setprecision(0) << CaptureInfo.brightness << "; ";
	else								 		output_file << str_trail_fill("", " ", 10, str_tmp) << "; ";
	if (CaptureInfo.autogain == False)			output_file << str_trail_fill("off", " ", 9, str_tmp) << "; ";
	else if (CaptureInfo.autogain == True)		output_file << str_trail_fill("on", " ", 9, str_tmp) << "; ";
	else								 		output_file << str_trail_fill("", " ", 9, str_tmp) << "; ";
	if (CaptureInfo.histmin >= 0) 				output_file << std::setfill(' ') << std::setw(13) << std::setprecision(0) << CaptureInfo.histmin << "; ";
	else								 		output_file << str_trail_fill("", " ", 13, str_tmp) << "; ";
	if (CaptureInfo.histmax >= 0) 				output_file << std::setfill(' ') << std::setw(13) << std::setprecision(0) << CaptureInfo.histmax << "; ";
	else								 		output_file << str_trail_fill("", " ", 13, str_tmp) << "; ";
	if (CaptureInfo.histavg_pc >= 0) 			output_file << std::setfill(' ') << std::setw(13) << std::setprecision(0) << CaptureInfo.histavg_pc << "; ";
	else								 		output_file << str_trail_fill("", " ", 13, str_tmp) << "; ";
	if (CaptureInfo.noise >= 0) 				output_file << std::setfill(' ') << std::setw(8) << std::setprecision(2) << CaptureInfo.noise << "; ";
	else								 		output_file << str_trail_fill("", " ", 8, str_tmp) << "; ";
	output_file << str_trail_fill(CaptureInfo.prefilter, " ", 32, str_tmp) << "; ";
	if (CaptureInfo.temp_C >= 0) 				output_file << std::setfill(' ') << std::setw(23) << std::setprecision(1) << CaptureInfo.temp_C << "; ";
	else								 		output_file << str_trail_fill("", " ", 23, str_tmp) << "; ";
	output_file << str_trail_fill(CaptureInfo.target, " ", 300, str_tmp) << "; ";
	output_file << std::setfill(' ') << std::setw(5) << std::setprecision(3) << video_info.ROI_width << "; ";
	output_file << std::setfill(' ') << std::setw(6) << std::setprecision(3) << video_info.ROI_height << "; ";

	output_file << std::endl;
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
	(*logline) << std::setfill(' ') << std::setw(8) << std::setprecision(4) << std::fixed << video_info.duration << " s;";
	(*logline) << std::setfill(' ') << std::setw(8) << std::setprecision(3) << std::fixed << video_info.fps << " fr/s; ";
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

bool compare_string_fullfilenames(std::string string1, std::string string2) {
	std::string path1		= string1.substr(0, string1.find_last_of("\\"));
	std::string path2		= string2.substr(0, string2.find_last_of("\\"));
	std::string filename1	= string1.substr(string1.find_last_of("\\") + 1, string1.size() - (string1.find_last_of("\\") + 1));
	std::string filename2	= string2.substr(string2.find_last_of("\\") + 1, string2.size() - (string2.find_last_of("\\") + 1));
	size_t min_path_size = MIN(path1.size(), path2.size());
	lowercase_string(&path1);
	lowercase_string(&path2);
	lowercase_string(&filename1);
	lowercase_string(&filename2);

	if (path1.compare(path2) == 0)														return (filename1 < filename2);
	if (path1.size() == path2.size())													return (path1 < path2);
	if (path1.substr(0, min_path_size).compare(path2.substr(0, min_path_size)) == 0)	return (path1 < path2);
																						return (path1.substr(0, min_path_size) < path2.substr(0, min_path_size));
}

void dtcSortLog(const char* source_filename, const char* dest_filename) {
	std::vector<std::string>	lines;
	std::vector<std::string>	sort_fields;
	int nb_header_lines = 3;
	std::string line;

/*	//Writes header
	dtcWriteLogHeader(dest_filename);
	//Count header lines #
	std::ifstream TmpFile(dest_filename, std::ifstream::in);
	while (std::getline(TmpFile, line)) nb_header_lines++;
	TmpFile.close();*/

	//Read input file ignoring header
	std::ifstream InputFile(source_filename, std::ifstream::in);
	std::ofstream OutputFile(dest_filename, std::ofstream::out);
	int nb_lines = 0;
	while (std::getline(InputFile, line)) {
		if (nb_lines < nb_header_lines) OutputFile << line << std::endl;
		else {
			lines.push_back(line);
			sort_fields.push_back(line.substr(134, 389-134));
		}
		nb_lines++;
	}
	InputFile.close();

	//Sorts field
	// sort indexes based on comparing values in v using std::stable_sort instead of std::sort to avoid unnecessary index re-orderings when v contains elements of equal values 
	//vector<size_t> sort_indexes(const vector<T>& v) {
	std::vector<size_t> sorted_indexes(lines.size());
	iota(sorted_indexes.begin(), sorted_indexes.end(), 0);
	stable_sort(sorted_indexes.begin(), sorted_indexes.end(), [&](size_t i1, size_t i2) {return compare_string_fullfilenames(sort_fields[i1], sort_fields[i2]); });

	//Write sorted file
//	std::ofstream OutputFile(dest_filename, std::ios_base::app);
	size_t index = 0;
	while (index < sorted_indexes.size()) OutputFile << lines[sorted_indexes[index++]] << std::endl;
	OutputFile.close();
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