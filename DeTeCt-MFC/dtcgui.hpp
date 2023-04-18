#pragma once
#if !defined(_MSC_VER)
#include <unistd.h>
#endif

#include "dtc.h"
#include "img2.hpp"
//extern "C" {
	#include "max.h"
	#include "datation.h"
//}
#include "serfmt.h"
#include "wrapper.h"
#include "wrapper2.hpp"
#include "auxfunc.hpp"
#include "datation2.hpp"
#include "dtcas3.hpp"

extern std::string app_title;

extern std::string message_lines[MAX_STRING];

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

struct AcquisitionFilesList {
	std::vector<std::string> file_list				= {};
	std::vector<std::string> acquisition_file_list	= {};
	std::vector<int>		 nb_prealigned_frames	= {};
	std::vector<int64>		 acquisition_size		= {};
};

enum class _Instance_type { autostakkert_parent, parent, autostakkert_single, single, autostakkert_child, child };
typedef enum _Instance_type Instance_type;

void			read_files(std::string folder, AcquisitionFilesList *acquisition_files);

//int				itemcmp(const void *a, const void *b);
int				framecmp(const void *a, const void *b);

int				detect_impact(DTCIMPACT *dtc, DTCIMPACT *dtcout, double meanValue, LIST *list, ITEM** dtcMax, double radius, double incrLum, int impact_frames_min);


//int				detect(std::vector<std::string> current_file_list, OPTS *opts, std::string scan_folder_path);
int				detect(std::vector<std::string> current_file_list, std::string scan_folder_path);

char			*dtc_full_filename			(const char *acquisition_filename, const char *suffix,						const char *path_name, char *full_filename);
char			*dtc_full_filename_2suffix	(const char *acquisition_filename, const char *suffix, const char *suffix2,	const char *path_name, char *full_filename);

void			zip(char *zipfilename, char *item_to_be_zipped, std::wstring output_filename, int* log_counter);

Instance_type	DisplayInstanceType(int *nbinstances);

void			WriteIni();
void			AcquisitionFileListToQueue(AcquisitionFilesList* pacquisition_files, const CString tag_current, const size_t index_current, const CString out_directory, int* acquisitions_to_be_processed);

//BOOL			RemoveFromIni(const CString line_to_remove);