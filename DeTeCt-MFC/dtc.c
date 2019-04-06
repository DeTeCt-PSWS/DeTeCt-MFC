/********************************************************************************/
/*                                                                              */
/*        DTC        (c) Luis Calderon, Marc Delcroix 2012-                                                                        */
/*                                                                              */
/********************************************************************************/
#include "common.h"


#include <time.h>
#include <stdio.h>
#include <sys/stat.h>
#if !defined(_MSC_VER)
#include <unistd.h>
#endif

#include "dtc.h"
#include "cmdline.h"
#include "img.h"
#include "max.h"
#include "wrapper.h"
#include "datation.h"
#include "serfmt.h"
OPTS opts;

int main(int argc, char** argv)
{
	//#define TEST
#ifdef _TEST //_DEBUG
	//        char* argv_debug[] = { "DeTeCt.exe", "-ADUdtcdetail", "-ignore", "-dateonly", "-debug", "-ifile", "D:\\Work\\Impact\\dtc\\movies\\Jup_G_03062010_055120_0000.fits", "-ofile", "test_debug.jpg", "-dfile", "D:\\Work\\Impact\\dtc\\movies\\dark.tif", NULL};
	//        char* argv_debug[] = { "DeTeCt.exe", "-ADUdtcdetail", "-ignore", "-dateonly", "-debug", "-ifile", "D:\\Work\\Impact\\dtc\\movies\\Genika\\log__2013-11-21_T_00-05-10-0783_L.ser", "-ofile", "test_debug.jpg", "-dfile", "D:\\Work\\Impact\\dtc\\movies\\dark.tif", NULL};
	//        char* argv_debug[] = { "DeTeCt.exe", "-ADUdtcdetail", "-ignore", "-dateonly", "-debug", "-ifile", "D:\\Work\\Impact\\dtc\\movies\\No_impact_Emil_Kraaikamp.avi", "-ofile", "test_debug.jpg", "-dfile", "D:\\Work\\Impact\\dtc\\movies\\dark.tif", NULL};
	//        char* argv_debug[] = { "DeTeCt.exe", "-ADUdtcdetail", "-ignore", "-dateonly", "-debug", "-ifile", "D:\\Work\\Impact\\dtc\\movies\\FireCapture\\Jup_164624_0000.fit", "-ofile", "test_debug.jpg", "-dfile", "D:\\Work\\Impact\\dtc\\movies\\dark.tif", NULL};
	//        char* argv_debug[] = { "DeTeCt.exe", "-ADUdtcdetail", "-ignore", "-dateonly", "-debug", "-ifile", "D:\\Work\\Impact\\dtc\\movies\\jupiter_2011_08_30_052421_R_Tif_0\\F0.Tif", "-ofile", "test_debug.jpg", "-dfile", "D:\\Work\\Impact\\dtc\\movies\\dark.tif", NULL};
	//        char* argv_debug[] = { "DeTeCt.exe", "-ADUdtcdetail", "-ignore", "-dateonly", "-debug", "-ifile", "D:\\Work\\Impact\\dtc\\movies\\FITS\\Jup_G_03062010_055120_0000.fits", "-ofile", "test_debug.jpg", "-dfile", "D:\\Work\\Impact\\dtc\\movies\\dark.tif", NULL};
	//        char* argv_debug[] = { "DeTeCt.exe", "-ADUdtcdetail", "-ignore", "-dateonly", "-debug", "-ifile", "D:\\Work\\Impact\\dtc\\movies\\\jupiter_2011_08_30_052421_R\\jupiter_2011_08_30_052421_R_image_0.tif", "-ofile", "test_debug.jpg", "-dfile", "D:\\Work\\Impact\\dtc\\movies\\dark.tif", NULL};
	//        char* argv_debug[] = { "DeTeCt.exe", "-ADUdtcdetail", "-ignore", "-dateonly", "-debug", "-ifile", "D:\\Work\\Impact\\dtc\\movies\\No_impact_Emil_Kraaimkap.avi", "-ofile", "test_debug.jpg", "-dfile", "D:\\Work\\Impact\\dtc\\movies\\dark.tif", NULL};
	//        char* argv_debug[] = { "DeTeCt.exe", "-ADUdtcdetail", "-ignore", "-dateonly", "-debug", "-ifile", "D:\\Work\\Impact\\dtc\\movies\\FITS\\2\\000000_20120808_064633_296.fits", "-ofile", "test_debug.jpg", "-dfile", "D:\\Work\\Impact\\dtc\\movies\\dark.tif", NULL};
	//        char* argv_debug[] = { "DeTeCt.exe", "-ADUdtcdetail", "-ignore", "-dateonly", "-debug", "-ifile", "D:\\Work\\Impact\\dtc\\movies\\FITS\\jupiter_20090904\\B_Fit\\nb1.fit", "-ofile", "test_debug.jpg", "-dfile", "D:\\Work\\Impact\\dtc\\movies\\dark.tif", NULL};
	//        char* argv_debug[] = { "DeTeCt.exe", "-ADUdtcdetail", "-ignore", "-dateonly", "-debug", "-ifile", "D:\\Work\\Impact\\dtc\\movies\\jupiter_2011_08_30_052421_R_Bmp\\F0.Bmp", "-ofile", "test_debug.jpg", "-dfile", "D:\\Work\\Impact\\dtc\\movies\\dark.tif", NULL};
	//        char* argv_debug[] = { "DeTeCt.exe", "-ADUdtcdetail", "-ignore", "-ifile", "D:\\Work\\Impact\\dtc\\bugs\\Impact_detection\\details\\Piotr_Jup_26112013_1_225723_redlum_0000_1.jpg", "-ofile", "test_debug.jpg", "-dfile", "D:\\Work\\Impact\\dtc\\movies\\dark.tif", NULL};
	//        char* argv_debug[] = { "DeTeCt.exe", "-ADUdtcdetail", "-ignore", "-dateonly", "-debug", "-ifile", "D:\\Work\\Impact\\dtc\\bugs\\PRo\\ju420131019-052451037.ser","-ofile", "test_debug.jpg", "-dfile", "D:\\Work\\Impact\\dtc\\movies\\dark.tif", NULL};
	//        char* argv_debug[] = { "DeTeCt.exe", "-ADUdtcdetail", "-ignore", "-dateonly", "-debug", "-ifile", "D:\\Work\\Impact\\dtc\\bugs\\PRo\\jup620121025-004033463.ser","-ofile", "test_debug.jpg", "-dfile", "D:\\Work\\Impact\\dtc\\movies\\dark.tif", NULL};
	//        char* argv_debug[] = { "DeTeCt.exe", "-ADUdtcdetail", "-ignore", "-debug", "-ifile", "D:\\Work\\Impact\\dtc\\bugs\\FLocklear\\Jup_011213_055747.avi","-ofile", "test_debug.jpg", "-dfile", "D:\\Work\\Impact\\dtc\\movies\\dark.tif", NULL};
	//        char* argv_debug[] = { "DeTeCt.exe", "-ADUdtcdetail", "-ignore", "-debug", "-ifile", "D:\\Work\\Impact\\dtc\\v2.0\\DeTeCt\\Debug\\FireCapture\\FC2.3beta16\\Jup_021005.avi","-ofile", "test_debug.jpg", "-dfile", "D:\\Work\\Impact\\dtc\\movies\\dark.tif", NULL};
	//        char* argv_debug[] = { "DeTeCt.exe", "-ADUdtcdetail", "-ignore", "-debug", "-ifile", "D:\\Work\\Impact\\dtc\\v2.0\\DeTeCt\\Debug\\FireCapture\\FC2.3beta16\\Jup_133215.avi","-ofile", "test_debug.jpg", "-dfile", "D:\\Work\\Impact\\dtc\\movies\\dark.tif", NULL};
	//        char* argv_debug[] = { "DeTeCt.exe", "-ADUdtcdetail", "-ignore", "-debug", "-ifile", "D:\\Work\\Impact\\dtc\\movies\\Genika\\log__2013-11-21_T_00-05-10-0783_L.ser","-ofile", "test_debug.jpg", "-dfile", "D:\\Work\\Impact\\dtc\\movies\\dark.tif", NULL};
	//        char* argv_debug[] = { "DeTeCt.exe", "-ADUdtcdetail", "-ignore", "-debug", "-ifile", "D:\\Work\\Impact\\dtc\\movies\\PLAMx_20121125-193349020TU_L.ser","-ofile", "test_debug.jpg", "-dfile", "D:\\Work\\Impact\\dtc\\movies\\dark.tif", NULL};
	//        char* argv_debug[] = { "DeTeCt.exe", "-ADUdtcdetail", "-ignore", "-debug", "-ifile", "D:\\Work\\Impact\\dtc\\movies\\Genika\\ura_b__2014-10-02_T_00-52-25-0687_IR685.ser","-ofile", "test_debug.jpg", "-dfile", "D:\\Work\\Impact\\dtc\\movies\\dark.tif", NULL};
	//        char* argv_debug[] = { "DeTeCt.exe", "-ADUdtcdetail", "-ignore", "-debug", "-ifile", "D:\\Work\\Impact\\dtc\\movies\\PLX\\ju_20141029-040802282_XDu.avi","-ofile", "test_debug.jpg", "-dfile", "D:\\Work\\Impact\\dtc\\movies\\dark.tif", NULL};
	//        char* argv_debug[] = { "DeTeCt.exe", "-ADUdtcdetail", "-ignore", "-debug", "-ifile", "C:\\Work\\jupiter_20140914\\ir685\\2014-09-14-0432_8-MD-IR685-1.ser","-ofile", "test_debug.jpg", "-dfile", "C:\\Work\\jupiter_20140914\\ir685\\dark.tif", NULL};       
	//        char* argv_debug[] = { "DeTeCt.exe", "-ADUdtcdetail", "-ignore", "-debug", "-ifile", "D:\\Work\\Impact\\dtc\\movies\\SharpCap\\Capture 2014-11-10T08_39_28.ser","-ofile", "test_debug.jpg", NULL};
	//        char* argv_debug[] = { "DeTeCt.exe", "-ADUdtcdetail", "-ignore", "-debug", "-ifile", "D:\\Work\\Impact\\dtc\\movies\\SharpCap\\Capture 2014-11-09T15_18_30\\0001.fits","-ofile", "test_debug.jpg", NULL};
	//        char* argv_debug[] = { "DeTeCt.exe", "-ADUdtcdetail", "-ignore", "-debug", "-ifile", "D:\\Work\\Impact\\dtc\\movies\\PLX\\bug\\ju_20141029-041551190_XDu.ser","-ofile", "test_debug.jpg", NULL};
	//        char* argv_debug[] = { "DeTeCt.exe", "-ADUdtcdetail", "-ignore", "-debug", "-ifile", "D:\\Work\\Impact\\dtc\\movies\\PLX\\JLRGB20121031-010133061.avi","-ofile", "test_debug.jpg", NULL};
	//        char* argv_debug[] = { "DeTeCt.exe", "-ADUdtcdetail", "-ignore", "-debug", "-ifile", "D:\\Work\\Impact\\dtc\\movies\\PLX\\bug_XDu2\\jup_20141105-035824262_XDu.ser","-ofile", "test_debug.jpg", NULL};
	//        char* argv_debug[] = { "DeTeCt.exe", "-ADUdtcdetail", "-ignore", "-debug", "-ifile", "D:\\Work\\Impact\\dtc\\movies\\SharpCap\\Capture 2014-11-09T14_23_22\\0001.fits","-ofile", "test_debug.jpg", NULL};
	//        char* argv_debug[] = { "DeTeCt.exe", "-ADUdtcdetail", "-ignore", "-debug", "-ifile", "D:\\Work\\Impact\\dtc\\movies\\FITS\\2\\000000_20120808_064633_296.fits","-ofile", "test_debug.jpg", NULL};
	//        char* argv_debug[] = { "DeTeCt.exe", "-ADUdtcdetail", "-ignore", "-debug", "-ifile", "D:\\Work\\Impact\\dtc\\movies\\Firecapture\\301114_131123.avi","-ofile", "test_debug.jpg", NULL};
	//        char* argv_debug[] = { "DeTeCt.exe", "-ADUdtcdetail", "-ignore", "-debug", "-ifile", "D:\\Work\\Impact\\dtc\\movies\\bugs\\ACo\\J_2015Jan17_044203_RGB.avi", "-ofile", "test_debug.jpg", NULL };
	//        char* argv_debug[] = { "DeTeCt.exe", "-ADUdtcdetail", "-ignore", "-debug", "-ifile", "D:\\Work\\Impact\\dtc\\movies\\bugs\\bug_XDu2\\jup_20141105-035824262_XDu.ser", "-ofile", "test_debug.jpg", NULL };
	//        char* argv_debug[] = { "DeTeCt.exe", "-ADUdtcdetail", "-ignore", "-debug", "-ifile", "D:\\Work\\Impact\\dtc\\movies\\bugs\\MDe_dark\\2015-02-07-2255_1-MD-DARK-6.ser", "-ofile", "test_debug.jpg", "-dfile", "D:\\Work\\Impact\\dtc\\movies\\bugs\\MDe_dark\\dark.tif", NULL };
	//        char* argv_debug[] = { "DeTeCt.exe", "-ADUdtcdetail", "-ignore", "-debug", "-ifile", "D:\\Work\\Impact\\dtc\\movies\\bugs\\MDe_v2.0.2\\2015-02-10-2332_5-MD-B-1.ser", "-ofile", "test_debug.jpg", "-dfile", "D:\\Work\\Impact\\dtc\\movies\\bugs\\MDe_v2.0.2\\dark.tif", NULL };
	//        char* argv_debug[] = { "DeTeCt.exe", "-ADUdtcdetail", "-ignore", "-debug", "-ifile", "D:\\Work\\Impact\\dtc\\movies\\bugs\\MDe_v2.0.2\\2015-02-10-2332_5-MD-B-1.ser", "-ofile", "test_debug.jpg", NULL };
	//        char* argv_debug[] = { "DeTeCt.exe", "-ADUdtcdetail", "-ignore", "-debug", "-ifile", "D:\\Work\\Impact\\dtc\\movies\\bugs\\MSm\\Jup_B_02_08_2011_043306.ser", "-ofile", "test_debug.jpg", NULL };
	//        char* argv_debug[] = { "DeTeCt.exe", "-ADUdtcdetail", "-ignore", "-debug", "-ifile", "D:\\Work\\Impact\\dtc\\movies\\FITS\\Phung_fits\\jupiter_0001.fit", "-ofile", "test_debug.jpg", NULL };
	//        char* argv_debug[] = { "DeTeCt.exe", "-ADUdtcdetail", "-ignore", "-debug", "-ifile", "D:\\Work\\Impact\\dtc\\movies\\FITS\\jupiter_20090904\\B_Fit\\nb1.fit", "-ofile", "test_debug.jpg", NULL };
	//        char* argv_debug[] = { "DeTeCt.exe", "-ADUdtcdetail", "-ignore", "-debug", "-ifile", "D:\\Work\\Impact\\dtc\\movies\\bugs\\MDe_dark\\2015-02-07-2255_1-MD-DARK-6.ser", "-ofile", "test_debug.jpg", "-dfile", "D:\\Work\\Impact\\dtc\movies\\bugs\\MDe_dark\\dark.tif", NULL };
	//        char* argv_debug[] = { "DeTeCt.exe", "-ADUdtcdetail", "-ignore", "-debug", "-ifile", "C:\\work\\test_dark\\ch4\\2016-05-17-2106_4-MD-CH4.ser", "-ofile", "test_debug.jpg", "-dfile", "C:\\work\\test_dark\\ch4\\dark.tif", NULL };
	//        char* argv_debug[] = { "DeTeCt.exe", "-ADUdtcdetail", "-ignore", "-debug", "-ifile", "D:\\Work\\Impact\\dtc\\bugs_tests\\Oleg_dark\\Sat_015101_pipp.ser", "-ofile", "test_debug.jpg", "-dfile", "D:\\Work\\Impact\\dtc\\bugs_tests\\Oleg_dark\\dark.tif", NULL };
	char* argv_debug[] = { "DeTeCt.exe", "-ADUdtcdetail", "-ignore", "-debug", "-ifile", "D:\\Work\\Impact\\dtc\\bugs_tests\\Marc_broken_FC_log\\2016-06-27-2107_1-MD-R.ser", "-ofile", "test_debug.jpg", NULL };
	//  char* argv_debug[] = { "DeTeCt.exe", "-ADUdtcdetail", "-ignore", "-debug", "-ifile", "D:\\Work\\Impact\\dtc\\bugs_tests\\Oleg_FC_local_date\\Sat_003759.ser", "-ofile", "test_debug.jpg", NULL };

	fprintf(stderr, "main: DEBUG mode\n");
	parse_command_line_options(argc, argv_debug, &opts);
#else
	parse_command_line_options(argc, argv, &opts);
#endif
	if (opts.videotest) {
		IplImage *pFrame = NULL;
		char ofilenamenormframe[MAX_STRING];
		char ofilenamenorm[MAX_STRING];
		char ext[EXT_MAX];
		CvPoint minPoint = { 0,0 };
		CvPoint maxPoint;
		double minLum;
		double maxLum;

		init_string(ofilenamenormframe);
		init_string(ofilenamenorm);

		get_fileextension(opts.filename, ext, EXT_MAX);
		lcase(ext, ext);
		strncpy(ofilenamenorm, opts.ofilename, strlen(opts.ofilename) - 4);
		ofilenamenorm[strlen(opts.ofilename) - 4] = '\0';
		sprintf(ofilenamenormframe, "%s%s", ofilenamenorm, VIDEOTEST_SUFFIX);
		if ((!strcmp(ext, "ser")) || ((!strcmp(ext, "fit")) || (!strcmp(ext, "fits"))) || ((!strcmp(ext, "bmp")) || (!strcmp(ext, "dib")) || (!strcmp(ext, "jpeg")) || (!strcmp(ext, "jpg")) || (!strcmp(ext, "jpe")) || (!strcmp(ext, "jp2")) || (!strcmp(ext, "png")) || (!strcmp(ext, "pbm")) || (!strcmp(ext, "pgm")) || (!strcmp(ext, "ppm")) || (!strcmp(ext, "sr")) || (!strcmp(ext, "ras")) || (!strcmp(ext, "tiff")) || (!strcmp(ext, "tif")))) {
			int framecount;
			int frame_error = 0;
			DtcCapture *pCapture;

			if (!(pCapture = dtcCaptureFromFile(opts.filename, &framecount))) {
				fprintf(stderr, "ERROR in main: Cannot open file %s\n", opts.filename);
				return EXIT_FAILURE;
			}
			pFrame = dtcQueryFrame(pCapture, opts.ignore, &frame_error);
			cvMinMaxLoc(pFrame, &minLum, &maxLum, &minPoint, &maxPoint, NULL);
			if (maxLum>255) {
				cvConvertScale(pFrame, pFrame, 1.0 / 255.0, 0);
			}
			fprintf(stderr, "main: Saving %s\n", ofilenamenormframe);
			cvSaveImage(ofilenamenormframe, pFrame, 0);
			fprintf(stderr, "main: Releasing capture\n");
			dtcReleaseCapture(pCapture);
			pCapture = NULL;
		}
		else {
			CvCapture *capture;

			fprintf(stderr, "main: Opening %s\n", opts.filename);
			if (!(capture = cvCaptureFromFile(opts.filename))) {
				fprintf(stderr, "main: Cannot open file %s\n", opts.filename);
				return EXIT_FAILURE;
			}
			fprintf(stderr, "main: Frame count %d\n", (int)(cvGetCaptureProperty(capture, CV_CAP_PROP_FRAME_COUNT)));
			fprintf(stderr, "main: FPS %d\n", (int)(cvGetCaptureProperty(capture, CV_CAP_PROP_FPS)));
			fprintf(stderr, "main: Reading frame\n");
			pFrame = cvQueryFrame(capture);
			fprintf(stderr, "main: Saving %s\n", ofilenamenormframe);
			cvSaveImage(ofilenamenormframe, pFrame, 0);
			fprintf(stderr, "main: Releasing capture\n");
			cvReleaseCapture(&capture);
			capture = NULL;
		}
		return EXIT_SUCCESS;
	}
	else {
		int fps_int = 0;
		double fps_real = 0;
		DtcCapture *pCapture;
		LIST ptlist = { 0,0,NULL,NULL };
		DTCIMPACT dtc;

		IplImage *pFrame = NULL;
		IplImage *pGryImg = NULL;
		IplImage *pRefImg = NULL;
		IplImage *pDifImg = NULL;
		IplImage *pMskImg = NULL;
		IplImage *pThrImg = NULL;
		IplImage *pSmoImg = NULL;
		IplImage *pHisImg = NULL;
		IplImage *pTrkImg = NULL;
		IplImage *pOVdImg = NULL;
		IplImage *pADUdtcImg = NULL;
		IplImage *pADUavgImg = NULL;

		CvMat *pGryMat = NULL;
		CvMat *pRefMat = NULL;
		CvMat *pDifMat = NULL;
		CvMat *pMskMat = NULL;
		CvMat *pADUavgMat = NULL;
		CvMat *pADUmaxMat = NULL;
		CvMat *pADUdtcMat = NULL;
		CvMat *pADUavgMatFrame = NULL;
		CvMat *pADUdarkMat = NULL;

		CvVideoWriter *pWriter = NULL;

		CvRect croi = { 0, 0, 0, 0 };

		CvPoint minPoint = { 0,0 };
		CvPoint maxPoint = { 0,0 };

		CvScalar lum;

		double minLum = 0;
		double maxLum = 0;
		double maxLum2;
		double minLummax;
		double maxLummax;

		int nframe = 0;
		int framecount = 0;

		int pGryImg_height = 0;
		int pGryImg_width = 0;
		char ofilenamenormframe[MAX_STRING];
		char ofilenamemean[MAX_STRING];
		char ofilenamenorm[MAX_STRING];

		time_t start_process_time = 0;
		time_t end_process_time = 0;
		char comment[MAX_STRING];
		double duration;

		double start_time;
		double end_time;
		TIME_TYPE timetype;

		int nb_impact = -1;
		int frame_error = 0;
		int frame_errors = 0;
		int darkfile_ok = 0;

		lum.val[0] = 0.0;
		init_string(ofilenamenormframe);
		init_string(ofilenamemean);
		init_string(ofilenamenorm);
		init_string(comment);
		/*********************************INITIALIZATION******************************************/
		if (opts.debug) {
			fprintf(stderr, "main: Init\n");
			time(&start_process_time);
		}
		if (!(pCapture = dtcCaptureFromFile(opts.filename, &framecount))) {
			fprintf(stderr, "main: Cannot open file %s\n", opts.filename);
			return EXIT_FAILURE;
		}
		switch (pCapture->type) {
		case CAPTURE_SER:
			/*                                                                                                                        if (opts.debug) { fprintf(stderr, "main: Image pointer %d\n", pCapture->u.sercapture->image); }*/
			nframe = pCapture->u.sercapture->header.FrameCount;
			break;
		case CAPTURE_FITS:
		case CAPTURE_FILES:
			nframe = pCapture->u.filecapture->FrameCount;
			break;
		default: // CAPTURE_CV
			if (opts.debug) { fprintf(stderr, "main: nframes = %d\n", nframe); }
			nframe = (int)(dtcGetCaptureProperty(pCapture, CV_CAP_PROP_FRAME_COUNT));
			/*                        while ((pFrame = dtcQueryFrame(pCapture, opts.ignore, &frame_error)))        {
			if (opts.debug) { fprintf(stderr, "main: nframes = %d\n",nframe); }
			nframe++;
			}*/
		}
		if ((nframe>0) && (nframe<opts.minframes)) {
			fprintf(stdout, "WARNING in main: only %d frames (minimum is %d), stopping processing\n", nframe, opts.minframes);
			dtcReleaseCapture(pCapture);
			pCapture = NULL;
			if (opts.debug) { fprintf(stderr, "WARNING in main: only %d frames (minimum is %d), stopping processing\n", nframe, opts.minframes); }
			return EXIT_SUCCESS;
		}
		if (opts.debug) { fprintf(stderr, "main: nframes = %d\n", nframe); }
		/*                                                                                                                        if (opts.debug) { fprintf(stderr, "main: Image pointer %d\n", pCapture->u.sercapture->image); }*/
		dtcGetDatation(pCapture, opts.filename, nframe, &start_time, &end_time, &duration, &fps_real, &timetype, comment);
		if (!opts.ADUdtconly) {
			if (opts.debug) { fprintf(stderr, "main: fps real= %f\n", fps_real); }
			if (fps_real<0.02) {
				fps_int = (int)dtcGetCaptureProperty(pCapture, CV_CAP_PROP_FPS);
			}
			else {
				fps_int = (int)(floor(0.5 + fps_real));
			}
			if (opts.debug) { fprintf(stderr, "main: fps int = %d\n", fps_int); }
		}
		if (opts.dateonly) {
			if (nframe>0) {
				dtcWriteLog(argv[0], start_time, end_time, duration, fps_real, timetype, opts.filename, comment, -1, 1);
			}
			dtcReleaseCapture(pCapture);
			pCapture = NULL;
			return EXIT_SUCCESS;
		}
		/*                                                                                                                        if (opts.debug) { fprintf(stderr, "main: Releasing capture %s\n", opts.filename); }
		dtcReleaseCapture(pCapture);
		if (opts.debug) { fprintf(stderr, "main: Recreating capture %s\n", opts.filename); }
		if (!(pCapture = dtcCaptureFromFile(opts.filename, &framecount))) {
		fprintf(stderr, "ERROR in main: Cannot open file %s\n", opts.filename);
		return EXIT_FAILURE;
		}*/
		if (opts.debug) { fprintf(stderr, "main: Getting ROI\n"); }
		if (opts.wROI && opts.hROI)
		{
			croi = cvRect(0, 0, opts.wROI, opts.hROI);
		}
		else
		{
			/*                dtcReinitCaptureRead(pCapture, opts.filename);*/
			if (opts.debug) { fprintf(stderr, "main: Getting ROIcCM\n"); }
			croi = dtcGetFileROIcCM(pCapture, opts.ignore);
			if (opts.debug) { fprintf(stderr, "main: Reinitializing capture\n"); }
			dtcReinitCaptureRead(&pCapture, opts.filename);
			/*                if (!(pCapture = dtcCaptureFromFile(opts.filename, &framecount))) {
			fprintf(stderr, "main: Cannot open file %s\n", opts.filename);
			return EXIT_FAILURE;
			}*/
		}
		if (opts.debug) { fprintf(stderr, "main: ROI ok\n"); }
		if (opts.viewDif) { cvNamedWindow("Differential frame", CV_WINDOW_AUTOSIZE); }
		if (opts.viewRef) { cvNamedWindow("Reference", CV_WINDOW_AUTOSIZE); }
		if (opts.viewROI) { cvNamedWindow("ROI", CV_WINDOW_AUTOSIZE); }
		if (opts.viewTrk) { cvNamedWindow("Tracking", CV_WINDOW_AUTOSIZE); }
		if (opts.viewMsk) { cvNamedWindow("Mask", CV_WINDOW_AUTOSIZE); }
		if (opts.viewThr) { cvNamedWindow("Threshold applied", CV_WINDOW_AUTOSIZE); }
		if (opts.viewSmo) { cvNamedWindow("Filter applied", CV_WINDOW_AUTOSIZE); }
		if (opts.viewRes) { cvNamedWindow("Result frame", CV_WINDOW_AUTOSIZE); }
		if (opts.viewHis) { cvNamedWindow("Histogram", CV_WINDOW_AUTOSIZE); }

		nframe = 0;

		if (opts.darkfilename) {
			if (!(pADUdarkMat = cvLoadImageM(opts.darkfilename, CV_LOAD_IMAGE_GRAYSCALE))) {
				fprintf(stderr, "Warning in main: cannot read dark frame %s\n", opts.darkfilename);
				darkfile_ok = 0;
			}
			else {
				darkfile_ok = 1;
			}
		}
		if (opts.debug) { fprintf(stderr, "main: Reading frames\n"); }

		/*********************************CAPTURE READING******************************************/
		while ((pFrame = dtcQueryFrame(pCapture, opts.ignore, &frame_error)))
		{
			nframe++;
			if (!(frame_error) == 0) {
				frame_errors += 1;
			}
			else {
				CvPoint cm;
				CvRect roi;

				if (opts.debug) { fprintf(stderr, "main: Processing frame %04d\n", nframe); }
				pGryMat = dtcGetGrayMat(pFrame);
				if (darkfile_ok == 1) {
					if ((pADUdarkMat->rows != pGryMat->rows) || (pADUdarkMat->cols != pGryMat->cols)) {
						fprintf(stderr, "WARNING in main: dark frame %s differs from frame properties(%d vs %d rows, %d vs %d cols)\n", opts.darkfilename, pADUdarkMat->rows, pGryMat->rows, pADUdarkMat->cols, pGryMat->cols);
						darkfile_ok = 0;
					}
					else {
						CvMat *pGryDarkMat = NULL;

						pGryDarkMat = cvCreateMat(pGryMat->rows, pGryMat->cols, pGryMat->type);
						cvSub(pGryMat, pADUdarkMat, pGryDarkMat, NULL);
						cvThreshold(pGryDarkMat, pGryMat, 0, 0, CV_THRESH_TOZERO);
						/*                                                cvCopy(pGryDarkMat,pGryMat,NULL);*/
						cvReleaseMat(&pGryDarkMat);
					}
				}
				cm = dtcGetGrayMatCM(pGryMat);
				if (opts.debug) { fprintf(stderr, "main: Applying ROI (%d,%d,%d,%d)\n", cm.x - croi.width / 2, cm.y - croi.height / 2, croi.width, croi.height); }
				roi = cvRect(cm.x - croi.width / 2, cm.y - croi.height / 2, croi.width, croi.height);
				pGryMat = dtcReduceMatToROI(&pGryMat, roi);
				pGryImg = cvCreateImage(cvSize(pGryMat->cols, pGryMat->rows), pFrame->depth, 1);
				cvConvertScale(pGryMat, pGryImg, 1, 0);
				/*                if (opts.ofilename && opts.allframes) {
				strncpy(ofilenamenorm,opts.ofilename,strlen(opts.ofilename)-4);
				sprintf(ofilenamenormframe,"%s" & SINGLE_PREFIX & "%05d.jpg",ofilenamenorm,nframe);
				cvSaveImage(ofilenamenormframe, pGryImg, 0);
				} */

				/*******************FIRST FRAME PROCESSING*******************/
				if (nframe == 1) {
					if (opts.debug) { fprintf(stderr, "main: Getting information from first frame\n"); }
					if (opts.verbose) {
						printf("%s (%s) v%s%s %s by %s\n", PROGNAME, LONGNAME, VERSION_NB, VERSION_MSVC, VERSION_DATE, COPYRIGHT);
						printf("ROI:%3dx%3d\n\n", croi.width, croi.height);

					}
					if (!opts.ADUdtconly) {
						if (!opts.wait && (opts.viewROI || opts.viewTrk || opts.viewDif || opts.viewRef ||
							opts.viewThr || opts.viewSmo || opts.viewRes || opts.viewHis)) {
							if (fps_int>0) {
								opts.wait = (int)(1000 / ceilf(fps_int));
							}
							else {
								opts.wait = (int)(1000 / 25);
							}
						}
						nb_impact = 0;
						init_list(&ptlist, (fps_int * opts.timeImpact));
						init_dtc_struct(&dtc);
						pRefImg = cvCreateImage(cvSize(pGryImg->width, pGryImg->height), IPL_DEPTH_8U, 1);
						pRefImg->origin = pGryImg->origin;
						if (opts.viewDif || opts.viewRes || opts.viewHis || (opts.ovfname && (opts.ovtype == OTYPE_DIF || opts.ovtype == OTYPE_HIS))) {
							pDifImg = cvCreateImage(cvSize(pGryImg->width, pGryImg->height), IPL_DEPTH_8U, 1);
							pDifImg->origin = pGryImg->origin;
						}
						pRefMat = cvCreateMat(pGryImg->height, pGryImg->width, CV_32FC1);
						pDifMat = cvCreateMat(pGryImg->height, pGryImg->width, CV_32FC1);
					}
					if (opts.ostype == OTYPE_ADU) {
						pADUavgMat = cvCreateMat(pGryImg->height, pGryImg->width, CV_32FC1);
						cvSetZero(pADUavgMat);
						pADUmaxMat = cvCreateMat(pGryImg->height, pGryImg->width, CV_32FC1);
						cvSetZero(pADUmaxMat);
						pADUdtcImg = cvCreateImage(cvSize(pGryImg->width, pGryImg->height), IPL_DEPTH_8U, 1);
						if ((opts.ofilename) && (opts.allframes)) {
							pADUdtcMat = cvCreateMat(pGryImg->height, pGryImg->width, CV_32FC1);
							pADUavgMatFrame = cvCreateMat(pGryImg->height, pGryImg->width, CV_32FC1);
						}
					}
					if (!opts.ADUdtconly) {
						if (opts.thrWithMask || opts.viewMsk || (opts.ovfname && (opts.ovtype == OTYPE_MSK))) {
							pMskImg = cvCreateImage(cvSize(pGryImg->width, pGryImg->height), IPL_DEPTH_8U, 1);
							pMskMat = cvCreateMat(pGryImg->height, pGryImg->width, CV_32FC1);
						}
						if (opts.viewThr) {
							pThrImg = cvCreateImage(cvSize(pGryImg->width, pGryImg->height), IPL_DEPTH_8U, 1);
						}
						if (opts.filter.type >= 0 || opts.viewSmo) {
							pSmoImg = cvCreateImage(cvSize(pGryImg->width, pGryImg->height), IPL_DEPTH_8U, 1);
						}
						if (opts.viewTrk || (opts.ovtype == OTYPE_TRK && opts.ovfname)) {
							pTrkImg = cvCreateImage(cvGetSize(pFrame), pFrame->depth, pFrame->nChannels);
						}
						pRefImg = cvClone(pGryImg);
						cvConvert(pRefImg, pRefMat);
						if (opts.debug) { fprintf(stderr, "main: Information from first frame ok\n"); }
					}
				}
				/*******************EVERY FRAME PROCESSING*******************/
				if (opts.debug) { fprintf(stderr, "main: Converting\n"); }
				cvReleaseMat(&pGryMat);
				pGryMat = NULL;
				pGryMat = cvCreateMat(pGryImg->height, pGryImg->width, CV_32FC1);
				cvConvert(pGryImg, pGryMat);
				if (!opts.ADUdtconly) {
					cvAbsDiff(pGryMat, pRefMat, pDifMat);
				}
				if (opts.debug) { fprintf(stderr, "main: Checking ADCdetail\n"); }
				/*ADUdtc algorithm******************************************/
				if (opts.ostype == OTYPE_ADU) {
					cvAdd(pADUavgMat, pGryMat, pADUavgMat, NULL);
					cvMax(pADUmaxMat, pGryMat, pADUmaxMat);
					if (opts.ofilename && opts.allframes) {
						cvConvertScale(pADUavgMat, pADUavgMatFrame, 1.0 / (nframe - frame_errors), 0);
						/*                                cvMinMaxLoc(pADUavgMatFrame, &minLum, &maxLum, &minPoint, &maxPoint, NULL);
						if (maxLum>255) {
						cvConvertScale(pADUavgMatFrame, pADUavgMatFrame, 1.0/255.0, 0);
						}*/
						cvSub(pADUmaxMat, pADUavgMatFrame, pADUdtcMat, NULL);
						cvMinMaxLoc(pADUdtcMat, &minLum, &maxLum, &minPoint, &maxPoint, NULL);
						cvConvertScale(pADUdtcMat, pADUdtcMat, 255.0 / maxLum, 0);
						cvConvert(pADUdtcMat, pADUdtcImg);
						strncpy(ofilenamenorm, opts.ofilename, strlen(opts.ofilename) - 4);
						ofilenamenorm[strlen(opts.ofilename) - 4] = '\0';
						sprintf(ofilenamenormframe, "%s%s%05d.jpg", ofilenamenorm, DTC_MAX_FRAME_PREFIX, nframe);
						cvSaveImage(ofilenamenormframe, pADUdtcImg, 0);
					}
				}
				if (opts.debug) { fprintf(stderr, "main: Checking options 0\n"); }
				if (!opts.ADUdtconly) {
					if (pDifImg) {
						cvConvert(pDifMat, pDifImg);
						if (opts.viewDif) {
							cvShowImage("Differential frame", pDifImg);
						}
						if (nframe == opts.nsaveframe && opts.ofilename && opts.ostype == OTYPE_DIF) {
							cvSaveImage(opts.ofilename, pDifImg, 0);
						}
					}
				}
				cvThreshold(pDifMat, pDifMat, opts.threshold, 0.0, CV_THRESH_TOZERO);
				if (opts.debug) { fprintf(stderr, "main: Checking options 1\n"); }
				if (!opts.ADUdtconly) {
					if (pThrImg) {
						cvConvert(pDifMat, pThrImg);
						cvShowImage("Threshold applied", pThrImg);
					}
					if (opts.filter.type >= 0) {
						cvSmooth(pDifMat, pDifMat, opts.filter.type, opts.filter.param[0], opts.filter.param[1], opts.filter.param[2], opts.filter.param[3]);
					}
					if (opts.viewSmo) {
						cvConvert(pDifMat, pSmoImg);
						cvShowImage("Filter applied", pSmoImg);
					}
					if (opts.viewRef) {
						cvConvert(pRefMat, pRefImg);
						cvShowImage("Reference", pRefImg);
					}
					if (pMskMat && pMskImg) {
						cvThreshold(pDifMat, pMskMat, 0.0, 255.0, CV_THRESH_BINARY_INV);
						cvConvert(pMskMat, pMskImg);
					}
					cvRunningAvg(pGryMat, pRefMat, opts.learningRate, opts.thrWithMask ? pMskImg : NULL);
					if (opts.debug) { fprintf(stderr, "main: Checking options 2\n"); }
					if (pDifImg && opts.viewRes) {
						cvConvert(pDifMat, pDifImg);
						if (opts.viewRes) {
							cvShowImage("Result frame", pDifImg);
						}
					}
					if (opts.viewHis || (opts.ovfname && (opts.ovtype == OTYPE_HIS))) {
						pHisImg = dtcGetHistogramImage(pDifImg, (int)opts.histScale, opts.threshold);
						if (opts.viewHis) {
							cvShowImage("Histogram", pHisImg);
						}
					}
					if (opts.viewMsk && pMskImg) {
						cvShowImage("Mask", pMskImg);
					}
					cvMinMaxLoc(pDifMat, &minLum, &maxLum, &minPoint, &maxPoint, NULL);
					add_tail_item(&ptlist, create_item(create_point(nframe - frame_errors, maxLum, maxPoint.x, maxPoint.y)));
				}
				if (opts.debug) { fprintf(stderr, "main: Checking options 3\n"); }
				if (opts.verbose) {
					if (!opts.ADUdtconly) {
						lum = cvSum(pDifMat);
						printf("%05d %12.6f %5.0f @ (%4d,%4d) %5.0f @ (%4d,%4d)\n", nframe, lum.val[0], minLum, minPoint.x, minPoint.y, maxLum, maxPoint.x, maxPoint.y);
						fflush(stdout);
					}
					if (opts.debug) { fprintf(stderr, "main: Verbose ok\n"); }
				}
				if (!opts.ADUdtconly) {
					if (ptlist.size == ptlist.maxsize) {
						if (opts.debug) { fprintf(stderr, "main: Detecting impacts\n"); }
						nb_impact += detect_impact(&dtc, &ptlist, fps_int, opts.radius, opts.incrLumImpact, opts.incrFrameImpact);
					}
					if (opts.viewROI) {
						cvShowImage("ROI", pGryImg);
					}
					if (pTrkImg) {
						cvCopy(pFrame, pTrkImg, NULL);
						dtcDrawCM(pTrkImg, cm);
						cvRectangle(pTrkImg, cvPoint(cm.x - croi.width / 2, cm.y - croi.height / 2), cvPoint(cm.x + croi.width / 2, cm.y + croi.height / 2), CV_RGB(0, 255, 0), 1, 8, 0);
						if (opts.viewTrk) {
							cvShowImage("Tracking", pTrkImg);
						}
					}
					if (opts.ovfname && opts.ovtype) {
						switch (opts.ovtype) {
						case OTYPE_DIF: pOVdImg = pDifImg; break;
						case OTYPE_TRK: pOVdImg = pTrkImg; break;
						case OTYPE_ROI: pOVdImg = pGryImg; break;
						case OTYPE_HIS: pOVdImg = pHisImg; break;
						case OTYPE_MSK: pOVdImg = pMskImg; break;
						}
						pWriter = dtcWriteVideo(opts.ovfname, pWriter, pCapture, pOVdImg);
					}
				}
				if (opts.wait && (cvWaitKey(opts.wait) == 27)) {
					break;
				}
				if (opts.debug) { fprintf(stderr, "main: Processing ok\n"); }
				pGryImg_height = pGryImg->height;
				pGryImg_width = pGryImg->width;
				cvReleaseImage(&pGryImg);
				pGryImg = NULL;
				cvReleaseMat(&pGryMat);
				pGryMat = NULL;
				/*cvReleaseImage(&pFrame);*/
				if (opts.debug) { fprintf(stderr, "main: Cleaning ok\n"); }
			}
		}
		/*********************************FINAL PROCESSING******************************************/
		if (opts.darkfilename) {
			if (darkfile_ok == 1) {
				cvReleaseMat(&pADUdarkMat);
				pADUdarkMat = NULL;
			}
		}
		if (nframe>0) {
			if (opts.debug) { fprintf(stderr, "main: Final processing (%d frames)\n", nframe); }
			if ((opts.ostype == OTYPE_ADU) && opts.ofilename && opts.allframes) {
				cvReleaseMat(&pADUdtcMat);
				pADUdtcMat = NULL;
				cvReleaseMat(&pADUavgMatFrame);
				pADUavgMatFrame = NULL;
			}
			if (opts.ovfname && opts.ovtype) {
				if (pWriter) {
					cvReleaseVideoWriter(&pWriter);
					pWriter = NULL;
				}
			}

			if (!opts.ADUdtconly) {
				if (ptlist.size < ptlist.maxsize)
					if (opts.debug) { fprintf(stderr, "main: Updating impacts\n"); }
				nb_impact += detect_impact(&dtc, &ptlist, fps_int, opts.radius, opts.incrLumImpact, opts.incrFrameImpact);
				if (opts.debug) { fprintf(stderr, "main: Cleaning list\n"); }
				delete_list(&ptlist);
			}
			if (opts.debug) { fprintf(stderr, "main: Processing ADUdtc\n"); }
			/*ADUdtc algorithm******************************************/
			if (opts.ostype == OTYPE_ADU) {
				if (opts.ofilename) {
					/*Mean image*/
					cvConvertScale(pADUavgMat, pADUavgMat, 1.0 / (nframe - frame_errors), 0);
					cvMinMaxLoc(pADUavgMat, &minLum, &maxLum, &minPoint, &maxPoint, NULL);
					cvMinMaxLoc(pADUmaxMat, &minLummax, &maxLummax, &minPoint, &maxPoint, NULL); /*Max-mean image*/
					pADUdtcMat = cvCreateMat(pGryImg_height, pGryImg_width, CV_32FC1);
					cvSub(pADUmaxMat, pADUavgMat, pADUdtcMat, NULL); /*Max-mean image*/
					cvConvertScale(pADUavgMat, pADUavgMat, 255.0 / maxLum, 0);
					pADUavgImg = cvCreateImage(cvSize(pGryImg_width, pGryImg_height), IPL_DEPTH_8U, 1);
					cvConvert(pADUavgMat, pADUavgImg);
					if (opts.detail) {
						strncpy(ofilenamemean, opts.ofilename, strlen(opts.ofilename) - 4);
						ofilenamemean[strlen(opts.ofilename) - 4] = '\0';
						strcat(ofilenamemean, MEAN_SUFFIX);
						cvSaveImage(ofilenamemean, pADUavgImg, 0);
						if (opts.verbose) { printf("\nMean image (ADU min,ADU max) =               (%5d,%5d) (image %s)\n", (int)minLum, (int)maxLum, ofilenamemean); }
					}
					cvReleaseImage(&pADUavgImg);
					pADUavgImg = NULL;
					/*Max image*/
					if (opts.verbose) {
						printf("Max image (ADU min,ADU max) =                (%5d,%5d)\n", (int)minLummax, (int)maxLummax);
					}
					/*Max-mean image*/
					cvConvert(pADUdtcMat, pADUdtcImg);
					cvMinMaxLoc(pADUdtcMat, &minLum, &maxLum, &minPoint, &maxPoint, NULL);
					if (opts.verbose) { printf("Max-Mean image image (ADU min,ADU max) =     (%5d,%5d) (image %s)\n", (int)minLum, (int)maxLum, opts.ofilename); }
					/*Max-mean normalized image*/
					cvConvertScale(pADUdtcMat, pADUdtcMat, 255.0 / maxLum, 0);
					cvConvert(pADUdtcMat, pADUdtcImg);
					strncpy(ofilenamenorm, opts.ofilename, strlen(opts.ofilename) - 4);
					ofilenamenorm[strlen(opts.ofilename) - 4] = '\0';
					strcat(ofilenamenorm, DTC_MAX_SUFFIX);
					cvSaveImage(ofilenamenorm, pADUdtcImg, 0);
					cvMinMaxLoc(pADUdtcMat, &minLum, &maxLum2, &minPoint, &maxPoint, NULL);
					if (opts.verbose) { printf("Max-Mean normalized image (ADU min,ADU max) =(%5d,%5d) (image %s)\n", (int)minLum, (int)maxLum2, ofilenamenorm); }
					/*Max-mean non normalized image*/
					if (opts.detail) {
						if (maxLum>255) {
							cvConvertScale(pADUdtcMat, pADUdtcMat, maxLum / (255.0*255.0), 0);
						}
						else {
							cvConvertScale(pADUdtcMat, pADUdtcMat, maxLum / 255.0, 0);
						}
						cvConvert(pADUdtcMat, pADUdtcImg);
						strncpy(ofilenamenorm, opts.ofilename, strlen(opts.ofilename) - 4);
						ofilenamenorm[strlen(opts.ofilename) - 4] = '\0';
						strcat(ofilenamenorm, DTC_SUFFIX);
						cvSaveImage(opts.ofilename, pADUdtcImg, 0);
						if (opts.debug) { fprintf(stderr, "main: Max image saved\n"); }
					}
					cvReleaseMat(&pADUdtcMat);
					pADUdtcMat = NULL;
				}
				cvReleaseMat(&pADUavgMat);
				pADUavgMat = NULL;
				cvReleaseMat(&pADUmaxMat);
				pADUmaxMat = NULL;
				cvReleaseImage(&pADUdtcImg);
				pADUdtcImg = NULL;
			}
			if (opts.ignore) {
				dtcCorrectDatation(pCapture, &start_time, &end_time, &duration, &fps_real, &timetype, comment);
			}
			dtcWriteLog(argv[0], start_time, end_time, duration, fps_real, timetype, opts.filename, comment, nb_impact, 1);
			/*FINAL CLEANING**************************************/
			if (opts.debug) { fprintf(stderr, "main: Cleaning\n"); }
			if (opts.viewDif) { cvDestroyWindow("Differential frame"); }
			if (opts.viewRef) { cvDestroyWindow("Reference"); }
			if (opts.viewROI) { cvDestroyWindow("ROI"); }
			if (opts.viewTrk) { cvDestroyWindow("Tracking"); }
			if (opts.viewMsk) { cvDestroyWindow("Mask"); }
			if (opts.viewThr) { cvDestroyWindow("Threshold applied"); }
			if (opts.viewSmo) { cvDestroyWindow("Filter applied"); }
			if (opts.viewRes) { cvDestroyWindow("Result frame"); }
			if (opts.viewHis) { cvDestroyWindow("Histogram"); }
			if (!opts.ADUdtconly) {
				if (opts.thrWithMask || opts.viewMsk || (opts.ovfname && (opts.ovtype == OTYPE_MSK))) {
					cvReleaseImage(&pMskImg);
					pMskImg = NULL;
					cvReleaseMat(&pMskMat);
					pMskMat = NULL;
				}
				if (opts.viewThr) {
					cvReleaseImage(&pThrImg);
					pThrImg = NULL;
				}
				if (opts.filter.type >= 0 || opts.viewSmo) {
					cvReleaseImage(&pSmoImg);
					pSmoImg = NULL;
				}
				if (opts.viewTrk || (opts.ovtype == OTYPE_TRK && opts.ovfname)) {
					cvReleaseImage(&pTrkImg);
					pTrkImg = NULL;
				}
				cvReleaseImage(&pRefImg);
				pRefImg = NULL;
				if (opts.viewDif || opts.viewRes || opts.viewHis || (opts.ovfname && (opts.ovtype == OTYPE_DIF || opts.ovtype == OTYPE_HIS))) {
					cvReleaseImage(&pDifImg);
					pDifImg = NULL;
				}
				cvReleaseMat(&pRefMat);
				pRefMat = NULL;
				cvReleaseMat(&pDifMat);
				pDifMat = NULL;
				if (opts.viewHis || (opts.ovfname && (opts.ovtype == OTYPE_HIS))) {
					cvReleaseImage(&pHisImg);
					pHisImg = NULL;
				}
			}
		}
		else {
			fprintf(stderr, "ERROR in main: no frame to process\n");
		}

		dtcReleaseCapture(pCapture);
		pCapture = NULL;
		if (opts.debug) {
			time(&end_process_time);
			fprintf(stderr, "main: computation time = %d s\n", (int)(end_process_time - start_process_time));
		}
	}
	fflush(stderr);
	fflush(stdout);
	if (opts.debug) { fprintf(stderr, "main: Exiting\n"); }
	return EXIT_SUCCESS;
}