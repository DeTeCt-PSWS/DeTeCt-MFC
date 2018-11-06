/********************************************************************************/
/*                                                                              */
/*	DTC	(c) Luis Calderon, Marc Delcroix (delcroix.marc@free.fr) 2012-			*/
/*                                                                              */
/*    CMDLINE: Command line options definition functions						*/
/*                                                                              */
/********************************************************************************/
#include "common.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <opencv/cv.h>

#include "cmdline.h"
#include "dtc.h"

char *get_arg(char ***argv);

void parse_command_line_options(int argc, char **argv, OPTS *opts)
{
	int error;

	char *arg=NULL;
	char *p=NULL;
	
	opts->filename = opts->ofilename = opts->darkfilename = opts->ovfname = opts->sfname = NULL;
	opts->nsaveframe = 0;
	opts->ostype = OTYPE_NO;
	opts->ovtype = OTYPE_NO;
	opts->timeImpact = 4;
	opts->incrLumImpact = 0.9;
	opts->incrFrameImpact = 10;
	opts->radius = 10.0;
	opts->nframesROI  = 0;
	opts->bayer=-1;
	opts->medSize = 5;
	opts->wait    = 0;
	opts->facSize = 0.9;
	opts->secSize = 1.05;
	opts->threshold = 0.0;
	opts->learningRate = 0.0;
	opts->thrWithMask = 0;
	opts->histScale = 1;
	opts->viewROI = 0;
	opts->viewTrk = 0;
	opts->viewDif = 0;	
	opts->viewRef = 0;
	opts->viewThr = 0;
	opts->viewSmo = 0;
	opts->viewRes = 0;
	opts->viewHis = 0;
	opts->verbose = 0;
	opts->filter.type = -1;
	opts->filter.param[0] = 3;
	opts->filter.param[1] = 3;
	opts->filter.param[2] = 0;
	opts->filter.param[3] = 0;
	opts->debug = 0;
    opts->ADUdtconly = 0;
    opts->detail = 0;	
    opts->allframes = 0;	
    opts->minframes = 3;	
    opts->dateonly = 0;	
    opts->ignore = 0;	
    opts->videotest = 0;	
	
    for (error = 0; *++argv && !error;)
    {
		if (!strncmp(*argv, "-ifile", 3))
		{
			if (!(arg = get_arg(&argv)))
			{
				++error;
				break;
			}
			opts->filename = arg;
        } 
		else if (!strncmp(*argv, "-ofile", 3))
		{
			if (!(arg = get_arg(&argv)))
			{
				++error;
				break;
			}
			opts->ofilename = arg;
/*			opts->ADUdtc = 0;*/
			opts->ostype = OTYPE_ADU;
        } 
		else if (!strncmp(*argv, "-dfile", 3))
		{
			if (!(arg = get_arg(&argv)))
			{
				++error;
				break;
			}
			opts->darkfilename = arg;
        } 
        else if (!strncmp(*argv, "-nsaveframe", 11) || !strncmp(*argv, "-nsf", 4))
        {
			if (!(arg = get_arg(&argv)))
			{
				++error;
				break;
			}

            if (!strncmp(arg, "dif", 3))
                opts->ostype = OTYPE_DIF;
            else if (!strncmp(arg, "trk", 3))
                opts->ostype = OTYPE_TRK;
            else if (!strncmp(arg, "roi", 3))
                opts->ostype = OTYPE_ROI;
            else if (!strncmp(arg, "his", 3))
                opts->ostype = OTYPE_HIS;
            else if (!strncmp(arg, "ADUdtcdetail", 12)) {
            	opts->ostype = OTYPE_ADU;
				opts->detail=1; }
            else if (!strncmp(arg, "ADUdtc", 3))
            	opts->ostype = OTYPE_ADU;
           else
            {
                ++error;
            	break;
            }
            opts->nsaveframe = (p = strchr(arg, ':')) ? atoi(++p) : 0;
		}
        else if (!strncmp(*argv, "-ovfile", 3))
        {
			if (!(arg = get_arg(&argv)))
			{
				++error;
				break;
			}

            if (!strncmp(arg, "dif", 3))
                opts->ovtype = OTYPE_DIF;
            else if (!strncmp(arg, "trk", 3))
                opts->ovtype = OTYPE_TRK;
            else if (!strncmp(arg, "roi", 3))
                opts->ovtype = OTYPE_ROI;
            else if (!strncmp(arg, "his", 3))
                opts->ovtype = OTYPE_HIS;
            else
            {
                ++error;
            	break;
            }
            opts->ovfname = (p = strchr(arg, ':')) ? ++p : NULL;
		}
		else if (!strncmp(*argv, "-bayer", 4))
		{
			if (!(arg = get_arg(&argv)))
			{
				++error;
				break;
			}
			if (strcmp(arg,"BG")==0) {
				opts->bayer = CV_BayerBG2BGR;
			} else if (strcmp(arg,"GB")==0) {
				opts->bayer = CV_BayerGB2BGR;
			} else if (strcmp(arg,"RG")==0) {
				opts->bayer = CV_BayerRG2BGR;
			} else if (strcmp(arg,"GR")==0) {
				opts->bayer = CV_BayerGR2BGR;
			} else {
				opts->bayer = -1;
			}
		}
		else if (!strncmp(*argv, "-medSize", 5) || !strncmp(*argv, "-mS", 3))
		{
			if (!(arg = get_arg(&argv)))
			{
				++error;
				break;
			}
            opts->medSize = atoi(arg);
		} else if (!strncmp(*argv, "-nframesROI", 6) || !strncmp(*argv, "-nfrROI", 7)) {
			if (!(arg = get_arg(&argv)))
			{
				++error;
				break;
			}
            opts->nframesROI = atol(arg);
		} else if (!strncmp(*argv, "-wROI", 3)) {
			if (!(arg = get_arg(&argv)))
			{
				++error;
				break;
			}
            opts->wROI = atoi(arg);
		} else if (!strncmp(*argv, "-hROI", 3)) {
			if (!(arg = get_arg(&argv)))
			{
				++error;
				break;
			}
            opts->hROI = atoi(arg);

		} else if (!strncmp(*argv, "-wait", 2)) {
			if (!(arg = get_arg(&argv)))
			{
				++error;
				break;
			}
            opts->wait = atoi(arg);
		} else if (!strncmp(*argv, "-facSize", 5) || !strncmp(*argv, "-fS", 3)) {
			if (!(arg = get_arg(&argv)))
			{
				++error;
				break;
			}
            opts->facSize = atof(arg);
		} else if (!strncmp(*argv, "-secSize", 8) || !strncmp(*argv, "-sS", 3)) {
			if (!(arg = get_arg(&argv)))
			{
				++error;
				break;
			}
            opts->secSize = atof(arg);
		} else if (!strncmp(*argv, "-threshold", 4)) {
			if (!(arg = get_arg(&argv)))
			{
				++error;
				break;
			}
            opts->threshold = atof(arg);
        } else if (!strncmp(*argv, "-learningRate", 13) || !strncmp(*argv, "-lR", 3)) {
			if (!(arg = get_arg(&argv)))
			{
				++error;
				break;
			}
            opts->learningRate = atof(arg);
        } else if (!strncmp(*argv, "-timeImpact", 11) || !strncmp(*argv, "-tI", 3)) {
			if (!(arg = get_arg(&argv))) {
				++error;
				break;
			}
            opts->timeImpact = atoi(arg);
        } else if (!strncmp(*argv, "-incrLumImpact", 14) || !strncmp(*argv, "-iLI", 4)) {
			if (!(arg = get_arg(&argv))) {
				++error;
				break;
			}
            opts->incrLumImpact = atoi(arg);
        } else if (!strncmp(*argv, "-incrFrmImpact", 14) || !strncmp(*argv, "-iFI", 4)) {
			if (!(arg = get_arg(&argv))) {
				++error;
				break;
			}
            opts->incrFrameImpact = atoi(arg);
        } else if (!strncmp(*argv, "-radius", 7) || !strncmp(*argv, "-rad", 4)) {
			if (!(arg = get_arg(&argv))) {
				++error;
				break;
			}
            opts->radius = atof(arg);
        } else if (!strncmp(*argv, "-filter", 7)) {
			if (!(arg = get_arg(&argv)))
			{
				++error;
				break;
			}
            if (!strncmp(arg, "gaussian", 4))
                opts->filter.type = CV_GAUSSIAN;
            else if (!strncmp(arg, "blur", 4))
                opts->filter.type = CV_BLUR;
            else if (!strncmp(arg, "median", 3))
                opts->filter.type = CV_MEDIAN;
            else if (!strncmp(arg, "no", 2))
                opts->filter.type = -1;
            else
            {
                ++error;
             	break;
            }
            arg = strchr(arg, ':');
            if (arg) arg = strtok(++arg, ",");
            for (int i = 0; arg && (i < 4); i++)
            {
            	opts->filter.param[i] = atoi(arg);
            	arg = strtok(NULL, ",");
            }
        } else if (!strncmp(*argv, "-thWithMask", 12) || !strncmp(*argv, "-tWM", 4)) {
                opts->thrWithMask = 1;
        } else if (!strncmp(*argv, "-viewTrk", 8) || !strncmp(*argv, "-vTrk", 5)) {
                opts->viewTrk = 1;
        } else if (!strncmp(*argv, "-viewROI", 8) || !strncmp(*argv, "-vROI", 5)) {
                opts->viewROI = 1;
        } else if (!strncmp(*argv, "-viewDif", 8) || !strncmp(*argv, "-vDif", 5)) {
                opts->viewDif = 1;
        } else if (!strncmp(*argv, "-viewRef", 8) || !strncmp(*argv, "-vRef", 5)) {
                opts->viewRef = 1;
        } else if (!strncmp(*argv, "-viewMsk", 8) || !strncmp(*argv, "-vMsk", 5)) {
                opts->viewMsk = 1;
        } else if (!strncmp(*argv, "-viewThr", 8) || !strncmp(*argv, "-vThr", 5)) {
                opts->viewThr = 1;
        } else if (!strncmp(*argv, "-viewRes", 8) || !strncmp(*argv, "-vRes", 5)) {
                opts->viewRes = 1;
        } else if (!strncmp(*argv, "-viewHis", 8) || !strncmp(*argv, "-vHis", 5)) {
            opts->viewHis = 1;
			arg = strchr(*argv, ':');
            if (arg) opts->histScale = atoi(++arg);
        } else if (!strncmp(*argv, "-viewSmo", 8) || !strncmp(*argv, "-vSmo", 5)) {
                opts->viewSmo = 1;
		} else if (!strncmp(*argv, "-verbose", 4)) {
                opts->verbose = 1;
		} else if (!strncmp(*argv, "-debug", 4)) {
                opts->debug = 1;
		} else if (!strncmp(*argv, "-videotest", 4)) {
                opts->videotest = 1;
		} else if (!strncmp(*argv, "-ADUdtcdetail", 12)) {
                opts->detail = 1;
/*            	opts->ADUdtc = 0;*/
            	opts->ostype = OTYPE_ADU;
		} else if (!strncmp(*argv, "-ADUdtcframes", 12)) {
                opts->allframes = 1;
/*            	opts->ADUdtc = 0;*/
            	opts->ostype = OTYPE_ADU;
		} else if (!strncmp(*argv, "-ADUdtconly", 10)) {
                opts->ADUdtconly = 1;
            	opts->ostype = OTYPE_ADU;
        } else if (!strncmp(*argv, "-minframes", 3)) {
			if (!(arg = get_arg(&argv))) {
				++error;
				break;
			}
            opts->minframes = atoi(arg);
        } else if (!strncmp(*argv, "-dateonly", 5)) {
            opts->dateonly = 1;
        } else if (!strncmp(*argv, "-ignore", 4)){
            opts->ignore = 1;
		} else {
            if (**argv != '-') {
                opts->filename = *argv;
                if (argv[1])   /* shouldn't be any more args after filename */
                    ++error;
            } else
                ++error;   /* not expecting any other options */
        }
    }
    
    if (opts->ostype == OTYPE_ADU && !opts->ofilename)
    {
        fprintf(stderr, "ERROR in parse_command_line_options: use -ofile to provide an output image file\n\n");
        exit(EXIT_FAILURE);
    }

    if (!opts->filename)// && !opts->dirname)
        ++error;

    if (error) {
        fprintf(stderr, "\n%s (%s) %s%s %s (c) %s\n", PROGNAME, LONGNAME, VERSION_NB, VERSION_MSVC, VERSION_DATE, COPYRIGHT);
        fprintf(stderr,
          "Usage:  %s [-ifile         | -if     <video file name>]\n"
          "            [-ofile         | -of     <output file name>]\n"
          "            [-dfile         | -df     <dark file name>]\n"
		  "            [-ADUdtconly]\n"
          "            [-ADUdtcdetail]\n"
          "            [-ADUdtcframes]\n"
		  "            [-minframes]    | -min    <integer>]\n"
		  "            [-ignore]       | -ign    <integer>]\n"		  
		  "            [-bayer ]       | -bay   = <BG|GB|RG|GR>]\n"		  
          "            [-medSize       | -mS     <integer>]\n"
          "            [-nframesROI    | -nfrROI <integer>]\n"
          "            [-wROI          | -wR     <integer>]\n"
          "            [-hROI          | -hR     <integer>]\n"
          "            [-wait          | -w      <integer>]\n"
          "            [-facSize       | -fS     <real>]\n"
          "            [-secSize       | -sS     <real>]\n"
          "            [-threshold     | -thr    <real>]\n"
          "            [-learningRate  | -lR     <real>]\n"
          "            [-timeImpact    | -tI     <integer>]\n"
          "            [-incrLumImpact | -iLI    <integer>]\n"
          "            [-incrFrmImpact | -iFI    <integer>]\n"
          "            [-radius        | -rad    <real>]\n"
          "            [-thWithMask    | -tWM]\n"
          "            [-filter[=]<filter_name>[,width][,height]\n"
          "            [-viewROI       | -vROI]\n"
          "            [-viewDif       | -vDif]\n"
          "            [-viewRef       | -vRef]\n"
          "            [-viewThr       | -vThr]\n"
          "            [-viewMsk       | -vMsk]\n"
          "            [-viewSmo       | -vSmo]\n"
          "            [-viewRes       | -vRes]\n"
          "            [-viewHis       | -vHis]\n"          
          "            [-viewTrk       | -vTrk]\n"
/*          "            [-nsaveframe    | -nsf <operation>]\n"
		  "			   [-ADUdtc        ]\n",*/
		  "            [-dateonly]\n"
          "            [-verbose       | -ver]\n"
          "            [-debug         | -deb]\n"
          "            [-videotest     | -vid]\n",
          PROGNAME);
        fprintf(stderr,
          "Explanations:\n"
/*          " ADUdtconly          : use only ADUdtc algorithm (no individual frame analysis)\n"
          "                   disables most of following options\n"*/
          " ADUdtconly      : only generate detection images\n"
		  "                     (no attempt to detect directly impacts on individual frames,\n"
          "                      suggested for DeTeCt project, disable most of following options)\n"
          " ADUdtcdetail    : save mean and non normalized detection images\n"
          " ADUdtcframes    : save of all individual detection images\n"
		  " minframes       : minimum number of frames to start processing\n"
		  " ignore          : ignore SER or FITs incorrect frames\n"
		  " bayer           : debayer image from code BG, GB, RG, GR\n"
          " medSize         : buffer size for median filter (ROI calculation)\n"
          " nfrROI          : number of frames for ROI calculation\n"
          " wROI            : fixed ROI width\n"
          " hROI            : fixed ROI height\n"
          " wait            : milliseconds between frame visualization\n"
          " facSize         : distance factor for ROI calculation\n"
          " secSize         : security factor for ROI calculation\n"
          " threshold       : luminance minimum level\n"
          " learningRate    : alpha parameter for Alpha Blending\n"
          " timeImpact      : estimated time (s) of an impact\n"
          " incrLumImpact   : estimated increment of luminance\n"
          "                     (mean value factor) for an impact\n"
          " incrFrmImpact   : estimated number of frames for an impact\n"
          " radius          : estimated radius of an impact\n"
          " thrWithMask     : use mask for moving background calculation\n"
          " viewROI         : view ROI region\n"
          " viewDif         : view differential frame (actual - reference)\n"          
          " viewRef         : view reference frame (background)\n"
          " viewThr         : view frame after threshold\n"
          " viewSmo         : view frame after filter\n"
          " viewMsk         : view mask for moving background calculation\n"
          " viewRes         : view result (final) frame\n"
          " viewHis         : view histogram (result frame)\n"
          " viewTrk         : view tracking\n"
/*          " nsaveframe      : perform <operation>\n"
          "                   <operation> can be:\n"
          "                    ADUdtc   obtain \"detection image\"\n"
          "                    ADUdtcdetail   obtain \"detection image\",\n"
		  "                         mean and non normalized detection images\n"*/
          " filter          : apply <filter_name> after threshold\n"
          "                     <filter_name> can be:\n"
          "                      median   (width x (height=width) square aperture\n"
          "                      gaussian (width x height gaussian kernel)\n"
          "                      blur     (width x height kernel (all 1's))\n"
          "                     Default width and height is 3 (it must be odd numbers)\n"
          " dateonly        : only output date informations from video file or acquisition log file\n"
		  "                     disable most of the other options)\n"
          " verbose         : print lum, min and max (and position) calculations for\n"
          "                     result (final) frame\n"
          " debug           : print debug messages\n"
          " videotest       : only test video file by reading and saving first file\n"
        );
        exit(EXIT_SUCCESS);
    }
}

char *get_arg(char ***argv)
{
	char *arg = NULL;
	
	if ((arg = strchr(**argv, '=')))
	{
		arg++;
	}
	else if (!*(++(*argv)))
	{
		arg = NULL;
	}
	else
	{
		arg = **argv; 
	}
	
	return arg;
}

void execute_detection_algorithm(char* folder, OPTS *opts) {
	return;
}