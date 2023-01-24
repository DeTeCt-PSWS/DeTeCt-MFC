#define _WIN32_WINNT _WIN32_WINNT_WINXP

#include <afx.h>
//#include <iostream>
#include "wrapper3.hpp"
#include "auxfunc.hpp"
#include "datation.h"

using namespace std;

/**********************************************************************************************
*
* @fn	void GetCreatedModifiedTimes(char *filename, double *pcreated_time, double *pmodified_time)
*
* @brief	Get creation time and last modified time from file information, in Julian Days LT
*
* @author	Marc
* @date		2019-04-24
*
* @param	filename   	File to be analyzed.
* @param	pcreated_time	Creation time in JD
* @param	pmodified_time 	Last modified time in JD
**************************************************************************************************/


void GetCreatedModifiedTimes(const char *filename, double *pcreated_time,  double *pmodified_time)
{
		CFileFind filefinder;
		CTime created_stamp;
		CTime modified_stamp;

		filefinder.FindFile(CA2W(filename));
		filefinder.FindNextFile();
		filefinder.GetCreationTime(created_stamp);
		filefinder.GetLastWriteTime(modified_stamp);
//DBOUT("Creation Date = " << created_stamp.Format(_T("%c")) << "\n");
//DBOUT("Modified Date = " << modified_stamp.Format(_T("%c")) << "\n");
		(*pcreated_time) = gregorian_calendar_to_jd(created_stamp.GetYear(), created_stamp.GetMonth(), created_stamp.GetDay(), created_stamp.GetHour(), created_stamp.GetMinute(), (double)(created_stamp.GetSecond()));
		(*pmodified_time) = gregorian_calendar_to_jd(modified_stamp.GetYear(), modified_stamp.GetMonth(), modified_stamp.GetDay(), modified_stamp.GetHour(), modified_stamp.GetMinute(), (double)(modified_stamp.GetSecond()));
}