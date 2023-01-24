#pragma once
#ifndef ___DTCAS3_H_
#define ___DTCAS3_H_

#include "dtc.h"
#include "img2.hpp"
//extern "C" {
#include "max.h"
#include "datation.h"
#include "serfmt.h"
//}
#include "wrapper.h"
#include "wrapper2.hpp"
#include "auxfunc.hpp"
#include <string>
#include <vector>

#define AUTOSTAKKERTFILENAME	"AutoStakkert.exe"
#define LAUTOSTAKKERTFILENAME	L"AutoStakkert.exe"

// ************** AS!3 session and WJ derot files *************
void read_autostakkert_session_file(std::string configfile, std::string *filename, std::vector<cv::Point> *cm_list, int *cm_list_start, int *cm_list_end, int *cm_frame_count);
void read_winjupos_file(const std::string winjupos_derotation_filename, std::string *filename, const std::string extension);

// ************** AS! processes *************
int		AutostakkertInstancesNumber();
BOOL	IsParentAutostakkert(DWORD *pASpid);
BOOL	IsParentDeTeCt(DWORD *pASpid);
BOOL	IsParentAutostakkertRunning(const DWORD ASpid);


// ************** General functions *************

#endif