#include "dtcas3.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdio>

bool starts_with(const std::string& s1, const std::string& s2) {
	return s2.size() <= s1.size() && s1.compare(0, s2.size(), s2) == 0;
}

std::vector<std::string> read_txt(std::string path) {
	std::ifstream file(path);
	std::string line;
	std::vector<std::string> lines;
	while (std::getline(file, line)) {
		lines.push_back(line);
	}
	return lines;
}

void read_config_file(std::string path, std::string *filename, std::vector<cv::Point> *cm_list) {
	std::ifstream file(path, std::ios::in);
	std::string line;
	std::vector<std::string> lines;
	float x, y;
	while (std::getline(file, line)) {
		if (starts_with(line, "  file")) {
			line = line.substr(line.find_first_of("e") + 1, line.length());
			while (line.find(' ') != std::string::npos) {
				line.erase(line.find(' '), 1);
			}
			*filename = line;
		}
		else if (starts_with(line, " f ")) {
			lines.push_back(line);
			line = line.substr(line.find_first_of("f") + 1, line.length());
			std::istringstream ss(line);
			ss >> x >> y;
			cm_list->push_back(cv::Point(x, y));
		}
	}
}

int detect(std::string filename, std::vector<cv::Point> cm_list, OPTS opts) {
	clock_t begin, end;
	int progress = 100;
	cv::setUseOptimized(true);
	try {
		opts.filename = strdup(filename.c_str());
		opts.ofilename = strdup(filename.substr(0, filename.find_last_of(".") + 1).append("jpg").c_str());
		std::wstring fname(filename.begin(), filename.end());
		std::string message = "Analysing filename " + filename;
		std::cout << message << std::endl;
		int fps_int = 0;
		double fps_real = 0;
		DtcCapture *pCapture;
		LIST ptlist = { 0,0,NULL,NULL };
		DTCIMPACT dtc;

		cv::Mat pFrame; // Input frame
		cv::Mat pGryMat; // Grey frame
		cv::Mat pRefMat; // Reference frame
		cv::Mat pDifMat; // Difference frame
		cv::Mat pMskMat; // Mask frame
		cv::Mat pHisMat; // Histogram frame
		cv::Mat pThrMat; // Threshold frame
		cv::Mat pSmoMat; // Smooth frame
		cv::Mat pTrkMat; // Tracking frame
		cv::Mat pOVdMat; // Output video frame
		cv::Mat pADUavgMat; // ADU average frame
		cv::Mat pADUmaxMat; // ADU max frame
		cv::Mat pADUdtcMat; // ADU detect frame
		cv::Mat pADUavgMatFrame; // ADU average frame2
		cv::Mat pADUdarkMat; // ADU dark frame
		cv::Mat pFirstFrameROIMat;
		cv::Rect pFirstFrameROI;

		cv::VideoWriter *pWriter = cv::makePtr<cv::VideoWriter>();
		cv::Rect croi = { 0, 0, 0, 0 };

		cv::Point minPoint = { 0,0 };
		cv::Point maxPoint = { 0,0 };
		cv::Point firstFrameCm;

		cv::Scalar lum;

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

		int nb_impact = 0;
		int frame_error = 0;
		int frame_errors = 0;
		int darkfile_ok = 0;

		lum.val[0] = 0.0;
		init_string(ofilenamenormframe);
		init_string(ofilenamemean);
		init_string(ofilenamenorm);
		init_string(comment);
		/*********************************INITIALIZATION******************************************/
		DBOUT("\nmain: Init\n")
			begin = clock();
		time(&start_process_time);
		if (!(pCapture = dtcCaptureFromFile2(opts.filename, &framecount))) {
			return EXIT_FAILURE;
		}
		switch (pCapture->type) {
		case CAPTURE_SER:
			nframe = pCapture->u.sercapture->header.FrameCount;
			break;
		case CAPTURE_FITS:
		case CAPTURE_FILES:
			nframe = pCapture->u.filecapture->FrameCount;
			break;
		default: // CAPTURE_CV
			nframe = (int)(dtcGetCaptureProperty(pCapture, CV_CAP_PROP_FRAME_COUNT));
		}
		if ((nframe > 0) && (nframe < opts.minframes)) {
			dtcReleaseCapture(pCapture);
			pCapture = NULL;
			return EXIT_FAILURE;
		}
		dtcGetDatation((DtcCapture*)pCapture, opts.filename, nframe, &start_time, &end_time, &duration, &fps_real, &timetype, comment);
		if (!opts.ADUdtconly) {
			if (fps_real < 0.02) {
				fps_int = (int)dtcGetCaptureProperty(pCapture, CV_CAP_PROP_FPS);
			}
			else {
				fps_int = (int)(floor(0.5 + fps_real));
			}
		}
		if (opts.dateonly) {
			if (nframe > 0) {
				dtcWriteLog(opts.filename, start_time, end_time, duration, fps_real, timetype, opts.filename, comment, -1, 1);
			}
			dtcReleaseCapture(pCapture);
			pCapture = NULL;
			return 0;
		}
		if (opts.wROI && opts.hROI) {
			croi = cv::Rect(0, 0, opts.wROI, opts.hROI);
		}
		else {
			croi = dtcGetFileROIcCM(pCapture, opts.ignore, 0);
			dtcReinitCaptureRead(&pCapture, opts.filename);
		}
		if (opts.viewDif) { cv::namedWindow("Differential frame", CV_WINDOW_AUTOSIZE); }
		if (opts.viewRef) { cv::namedWindow("Reference", CV_WINDOW_AUTOSIZE); }
		if (opts.viewROI) { cv::namedWindow("ROI", CV_WINDOW_AUTOSIZE); }
		if (opts.viewTrk) { cv::namedWindow("Tracking", CV_WINDOW_AUTOSIZE); }
		if (opts.viewMsk) { cv::namedWindow("Mask", CV_WINDOW_AUTOSIZE); }
		if (opts.viewThr) { cv::namedWindow("Threshold applied", CV_WINDOW_AUTOSIZE); }
		if (opts.viewSmo) { cv::namedWindow("Filter applied", CV_WINDOW_AUTOSIZE); }
		if (opts.viewRes) { cv::namedWindow("Result frame", CV_WINDOW_AUTOSIZE); }
		if (opts.viewHis) { cv::namedWindow("Histogram", CV_WINDOW_AUTOSIZE); }
		nframe = 0;

		if (opts.darkfilename) {
			if (!(pADUdarkMat = cv::imread(opts.darkfilename, CV_LOAD_IMAGE_GRAYSCALE)).data) {
				darkfile_ok = 0;
			}
			else {
				darkfile_ok = 1;
			}
		}
		/*********************************CAPTURE READING******************************************/
		while ((pFrame = dtcQueryFrame2(pCapture, opts.ignore, &frame_error)).data) {
			nframe++;
			if (!(frame_error) == 0) {
				frame_errors += 1;
			}
			else {
				cv::Point cm;
				cv::Rect roi;
				pGryMat = dtcGetGrayMat(pFrame, pCapture);
				if (darkfile_ok == 1) {
					if ((pADUdarkMat.rows != pGryMat.rows) || (pADUdarkMat.cols != pGryMat.cols)) {
						darkfile_ok = 0;
					}
					else {
						cv::Mat pGryDarkMat;
						pGryDarkMat = cv::Mat(pGryMat.size(), pGryMat.type());
						cv::subtract(pGryMat, pADUdarkMat, pGryDarkMat);
						cv::threshold(pGryDarkMat, pGryMat, 0, 0, CV_THRESH_TOZERO);
						pGryDarkMat.release();
					}
				}
				/*
				int rectWidth = croi.width;
				int rectHeight = croi.height;
				if (rectWidth > pGryMat.cols) rectWidth = pGryMat.cols - (rectWidth - pGryMat.cols);
				if (rectHeight > pGryMat.rows) rectHeight = pGryMat.rows - (rectHeight - pGryMat.rows);
				int rectX = cm.x - rectWidth / 2;
				int rectY = cm.y - rectHeight / 2;
				if (rectX < 0) rectX = 0;
				if (rectY < 0) rectY = 0;
				roi = cv::Rect(rectX, rectY, rectWidth, rectHeight);
				*/
				/*******************FIRST FRAME PROCESSING*******************/
				if (nframe == 1) {
					pGryMat.copyTo(pFirstFrameROIMat);
					pFirstFrameROIMat = dtcApplyMask(pFirstFrameROIMat.clone());
					firstFrameCm = dtcGetGrayMatCM(pFirstFrameROIMat);
					//cv::minMaxLoc(pFirstFrameROIMat, NULL, NULL, NULL, &cm);
					roi = cv::Rect(firstFrameCm.x - croi.width / 2, firstFrameCm.y - croi.height / 2, croi.width, croi.height);
					pFirstFrameROIMat = dtcReduceMatToROI(pFirstFrameROIMat.clone(), roi);
					pFirstFrameROI = cv::Rect(roi);
					if (opts.verbose) {
						printf("%s (%s) v%s%s %s by %s\n", PROGNAME, LONGNAME, VERSION_NB, VERSION_MSVC, VERSION_DATE, COPYRIGHT);
						printf("ROI:%3dx%3d\n\n", croi.width, croi.height);
					}
					if (!opts.ADUdtconly) {
						if (!opts.wait && (opts.viewROI || opts.viewTrk || opts.viewDif || opts.viewRef || opts.viewThr || opts.viewSmo || opts.viewRes || opts.viewHis)) {
							if (fps_int > 0) {
								opts.wait = (int)(1000 / ceilf(fps_int));
							}
							else {
								opts.wait = (int)(1000 / 25);
							}
						}
						nb_impact = 0;
						init_list(&ptlist, (fps_int * opts.timeImpact));
						init_dtc_struct(&dtc);
						pFirstFrameROIMat.copyTo(pDifMat);
					}
					if (opts.ostype == OTYPE_ADU) {
						pADUavgMat = cv::Mat::zeros(pGryMat.rows, pGryMat.cols, CV_32FC1);
						pADUmaxMat = cv::Mat::zeros(pGryMat.rows, pGryMat.cols, CV_32FC1);
						if ((opts.ofilename) && (opts.allframes)) {
							pADUdtcMat = cv::Mat(pFirstFrameROIMat.rows, pFirstFrameROIMat.cols, CV_32FC1);
							pADUavgMatFrame = cv::Mat(pFirstFrameROIMat.rows, pFirstFrameROIMat.cols, CV_32FC1);
						}
					}
					if (!opts.ADUdtconly) {
						if (opts.thrWithMask || opts.viewMsk || (opts.ovfname && (opts.ovtype == OTYPE_MSK))) {
							pMskMat = cv::Mat(pFirstFrameROIMat.rows, pFirstFrameROIMat.cols, CV_32FC1);
						}
						if (opts.viewThr) {
							pThrMat = cv::Mat(pFirstFrameROIMat.rows, pFirstFrameROIMat.cols, CV_32FC1);
						}
						if (opts.filter.type >= 0 || opts.viewSmo) {
							pSmoMat = cv::Mat(pFirstFrameROIMat.rows, pFirstFrameROIMat.cols, CV_32FC1);
						}
						if (opts.viewTrk || (opts.ovtype == OTYPE_TRK && opts.ovfname)) {
							pTrkMat = cv::Mat(pFrame.rows, pFrame.cols, pFrame.depth(), pFrame.channels());
						}
						pFirstFrameROIMat.copyTo(pRefMat);
					}
				}
				/*******************EVERY FRAME PROCESSING*******************/
				int x_offset, y_offset;
				roi = dtcCorrelateROI(pGryMat, pFirstFrameROIMat, pFirstFrameROI, &x_offset, &y_offset);
				cm.x = firstFrameCm.x + x_offset;
				cm.y = firstFrameCm.y + y_offset;
				//cm = dtcGetGrayMatCM(pGryMat);
				pGryMat = dtcReduceMatToROI(pGryMat, roi);
				if (!opts.ADUdtconly) {
					cv::absdiff(pGryMat, pRefMat, pDifMat);
				}
				/*ADUdtc algorithm******************************************/
				if (opts.ostype == OTYPE_ADU) {
					pADUavgMat.convertTo(pADUavgMat, CV_32FC1);
					pADUmaxMat.convertTo(pADUmaxMat, CV_32FC1);
					pGryMat.convertTo(pGryMat, CV_32FC1);
					cv::add(pADUavgMat, pGryMat, pADUavgMat);
					cv::max(pADUmaxMat, pGryMat, pADUmaxMat);
					if (opts.ofilename && opts.allframes) {
						pADUavgMat.convertTo(pADUavgMatFrame, pADUavgMat.type(), 1.0 / (nframe - frame_errors), 0);
						cv::subtract(pADUmaxMat, pADUavgMatFrame, pADUdtcMat);
						pADUavgMat.convertTo(pADUavgMat, CV_8UC1);
						pADUmaxMat.convertTo(pADUmaxMat, CV_8UC1);
						pGryMat.convertTo(pGryMat, CV_8UC1);
						cv::minMaxLoc(pADUdtcMat, &minLum, &maxLum, &minPoint, &maxPoint);
						pADUdtcMat.convertTo(pADUdtcMat, pADUdtcMat.type(), 255.0 / maxLum, 0);
						strncpy(ofilenamenorm, opts.ofilename, strlen(opts.ofilename) - 4);
						ofilenamenorm[std::strlen(opts.ofilename) - 4] = '\0';
						sprintf(ofilenamenormframe, "%s%s%05d.jpg", ofilenamenorm, DTC_MAX_FRAME_PREFIX, nframe);
						cv::imwrite(ofilenamenormframe, pADUdtcMat);
					}
				}
				if (!opts.ADUdtconly) {
					if (pDifMat.data) {
						if (opts.viewDif) {
							cv::imshow("Differential frame", pDifMat);
							cv::waitKey(1);
						}
						if (nframe == opts.nsaveframe && opts.ofilename && opts.ostype == OTYPE_DIF) {
							cv::imwrite(opts.ofilename, pDifMat);
						}
					}
				}
				cv::threshold(pDifMat, pThrMat, opts.threshold, 0.0, CV_THRESH_TOZERO);
				if (!opts.ADUdtconly) {
					if (opts.viewThr) {
						cv::imshow("Threshold applied", pThrMat);
						cv::waitKey(1);
					}
					if (opts.filter.type >= 0) {
						cv::medianBlur(pDifMat, pSmoMat, 3);
					}
					if (opts.viewSmo) {
						cv::imshow("Filter applied", pSmoMat);
						cv::waitKey(1);
					}
					if (opts.viewRef) {
						cv::imshow("Reference", pRefMat);
						cv::waitKey(1);
					}
					if (pMskMat.data) {
						cv::threshold(pDifMat, pMskMat, 0.0, 255.0, CV_THRESH_BINARY_INV);
					}
					pRefMat.convertTo(pRefMat, CV_32FC1);
					pGryMat.convertTo(pGryMat, CV_32FC1);
					pMskMat.convertTo(pMskMat, CV_8UC1);
					cv::accumulateWeighted(pGryMat, pRefMat, opts.learningRate, opts.thrWithMask ? pMskMat : cv::noArray());
					pRefMat.convertTo(pRefMat, CV_8UC1);
					pGryMat.convertTo(pGryMat, CV_8UC1);
					if (pDifMat.data && opts.viewRes) {
						if (opts.viewRes) {
							cv::imshow("Result frame", pDifMat);
							cv::waitKey(1);
						}
					}
					if (opts.viewHis || (opts.ovfname && (opts.ovtype == OTYPE_HIS))) {
						pHisMat = dtcGetHistogramImage(pDifMat, opts.histScale, opts.threshold);
						if (opts.viewHis) {
							cv::imshow("Histogram", pHisMat);
							cv::waitKey(1);
						}
					}
					if (opts.viewMsk && pMskMat.data) {
						cv::imshow("Mask", pMskMat);
						cv::waitKey(30);
					}
					cv::minMaxLoc(pDifMat, &minLum, &maxLum, &minPoint, &maxPoint);
					add_tail_item(&ptlist, create_item(create_point(nframe - frame_errors, maxLum, maxPoint.x, maxPoint.y)));
				}
				if (opts.verbose) {
					if (!opts.ADUdtconly) {
						lum = cv::sum(pDifMat);
						printf("%05d %12.6f %5.0f @ (%4d,%4d) %5.0f @ (%4d,%4d)\n", nframe, lum.val[0], minLum, minPoint.x, minPoint.y, maxLum, maxPoint.x, maxPoint.y);
						fflush(stdout);
					}
					if (opts.debug) { DBOUT("main: Verbose ok\n"); }
				}
				if (!opts.ADUdtconly) {
					if (ptlist.size > ptlist.maxsize) {
						nb_impact += detect_impact(&dtc, &ptlist, fps_int, opts.radius, opts.incrLumImpact, opts.incrFrameImpact);
					}
					if (opts.viewROI) {
						cv::imshow("ROI", pGryMat);
						cv::waitKey(1);
					}
					if (pTrkMat.data) {
						pFrame.copyTo(pTrkMat);
						Image pTrkImg;
						pTrkImg.frame = pTrkMat;
						pTrkImg.roi = cv::Rect();
						dtcDrawCM(pTrkImg, cm);
						cv::rectangle(pTrkImg.frame, roi, CV_RGB(0, 255, 0), 1, 8, 0);
						if (opts.viewTrk) {
							cv::imshow("Tracking", pTrkImg.frame);
							cv::waitKey(1);
						}
					}
					if (opts.ovfname && opts.ovtype) {
						switch (opts.ovtype) {
						case OTYPE_DIF: pOVdMat = pDifMat; break;
						case OTYPE_TRK: pOVdMat = pTrkMat; break;
						case OTYPE_ROI: pOVdMat = pGryMat; break;
						case OTYPE_HIS: pOVdMat = pHisMat; break;
						case OTYPE_MSK: pOVdMat = pMskMat; break;
						}
						pWriter = dtcWriteVideo(opts.ovfname, *pWriter, pCapture, pOVdMat);
					}
				}
				if (opts.wait && (cvWaitKey(opts.wait) == 27)) {
					break;
				}
				pGryImg_height = pGryMat.rows;
				pGryImg_width = pGryMat.cols;
				pGryMat.release();
				pGryMat = NULL;
			}
			std::cout << "Frame " << nframe << std::endl;
		}
		/*********************************FINAL PROCESSING******************************************/
		if (opts.darkfilename) {
			if (darkfile_ok == 1) {
				pADUdarkMat.release();
				pADUdarkMat = NULL;
			}
		}
		if (nframe > 0) {
			std::cout << "Frame " << nframe << std::endl;
			if ((opts.ostype == OTYPE_ADU) && opts.ofilename && opts.allframes) {
				pADUdtcMat.release();
				pADUdtcMat = NULL;
				pADUavgMatFrame.release();
				pADUavgMatFrame = NULL;
			}
			if (opts.ovfname && opts.ovtype) {
				if (pWriter) {
					pWriter->release();
					pWriter = nullptr;
				}
			}
			if (!opts.ADUdtconly) {
				if (ptlist.size < ptlist.maxsize) {
					nb_impact += detect_impact(&dtc, &ptlist, fps_int, opts.radius, opts.incrLumImpact, opts.incrFrameImpact);
				}
				delete_list(&ptlist);
			}
			/*ADUdtc algorithm******************************************/
			if (opts.ostype == OTYPE_ADU) {
				if (opts.ofilename) {
					/*Mean image*/
					pADUavgMat.convertTo(pADUavgMat, CV_32FC1);
					pADUmaxMat.convertTo(pADUmaxMat, CV_32FC1);
					pGryMat.convertTo(pGryMat, CV_32FC1);
					pADUavgMat.convertTo(pADUavgMat, pADUavgMat.type(), 1.0 / (nframe - frame_errors), 0);
					cv::minMaxLoc(pADUavgMat, &minLum, &maxLum, &minPoint, &maxPoint);
					cv::minMaxLoc(pADUmaxMat, &minLummax, &maxLummax, &minPoint, &maxPoint); /*Max-mean image*/
					pADUdtcMat = cv::Mat(pGryImg_height, pGryImg_width, CV_32FC1);
					cv::subtract(pADUmaxMat, pADUavgMat, pADUdtcMat); /*Max-mean image*/
					pADUavgMat.convertTo(pADUavgMat, pADUavgMat.type(), 255.0 / maxLum, 0);
					if (opts.detail) {
						//strncpy(ofilenamemean, opts.ofilename, std::strlen(opts.ofilename) - 4);
						snprintf(ofilenamemean, strlen(opts.ofilename) - 4, "%s", opts.ofilename);
						ofilenamemean[std::strlen(ofilenamemean)] = '\0';
						strcat(ofilenamemean, MEAN_SUFFIX);
						cv::imwrite(ofilenamemean, pADUavgMat);
						if (opts.verbose) {
							printf("\nMean image (ADU min,ADU max) =               (%5d,%5d) (image %s)\n", (int)minLum, (int)maxLum, ofilenamemean);
						}
					}
					/*Max image*/
					if (opts.verbose) {
						printf("Max image (ADU min,ADU max) =                (%5d,%5d)\n", (int)minLummax, (int)maxLummax);
					}
					/*Max-mean image*/
					cv::minMaxLoc(pADUdtcMat, &minLum, &maxLum, &minPoint, &maxPoint);
					if (opts.verbose) {
						printf("Max-Mean image image (ADU min,ADU max) =     (%5d,%5d) (image %s)\n", (int)minLum, (int)maxLum, opts.ofilename);
					}
					/*Max-mean normalized image*/
					pADUdtcMat.convertTo(pADUdtcMat, pADUavgMat.type(), 255.0 / maxLum, 0);
					snprintf(ofilenamenorm, strlen(opts.ofilename) - 4, "%s", opts.ofilename);
					//strncpy(ofilenamenorm, opts.ofilename, strlen(opts.ofilename) - 4);
					ofilenamenorm[strlen(ofilenamenorm)] = '\0';
					strcat(ofilenamenorm, DTC_MAX_SUFFIX);
					DBOUT(ofilenamenorm);
					cv::imwrite(ofilenamenorm, pADUdtcMat);
					cv::minMaxLoc(pADUdtcMat, &minLum, &maxLum2, &minPoint, &maxPoint);
					if (opts.verbose) { printf("Max-Mean normalized image (ADU min,ADU max) =(%5d,%5d) (image %s)\n", (int)minLum, (int)maxLum2, ofilenamenorm); }
					/*Max-mean non normalized image*/
					if (opts.detail) {
						if (maxLum > 255) {
							pADUdtcMat.convertTo(pADUdtcMat, pADUdtcMat.type(), maxLum / (255.0*255.0), 0);
						}
						else {
							pADUdtcMat.convertTo(pADUdtcMat, pADUdtcMat.type(), maxLum / 255.0, 0);
						}
						pADUavgMat.convertTo(pADUavgMat, CV_8UC1);
						pADUmaxMat.convertTo(pADUmaxMat, CV_8UC1);
						pGryMat.convertTo(pGryMat, CV_8UC1);
						snprintf(ofilenamenorm, strlen(opts.ofilename) - 4, "%s", opts.ofilename);
						//strncpy(ofilenamenorm, opts.ofilename, strlen(opts.ofilename) - 4);
						ofilenamenorm[std::strlen(ofilenamenorm)] = '\0';
						strcat(ofilenamenorm, DTC_SUFFIX);
						cv::imwrite(opts.ofilename, pADUdtcMat);
					}
					pADUdarkMat.release();
					pADUdtcMat = NULL;
				}
				pADUavgMat.release();
				pADUavgMat = NULL;
				pADUmaxMat.release();
				pADUmaxMat = NULL;
			}
			if (opts.ignore) {
				dtcCorrectDatation((DtcCapture*)pCapture, &start_time, &end_time, &duration, &fps_real, &timetype, comment);
			}
			dtcWriteLog(opts.filename, start_time, end_time, duration, fps_real, timetype, opts.filename, comment, nb_impact, 1);
			/*FINAL CLEANING**************************************/
			if (opts.viewDif) { cv::destroyWindow("Differential frame"); }
			if (opts.viewRef) { cv::destroyWindow("Reference"); }
			if (opts.viewROI) { cv::destroyWindow("ROI"); }
			if (opts.viewTrk) { cv::destroyWindow("Tracking"); }
			if (opts.viewMsk) { cv::destroyWindow("Mask"); }
			if (opts.viewThr) { cv::destroyWindow("Threshold applied"); }
			if (opts.viewSmo) { cv::destroyWindow("Filter applied"); }
			if (opts.viewRes) { cv::destroyWindow("Result frame"); }
			if (opts.viewHis) { cv::destroyWindow("Histogram"); }
			if (!opts.ADUdtconly) {
				if (opts.thrWithMask || opts.viewMsk || (opts.ovfname && (opts.ovtype == OTYPE_MSK))) {
					pMskMat.release();
					pMskMat = NULL;
				}
				if (opts.viewThr) {
					pThrMat.release();
					pThrMat = NULL;
				}
				if (opts.filter.type >= 0 || opts.viewSmo) {
					pSmoMat.release();
					pSmoMat = NULL;
				}
				if (opts.viewTrk || (opts.ovtype == OTYPE_TRK && opts.ovfname)) {
					pTrkMat.release();
					pTrkMat = NULL;
				}
				if (opts.viewDif || opts.viewRes || opts.viewHis || (opts.ovfname && (opts.ovtype == OTYPE_DIF || opts.ovtype == OTYPE_HIS))) {
					pDifMat.release();
					pDifMat = NULL;
				}
				pRefMat.release();
				pRefMat = NULL;
				if (opts.viewHis || (opts.ovfname && (opts.ovtype == OTYPE_HIS))) {
					pHisMat.release();
					pHisMat = NULL;
				}
				pFirstFrameROIMat.release();
				pFirstFrameROIMat = NULL;
			}
		}
		dtcReleaseCapture(pCapture);
		pCapture = NULL;
		time(&end_process_time);
		end = clock();
		int computation_time = (int)(end_process_time - start_process_time);
		std::cout << "Computation time: " << computation_time << " seconds" << std::endl;
		fflush(stderr);
		fflush(stdout);
		return 0;
	}
	catch (cv::Exception& e) {
		std::cout << e.what() << std::endl;
	}
}

