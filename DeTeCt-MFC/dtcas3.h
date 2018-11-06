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

bool starts_with(const std::string& s1, const std::string& s2);

std::vector<std::string> read_txt(std::string path);

void read_config_file(std::string path, std::string *filename, std::vector<cv::Point> *cm_list);

void detect_autostakkert(std::string path);

#endif