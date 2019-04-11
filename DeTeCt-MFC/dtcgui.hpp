#pragma once

#include <string>
#include <iostream>
#include <stdio.h>
#include <sys/stat.h>
#if !defined(_MSC_VER)
#include <unistd.h>
#endif

#include "dtc.h"
#include "img2.h"
extern "C" {
	#include "max.h"
	#include "datation.h"
	//#include "serfmt.h"
}
#include "serfmt.h"
#include "wrapper.h"
#include "wrapper2.h"
#include "auxfunc.h"
#include "datation2.h"
//#include "max2.h"
#include "dtcas3.h"

#include <vector>

extern std::string full_version;

struct FrameOrder {
	bool operator()(ITEM* a, ITEM* b) {
		return a->point->frame < b->point->frame;
	}
};

struct BrightnessOrder {
	bool operator()(ITEM* a, ITEM* b) {
		return a->point->val > b->point->val;
	}
};

void	read_files(std::string folder, std::vector<std::string> *file_list, std::vector<std::string> *acquisition_file_list);

int		itemcmp(const void *a, const void *b);

int		framecmp(const void *a, const void *b);

int		detect_impact(DTCIMPACT *dtc, DTCIMPACT *dtcout, double meanValue, LIST *list, ITEM** dtcMax, int fps, double radius,
	double incrLum, int incrFrame);

//int		detect(std::vector<std::string> file_list, OPTS opts);

int		detect(std::vector<std::string> file_list, OPTS opts, std::string scan_folder_path);

void	StreamDeTeCtOSversions(std::wstringstream *ss);

void	GetOSversion(std::string *pos_version);