#ifndef __CMDLINE_H__
#define __CMDLINE_H__

//#include "common.h"

#define OTYPE_NO		0
#define OTYPE_DIF		1
#define OTYPE_TRK		2
#define OTYPE_ROI		3
#define OTYPE_HIS		4
#define OTYPE_MSK		5
#define OTYPE_ADU		6

// Additions by Jon
#define FILTER_BLUR		1
#define FILTER_MEDIAN	2
#define FILTER_GAUSSIAN 3

	struct Filter
	{
		int type;   // CV_BLUR, CV_MEDIAN o CV_GAUSSIAN
		int param[4]; // width, height, sigma, 0
	};

	//void parse_command_line_options(int argc, char **argv, OPTS *opts);

	//char *get_arg(char ***argv);

#endif /* __CMDLINE_H__ */
