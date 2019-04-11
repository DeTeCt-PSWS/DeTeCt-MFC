#pragma once
#ifndef ___DTCAS3_H_
#define ___DTCAS3_H_

#include "dtc.h"
#include "img2.h"
extern "C" {
#include "max.h"
#include "datation.h"
#include "serfmt.h"
}
#include "wrapper.h"
#include "wrapper2.h"
#include "auxfunc.h"
#include <string>
#include <vector>

extern std::string autostakkert_extension;

bool starts_with(const std::string& s1, const std::string& s2);

bool replace(std::string& str, const std::string& from, const std::string& to);

std::vector<std::string> read_txt(std::string path);

void read_autostakkert_file(std::string configfile, std::string *filename, std::vector<cv::Point> *cm_list);

void read_autostakkert_config_line(std::string line, std::string *filename, std::vector<cv::Point> *cm_list);

/*void detect_autostakkert(std::string path);*/

#endif