void detect_autostakkert(std::string path) {
	opts.filename = opts.ofilename = opts.darkfilename = opts.ovfname = opts.sfname = NULL;
	opts.nsaveframe = 0;
	//opts.ostype = OTYPE_ADU;
	opts.ostype = OTYPE_NO;
	opts.ovtype = OTYPE_NO;
	opts.timeImpact = 4;
	opts.incrLumImpact = 0.9;
	opts.incrFrameImpact = 10;
	opts.radius = 10.0;
	opts.nframesROI = 1;
	opts.bayer = -1;
	opts.medSize = 5;
	opts.wait = 1;
	opts.facSize = 0.9;
	opts.secSize = 1.05;
	opts.threshold = 30.0;
	opts.learningRate = 0.8;
	opts.thrWithMask = 0;
	opts.histScale = 0.8;
	opts.viewROI = 0;
	opts.viewTrk = 0;
	opts.viewDif = 0;
	opts.viewRef = 0;
	opts.viewThr = 0;
	opts.viewSmo = 0;
	opts.viewRes = 0;
	opts.viewHis = 0;
	opts.viewMsk = 0;
	opts.verbose = 0;
	opts.filter.type = -1;
	opts.filter.param[0] = 3;
	opts.filter.param[1] = 3;
	opts.filter.param[2] = 0;
	opts.filter.param[3] = 0;
	opts.debug = 0;
	opts.ADUdtconly = 0;
	opts.detail = 0;
	opts.allframes = 0;
	opts.minframes = 3;
	opts.dateonly = 0;
	opts.ignore = 0;
	opts.videotest = 0;
	opts.wROI = 0;
	opts.hROI = 0;
	std::vector<cv::Point> cm_list;
	std::string filename;
	for (std::string file : read_txt(path)) {
		read_config_file(file, &filename, &cm_list);
		std::cout << filename << std::endl;
		detect(filename, cm_list, opts);
	}
}