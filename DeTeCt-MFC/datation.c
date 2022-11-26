/********************************************************************************/
/*                                                                              */
/*	DTC	(c) Marc Delcroix (delcroix.marc@free.fr) 2012-							*/
/*                                                                              */
/*    DATATION: Detection of datation information functions						*/
/*                                                                              */
/********************************************************************************/

//#include <xkeycheck.h>

//#include "common.h"
#include <time.h>
//#include <stdio.h>
//#include <sys/stat.h>


#include "serfmt.h"
#include "datation.h"
//#include "dtc.h"
#include "wrapper3.h"


const double FPS_MIN		=	0.01;
const double FPS_MAX		=	2000.0;
const double DURATION_MIN	=	0.0005;				/* 1.0/FPS_MAX; */
const double DURATION_MAX	=	ONE_DAY_SEC;

//internal functions

bool IsDateValid(double julianday);
bool IsDurationValid(double duration);
bool IsFPSValid(double fps);
void CorrectDatationFromPIPP(int nbframes, double* pstart_time, double* pend_time, double* pduration, PIPPInfo* pipp_info, char* comment);

/*****************************************************************************************/
/*******************MAIN FUNCTION to get datation of capture******************************/
/*****************************************************************************************/

void dtcGetDatation(DtcCapture* pcapture, char* filename, int nbframes, double* pstart_time, double* pend_time, double* pduration, double* pfps, TIME_TYPE* ptimetype, PIPPInfo* pipp_info, char* comment, Planet_type* pplanet, Datation_source* pdatation_source)
{
	double JD_init = gregorian_calendar_to_jd(1, 1, 1, 0, 0, 0);

	if (InStr(filename, PIPP_STRING) < 0) { // not PIPP file
		(*pipp_info).isPIPP = FALSE;
		dtcGetDatationForFilename(pcapture, filename, nbframes, pstart_time, pend_time, pduration, pfps, ptimetype, comment, pplanet, pdatation_source); // Non PIPP capture file
	}
	else { // From PIPP file
		(*pipp_info).isPIPP = TRUE;

		if ((*pipp_info).capture_exists) { // Original capture used for PIPP exists
			// tbd nbframes for capture_file to estimate + pfps ?
			DtcCapture*	pCapture_original;
			int			nbframes_original;

			pCapture_original = dtcCaptureFromFile2((*pipp_info).capture_filename, &nbframes_original);
			dtcGetDatationForFilename(pCapture_original, (*pipp_info).capture_filename, nbframes_original, pstart_time, pend_time, pduration, pfps, ptimetype, comment, pplanet, pdatation_source);
			CorrectDatationFromPIPP(nbframes_original, pstart_time, pend_time, pduration, pipp_info, comment);
		}
		else {
			dtcGetDatationForFilename(pcapture, filename, nbframes, pstart_time, pend_time, pduration, pfps, ptimetype, comment, pplanet, pdatation_source); // PIPP file, to get at least duration from file info, or more if acquisition log exists or FITS/SER datation
			if (!pdatation_source->filename) {
				if ((!pdatation_source->acquisition_log_file) && (!pdatation_source->ser_file) && (!pdatation_source->ser_file_timestamp) && (!pdatation_source->fits_file) && (pdatation_source->file_info)) { // date info not valid is checked from PIPP video system file info (PIPP process after acquisition)
					(*pstart_time) = JD_init;
					(*pend_time) = JD_init;
				} 	// tbd checks if filename comes from capture file or PIPP ?
			}
			if (strlen((*pipp_info).capture_filename) > 3) { // Original capture used for PIPP does not exists but filename is available
				double start_time_capture = JD_init;
				double mid_time_capture = JD_init;
				Planet_type planet_capture = Notdefined;
				dtcGetDatationFromFilename((*pipp_info).capture_filename, &start_time_capture, &mid_time_capture, ptimetype, &planet_capture); // Only info for orginial capture from capture filename
				// reconciliate capture timing and pipp video timing
				(*pplanet) = planet_capture;
				if ((!IsDateValid((*pstart_time))) || (!IsDateValid((*pend_time)))) {
						if (IsDateValid(start_time_capture)) {
							(*pstart_time) = start_time_capture;
							(*pend_time) = (*pstart_time) + (*pduration) / (ONE_DAY_SEC);
							CorrectDatationFromPIPP(nbframes, pstart_time, pend_time, pduration, pipp_info, comment);
						}
						else if (IsDateValid(mid_time_capture)) {
							(*pstart_time) = mid_time_capture - (*pduration) / (2.0 * ONE_DAY_SEC);
							(*pend_time) = mid_time_capture + (*pduration) / (2.0 * ONE_DAY_SEC);
							CorrectDatationFromPIPP(nbframes, pstart_time, pend_time, pduration, pipp_info, comment);
						}
					}
			} else if ((IsDateValid((*pstart_time))) && (IsDateValid((*pend_time)))) CorrectDatationFromPIPP(nbframes, pstart_time, pend_time, pduration, pipp_info, comment);
		}
	}
}

void dtcGetDatationForFilename(DtcCapture *capture, char *filename, int nbframes, double *pstart_time, double *pend_time, double *pduration, double *pfps, TIME_TYPE *ptimetype, char *comment, Planet_type *planet, Datation_source *pdatation_source)
{
	double JD_init = gregorian_calendar_to_jd(1, 1, 1, 0, 0, 0);

	/*	time_t now;
		struct tm *pnow_tm=malloc(sizeof *pnow_tm);*/

	double start_time_file = JD_init;
	//double mid_time_file = JD_init; // for PIPP
	double end_time_file = JD_init;
	TIME_TYPE timetype_file = Undefined;
	double duration_file = 0.0;
	double fps_file = 0.0;

	double start_time_filename = JD_init;
	double mid_time_filename = JD_init;
	TIME_TYPE timetype_filename = Undefined;

	double start_time_ser = JD_init;
	double end_time_ser = JD_init;
	TIME_TYPE timetype_ser = Undefined;
	double duration_ser = 0.0;
	double fps_ser = 0.0;

	double start_time_fits = JD_init;
	double end_time_fits = JD_init;
	TIME_TYPE timetype_fits = Undefined;
	double duration_fits = 0.0;
	double fps_fits = 0.0;

	double start_time_log = JD_init;
	double end_time_log = JD_init;
	TIME_TYPE timetype_log = Undefined;
	double duration_log = 0.0;
	double fps_log = 0.0;
	long nbframes_log = 0;
	int timezone = -24;

	double time_tmp;
	char comment2[MAX_STRING] = { 0 };
	char software[MAX_STRING] = { 0 };

	// _log.txt file
	//		option -nr = quality + reorder
	// -nr file to ignore
	//		Total input frames: xx
	//		Total output frames: yy:
	// ignore file if total input > total output

	//manages pipp extensions
	//if (InStr(filename, PIPP_STRING)) (*pipp_info).isPIPP = TRUE; else (*pipp_info).isPIPP = FALSE;
	*planet = Notdefined;
	Planet_type planet_fromfilename = Notdefined;

	if (debug_mode) { fprintf(stdout, "dtcGetDatation: Initializing\n"); }
	(*pstart_time) = JD_init;
	(*pend_time) = JD_init;
	(*ptimetype) = Undefined;
	(*pduration) = 0.0;
	(*pfps) = 0.0;

	pdatation_source->acquisition_log_file	= FALSE;
	pdatation_source->ser_file				= FALSE;
	pdatation_source->ser_file_timestamp	= FALSE;
	pdatation_source->fits_file				= FALSE;
	pdatation_source->file_info				= FALSE;
	pdatation_source->filename				= FALSE;
	strcpy(pdatation_source->acquisition_software, "");
	strcpy(software, "");

	if (capture == NULL) {
		strcpy(comment, "cannot open file");
		(*planet) = Notdefined;
		return;
	}


	/*	now=time(NULL);
		pnow_tm=localtime(&now);
		JD_max=gregorian_calendar_to_jd(pnow_tm->tm_year+1900, pnow_tm->tm_mon+1, pnow_tm->tm_mday, pnow_tm->tm_hour, pnow_tm->tm_min, (double) (pnow_tm->tm_sec))+1;*/
		/*fprintf(stdout,"dtcGetDatation: JD_max = %f\n",JD_max);*/

	/********** Date from fileinfo **********/
	switch (capture->type)
	{
	case CAPTURE_SER:
	case CAPTURE_CV:
		//	case CAPTURE_FILES:
		//	case CAPTURE_FITS:
		if (debug_mode) { fprintf(stdout, "dtcGetDatation: Reading information from file\n"); }
		dtcGetDatationFromFileInfo(capture, filename, nbframes, &start_time_file, &end_time_file, &duration_file, &fps_file);
		timetype_file = LT; 
		if (!IsDateValid(start_time_file) && !IsDateValid(end_time_file)) timetype_file = Undefined;
		if (debug_mode) {
			fprintf(stdout, "dtcGetDatation: FILE Start    = %f (", start_time_file);
			fprint_jd(stdout, start_time_file);
			fprintf(stdout, ")\n");
			fprintf(stdout, "dtcGetDatation: FILE End      = %f (", end_time_file);
			fprint_jd(stdout, end_time_file);
			fprintf(stdout, ")\n");
			fprintf(stdout, "dtcGetDatation: FILE Time     = ");
			fprint_timetype(stdout, timetype_file);
			fprintf(stdout, "\n");
			fprintf(stdout, "dtcGetDatation: FILE Duration = %lf\n", duration_file);
			fprintf(stdout, "dtcGetDatation: FILE fps      = %lf\n\n", fps_file);
		}
		break;
	case CAPTURE_FILES:
	case CAPTURE_FITS:
	default:
		if (debug_mode) {
			fprintf(stdout, "dtcGetDatation: FILES/FITS Start    = %fUT, %f (", capture->u.filecapture->StartTimeUTC_JD, capture->u.filecapture->StartTime_JD);
			fprint_jd(stdout, capture->u.filecapture->StartTimeUTC_JD);
			fprintf(stdout, ", ");
			fprint_jd(stdout, capture->u.filecapture->StartTime_JD);
			fprintf(stdout, ")\n");
			fprintf(stdout, "dtcGetDatation: FILES/FITS End      = %fUT, %f (", capture->u.filecapture->EndTimeUTC_JD, capture->u.filecapture->EndTime_JD);
			fprint_jd(stdout, capture->u.filecapture->EndTimeUTC_JD);
			fprintf(stdout, ", ");
			fprint_jd(stdout, capture->u.filecapture->EndTime_JD);
			fprintf(stdout, ")\n");
			fprintf(stdout, "dtcGetDatation: FILES/FITS Time     = ");
			fprint_timetype(stdout, timetype_file);
			fprintf(stdout, "\n");
			fprintf(stdout, "dtcGetDatation: FILES/FITS Duration = %lf\n", duration_file);
			fprintf(stdout, "dtcGetDatation: FILES/FITS fps      = %lf\n\n", fps_file);
		}
		break;
	}
//	if ((*pipp_info).isPIPP) {
//		if (IsDateValid((*pipp_info).start_time)) {
			//start_time_file = (*pipp_info).start_time;
//			end_time_file = start_time_file + duration_file / (2.0 * ONE_DAY_SEC);
//			mid_time_file = (start_time_file + end_time_file) / 2.0;
//			strcat(comment, ", pipp date");
//		}
//		else if (IsDateValid((*pipp_info).mid_time)) {
//			mid_time_file = (*pipp_info).mid_time;
//			start_time_file = mid_time_file - duration_file / (2.0 * ONE_DAY_SEC);
//			strcat(comment, ", pipp date");
//		}
//		else {
//			start_time_file = JD_init;
//			end_time_file = JD_init;
//		}
//	}

	/********** Date from filename **********/
	//if (((*pipp_info).isPIPP) && (strlen((*pipp_info).capture_filename)>3))	dtcGetDatationFromFilename((*pipp_info).capture_filename,	&start_time_filename, &mid_time_filename, &planet_fromfilename); else 
	dtcGetDatationFromFilename(filename,						&start_time_filename, &mid_time_filename, &timetype_filename, &planet_fromfilename);
	//if ((*pipp_info).isPIPP) { //PIPP datation
	//	double delta_start			= 0;
	//	double duration_adjusted	= duration_file;
	//	if (nbframes > 0) { // Checks if number of frames have been truncated
	//		if (((*pipp_info).start_frame > 1) || ((*pipp_info).total_output_frames < nbframes)) {
	//			if ((*pipp_info).start_frame > 1)					delta_start = ((*pipp_info).start_frame - 1) * duration_file / nbframes;
	//			if ((*pipp_info).total_output_frames < nbframes)	duration_adjusted = (*pipp_info).total_output_frames * duration_file / nbframes;
	//			strcat(comment, ", pipp duration");
	//		}
	//	}
	//	if (IsDateValid((*pipp_info).mid_time)) {
	//		start_time_file =	(*pipp_info).mid_time - duration_adjusted / (2.0 * ONE_DAY_SEC); 
	//		end_time_file =		(*pipp_info).mid_time + duration_adjusted / (2.0 * ONE_DAY_SEC);
	//		strcat(comment, ", pipp date");
	//	} else if (IsDateValid((*pipp_info).start_time)) {
	//		start_time_file =	(*pipp_info).start_time;
	//		end_time_file = start_time_file + duration_adjusted / (ONE_DAY_SEC);
	//		strcat(comment, ", pipp date");
	//	} else if (IsDateValid(mid_time_filename)) {
	//		start_time_file =	mid_time_filename - duration_adjusted / (2.0 * ONE_DAY_SEC) + delta_start;
	//		end_time_file =		mid_time_filename + duration_adjusted / (2.0 * ONE_DAY_SEC);
	//	}
	//}

	if (IsDateValid(start_time_filename)) {
		if (((start_time_file - start_time_filename) > 0) && ((start_time_file - start_time_filename) < (30.0 * 60.0) / ONE_DAY_SEC)) { // delta < 30min, no timezone
			start_time_file = start_time_filename; // use filename information
			if (timetype_filename == UT) timetype_file = UT;
			else timetype_file = LT; // same time; as time_file is local, everything is local
			if (IsDurationValid(duration_file)) { // duration ok, corrects end_time - duration_file : 2?
				end_time_file = start_time_filename + duration_file / ONE_DAY_SEC;
			}
			else { // corrects duration, and end_time file accordingly
				duration_file = (end_time_file - start_time_filename) * ONE_DAY_SEC; // WARNING: assumes this is correct
				end_time_file = start_time_filename + duration_file / ONE_DAY_SEC;
			}
			if (fps_file == 0) fps_file = nbframes / duration_file;
			pdatation_source->filename = TRUE;
		}
		else if (fabs(start_time_file - start_time_filename) < (0.5 + (30.0 * 60.0) / ONE_DAY_SEC)) { // potential timezone < 12h30
			//timezone = (int)(floor(fabs(start_time_file - start_time_filename) * 24.0));
			timezone = (int)(round((start_time_file - start_time_filename) * 24.0));
			//if ((start_time_file - start_time_filename) < 0)  timezone = -timezone - 1;
			start_time_file =	start_time_filename;
			end_time_file -=	 timezone / 24.0;
			timetype_file = UT;
			if (IsDurationValid(duration_file)) { // duration ok, corrects end_time, duration / 2 ?
				end_time_file =	start_time_filename + duration_file / ONE_DAY_SEC;
			}
			else { // corrects duration, and end_time file accordingly
				duration_file = (end_time_file - start_time_filename) * ONE_DAY_SEC; // WARNING: assumes this is correct
				end_time_file = start_time_filename + duration_file / ONE_DAY_SEC;
			}
		}
		else {
			start_time_file = start_time_filename; // use filename information
			if (timetype_filename == UT) timetype_file = UT;
			else timetype_file = Undefined; // same time; as time_file is local, everything is local
			if (IsDurationValid(duration_file)) { // duration ok, corrects end_time, duration / 2 ?
				end_time_file = start_time_filename + duration_file / ONE_DAY_SEC;
			}
			else end_time_file = start_time_file;		// use filename information
			if (fps_file == 0) fps_file = nbframes / duration_file;
			pdatation_source->filename = TRUE;
		}
		strcpy(comment, "file info+filename");
	}
	else if (IsDateValid(mid_time_filename)) {
		if (((start_time_file - mid_time_filename) > 0) && ((start_time_file - mid_time_filename) < (30.0 * 60.0) / ONE_DAY_SEC / 2.0)) { // delta duration < 15min, no timezone
			if (timetype_filename == UT) timetype_file = UT;
			else timetype_file = LT; // same time; as time_file is local, everything is local
			if (IsDurationValid(duration_file)) {		// duration ok, corrects end_time, duration / 2 ?
				start_time_file = mid_time_filename - duration_file / (2.0 * ONE_DAY_SEC);	// use filename information
				end_time_file = start_time_file + duration_file / ONE_DAY_SEC;				// use filename information
			}
			else {																				// corrects duration, and end_time file accordingly
				duration_file = (mid_time_filename - start_time_file) * ONE_DAY_SEC;			// WARNING: assumes this is correct
				start_time_file =	mid_time_filename - duration_file / (2.0 * ONE_DAY_SEC);	// use filename information
				end_time_file =		mid_time_filename + duration_file / (2.0 * ONE_DAY_SEC);	// use filename information
			}
			if (fps_file == 0) fps_file = nbframes / duration_file;
			pdatation_source->filename = TRUE;
		}
		else if (fabs(start_time_file - mid_time_filename) < (0.5 + (30.0 * 60.0) / ONE_DAY_SEC / 2.0)) {// potential timezone < 12h30
			//timezone = (int)(floor(fabs(start_time_file - mid_time_filename) * 24.0));
			timezone = (int)(round((start_time_file - mid_time_filename) * 24.0));
			//if (((start_time_file + end_time_file) / 2.0 - mid_time_filename) < 0)  timezone = -timezone - 1;
			start_time_file -= timezone / 24.0;
			end_time_file	-= timezone / 24.0;
			timetype_file = UT;
			if (IsDurationValid(duration_file)) {			// duration ok, corrects end_time, duration / 2 ?
				//start_time_file =	mid_time_filename + timezone / 24.0 - duration_file / (2.0 * ONE_DAY_SEC);	// use filename information
				//end_time_file =		mid_time_filename + timezone / 24.0 + duration_file / (2.0 * ONE_DAY_SEC);	// use filename information
				start_time_file =	mid_time_filename - duration_file / (2.0 * ONE_DAY_SEC);	// use filename information
				end_time_file =		mid_time_filename + duration_file / (2.0 * ONE_DAY_SEC);	// use filename information
			}
			else {											// corrects duration, and end_time file accordingly
				duration_file = (end_time_file - mid_time_filename) * 2.0 * ONE_DAY_SEC;		// WARNING: assumes this is correct
				//start_time_file =	mid_time_filename + timezone / 24.0 - duration_file / (2.0 * ONE_DAY_SEC);	// use filename information
				//end_time_file =		mid_time_filename + timezone / 24.0 + duration_file / (2.0 * ONE_DAY_SEC);	// use filename information
				start_time_file =	mid_time_filename - duration_file / (2.0 * ONE_DAY_SEC);	// use filename information
				end_time_file =		mid_time_filename + duration_file / (2.0 * ONE_DAY_SEC);	// use filename information
			}
			if (fps_file == 0) fps_file = nbframes / duration_file;
			pdatation_source->filename = TRUE;
		}
		else {
			//time from file info
			if (timetype_filename == UT) timetype_file = UT;
			else timetype_file = Undefined; // same time; as time_file is local, everything is local
			//timetype_file = UT; //timetype should be Undefined, or not set ??????????
			if (IsDurationValid(duration_file)) {		// duration ok, corrects end_time, duration / 2 ?
				start_time_file = mid_time_filename - duration_file / (2.0 * ONE_DAY_SEC);	// use filename information
				end_time_file = start_time_file + duration_file / ONE_DAY_SEC;				// use filename information
			}
			else {
				start_time_file = mid_time_filename;	// use filename information
				end_time_file = start_time_file;		// use filename information
			}
			if (fps_file == 0) fps_file = nbframes / duration_file;
			pdatation_source->filename = TRUE;
		}
		strcpy(comment, "file info+filename");
	}
	
	switch (capture->type)
	{
		case CAPTURE_SER:
/********** Date from ser file **********/	
/* Attempting date from SER file */
			start_time_ser=capture->u.sercapture->StartTimeUTC_JD;
			timetype_ser=UT;
			if (IsDateValid(capture->u.sercapture->StartTimeUTC_JD) && (fabs(timezone)>12) && (fabs(floor(0.5+capture->u.sercapture->StartTime_JD-capture->u.sercapture->StartTimeUTC_JD)*24)<=12)) {
				timezone=(int) floor(0.5+(capture->u.sercapture->StartTime_JD-capture->u.sercapture->StartTimeUTC_JD)*24);
			}
												if (debug_mode) { fprintf(stdout,"dtcGetDatation: Reading information from ser file\n"); }
			serReadTimeStamps(capture->u.sercapture);
			if ((capture->u.sercapture->TimeStampExists) || (IsDateValid(capture->u.sercapture->StartTimeUTC_JD) && IsDateValid(capture->u.sercapture->EndTimeUTC_JD))) {
/* End date available from SER file */
				timetype_ser=UT;
				duration_ser=(capture->u.sercapture->EndTimeUTC_JD - capture->u.sercapture->StartTimeUTC_JD)*ONE_DAY_SEC;
				if (!IsDurationValid(duration_ser)) { // duration <0 ? DURATION_MIN ?
					duration_ser=0;
					fps_ser = 0;
					end_time_ser=capture->u.sercapture->EndTimeUTC_JD;
				} else {
					fps_ser=(capture->u.sercapture->header.FrameCount-1)/duration_ser;
					duration_ser=duration_ser+1/fps_ser;
					end_time_ser=capture->u.sercapture->EndTimeUTC_JD+1.0/fps_ser/ONE_DAY_SEC;
				}
			} else {
				end_time_ser=capture->u.sercapture->EndTimeUTC_JD;
				if (end_time_ser>start_time_ser) {
					timetype_ser=UT;
				}
			}
			if (IsDateValid(capture->u.sercapture->EndTimeUTC_JD) && (abs(timezone)>12) && (fabs(floor(0.5+capture->u.sercapture->EndTime_JD-capture->u.sercapture->EndTimeUTC_JD)*24)<=12)) {
				timezone=(int) floor(0.5+(capture->u.sercapture->EndTime_JD-capture->u.sercapture->EndTimeUTC_JD)*24);
			}

												if (debug_mode) {
													fprintf(stdout,"dtcGetDatation: SER  Start    = %f (", start_time_ser);
													fprint_jd(stdout, start_time_ser);
													fprintf(stdout,")\n");
													fprintf(stdout,"dtcGetDatation: SER  End      = %f (", end_time_ser);
													fprint_jd(stdout, end_time_ser);
													fprintf(stdout,")\n");
													fprintf(stdout,"dtcGetDatation: SER  Time     = ");
													fprint_timetype(stdout, timetype_ser);
													fprintf(stdout,"\n");
													if (abs(timezone)<=12) {
														fprintf(stdout,"dtcGetDatation: SER  timezone = %d\n",timezone);
													}
													fprintf(stdout,"dtcGetDatation: SER  Duration = %lf\n", duration_ser);
													fprintf(stdout,"dtcGetDatation: SER  fps      = %lf\n\n",fps_ser);
												}			
			if (IsDateValid(end_time_ser)) {
				strcpy(comment,"ser file");
				pdatation_source->ser_file = TRUE;
				if (capture->u.sercapture->TimeStampExists) pdatation_source->ser_file_timestamp = TRUE;
				if (IsDurationValid(duration_ser)) {
					(*pduration)=duration_ser;
				} else if (IsFPSValid(*pfps)) {
					(*pduration)=nbframes/(*pfps);
					strcat(comment,", duration estimated");
				} else if (IsDurationValid(duration_file)) {				/* from file */
					(*pduration)=duration_file;
				}
				(*ptimetype)=timetype_ser;
				(*pfps)=fps_ser;
				(*pend_time)=end_time_ser;
				if (IsDateValid(start_time_ser)) {
					(*pstart_time)=start_time_ser;
				} else {
					(*pstart_time)=(*pend_time)-(*pduration)/ONE_DAY_SEC;
					strcat(comment,", start date estimated");
				}
			}
			if (IsDateValid(start_time_ser)) {
				strcpy(comment,"ser file");
				pdatation_source->ser_file = TRUE;
				if (IsDurationValid(duration_ser)) {
					(*pduration)=duration_ser;
				} else if (IsFPSValid(*pfps)) {
					(*pduration)=nbframes/(*pfps);
					strcat(comment,", duration estimated");
				} else if (IsDurationValid(duration_file)) {				/* from file */
					(*pduration)=duration_file;
				}
				(*ptimetype)=timetype_ser;
				(*pfps)=fps_ser;
				(*pstart_time)=start_time_ser;
				if (end_time_ser>start_time_ser) {
					(*pend_time)=end_time_ser;
				} else {
					(*pend_time)=(*pstart_time)+(*pduration)/ONE_DAY_SEC;
					strcat(comment,", end date estimated");
				}
			}
			break;
		case CAPTURE_FILES:
			start_time_fits=capture->u.filecapture->StartTime_JD;
			end_time_fits=capture->u.filecapture->EndTime_JD;
			timetype_fits=LT;
			duration_fits=(capture->u.filecapture->EndTime_JD - capture->u.filecapture->StartTime_JD)*ONE_DAY_SEC;
			if (!IsDurationValid(duration_fits)) {// duration <0 ? DURATION_MIN ?
					duration_fits=0;
			}
			if (fabs(duration_fits)<DURATION_MIN) {
				fps_fits=0;
				end_time_fits=JD_init;
			} else {
				fps_fits=(capture->u.filecapture->LastFileIdx-capture->u.filecapture->FirstFileIdx+1)/duration_fits;
				duration_fits=duration_fits+1/fps_fits;
				end_time_fits=capture->u.filecapture->EndTime_JD+1/fps_fits/ONE_DAY_SEC;
			}	
												if (debug_mode) {
													fprintf(stdout,"dtcGetDatation: FILES Start    = %f (", start_time_fits);
													fprint_jd(stdout, start_time_fits);
													fprintf(stdout,")\n");
													fprintf(stdout,"dtcGetDatation: FILES End      = %f (", end_time_fits);
													fprint_jd(stdout, end_time_fits);
													fprintf(stdout,")\n");
													fprintf(stdout,"dtcGetDatation: FILES Time     = ");
													fprint_timetype(stdout, timetype_fits);
													fprintf(stdout,"\n");
													fprintf(stdout,"dtcGetDatation: FILES Duration = %lf\n", duration_fits);
													fprintf(stdout,"dtcGetDatation: FILES fps      = %lf\n\n",fps_fits);
												}

			dtcGetDatationFromFilename(filename, &start_time_filename, &mid_time_filename, &timetype_filename, &planet_fromfilename);;
			if (IsDateValid(start_time_filename)) {
				if (((start_time_fits - start_time_filename) > 0) && ((start_time_fits - start_time_filename) < (30.0 * 60.0) / ONE_DAY_SEC)) { // delta < 30min, no timezone
					start_time_fits = start_time_filename; // use filename information
					if (timetype_filename == UT) timetype_file = UT;
					else timetype_file = LT; // same time; as time_file is local, everything is local
					if (IsDurationValid(duration_fits)) { // duration ok, corrects end_time - duration_fits : 2?
						end_time_fits = start_time_filename + duration_fits / ONE_DAY_SEC;
					}
					else { // corrects duration, and end_time file accordingly
						duration_fits = (end_time_fits - start_time_filename) * ONE_DAY_SEC; // WARNING: assumes this is correct
						end_time_fits = start_time_filename + duration_fits / ONE_DAY_SEC;
					}
					if (fps_fits == 0) fps_fits = nbframes / duration_fits;
					pdatation_source->filename = TRUE;
				}
				else if (fabs(start_time_fits - start_time_filename) < (0.5 + (30.0 * 60.0) / ONE_DAY_SEC)) { // potential timezone < 12h30
					//timezone = (int)(floor(fabs(start_time_fits - start_time_filename) * 24.0));
					timezone = (int)(round((start_time_fits - start_time_filename) * 24.0));
					//if ((start_time_fits - start_time_filename) < 0)  timezone = -timezone - 1;
					start_time_fits = start_time_filename;
					end_time_fits -= timezone / 24.0;
					timetype_fits = UT;
					if (IsDurationValid(duration_fits)) { // duration ok, corrects end_time, duration / 2 ?
						end_time_fits = start_time_filename + duration_fits / ONE_DAY_SEC;
					}
					else { // corrects duration, and end_time file accordingly
						duration_fits = (end_time_fits - start_time_filename) * ONE_DAY_SEC; // WARNING: assumes this is correct
						end_time_fits = start_time_filename + duration_fits / ONE_DAY_SEC;
					}
				}
				else {
					start_time_fits = start_time_filename; // use filename information
					if (timetype_filename == UT) timetype_file = UT;
					else timetype_file = Undefined; // same time; as time_file is local, everything is local
					if (IsDurationValid(duration_fits)) { // duration ok, corrects end_time, duration / 2 ?
						end_time_fits = start_time_filename + duration_fits / ONE_DAY_SEC;
					}
					else end_time_fits = mid_time_filename;	// use filename information
					if (fps_fits == 0) fps_fits = nbframes / duration_fits;
					pdatation_source->filename = TRUE;
				}
				strcpy(comment, "file info+filename");
			}
			else if (IsDateValid(mid_time_filename)) {
				if (((start_time_fits - mid_time_filename) > 0) && ((start_time_fits - mid_time_filename) < (30.0 * 60.0) / ONE_DAY_SEC / 2.0)) { // delta duration < 15min, no timezone
					if (timetype_filename == UT) timetype_file = UT;
					else timetype_file = LT; // same time; as time_file is local, everything is local
					if (IsDurationValid(duration_fits)) {		// duration ok, corrects end_time, duration / 2 ?
						start_time_fits = mid_time_filename - duration_fits / (2.0 * ONE_DAY_SEC);	// use filename information
						end_time_fits = start_time_fits + duration_fits / ONE_DAY_SEC;				// use filename information
					}
					else {																				// corrects duration, and end_time file accordingly
						duration_fits = (mid_time_filename - start_time_fits) * ONE_DAY_SEC;			// WARNING: assumes this is correct
						start_time_fits = mid_time_filename - duration_fits / (2.0 * ONE_DAY_SEC);	// use filename information
						end_time_fits = mid_time_filename + duration_fits / (2.0 * ONE_DAY_SEC);	// use filename information
					}
					if (fps_fits == 0) fps_fits = nbframes / duration_fits;
					pdatation_source->filename = TRUE;
				}
				else if (fabs(start_time_fits - mid_time_filename) < (0.5 + (30.0 * 60.0) / ONE_DAY_SEC / 2.0)) {// potential timezone < 12h30
					//timezone = (int)(floor(fabs(start_time_fits - mid_time_filename) * 24.0));
					timezone = (int)(round((start_time_fits - mid_time_filename) * 24.0));
					//if (((start_time_fits + end_time_fits) / 2.0 - mid_time_filename) < 0)  timezone = -timezone - 1;
					start_time_fits -= timezone / 24.0;
					end_time_fits -= timezone / 24.0;
					timetype_fits = UT;
					if (IsDurationValid(duration_fits)) {			// duration ok, corrects end_time, duration / 2 ?
						//start_time_fits = mid_time_filename + timezone / 24.0 - duration_fits / (2.0 * ONE_DAY_SEC);	// use filename information
						//end_time_fits = mid_time_filename + timezone / 24.0 + duration_fits / (2.0 * ONE_DAY_SEC);	// use filename information
						start_time_fits = mid_time_filename - duration_fits / (2.0 * ONE_DAY_SEC);	// use filename information
						end_time_fits = mid_time_filename + duration_fits / (2.0 * ONE_DAY_SEC);	// use filename information
					}
					else {											// corrects duration, and end_time file accordingly
						duration_fits = (end_time_fits - mid_time_filename) * 2.0 * ONE_DAY_SEC;		// WARNING: assumes this is correct
						//start_time_fits = mid_time_filename + timezone / 24.0 - duration_fits / (2.0 * ONE_DAY_SEC);	// use filename information
						//end_time_fits = mid_time_filename + timezone / 24.0 + duration_fits / (2.0 * ONE_DAY_SEC);	// use filename information
						start_time_fits = mid_time_filename - duration_fits / (2.0 * ONE_DAY_SEC);	// use filename information
						end_time_fits = mid_time_filename + duration_fits / (2.0 * ONE_DAY_SEC);	// use filename information
					}
					if (fps_fits == 0) fps_fits = nbframes / duration_fits;
					pdatation_source->filename = TRUE;
				}
				else {
					if (timetype_filename == UT) timetype_file = UT;
					else timetype_file = Undefined; // same time; as time_file is local, everything is local
					if (IsDurationValid(duration_fits)) {		// duration ok, corrects end_time, duration / 2 ?
						start_time_fits = mid_time_filename - duration_fits / (2.0 * ONE_DAY_SEC);	// use filename information
						end_time_fits = start_time_fits + duration_fits / ONE_DAY_SEC;				// use filename information
					}
					else {
						start_time_fits = mid_time_filename;	// use filename information
						end_time_fits = start_time_fits;				// use filename information
					}
					if (fps_fits == 0) fps_fits = nbframes / duration_fits;
					pdatation_source->filename = TRUE;
				}
				strcpy(comment, "file info+filename");
			}

			if (IsDateValid(start_time_fits)) {
				strcpy(comment, "file info");
				pdatation_source->fits_file = TRUE;
				(*pfps) = fps_fits;
				if (IsDurationValid(duration_fits)) {
					(*pduration) = duration_fits;
				}
				else if (IsFPSValid(*pfps)) {
					(*pduration) = nbframes / (*pfps);
					strcat(comment, ", duration estimated");
					/*				} else if ((duration_file>DURATION_MIN) && (duration_file<=DURATION_MAX)) {
										(*pduration)=duration_file;*/
				}
				(*ptimetype) = timetype_fits;
				(*pstart_time) = start_time_fits;
				if ((end_time_fits > (*pstart_time)) && ((end_time_fits - (*pstart_time)) < ONE_DAY_SEC / 2.0)) {
					(*pend_time) = end_time_fits;
				}
				else {
					(*pend_time) = (*pstart_time) + (*pduration) / ONE_DAY_SEC;
					strcat(comment, ", end date estimated");
				}
			}

			break;
		case CAPTURE_FITS:
/********** Date from FITS file **********/	
			start_time_fits=capture->u.filecapture->StartTimeUTC_JD;
			end_time_fits=capture->u.filecapture->EndTimeUTC_JD;
			timetype_fits=UT;
			duration_fits=(capture->u.filecapture->EndTimeUTC_JD - capture->u.filecapture->StartTimeUTC_JD)*ONE_DAY_SEC;
			if (!IsDurationValid(duration_fits)) {// duration <0 ? DURATION_MIN ?
					duration_fits=0;
			}
			if (fabs(duration_fits)<DURATION_MIN) {
				fps_fits=0;
				end_time_fits=JD_init;
			} else {
				fps_fits=((double)capture->u.filecapture->LastFileIdx - (double)capture->u.filecapture->FirstFileIdx+1)/duration_fits;
				duration_fits=duration_fits+1/fps_fits;
				end_time_fits=capture->u.filecapture->EndTimeUTC_JD+1/fps_fits/ONE_DAY_SEC;
			}	
												if (debug_mode) {
													fprintf(stdout,"dtcGetDatation: FITS Start    = %f (", start_time_fits);
													fprint_jd(stdout, start_time_fits);
													fprintf(stdout,")\n");
													fprintf(stdout,"dtcGetDatation: FITS End      = %f (", end_time_fits);
													fprint_jd(stdout, end_time_fits);
													fprintf(stdout,")\n");
													fprintf(stdout,"dtcGetDatation: FITS Time     = ");
													fprint_timetype(stdout, timetype_fits);
													fprintf(stdout,"\n");
													fprintf(stdout,"dtcGetDatation: FITS Duration = %lf\n", duration_fits);
													fprintf(stdout,"dtcGetDatation: FITS fps      = %lf\n\n",fps_fits);
												}
			if (IsDateValid(start_time_fits)) {
				strcpy(comment,"FITS info");
				pdatation_source->fits_file = TRUE;

				(*pfps)=fps_fits;
				if (IsDurationValid(duration_fits)) {
					(*pduration)=duration_fits;
				} else if (IsFPSValid(*pfps)) {
					(*pduration)=nbframes/(*pfps);
					strcat(comment,", duration estimated");
/*				} else if ((duration_file>DURATION_MIN) && (duration_file<=DURATION_MAX)) {
					(*pduration)=duration_file;*/
				}
				(*ptimetype)=timetype_fits;
				(*pstart_time)=start_time_fits;
				if ((end_time_fits>(*pstart_time)) && ((end_time_fits-(*pstart_time))<ONE_DAY_SEC/2.0)) {
					(*pend_time)=end_time_fits;
				} else {
					(*pend_time)=(*pstart_time)+(*pduration)/ONE_DAY_SEC;
					strcat(comment,", end date estimated");
				}
			}
			break;
		case CAPTURE_CV:
		default: // CAPTURE_CV
			break;
	}
/********** Calculation of timezone if possible **********/	
if ((timezone<-12) && (*ptimetype)==UT) {
	if (IsDateValid(*pend_time)) {
		timezone=(int) floor(0.5+(end_time_file-(*pend_time))*24);
		if (fabs(timezone)>12) {
			timezone=-24;
		}
	} else if (IsDateValid(*pstart_time)) {
		timezone=(int) floor(0.5+(end_time_file-(*pstart_time))*24);
		if (fabs(timezone)>13) {
			timezone=-24;
		}
	}
}
/********** Date from log file **********/	
											if (debug_mode) { fprintf(stdout,"dtcGetDatation: Reading information from log file\n"); }
//	if (((*pipp_info).isPIPP) && (strlen((*pipp_info).capture_filename) > 0))	dtcGetInfoDatationFromLogFile((*pipp_info).capture_filename,	&start_time_log, &end_time_log, &duration_log, &fps_log, &nbframes_log, &timetype_log, comment2, planet, software, &capture->CaptureInfo);	else
	dtcGetInfoDatationFromLogFile(filename,							&start_time_log, &end_time_log, &duration_log, &fps_log, &nbframes_log, &timetype_log, comment2, planet, software, &capture->CaptureInfo);
	if (*planet == Notdefined) *planet = planet_fromfilename;

	if ((duration_log<DURATION_MIN) && (IsDurationValid(end_time_log-start_time_log)*ONE_DAY_SEC)) {
		duration_log=(end_time_log-start_time_log)*ONE_DAY_SEC;
	}
											if (debug_mode) {
												fprintf(stdout,"dtcGetDatation: LOG  Start    = %f (", start_time_log);
												fprint_jd(stdout, start_time_log);
												fprintf(stdout,")\n");
												fprintf(stdout,"dtcGetDatation: LOG  End      = %f (", end_time_log);
												fprint_jd(stdout, end_time_log);
												fprintf(stdout,")\n");
												fprintf(stdout,"dtcGetDatation: LOG  Time     = ");
												fprint_timetype(stdout, timetype_log);
												fprintf(stdout,"\n");
												fprintf(stdout,"dtcGetDatation: LOG  Duration = %lf\n", duration_log);
												fprintf(stdout,"dtcGetDatation: LOG  fps      = %lf\n\n",fps_log);
												fprintf(stdout,"dtcGetDatation: Comment       = %s\n\n",comment2);
												fprintf(stdout, "dtcGetDatation: LOG nframe      = %d\n\n", nbframes_log);
											}
	if (nbframes_log != nbframes) { fprintf(stdout, "WARNING: real number of frames %ld differs from theorical number of frames %ld, using real number.\n", nbframes, nbframes_log); }
/********** Use log file information if available **********/	
//	if ((*pipp_info).isPIPP) { //PIPP datation
//		double delta_start = 0;
//		double duration_adjusted = duration_log;
//		if (nbframes > 0) { // Checks if number of frames have been truncated
//			if (((*pipp_info).start_frame > 1) || ((*pipp_info).total_output_frames < nbframes)) {
//				if ((*pipp_info).start_frame > 1)					delta_start = ((*pipp_info).start_frame - 1) * duration_log / nbframes;
//				if ((*pipp_info).total_output_frames < nbframes)	duration_adjusted = (*pipp_info).total_output_frames * duration_log / nbframes;
//				strcat(comment, ", pipp duration");
//			}
//		}
//		duration_log = duration_adjusted;
//		start_time_log = start_time_log + delta_start;
//		end_time_log = start_time_log + duration_adjusted;
//	}

	if (IsDateValid(start_time_log)) {
		strcpy(comment,comment2);
		pdatation_source->acquisition_log_file = TRUE;
		strcpy(pdatation_source->acquisition_software, software);

		(*ptimetype)=timetype_log;
		(*pstart_time)=start_time_log;
		if (IsFPSValid(fps_log)) {
			(*pfps)=fps_log;
		}
		if (IsDurationValid(duration_log)) {
			(*pduration)=duration_log;
		} else if ((*pduration)<DURATION_MIN) {
			if (IsFPSValid(*pfps)) {
				(*pduration)=nbframes/(*pfps);
				strcat(comment,", duration calculated");
			} else if (IsDurationValid(duration_file)) {
				(*pduration)=duration_file;
			}
		}
		if (end_time_log>start_time_log) {
			(*pend_time)=end_time_log;
			if ((*pduration)<DURATION_MIN) {
				(*pduration)=((*pend_time)-start_time_log)*ONE_DAY_SEC;
			}
		} else {
			(*pend_time)=(*pstart_time)+(*pduration)/ONE_DAY_SEC;
			strcat(comment,", end date estimated");
		}
	}
/********** No date from log/FITS/SER => use file info, with duration from others **********/	
	if (!IsDateValid(*pstart_time)) {
		strcpy(comment,"file info");
		pdatation_source->file_info = TRUE;
		if (IsDurationValid(duration_log)) {
			(*pduration)=duration_log;
			strcat(comment,", ");
			strcat(comment,comment2);
		} else if (((*pduration)<DURATION_MIN) && IsDurationValid(duration_file)) {
			(*pduration)=duration_file;
		}
		if (IsFPSValid(fps_log)) {
			(*pfps)=fps_log;
		} else if (duration_file>0) {
			(*pfps)=fps_file;
		}
		if (((*pduration)<DURATION_MIN) && (IsFPSValid(*pfps))) {
			(*pduration)=nbframes/(*pfps);
			strcat(comment,", duration estimated");
		}
		if (IsDateValid(*pend_time)) {
			(*pstart_time)=(*pend_time)-(*pduration)/ONE_DAY_SEC;
			strcat(comment,", start date estimated");
		} else {
			(*ptimetype)=timetype_file;
			(*pend_time)=end_time_file;
			//if (((*pend_time)>start_time_file) && (((*pend_time)-start_time_file)<ONE_DAY_SEC/2.0)) {
			if (IsDurationValid(((*pend_time) - start_time_file) * ONE_DAY_SEC)) {
				(*pstart_time)=start_time_file;
			} else {
				(*pstart_time)=(*pend_time)-(*pduration)/ONE_DAY_SEC;
				strcat(comment,", start date estimated");
			}
		}
	}
	if (!IsDateValid(*pend_time)) {
		strcpy(comment,"file info");
		pdatation_source->file_info = TRUE;

		if (IsDurationValid(duration_log)) {
			(*pduration)=duration_log;
			strcat(comment,", ");
			strcat(comment,comment2);
		} else if (IsDurationValid(duration_file)) {
			(*pduration)=duration_file;
		}
		if (IsFPSValid(fps_log)) {
			(*pfps)=fps_log;
		} else if (duration_file>0) {
			(*pfps)=fps_file;
		}
		if (((*pduration)<DURATION_MIN) && (IsFPSValid(*pfps))) {
			(*pduration)=nbframes/(*pfps);
			strcat(comment,", duration estimated");
		}		
		if (IsDateValid(*pstart_time)) {
			(*pend_time)=(*pstart_time)+(*pduration)/ONE_DAY_SEC;
			strcat(comment,", end date estimated");
		} else {
			(*ptimetype)=timetype_file;
			(*pstart_time)=start_time_file;
			if (((*pstart_time)<end_time_file) && ((end_time_file-(*pstart_time))<0.5)) {
				(*pend_time)=end_time_file;
			} else {
				(*pend_time)=(*pstart_time)+(*pduration)/ONE_DAY_SEC;
				strcat(comment,", end date estimated");
			}
		}
	}
/********** LT to UT if available **********/	
	//if (!((*ptimetype) == UT) && (abs(timezone) <= 12)) {
	if (((*ptimetype)==LT) && (abs(timezone)<=12)) {
		(*ptimetype)=UT;
		(*pstart_time)=(*pstart_time)-timezone/24.0;
		(*pend_time)=(*pend_time)-timezone/24.0;
	}
/********** Derive LT or UT from file info **********/	
	if ((*ptimetype)==Undefined) {
		//if (fabs(end_time_file-(*pend_time))*24.0<1.0/60.0) {
		if (timetype_file != Undefined) { // new
//			if ((fabs(end_time_file-(*pend_time)) < (0.5 / 24.0)) && (timetype_file != Undefined)) {
			if (fabs(end_time_file-(*pend_time)) < (0.5 / 24.0)) {
				(*ptimetype) = timetype_file;
			}
			else {
				timezone = (int)floor(0.5 + ((end_time_file)-(*pend_time)) * 24.0);
				if (fabs(((end_time_file)-(*pend_time)) * 24.0 - timezone) < 0.5 / 60.0) {
					(*ptimetype) = UT;
				}
			}
		}
	}
/********** No duration => derive end time from file info **********/	
	if ((fabs(((*pend_time)-(*pstart_time))*(ONE_DAY_SEC))<0.1) && (fabs(end_time_file-(*pstart_time))<13.0/24.0) && ((*ptimetype)==UT)) {
		(*pduration)=(end_time_file-floor(0.5+(end_time_file-(*pstart_time))*24)/24.0-(*pstart_time))*ONE_DAY_SEC;
		if (IsDurationValid(*pduration)) (*pend_time)=(*pstart_time)+(*pduration)/ONE_DAY_SEC;
		else (*pduration) = 0;
	}

/********** Calculates fps if necessary **********/	
	if ((!IsFPSValid(*pfps)) && (IsDurationValid(*pduration))) {
		(*pfps)=nbframes/(*pduration);
		strcat(comment,", fps calculated");
	}
/********** Sanity checks/corrections **********/	
	if ((*pduration)<0) {
		(*pduration)=-(*pduration);
		time_tmp=(*pstart_time);
		(*pstart_time)=(*pend_time);
		(*pend_time)=time_tmp;
	}
	if (!IsFPSValid(*pfps)) {
		(*pfps)=0.0;
	}
											if (debug_mode) {
												fprintf(stdout,"dtcGetDatation: FINAL Start    = %f (", (*pstart_time));
												fprint_jd(stdout, (*pstart_time));
												fprintf(stdout,")\n");
												fprintf(stdout,"dtcGetDatation: FINAL End      = %f (", (*pend_time));
												fprint_jd(stdout,(*pend_time));
												fprintf(stdout,")\n");
												fprintf(stdout,"dtcGetDatation: FINAL Time     = ");
												fprint_timetype(stdout,(*ptimetype));
												fprintf(stdout,"\n");
												fprintf(stdout,"dtcGetDatation: FINAL Duration = %lf\n", (*pduration));
												fprintf(stdout,"dtcGetDatation: FINAL fps      = %lf\n\n",(*pfps));
											}
/*	free(pnow_tm);*/
}
/*****************************************************************************************/
/*************************Correct datation after scan ************************************/
/*****************************************************************************************/

void dtcCorrectDatation(DtcCapture *capture, double *pstart_time, double *pend_time, double *pduration, double *pfps, TIME_TYPE *ptimetype, char *comment)
{
	char comment2[MAX_STRING] = { 0 };

/**** Correction of end date/duration if frames invalid ****/
	if (IsDurationValid(*pduration)) {
		switch (capture->type)
		{
			case CAPTURE_SER:
				if (capture->u.sercapture->ValidFrameCount<capture->u.sercapture->FrameCount) {
					(*pduration)=(*pduration)*capture->u.sercapture->ValidFrameCount/capture->u.sercapture->FrameCount;
					(*pend_time)=(*pstart_time)+(*pduration)/ONE_DAY_SEC;
					sprintf(comment2,", %zd frame",capture->u.sercapture->FrameCount-capture->u.sercapture->ValidFrameCount);
					strcat(comment,comment2);
					if ((capture->u.sercapture->FrameCount-capture->u.sercapture->ValidFrameCount)>1) {
						strcat(comment,"s");
					}
					strcat(comment," missing-end time corrected");
				}
				break;
			case CAPTURE_FITS:
				if (capture->u.filecapture->LastValidFileIdx<capture->u.filecapture->LastFileIdx) {
					(*pduration)=(*pduration)*(capture->u.filecapture->LastValidFileIdx-capture->u.filecapture->FirstFileIdx+1)/(capture->u.filecapture->LastFileIdx-capture->u.filecapture->FirstFileIdx+1);
					(*pend_time)=(*pstart_time)+(*pduration)/ONE_DAY_SEC;				
					sprintf(comment2,", %d end frame",capture->u.filecapture->LastFileIdx-capture->u.filecapture->LastValidFileIdx);
					strcat(comment,comment2);
					if ((capture->u.filecapture->LastFileIdx-capture->u.filecapture->LastValidFileIdx)>1) {
						strcat(comment,"s");
					}
					strcat(comment," missing-end time corrected");
				}
				break;
			default: // CAPTURE_CV
				break;
		}
	}	
											if (debug_mode) {
												fprintf(stdout,"dtcGetDatation: Start    = %f (", (*pstart_time));
												fprint_jd(stdout, (*pstart_time));
												fprintf(stdout,")\n");
												fprintf(stdout,"dtcGetDatation: Corr. End = %f (", (*pend_time));
												fprint_jd(stdout,(*pend_time));
												fprintf(stdout,")\n");
												fprintf(stdout,"dtcGetDatation: Time     = ");
												fprint_timetype(stdout,(*ptimetype));
												fprintf(stdout,"\n");
												fprintf(stdout,"dtcGetDatation: Corr.Dur = %lf\n", (*pduration));
												fprintf(stdout,"dtcGetDatation: fps      = %lf\n\n",(*pfps));
											}
}


/*****************************************************************************************/
/*******************Gets datation from file system information****************************/
/*****************************************************************************************/

void dtcGetDatationFromFileInfo(DtcCapture *capture, const char *filename, const int nbframes, double *pstart_time, double *pend_time, double *pDuration, double *pfps)
{
	//time_t	start_time_t;
	//time_t	end_time_t;
	//struct tm *pstart_time_tm;
	//struct tm *pend_time_tm;
	//struct stat videofile_info;
	double	duration_tmp	= 0;
	int		nbframes_opencv	= 0;
	
	nbframes_opencv=0;
	switch (capture->type) {
		case CAPTURE_SER:
			(*pfps)=0.0;
			break;
		case CAPTURE_FITS:
			(*pfps)=0.0;
			break;
		default: // CAPTURE_CV
			(*pfps) = dtcGetCaptureProperty(capture, CV_CAP_PROP_FPS);
			nbframes_opencv = (int) floor(0.5+dtcGetCaptureProperty(capture, CV_CAP_PROP_FRAME_COUNT));
			if ((*pfps) != 0) duration_tmp = nbframes_opencv / (*pfps);
	}
	
	(*pstart_time) =	gregorian_calendar_to_jd(1, 1, 1, 0, 0, 0);
	(*pend_time) =		gregorian_calendar_to_jd(1, 1, 1, 0, 0, 0);
//	if (((!(IGNORE_WJ_DEROTATION) || (InStr(filename, WJ_DEROT_STRING) < 0))) && (!(IGNORE_PIPP) || (InStr(filename, PIPP_STRING) < 0))) {
	if (((InStr(filename, WJ_DEROT_STRING) < 0)) && ((InStr(filename, PIPP_STRING) < 0))) { // WinJupos and PIPP file are generated *after* acquisition
		GetCreatedModifiedTimes(filename, pstart_time, pend_time);
		//stat(filename, &videofile_info);
		//start_time_t=videofile_info.st_ctime;
		//end_time_t=videofile_info.st_mtime;
		//pend_time_tm=localtime(&end_time_t);
		duration_tmp = (double)(((*pend_time) - (*pstart_time))*ONE_DAY_SEC);
	}
	//duration calculation
	if (IsDurationValid(duration_tmp)) { // duration /2 ?
		//duration correct, keeping start and end time
		(*pDuration) = duration_tmp;
	}
	//duration incorrect, estimation from nb of frames and fps
	else {
		if ((*pfps) > FPS_MIN) {
			if (nbframes > nbframes_opencv) {
				(*pDuration) = nbframes / (*pfps);
			}
			else {
				(*pDuration) = nbframes_opencv / (*pfps);
			}
		}
		else {
			(*pDuration) = 0;
		}
		if ((*pstart_time) > (*pend_time)) {
			(*pstart_time) = (*pend_time) - (*pDuration) / ONE_DAY_SEC;
		}
		else {
			(*pend_time) = (*pstart_time) + (*pDuration) / ONE_DAY_SEC;
		}
	}
}


/*****************************************************************************************/
/***************************Gets datation from filename***********************************/
/*****************************************************************************************/

BOOL dtcGetDatationFromFilename(const char *longfilename, double *pstart_time, double *pmid_time, TIME_TYPE *ptimetype, Planet_type *planet)
{
	char tmpline[MAX_STRING]			= { 0 };
	int year;
	int month;
	int day;
	int hour;
	int min;
	double sec;
	char filename[MAX_STRING]			= { 0 };
	char longfilename_lcase[MAX_STRING]	= { 0 };
	char filename_stripped[MAX_STRING]	= { 0 };
	char date_format[MAX_STRING]		= { 0 };
	double JD_init = gregorian_calendar_to_jd(1, 1, 1, 0, 0, 0);
	bool	pipp_file = FALSE;

	(*pstart_time) = JD_init;
	(*pmid_time) = JD_init;
	(*ptimetype) = Undefined;

	// Gets planet name if present
	*planet = Notdefined;
	lcase(longfilename, longfilename_lcase);
	if (InStr(longfilename_lcase, "saturn") >= 0) *planet = Saturn;
	else if (InStr(longfilename_lcase, "jupiter")>= 0) *planet = Jupiter;
	else if (InStr(longfilename_lcase, "jup") >= 0) *planet = Jupiter;
	else if (InStr(longfilename_lcase, "sat") >= 0) *planet = Saturn;
	else if (InStr(longfilename_lcase, "mercur") >= 0) *planet = Mercury;
	else if (InStr(longfilename_lcase, "venus") >= 0) *planet = Venus;
	else if (InStr(longfilename_lcase, "uranus") >= 0) *planet = Uranus;
	else if (InStr(longfilename_lcase, "neptun") >= 0) *planet = Neptun;

	//a\toto.42
	//123456789 - 9-1=8
	if (InRstr(longfilename, "\\") >= 0) mid(longfilename, InRstr(longfilename, "\\") + 1, strlen(longfilename) - InRstr(longfilename, "\\") - 1, filename);
	else strcpy(filename, longfilename);

	if ((InRstr(longfilename, "_UT") >= 0) || (InRstr(longfilename, "-UT") >= 0) || (InRstr(longfilename, "_ut.") >= 0) || (InRstr(longfilename, "-ut.") >= 0)) (*ptimetype) = UT;

	//manages pipp extensions
	if (InStr(filename, PIPP_STRING) >= 0) {
		pipp_file = TRUE;
		strcpy(filename, replace_str(filename, PIPP_STRING, ""));
		//strcpy(filename, replace_str(filename, "pipp_", ""));	in directory namme
		//strcpy(filename, replace_str(filename, "pipp", ""));	exotic
	}

	/* Sharpcap */
	// Capture 2014-11-10T08_39_28.CameraSettings
	//         0   45 78 01 34 67 9
	//         9   5  2  9  6  3    
	strcpy(date_format, "YY-MM-DD hh-mm-ss");
	strcpy(filename_stripped, filename);
	while (strlen(filename_stripped) > (strlen(date_format) + 2)) {
		if ((strcmp(mid(filename_stripped, 4, 1, tmpline), "-") == 0)
			&& (strcmp(mid(filename_stripped, 7, 1, tmpline), "-") == 0)
			&& (strcmp(mid(filename_stripped, 10, 1, tmpline), "T") == 0)
			&& (strcmp(mid(filename_stripped, 13, 1, tmpline), "_") == 0)
			&& (strcmp(mid(filename_stripped, 16, 1, tmpline), "_") == 0)) {

			year = atoi(mid(filename_stripped, 0, 4, tmpline));
			month = atoi(mid(filename_stripped, 5, 2, tmpline));
			day = atoi(mid(filename_stripped, 8, 2, tmpline));
			hour = atoi(mid(filename_stripped, 11, 2, tmpline));
			min = atoi(mid(filename_stripped, 14, 2, tmpline));
			sec = strtod(mid(filename_stripped, 17, 2, tmpline), NULL);
			/*if ((InRstr(filename_stripped, ".") >= 0)
			&& (strlen(filename_stripped) >= (InRstr(filename_stripped, ".") + 19 + (strlen(filename_stripped) - InRstr(filename_stripped, "."))))
			&& (strcmp(mid(filename_stripped, InRstr(filename_stripped, ".") - 3, 1, tmpline), "_") == 0)
			&& (strcmp(mid(filename_stripped, InRstr(filename_stripped, ".") - 6, 1, tmpline), "_") == 0)
			&& (strcmp(mid(filename_stripped, InRstr(filename_stripped, ".") - 9, 1, tmpline), "T") == 0)
			&& (strcmp(mid(filename_stripped, InRstr(filename_stripped, ".") - 12, 1, tmpline), "-") == 0)
			&& (strcmp(mid(filename_stripped, InRstr(filename_stripped, ".") - 15, 1, tmpline), "T") == 0)) {

			year = atoi(mid(filename_stripped, InRstr(filename_stripped, "\\") + 9, 4, tmpline));
			month = atoi(mid(filename_stripped, InRstr(filename_stripped, "\\") + 14, 2, tmpline));
			day = atoi(mid(filename_stripped, InRstr(filename_stripped, "\\") + 17, 2, tmpline));
			hour = atoi(mid(filename_stripped, InRstr(filename_stripped, "\\") + 20, 2, tmpline));
			min = atoi(mid(filename_stripped, InRstr(filename_stripped, "\\") + 23, 2, tmpline));
			sec = strtod(mid(filename_stripped, InRstr(filename_stripped, "\\") + 26, 2, tmpline), NULL);*/
			if (IsDateCorrect(year, month, day, hour, min, sec)) (*pstart_time) = gregorian_calendar_to_jd(year, month, day, hour, min, sec);
			if (!IsDateValid(*pstart_time)) (*pstart_time) = JD_init;
			else return TRUE;
		}
		right(filename_stripped, strlen(filename_stripped) - 1, tmpline);
		strcpy(filename_stripped, tmpline);
	}

	//jupiter_2011_08_11_051456_IR742.ser
	//       01   56 89 12 4 6 8
	strcpy(date_format, "_YYYY_MM_DD_HHMMSS");
	strcpy(filename_stripped, filename);
	while (strlen(filename_stripped) > (strlen(date_format) + 2)) {
		if ((strcmp(mid(filename_stripped, 0, 1, tmpline), "_") == 0)
			&& (strcmp(mid(filename_stripped, 5, 1, tmpline), "_") == 0)
			&& (strcmp(mid(filename_stripped, 8, 1, tmpline), "_") == 0)
			&& (strcmp(mid(filename_stripped, 11, 1, tmpline), "_") == 0)) {

			year = atoi(mid(filename_stripped, 1, 4, tmpline));
			month = atoi(mid(filename_stripped, 6, 2, tmpline));
			day = atoi(mid(filename_stripped, 9, 2, tmpline));
			hour = atoi(mid(filename_stripped, 12, 2, tmpline));
			min = atoi(mid(filename_stripped, 14, 2, tmpline));
			sec = atoi(mid(filename_stripped, 16, 2, tmpline));
			//if (strlen(filename_stripped) >= (18 + 1 + (strlen(filename_stripped) - InRstr(filename_stripped, ".")))) {
			//	sec = strtod(mid(filename_stripped, 16, 2, tmpline), NULL);
			if ((sec < 0) || (sec >= 60.0)) sec = 0;
			//}
			//else sec = 0;
			if (IsDateCorrect(year, month, day, hour, min, sec)) (*pstart_time) = gregorian_calendar_to_jd(year, month, day, hour, min, sec);
			if (!IsDateValid(*pstart_time)) (*pstart_time) = JD_init;
			else return TRUE;
		}
		right(filename_stripped, strlen(filename_stripped) - 1, tmpline);
		strcpy(filename_stripped, tmpline);
	}
	// WinJupos FireCapture/PIPP (mid_time)
	//2016-06-27-2107_1-MD-R.ser
	//0   45 78 01 3 567
	strcpy(date_format, "YYYY-MM-DD-HHMM_S");
	strcpy(date_format, "YYYY-MM-DD-HHMM.S");
	strcpy(filename_stripped, filename);
	while (strlen(filename_stripped) > (strlen(date_format) + 2)) {
		if ((strcmp(mid(filename_stripped, 4, 1, tmpline), "-") == 0)
			&& (strcmp(mid(filename_stripped, 7, 1, tmpline), "-") == 0)
			&& (strcmp(mid(filename_stripped, 10, 1, tmpline), "-") == 0)
			&& ((strcmp(mid(filename_stripped, 15, 1, tmpline), "_") == 0) || (strcmp(mid(filename_stripped, 15, 1, tmpline), ".") == 0))
		//	&& (strcmp(mid(filename_stripped, 17, 1, tmpline), "-") == 0))
			) {

			year = atoi(mid(filename_stripped, 0, 4, tmpline));
			month = atoi(mid(filename_stripped, 5, 2, tmpline));
			day = atoi(mid(filename_stripped, 8, 2, tmpline));
			hour = atoi(mid(filename_stripped, 11, 2, tmpline));
			min = atoi(mid(filename_stripped, 13, 2, tmpline));
			sec = strtod(mid(filename_stripped, 16, 1, tmpline), NULL)*6.0;

			if (IsDateCorrect(year, month, day, hour, min, sec)) (*pmid_time) = gregorian_calendar_to_jd(year, month, day, hour, min, sec);
			if (!IsDateValid(*pmid_time)) (*pmid_time) = JD_init;
			else return TRUE;
		}
		right(filename_stripped, strlen(filename_stripped) - 1, tmpline);
		strcpy(filename_stripped, tmpline);
	}
	// PVOL FireCapture (mid_time)
	//j2019-05-12_14-16-51_MD_Clear.ser
	// 1   56 89 12 45 78 0
	strcpy(date_format, "pYYYY-MM-DD_hh-mm-ss");
	strcpy(date_format, "pYYYY-MM-DD_hh-mm_");
	strcpy(filename_stripped, filename);
	while (strlen(filename_stripped) > (strlen(date_format) + 2)) {
		if ((strcmp(mid(filename_stripped, 5, 1, tmpline), "-") == 0)
			&& (strcmp(mid(filename_stripped, 8, 1, tmpline), "-") == 0)
			&& (strcmp(mid(filename_stripped, 11, 1, tmpline), "_") == 0)
			&& (strcmp(mid(filename_stripped, 14, 1, tmpline), "-") == 0)
			&& ((strcmp(mid(filename_stripped, 17, 1, tmpline), "-") == 0) || (strcmp(mid(filename_stripped, 17, 1, tmpline), "_") == 0))) {

			year = atoi(mid(filename_stripped, 1, 4, tmpline));
			month = atoi(mid(filename_stripped, 6, 2, tmpline));
			day = atoi(mid(filename_stripped, 9, 2, tmpline));
			hour = atoi(mid(filename_stripped, 12, 2, tmpline));
			min = atoi(mid(filename_stripped, 15, 2, tmpline));
			if (strcmp(mid(filename_stripped, 17, 1, tmpline), "-") == 0) sec = strtod(mid(filename_stripped, 18, 2, tmpline), NULL);
			else sec = 0;

			if (IsDateCorrect(year, month, day, hour, min, sec)) (*pmid_time) = gregorian_calendar_to_jd(year, month, day, hour, min, sec);
			if (!IsDateValid(*pmid_time)) (*pmid_time) = JD_init;
			else return TRUE;
		}
		right(filename_stripped, strlen(filename_stripped) - 1, tmpline);
		strcpy(filename_stripped, tmpline);
	}
	//wxAstroCapture			YYYYMMDD_hhmm_ss (from PIPP)
	//                          0   4 6 89 1 34     
	strcpy(date_format, "YYYYMMDD_hhmm_ss");
	strcpy(filename_stripped, filename);
	while (strlen(filename_stripped) > (strlen(date_format) + 2)) {
			if ((strcmp(mid(filename_stripped, 8, 1, tmpline), "_") == 0)
			&& (strcmp(mid(filename_stripped, 13, 1, tmpline), "_") == 0)) {

			year = atoi(mid(filename_stripped, 0, 4, tmpline));
			month = atoi(mid(filename_stripped, 4, 2, tmpline));
			day = atoi(mid(filename_stripped, 6, 2, tmpline));
			hour = atoi(mid(filename_stripped, 9, 2, tmpline));
			min = atoi(mid(filename_stripped, 11, 2, tmpline));
			sec = atoi(mid(filename_stripped, 14, 2, tmpline));

			if (IsDateCorrect(year, month, day, hour, min, sec)) (*pmid_time) = gregorian_calendar_to_jd(year, month, day, hour, min, sec);
			if (!IsDateValid(*pmid_time)) (*pmid_time) = JD_init;
			else return TRUE;
		}
		right(filename_stripped, strlen(filename_stripped) - 1, tmpline);
		strcpy(filename_stripped, tmpline);
	}
	//FireCapture1			YYYYMMDD_hhmmss (from PIPP)
	//                      0   4 6 89 1 3       
	strcpy(date_format, "YYYYMMDD_hhmmss");
	strcpy(filename_stripped, filename);
	while (strlen(filename_stripped) > (strlen(date_format) + 2)) {
		if (strcmp(mid(filename_stripped, 8, 1, tmpline), "_") == 0) {

			year =	atoi(mid(filename_stripped, 0, 4, tmpline));
			month =	atoi(mid(filename_stripped, 4, 2, tmpline));
			day =	atoi(mid(filename_stripped, 6, 2, tmpline));
			hour =	atoi(mid(filename_stripped, 9, 2, tmpline));
			min =	atoi(mid(filename_stripped, 11, 2, tmpline));
			sec =	atoi(mid(filename_stripped, 13, 2, tmpline));

			if (IsDateCorrect(year, month, day, hour, min, sec)) (*pmid_time) = gregorian_calendar_to_jd(year, month, day, hour, min, sec);
			if (!IsDateValid(*pmid_time)) (*pmid_time) = JD_init;
			else return TRUE;
		}
		right(filename_stripped, strlen(filename_stripped) - 1, tmpline);
		strcpy(filename_stripped, tmpline);
	}
	//SharpCap				DD_MM_YYYY hh_mm_ss (from PIPP)
	//                      0 23 56   01 34 67
	strcpy(date_format, "DD_MM_YYYY hh_mm_ss");
	strcpy(filename_stripped, filename);
	while (strlen(filename_stripped) > (strlen(date_format) + 2)) {
		if ((strcmp(mid(filename_stripped, 2, 1, tmpline), "_") == 0)
			&& (strcmp(mid(filename_stripped, 5, 1, tmpline), "_") == 0)
			&& (strcmp(mid(filename_stripped, 10, 1, tmpline), " ") == 0)
			&& (strcmp(mid(filename_stripped, 13, 1, tmpline), "_") == 0)
			&& (strcmp(mid(filename_stripped, 16, 1, tmpline), "_") == 0)) {

			day =	atoi(mid(filename_stripped, 0, 2, tmpline));
			month = atoi(mid(filename_stripped, 3, 2, tmpline));
			year =	atoi(mid(filename_stripped, 6, 4, tmpline));
			hour =	atoi(mid(filename_stripped, 11, 2, tmpline));
			min =	atoi(mid(filename_stripped, 14, 2, tmpline));
			sec =	atoi(mid(filename_stripped, 17, 2, tmpline));

			if (IsDateCorrect(year, month, day, hour, min, sec)) (*pmid_time) = gregorian_calendar_to_jd(year, month, day, hour, min, sec);
			if (!IsDateValid(*pmid_time)) (*pmid_time) = JD_init;
			else return TRUE;
		}
		right(filename_stripped, strlen(filename_stripped) - 1, tmpline);
		strcpy(filename_stripped, tmpline);
	}
	//IC Capture			YY-MM-DD hh-mm-ss (from PIPP)
	//                      0 23 56 89 12 45
	strcpy(date_format, "YY-MM-DD hh-mm-ss");
	strcpy(filename_stripped, filename);
	while (strlen(filename_stripped) > (strlen(date_format) + 2)) {
		if ((strcmp(mid(filename_stripped, 2, 1, tmpline), "-") == 0)
			&& (strcmp(mid(filename_stripped, 5, 1, tmpline), "-") == 0)
			&& (strcmp(mid(filename_stripped, 8, 1, tmpline), " ") == 0)
			&& (strcmp(mid(filename_stripped, 11, 1, tmpline), "-") == 0)
			&& (strcmp(mid(filename_stripped, 14, 1, tmpline), "-") == 0)) {

			year = atoi(mid(filename_stripped, 0, 2, tmpline));
			if (year >= 80)		year += 1900;
			else if (year < 80)	year += 2000;
			month = atoi(mid(filename_stripped, 3, 2, tmpline));
			day = atoi(mid(filename_stripped, 6, 2, tmpline));
			hour = atoi(mid(filename_stripped, 9, 2, tmpline));
			min = atoi(mid(filename_stripped, 12, 2, tmpline));
			sec = atoi(mid(filename_stripped, 15, 2, tmpline));

			if (IsDateCorrect(year, month, day, hour, min, sec)) (*pmid_time) = gregorian_calendar_to_jd(year, month, day, hour, min, sec);
			if (!IsDateValid(*pmid_time)) (*pmid_time) = JD_init;
			else return TRUE;
		}
		right(filename_stripped, strlen(filename_stripped)-1, tmpline);
		strcpy(filename_stripped, tmpline);
	}
	//FireCapture2			DDMMYY_hhmmss (from PIPP)
	//                      0 2 4 67 9 1
	strcpy(date_format, "DDMMYY_hhmmss");
	strcpy(filename_stripped, filename);
	while (strlen(filename_stripped) > (strlen(date_format) + 2)) {
		if (strcmp(mid(filename_stripped, 6, 1, tmpline), "_") == 0) {

			day =	atoi(mid(filename_stripped, 0, 2, tmpline));
			month =	atoi(mid(filename_stripped, 2, 2, tmpline));
			year =	atoi(mid(filename_stripped, 4, 2, tmpline));
			if (year >= 80)		year += 1900;
			else if (year < 80)	year += 2000;
			hour =	atoi(mid(filename_stripped, 7, 2, tmpline));
			min =	atoi(mid(filename_stripped, 9, 2, tmpline));
			sec =	atoi(mid(filename_stripped, 11, 2, tmpline));

			if (IsDateCorrect(year, month, day, hour, min, sec)) (*pmid_time) = gregorian_calendar_to_jd(year, month, day, hour, min, sec);
			if (!IsDateValid(*pmid_time)) (*pmid_time) = JD_init;
			else return TRUE;
		}
		right(filename_stripped, strlen(filename_stripped) - 1, tmpline);
		strcpy(filename_stripped, tmpline);
	}

	return FALSE;
}

/*****************************************************************************************/
/***************Gets datation from acquisition software log files*************************/
/*****************************************************************************************/

int dtcGetInfoDatationFromLogFile(const char *filename, double *jd_start_time_loginfo, double *jd_end_time_loginfo, double *pDuration, double *pfps, long *pnbframes, TIME_TYPE *ptimetype_log, char *comment, Planet_type *planet, char *software, DtcCaptureInfo *pCaptureInfo)
{
	struct stat logfile_info;
	time_t		log_time_t;
	struct tm 	*plog_time_tm;
	int			date1;
	int			date2;
	int			date3;
	int			date4;
	int			year_log;
	int			month_log;
	int			day_log;
	int 		hour_log;
	int 		min_log;
	double 		sec_log;
	double 		jd_log;
	
	char line[MAX_STRING]					= { 0 };
	char value[MAX_STRING]					= { 0 };
	char value2[MAX_STRING]					= { 0 };
	char fieldname[MAX_STRING]				= { 0 };
	char tmpline[MAX_STRING]				= { 0 };
	char tmpline2[MAX_STRING]				= { 0 };
	char logfilename[MAX_STRING]			= { 0 };
	char logfilename_rac[MAX_STRING]		= { 0 };
	char logfilename_dir[MAX_STRING]		= { 0 };
	char logfilename_short[MAX_STRING]		= { 0 };
	char logfilename_tmp[MAX_STRING]		= { 0 };
	char month_letter[4]					= { 0 };
	char software_version_string[MAX_STRING]= { 0 };
	int software_version_x86				= 0;
	double software_version					= 0.0;
	int software_beta						= -1;
	int year								= 0;
	int month								= 0;
	int day									= 0;
	int hour								= 0;
	int min									= 0;
	int pos									= 0;
	double sec								= 0.0;
	int year_tmp							= 0;
	int month_tmp							= 0;
	int day_tmp								= 0;
	int hour_tmp							= 0;
	int min_tmp								= 0;
	double sec_tmp							= 0;
	FILE *logfile;
	int end_time_flag						= 0;
	int year_end							= 0;
	int month_end							= 0;
	int day_end								= 0;
	int hour_end							= 0;
	int min_end								= 0;
	double sec_end							= 0.0;
	int year_mid							= 0;
	int month_mid							= 0;
	int day_mid								= 0;
	int hour_mid							= 0;
	double minsec_mid						= 0.0;
	double jd_mid_time_loginfo				= 0.0;
	int timezone							= 0;
	double JD_init							= gregorian_calendar_to_jd(1, 1, 1, 0, 0, 0);
	char date_value[MAX_STRING]				= { 0 };
	char start_value[MAX_STRING]			= { 0 };
	char end_value[MAX_STRING]				= { 0 };
	struct dirent *pDirent;
	DIR *pDir;

	(*jd_start_time_loginfo)				= JD_init;
	(*jd_end_time_loginfo)					= (*jd_start_time_loginfo);
	(*pDuration)							= 0.0;
	(*pfps)									= 0.0;
	(*ptimetype_log)							= Undefined;
	
	get_fileextension(filename,tmpline,EXT_MAX);
	left(filename,strlen(filename)-strlen(tmpline)-1,logfilename_rac);
	strcpy(logfilename_rac,replace_str(logfilename_rac,PIPP_STRING,""));
	//strcpy(logfilename_rac, replace_str(logfilename_rac, "pipp_", ""));	in directory namme
	//strcpy(logfilename_rac, replace_str(logfilename_rac, "pipp", ""));	exotic

	if (!(strrchr(filename, '\\')==NULL)) {
		left(filename, strlen(filename)-strlen(strrchr(filename, '\\'))+1,logfilename_dir);
		right(logfilename_rac, strlen(logfilename_rac) - strlen(logfilename_dir), logfilename_short);
	}
	else {
		strcpy(logfilename_dir, ".\\");
		strcpy(logfilename_short, logfilename_rac);
	}
										if (debug_mode) {fprintf(stdout, "dtcGetInfoDatationFromLogFile: Len filename %s=%zd\n", filename ,strlen(filename));}
/* Firecapture log */	
	strcpy(logfilename, logfilename_rac);
	strncat(logfilename, ".txt", strlen(".txt"));
	logfile=fopen(logfilename,"r");
										if (debug_mode) {fprintf(stdout, "dtcGetInfoDatationFromLogFile: Testing file %s\n", logfilename);}
/* Sharpcap log */	
	if (logfile==NULL) {
		strcpy(logfilename, logfilename_rac);
		strncat(logfilename, ".CameraSettings", strlen(".CameraSettings"));
		strncat(logfilename, ".txt", strlen(".txt"));
		logfile=fopen(logfilename,"r");
		if (logfile!=NULL) {						/*      Capture 2014-11-10T08_39_28.CameraSettings */
			strcpy(software,"SharpCap");			/*      1       9    4  7  0  3  6 */
			year=atoi(mid(logfilename,InRstr(logfilename,"\\")+9,4,tmpline));
			month=atoi(mid(logfilename,InRstr(logfilename,"\\")+14,2,tmpline));
			day=atoi(mid(logfilename,InRstr(logfilename,"\\")+17,2,tmpline));
			hour=atoi(mid(logfilename,InRstr(logfilename,"\\")+20,2,tmpline));
			min=atoi(mid(logfilename,InRstr(logfilename,"\\")+23,2,tmpline));
			sec=strtod(mid(logfilename,InRstr(logfilename,"\\")+26,2,tmpline),NULL);
			(*jd_start_time_loginfo)=gregorian_calendar_to_jd(year, month, day, hour, min, sec);

			stat(logfilename, &logfile_info);
			log_time_t=logfile_info.st_mtime;
			plog_time_tm=localtime(&log_time_t);
/* 		plog_time_tm->tm_year+1900, plog_time_tm->tm_mon+1, plog_time_tm->tm_mday pstart_time_tm->tm_hour, pstart_time_tm->tm_min, (double) (pstart_time_tm->tm_sec)*/
			year_log=plog_time_tm->tm_year+1900;
			month_log=plog_time_tm->tm_mon+1;
			day_log=plog_time_tm->tm_mday;
			hour_log=plog_time_tm->tm_hour;
			min_log=plog_time_tm->tm_min;
			sec_log=(double) (plog_time_tm->tm_sec);
			jd_log=gregorian_calendar_to_jd(year_log, month_log, day_log, hour_log, min_log, sec_log);

			timezone=abs((int) round((jd_log-(*jd_start_time_loginfo))*24));
			if ((timezone>=1) && (timezone<=12)) { 
				(*ptimetype_log)=UT;
				timezone=0;
			} else {
				(*ptimetype_log)=LT;
				timezone=0;
			}
											if (debug_mode) {fprintf(stdout, "dtcGetInfoDatationFromLogFile: Testing file %s\n", logfilename);}
			} else {
				strcpy(logfilename, logfilename_dir);
				strncat(logfilename, "CameraSettings.txt", strlen("CameraSettings.txt"));
				logfile = fopen(logfilename, "r");
				if (logfile != NULL) {						/*      CameraSettings;txt */
					strcpy(software, "SharpCap");
											if (debug_mode) { fprintf(stdout, "dtcGetInfoDatationFromLogFile: Testing file %s\n", logfilename); }
				} else {
/* Genika log */
/* ASICap log */
					strcpy(logfilename, filename);
					strncat(logfilename, ".txt", strlen(".txt"));
					logfile=fopen(logfilename,"r");
												if (debug_mode) {fprintf(stdout, "dtcGetInfoDatationFromLogFile: Testing file %s\n", logfilename);}
					if (logfile==NULL) {
/* Marc Delcroix's LucamRecorder log */	
						strcpy(logfilename, logfilename_rac);
						strncat(logfilename, "-Ser-Stream_info.Log", strlen("-Ser-Stream_info.Log"));
												if (debug_mode) {fprintf(stdout, "dtcGetInfoDatationFromLogFile: Testing file %s\n", logfilename);}
						logfile=fopen(logfilename,"r");
						if (logfile!=NULL) {
							strcpy(software,"Lucam Recorder");
						} else {
/* LucamRecorder log */	
							strcpy(logfilename, logfilename_rac);
							strncat(logfilename, "-Ser-Stream.Log", strlen("-Ser-Stream.Log"));
												if (debug_mode) {fprintf(stdout, "dtcGetInfoDatationFromLogFile: Testing file %s\n", logfilename);}
							logfile=fopen(logfilename,"r");
							if (logfile!=NULL) {
								strcpy(software,"Lucam Recorder");
							} else {
/* Marc Delcroix's LucamRecorder log fixed name */	
								strcpy(logfilename, logfilename_dir);
								strcat(logfilename, "stream_info.log");
														if (debug_mode) {fprintf(stdout, "dtcGetInfoDatationFromLogFile: Testing file %s\n", logfilename);}
								logfile=fopen(logfilename,"r");
								if (logfile!=NULL) {
									strcpy(software,"Lucam Recorder");
								} else {
/* LucamRecorder log fixed name */	
									strcpy(logfilename, logfilename_dir);
									strcat(logfilename, "Stream.Log");
														if (debug_mode) {fprintf(stdout, "dtcGetInfoDatationFromLogFile: Testing file %s\n", logfilename);}
									logfile=fopen(logfilename,"r");
									if (logfile!=NULL) {
										strcpy(software,"Lucam Recorder");
									} else {
/* PLxCapture log */	
										strcpy(logfilename, logfilename_rac);
										strncat(logfilename, ".plx", strlen(".plx"));
														if (debug_mode) {fprintf(stdout, "dtcGetInfoDatationFromLogFile: Testing file %s\n", logfilename);}
										logfile=fopen(logfilename,"r");
										if (logfile!=NULL) {
											strcpy(software,"PLxCapture");
										} else { /* PLX log filename with info at the end */
											pDir = opendir(logfilename_dir);
											if (pDir == NULL) {
												printf("ERROR in dtcGetInfoDatationFromLogFile: Cannot open directory '%s'\n", logfilename_dir);
												return EXIT_FAILURE;
											}
											while (((pDirent = readdir(pDir)) != NULL) && (logfile == NULL)) {
												get_fileextension(pDirent->d_name, tmpline, EXT_MAX);
												if ((strcmp(tmpline, "plx") == 0) && (strcmp(left(pDirent->d_name, strlen(logfilename_short), logfilename_tmp), logfilename_short) == 0)) {
													strcpy(logfilename, logfilename_dir);
													strcat(logfilename, pDirent->d_name);
													logfile = fopen(logfilename, "r");
												}
											}
											closedir(pDir);
											if (logfile != NULL) {
												strcpy(software, "PLxCapture");
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}
	
	if (logfile==NULL) {	
		return EXIT_FAILURE;
	} else {
		stat(logfilename, &logfile_info);
		log_time_t=logfile_info.st_mtime;
		plog_time_tm=localtime(&log_time_t);
/* 		plog_time_tm->tm_year+1900, plog_time_tm->tm_mon+1, plog_time_tm->tm_mday pstart_time_tm->tm_hour, pstart_time_tm->tm_min, (double) (pstart_time_tm->tm_sec)*/
		year_log=plog_time_tm->tm_year+1900;
		month_log=plog_time_tm->tm_mon+1;
		day_log=plog_time_tm->tm_mday;
		hour_log=plog_time_tm->tm_hour;
		min_log=plog_time_tm->tm_min;
		sec_log=(double) (plog_time_tm->tm_sec);
		jd_log=gregorian_calendar_to_jd(year_log, month_log, day_log, hour_log, min_log, sec_log);

		year=0;
		month=0;
		day=0;
		hour=0;
		min=0;
		sec=0.0;
		(*pDuration)=0.0;
		(*pfps)=0.0;
										if (debug_mode) {fprintf(stdout, "dtcGetInfoDatationFromLogFile: Processing file %s\n", logfilename);}
#pragma warning(suppress: 6324)
		while ((!feof(logfile)) && (strcpy(line, getline_ux_win(logfile)) != NULL)) {
			init_string(fieldname);
			init_string(value);
			init_string(value2);
/* Gets fieldname & value */
			strcpy(line,replace_str(line,"............................"," : "));
			strcpy(line,replace_str(line,"..........................."," : "));
			strcpy(line,replace_str(line,".........................."," : "));
			strcpy(line,replace_str(line,"........................."," : "));
			strcpy(line,replace_str(line,"........................"," : "));
			strcpy(line,replace_str(line,"......................."," : "));
			strcpy(line,replace_str(line,"......................"," : "));
			strcpy(line,replace_str(line,"....................."," : "));
			strcpy(line,replace_str(line,"...................."," : "));
			strcpy(line,replace_str(line,"..................."," : "));
			strcpy(line,replace_str(line,".................."," : "));
			strcpy(line,replace_str(line,"................."," : "));
			strcpy(line,replace_str(line,"................"," : "));
			strcpy(line,replace_str(line,"..............."," : "));
			strcpy(line,replace_str(line,".............."," : "));
			strcpy(line,replace_str(line,"............."," : "));
			strcpy(line,replace_str(line,"............"," : "));
			strcpy(line,replace_str(line,"..........."," : "));
			strcpy(line,replace_str(line,".........."," : "));
			strcpy(line,replace_str(line,"........."," : "));
			strcpy(line,replace_str(line,"........"," : "));
			strcpy(line,replace_str(line,"......."," : "));
			strcpy(line,replace_str(line,"......"," : "));
			strcpy(line,replace_str(line,"....."," : "));
			strcpy(line,replace_str(line,"...."," : "));
			strcpy(line,replace_str(line,"..."," : "));
			strcpy(line,replace_str(line,".."," : "));
			strcpy(line, replace_str(line, " = ", " : "));
			strcpy(line,replace_str(line,"="," : "));

			strcpy(line,replace_str(line,"mS:","mS : "));
										if (debug_mode) {fprintf(stdout, "dtcGetInfoDatationFromLogFile: Line |%s|, ", line);}		
			if (strstr(line," : ") == NULL) {
				strcpy(fieldname,line);
			} else {
				strncpy(fieldname,line,InStr(line," : "));
			}
			while ((fieldname[strlen(fieldname)-1] == ' ') && (strlen(fieldname)>0)) {
				strcpy(fieldname,left(fieldname,strlen(fieldname)-1,tmpline));
			}
			strcat(fieldname,"\0");
										if (debug_mode) {fprintf(stdout, "Field|%s|, ", fieldname);}												

			if (InStr(line," : ")>0) {
				strcpy(value,right(line,strlen(line)-InStr(line," : ")-strlen(" : "),tmpline));
				while ((value[0] == ' ') && (strlen(value)>0)) {
					strcpy(value,right(value,strlen(value)-1,tmpline));
				}
				while ((value[strlen(value)-1] == ' ') && (strlen(value)>0)) {
					strcpy(value,left(value,strlen(value)-1,tmpline));
				}
				strcat(value,"\0");
			}
									if (debug_mode) {fprintf(stdout, "Value|%s|\n", value);}		

/* Test software */
			if (strcmp(software,"") == 0) {
				if (strcmp(fieldname,"Filename")==0) {
					strcpy(software,"Firecapture");				 /* 1234567890123456789012345678901234567890 */
				} else if (strcmp(left(fieldname,13,tmpline),"FireCapture v")==0) { /* FireCapture v2.3 (beta 16) Settings */
					strcpy(software,"Firecapture");
					strcpy(tmpline,fieldname);
					software_version=strtod(mid(fieldname, InStr(fieldname,"FireCapture v")+strlen("FireCapture v"), InStr(fieldname," "), tmpline), NULL);
					if ((InStr(fieldname,"beta ")>0)) {
						software_beta=0;
						if (InStr(fieldname,")")>0) {
							software_beta = atoi(mid(fieldname,InStr(fieldname,"beta ")+strlen("beta "), InStr(fieldname,")")-InStr(fieldname,"beta ")-strlen("beta "), tmpline));
						}
					}
					if (debug_mode) {fprintf(stdout, "Firecapture v%1.1f beta %d\n",software_version, software_beta);}
					init_string(tmpline);
				} else if (strcmp(fieldname,"D�but de la capture")==0) {
					strcpy(software,"Genika");
				} else if (strcmp(fieldname,"___________________________________________________________________________")==0) {
					strcpy(software,"Genika");
				} else if (strncmp(fieldname,"Mod",3)==0) {
					strcpy(software,"Genika");
				} else if (strcmp(fieldname,"***********************  GENIKA ASTRO CAPTURE LOG FILE ************************************")==0) {
					strcpy(software,"Genika");
				}
				else if (strcmp(fieldname, "***********************  GENIKA TRIGGER CAPTURE LOG FILE ************************************") == 0) {
					strcpy(software, "Genika");
				} else if (strncmp(fieldname, "[ZWO", 4) == 0) {
					strcpy(software, "ASICap");
				} else if (strcmp(fieldname,"Date")==0) {
					if (strlen(value)<=10) {
						strcpy(software,"Avi Felopaul");
					} else {
						strcpy(software,"Genicap");
					}
				} else if ((strcmp(fieldname,"Capture start time")==0) || (strcmp(fieldname,"Start time of recording")==0)) {
					strcpy(software,"Lucam Recorder");
				}
											if (debug_mode) {fprintf(stdout, "dtcGetInfoDatationFromLogFile: Software detected %s\n\n", software);}		
			}
/**************************************************************************************************************/
/* FireCapture                                                                                                */
/**************************************************************************************************************/
			if (strcmp(software,"Firecapture")==0) {
/*					rem msgtxt = msgtxt && fieldname & "|" & value & "|"*/
/*	Possible syntax	lgth	sol d	m	y
 	ddMMyy			6		a				(ok)
	MMddyy			6		a				(ok)
	yyMMdd			6		a				(ok)
	
	ddMMyyyy		8		b				(ok)
	ddMMM.yy		8		c		x		(ok)
	dd_MM_yy		8		d				(ok)
	MMddyyyy		8		b				(ok)
	MMM.ddyy		8		x	+	x	+	ok
	MM_dd_yy		8		d				(ok)
	yyyyMMdd		8		b				(ok)
	yyMMM.dd		8		c		x		(ok)
	yy_MM_dd		8		d				(ok)
	
	ddMMM.yyyy		10		x	x	x	x	ok
	dd_MM_yyyy		10		e			x	(ok)
	dd_MMM._yy		10		f		x		(ok)
dd.MM.yyyy
MM.dd.yyyy
	MMM.ddyyyy		10		x	x	+	+	ok
	MM_dd_yyyy		10		e			x	(ok)
	MMM._dd_yy		10		x	+	x	+	ok
	yyyyMMM.dd		10		x	+	x	+	ok
	yyyy_MM_dd		10		x	x	+	+	ok
	yy_MMM._dd		10		f		x		(ok)
	
	dd_MMM._yyyy	12		x	+	x	x	ok
	MMM._dd_yyyy	12		x	x	+	+	ok
	yyyy_MMM._dd 	12 		x	x	x	+ 	ok	*/
				if (strcmp(fieldname, "Profile") == 0) { /* Profile=Jupiter */
					if  (strcmp(value, "Mercury") == 0) *planet = Mercury;
					else if (strcmp(value, "Venus") == 0) *planet = Venus;
					else if (strcmp(value, "Mars") == 0) *planet = Mars;
					else if (strcmp(value, "Jupiter") == 0) *planet = Jupiter;
					else if (strcmp(value, "Saturn") == 0) *planet = Saturn;
					else if (strcmp(value, "Uranus") == 0) *planet = Uranus;
					else if (strcmp(value, "Neptun") == 0) *planet = Neptun;
				}
				if (strcmp(fieldname,"Date")==0)  { 	/* Date=11.03.2011 */
					//(*ptimetype_log)=LT;
					(*ptimetype_log) = Undefined;
					if ((software_version>2.3) || ((software_version==2.3) && ((software_beta>=16) || (software_beta<=0)))) {
						strcpy(date_value,value);
					} else {
						switch (strlen(value)) {
							case 6:
								date1=atoi(mid(value,0,2,tmpline));
								date2=atoi(mid(value,2,2,tmpline));
								date3=atoi(mid(value,4,2,tmpline));				
								if ((date1==(year_log-2000)) && (date2==month_log)) { 					/*	yyMMdd */
									year=2000+date1;
									month=date2;
									day=date3;
								} else if ((date3==(year_log-2000)) && (date1==month_log)) { 			/*	MMddyy	*/
									month=date1;
									day=date2;
									year=2000+date3;
								} else { /*default*/ 													/* 	ddMMyy	*/
									day=date1;
									month=date2;
									year=2000+date3;
								}
								break;
							case 7: /* case with one accent in literal month */
								date1=atoi(mid(value,0,2,tmpline));
								date2=atoi(mid(value,2,2,tmpline));
								date3=atoi(mid(value,4,2,tmpline));				
								date4=atoi(mid(value,6,2,tmpline));				
							
								if (strcmp(mid(value,2,1,tmpline),".")==0) { 						/*	MM.ddyy	*/
									month=month_nb(mid(value,0,2,tmpline));
									day=atoi(mid(value,3,2,tmpline));
									year=2000+atoi(mid(value,5,2,tmpline));
								} else if (strcmp(mid(value,4,1,tmpline),".")==0) {
									month=month_nb(mid(value,2,2,tmpline));
									date1=atoi(mid(value,0,2,tmpline));
									date2=atoi(mid(value,5,2,tmpline));
									if (date1==(year_log-2000)) {									/*	yyMM.dd	*/
										year=2000+date1;
										day=date2;
									} else if (date2==(year_log-2000)) {							/*	ddMM.yy	*/
										year=2000+date2;
										day=date1;
									} else {
										if (debug_mode) {fprintf(stdout, "ERROR in dtcGetInfoDatationFromLogFile: Firecapture date format not detected for value %s\n", value); }
									}
								}
								break;
							case 8:
								date1=atoi(mid(value,0,2,tmpline));
								date2=atoi(mid(value,2,2,tmpline));
								date3=atoi(mid(value,4,2,tmpline));				
								date4=atoi(mid(value,6,2,tmpline));				
								if (strcmp(mid(value,3,1,tmpline),".")==0) { 						/*	MMM.ddyy	*/
									month=month_nb(mid(value,0,3,tmpline));
									day=atoi(mid(value,4,2,tmpline));
									year=2000+atoi(mid(value,6,2,tmpline));
								} else if (strcmp(mid(value,5,1,tmpline),".")==0) {
									month=month_nb(mid(value,2,3,tmpline));
									date1=atoi(mid(value,0,2,tmpline));
									date2=atoi(mid(value,6,2,tmpline));
									if (date1==(year_log-2000)) {									/*	yyMMM.dd	*/
										year=2000+date1;
										day=date2;
									} else if (date2==(year_log-2000)) {							/*	ddMMM.yy	*/
										year=2000+date2;
										day=date1;
									} else {
										if (debug_mode) {fprintf(stdout, "ERROR in dtcGetInfoDatationFromLogFile: Firecapture date format not detected for value %s\n", value); }
									}
								} else if ((strcmp(mid(value,2,1,tmpline),"_")==0) && (strcmp(mid(value,5,1,tmpline),"_")==0)) {
									date1=atoi(mid(value,0,2,tmpline));
									date2=atoi(mid(value,3,2,tmpline));
									date3=atoi(mid(value,6,2,tmpline));				
									if ((date1==(year_log-2000)) && (date2==month_log)) {			/*	yy_MM_dd	*/
										year=2000+date1;
										month=date2;
										day=date3;
									} else if ((date3==(year_log-2000)) && (date1==month_log)) {	/*	MM_dd_yy	*/
										month=date1;
										day=date2;
										year=2000+date3;
									} else { /*default*/											/*	dd_MM_yy	*/
										day=date1;
										month=date2;
										year=2000+date3;
									}
								} else if (((date1*100+date2)==year_log) && (date3==month_log)) {		/*	yyyyMMdd	*/
										year=date1*100+date2;
										month=date3;
										day=date4;
								} else if ((date3*100+date4)==year_log) {
									year=date3*100+date4;
									if (date1==month_log) {									/*	MMddyyyy	*/
										month=date1;
										day=date2;
									} else if (date2==month_log) {									/*	ddMMyyyy	*/
										day=date1;
										month=date2;
									}
								} else {
									if (debug_mode) {fprintf(stdout, "ERROR in dtcGetInfoDatationFromLogFile: Firecapture date format not detected for value %s\n", value); }
								}
								break;
							case 9: /* case with one accent in literal month */
								if (strcmp(mid(value,4,1,tmpline),".")==0) {						/*	ddMM.yyyy	*/
									day=atoi(mid(value,0,2,tmpline));
									month=month_nb(mid(value,2,2,tmpline));
									year=atoi(mid(value,5,4,tmpline));
								} else if (strcmp(mid(value,2,1,tmpline),".")==0) {
									month=month_nb(mid(value,0,2,tmpline));
									if (strcmp(mid(value,3,1,tmpline),"_")==0) {					/*	MM._dd_yy	*/
										day=atoi(mid(value,4,2,tmpline));
										year=2000+atoi(mid(value,7,2,tmpline));								
									} else {														/*	MM.ddyyyy	*/
										day=atoi(mid(value,3,2,tmpline));
										year=atoi(mid(value,5,4,tmpline));
									}
								}  else if (strcmp(mid(value,6,1,tmpline),".")==0) {				/*	yyyyMM.dd	*/
									year=atoi(mid(value,0,4,tmpline));
									month=month_nb(mid(value,4,2,tmpline));
									day=atoi(mid(value,7,2,tmpline));
								} else if (strcmp(mid(value,5,2,tmpline),"._")==0) {							
									month=month_nb(mid(value,3,2,tmpline));
									date1=atoi(mid(value,0,2,tmpline));
									date2=atoi(mid(value,7,2,tmpline));
									if (date1==(year_log-2000)) {											/*	yy_MM._dd	*/
										year=date1+2000;
										day=date2;
									} else if (date2==(year_log-2000)) {									/*	dd_MM._yy	*/
										day=date1;
										year=date2+2000;
									} else {
										if (debug_mode) {fprintf(stdout, "ERROR in dtcGetInfoDatationFromLogFile: Firecapture date format not detected for value %s\n", value); }
									}
								} else {							
									if (debug_mode) {fprintf(stdout, "ERROR in dtcGetInfoDatationFromLogFile: Firecapture date format not detected for value %s\n", value); }
								}
								break;
							case 10:
								if (strcmp(mid(value,5,1,tmpline),".")==0) {
									if (strcmp(mid(value, 2, 1, tmpline), ".") == 0) {			
										date1 = atoi(mid(value, 0, 2, tmpline));
										date2 = atoi(mid(value, 3, 2, tmpline));
										year = atoi(mid(value, 6, 4, tmpline));
										if (date2 <= 12) {											/*	dd.MM.yyyy	*/ /*NEW*/
											day=date1;
											month=date2;
										} else {													/*	MM.dd.yyyy	*/ /*NEW*/
											day=date2;
											month=date1;
										}
									}
									else {															/*	ddMMM.yyyy	*/
										day = atoi(mid(value, 0, 2, tmpline));
										month = month_nb(mid(value, 2, 3, tmpline));
										year = atoi(mid(value, 6, 4, tmpline));
									}
								} else if (strcmp(mid(value,3,1,tmpline),".")==0) {
									month=month_nb(mid(value,0,3,tmpline));
									if (strcmp(mid(value,4,1,tmpline),"_")==0) {					/*	MMM._dd_yy	*/
										day=atoi(mid(value,5,2,tmpline));
										year=2000+atoi(mid(value,8,2,tmpline));								
									} else {														/*	MMM.ddyyyy	*/
										day=atoi(mid(value,4,2,tmpline));
										year=atoi(mid(value,6,4,tmpline));
									}
								}  else if (strcmp(mid(value,7,1,tmpline),".")==0) {				/*	yyyyMMM.dd	*/
									year=atoi(mid(value,0,4,tmpline));
									month=month_nb(mid(value,4,3,tmpline));
									day=atoi(mid(value,8,2,tmpline));
								}  else if (strcmp(mid(value,4,1,tmpline),"_")==0) {				/*	yyyy_MM_dd	*/
									year=atoi(mid(value,0,4,tmpline));
									month=atoi(mid(value,5,2,tmpline));
									day=atoi(mid(value,8,2,tmpline));							
								} else if ((strcmp(mid(value,2,1,tmpline),"_")==0) && (strcmp(mid(value,5,1,tmpline),"_")==0)) {							
									year=atoi(mid(value,6,4,tmpline));
									date1=atoi(mid(value,0,2,tmpline));
									date2=atoi(mid(value,3,2,tmpline));
									if (date1==month_log) {											/*	MM_dd_yyyy	*/
										month=date1;
										day=date2;
									} else if (date2==month_log) {									/*	dd_MM_yyyy	*/
										day=date1;
										month=date2;
									} else {
										if (debug_mode) {fprintf(stdout, "ERROR in dtcGetInfoDatationFromLogFile: Firecapture date format not detected for value %s\n", value); }
									}
								} else if (strcmp(mid(value,6,2,tmpline),"._")==0) {							
									month=month_nb(mid(value,3,3,tmpline));
									date1=atoi(mid(value,0,2,tmpline));
									date2=atoi(mid(value,8,2,tmpline));
									if (date1==(year_log-2000)) {											/*	yy_MMM._dd	*/
										year=date1+2000;
										day=date2;
									} else if (date2==(year_log-2000)) {									/*	dd_MMM._yy	*/
										day=date1;
										year=date2+2000;
									} else {
										if (debug_mode) {fprintf(stdout, "ERROR in dtcGetInfoDatationFromLogFile: Firecapture date format not detected for value %s\n", value); }
									}
								} else {							
									if (debug_mode) {fprintf(stdout, "ERROR in dtcGetInfoDatationFromLogFile: Firecapture date format not detected for value %s\n", value); }
								}
								break;
							case 11: /* case with one accent in literal month */
								if (strcmp(mid(value,5,1,tmpline),".")==0) {						/*	dd_MM._yyyy	*/
									day=atoi(mid(value,0,2,tmpline));
									month=month_nb(mid(value,3,2,tmpline));
									year=atoi(mid(value,7,4,tmpline));
								} else if (strcmp(mid(value,2,1,tmpline),".")==0) {					/*	MM._dd_yyyy	*/
									month=month_nb(mid(value,0,2,tmpline));
									day=atoi(mid(value,4,2,tmpline));
									year=atoi(mid(value,7,4,tmpline));
								} else if (strcmp(mid(value,7,1,tmpline),".")==0) {					/*	yyyy_MM._dd 	*/
									year=atoi(mid(value,0,4,tmpline));
									month=month_nb(mid(value,5,2,tmpline));
									day=atoi(mid(value,9,2,tmpline));
								} else {
									if (debug_mode) {fprintf(stdout, "ERROR in dtcGetInfoDatationFromLogFile: Firecapture date format not detected for value %s\n", value); }
								}
								break;
							case 12:
								if (strcmp(mid(value,6,1,tmpline),".")==0) {						/*	dd_MMM._yyyy	*/
									day=atoi(mid(value,0,2,tmpline));
									month=month_nb(mid(value,3,3,tmpline));
									year=atoi(mid(value,8,4,tmpline));
								} else if (strcmp(mid(value,3,1,tmpline),".")==0) {					/*	MMM._dd_yyyy	*/
									month=month_nb(mid(value,0,3,tmpline));
									day=atoi(mid(value,5,2,tmpline));
									year=atoi(mid(value,8,4,tmpline));
								} else if (strcmp(mid(value,8,1,tmpline),".")==0) {					/*	yyyy_MMM._dd 	*/
									year=atoi(mid(value,0,4,tmpline));
									month=month_nb(mid(value,5,3,tmpline));
									day=atoi(mid(value,10,2,tmpline));
								} else {
									if (debug_mode) {fprintf(stdout, "ERROR in dtcGetInfoDatationFromLogFile: Firecapture date format not detected for value %s\n", value); }
								}
								break;
							default:
								if (debug_mode) {fprintf(stdout, "ERROR in dtcGetInfoDatationFromLogFile: Firecapture date format not detected for value %s\n", value); }
						}
												if (debug_mode) { fprintf(stdout,"dtcGetInfoDatationFromLogFile: y m d h m s|%d %d %d %d %d %f|\n", year, month, day, hour, min,sec); }
					}
/*	Pos. syntax	length
	HHmmss		6
	KKmmss		6		?
	KKmmss a	9
	HH_mm_ss	8
	KK_mm_ss	8		?
	KK_mm_ss a 	11 */
				} else if ((strcmp(fieldname,"Start")==0) || (strcmp(fieldname,"Start(UT)")==0)) { 	/* Start=01:01:47 */
					if (strcmp(fieldname, "Start(UT)") == 0) {
						(*ptimetype_log) = UT;
					}
					if ((software_version>2.3) || ((software_version == 2.3) && ((software_beta >= 16) || (software_beta <= 0)))) {
						strcpy(start_value,value);
					} else {
						if ((strlen(value)==8) || (strlen(value)==11)) {
							hour=atoi(mid(value,0,2,tmpline));
							min=atoi(mid(value,3,2,tmpline));
							sec=strtod(mid(value,6,2,tmpline),NULL);
							if ((strlen(value)==11) && (strcmp(mid(value,9,2,tmpline),"PM")==0) && (hour<12)){
								hour+=12;
							}
						} else { /* Start=170816 */
							hour=atoi(mid(value,0,2,tmpline));
							min=atoi(mid(value,2,2,tmpline));
							sec=strtod(mid(value,4,2,tmpline),NULL);
							if ((strlen(value)==9) && (strcmp(mid(value,7,2,tmpline),"PM")==0) && (hour<12)){
								hour+=12;
							}
						}
						(*jd_start_time_loginfo)=gregorian_calendar_to_jd(year, month, day, hour, min, sec);
												if (debug_mode) { fprintf(stdout,"dtcGetInfoDatationFromLogFile: y m d h m s|%d %d %d %d %d %f|\n", year, month, day, hour, min,sec); }
					}
				} else if ((strcmp(fieldname,"End")==0) || (strcmp(fieldname,"End(UT)")==0)) { 	/* End=01:01:47 */
					end_time_flag = 1;
					if (strcmp(fieldname, "End(UT)") == 0) {
						(*ptimetype_log) = UT;
					}
					if ((software_version>2.3) || ((software_version == 2.3) && ((software_beta >= 16) || (software_beta <= 0)))) {
						strcpy(end_value,value);
					} else {
						day_end=day;
						month_end=month;
						year_end=year;
						if ((strlen(value)==8) || (strlen(value)==11)) {
							hour_end=atoi(mid(value,0,2,tmpline));
							min_end=atoi(mid(value,3,2,tmpline));
							sec_end=strtod(mid(value,6,2,tmpline),NULL);
							if ((strlen(value)==11) && (strcmp(mid(value,9,2,tmpline),"PM")==0) && (hour_end<12)){
								hour_end+=12;
							}
						} else { /* Start=170816 */
							hour_end=atoi(mid(value,0,2,tmpline));
							min_end=atoi(mid(value,2,2,tmpline));
							sec_end=strtod(mid(value,4,2,tmpline),NULL);
							if ((strlen(value)==9) && (strcmp(mid(value,7,2,tmpline),"PM")==0) && (hour_end<12)){
								hour_end+=12;
							}
						}
						if (hour_end<hour) { /* Day change */
							day_end+=1;
						}
						(*jd_end_time_loginfo)=gregorian_calendar_to_jd(year_end, month_end, day_end, hour_end, min_end, sec_end);
						if (fabs((jd_log-(*jd_end_time_loginfo)-12.0/24.0))*ONE_DAY_SEC<=1.0) { /* = AM/PM not determined */
							(*jd_start_time_loginfo)=(*jd_start_time_loginfo)+12.0/24.0;
							(*jd_end_time_loginfo)=(*jd_end_time_loginfo)+12.0/24.0;
						}
					}
				} else if (strcmp(fieldname,"Date_format")==0) {
					if        (strcmp(value,"ddMMyy")==0) {
						day=atoi(mid(date_value,0,2,tmpline));
						month=atoi(mid(date_value,2,2,tmpline));
						year=atoi(mid(date_value,4,2,tmpline))+2000;				
					} else if (strcmp(value,"MMddyy")==0) {
						month=atoi(mid(date_value,0,2,tmpline));
						day=atoi(mid(date_value,2,2,tmpline));
						year=atoi(mid(date_value,4,2,tmpline))+2000;				
					} else if (strcmp(value,"yyMMdd")==0) {
						year=atoi(mid(date_value,0,2,tmpline))+2000;
						month=atoi(mid(date_value,2,2,tmpline));
						day=atoi(mid(date_value,4,2,tmpline));				
					} else if (strcmp(value,"ddMMyyyy")==0) {
						day=atoi(mid(date_value,0,2,tmpline));
						month=atoi(mid(date_value,2,2,tmpline));
						year=atoi(mid(date_value,4,4,tmpline));				
					} else if (strcmp(value,"ddMMMyy")==0) {
						day=atoi(mid(date_value,0,2,tmpline));
						month=month_nb(mid(date_value,2,strlen(date_value)-strlen(value)+3,tmpline));
						year=atoi(right(date_value,2,tmpline))+2000;
					} else if (strcmp(value,"dd_MM_yy")==0) {
						day=atoi(mid(date_value,0,2,tmpline));
						month=atoi(mid(date_value,3,2,tmpline));
						year=atoi(mid(date_value,6,2,tmpline))+2000;				
					} else if (strcmp(value,"MMddyyyy")==0) {
						month=atoi(mid(date_value,0,2,tmpline));
						day=atoi(mid(date_value,2,2,tmpline));
						year=atoi(mid(date_value,4,4,tmpline));				
					} else if (strcmp(value,"MMMddyy")==0) {
						month = month_nb(mid(date_value, 0, strlen(date_value) - strlen(value) + 3, tmpline));
						day=atoi(mid(date_value,strlen(date_value)-4,2,tmpline));
						year=atoi(right(date_value,2,tmpline))+2000;
					} else if (strcmp(value,"MM_dd_yy")==0) {
						month=atoi(mid(date_value,0,2,tmpline));
						day=atoi(mid(date_value,3,2,tmpline));
						year=atoi(mid(date_value,6,2,tmpline))+2000;				
					} else if (strcmp(value,"yyyyMMdd")==0) {
						year=atoi(mid(date_value,0,4,tmpline));
						month=atoi(mid(date_value,4,2,tmpline));
						day=atoi(mid(date_value,6,2,tmpline));				
					} else if (strcmp(value,"yyMMMdd")==0) {
						year=atoi(mid(date_value,0,2,tmpline))+2000;				
						month = month_nb(mid(date_value, 2, strlen(date_value) - strlen(value) + 3, tmpline));
						day=atoi(right(date_value,2,tmpline));
					} else if (strcmp(value,"yy_MM_dd")==0) {
						year=atoi(mid(date_value,0,2,tmpline))+2000;
						month=atoi(mid(date_value,3,2,tmpline));
						day=atoi(mid(date_value,6,2,tmpline));				
					} else if (strcmp(value,"ddMMMyyyy")==0) {
						day=atoi(mid(date_value,0,2,tmpline));
						month = month_nb(mid(date_value, 2, strlen(date_value) - strlen(value) + 3, tmpline));
						year=atoi(right(date_value,4,tmpline));
					} else if (strcmp(value,"dd_MM_yyyy")==0) {
						day=atoi(mid(date_value,0,2,tmpline));
						month=atoi(mid(date_value,3,2,tmpline));
						year=atoi(mid(date_value,6,4,tmpline));				
					} else if (strcmp(value,"dd_MMM_yy")==0) {
						day=atoi(mid(date_value,0,2,tmpline));
						month = month_nb(mid(date_value, 3, strlen(date_value) - strlen(value) + 3, tmpline));
						year=atoi(right(date_value,2,tmpline))+2000;				
					} else if (strcmp(value,"MMMddyyyy")==0) {
						month = month_nb(mid(date_value, 0, strlen(date_value) - strlen(value) + 3, tmpline));
						day=atoi(mid(date_value,strlen(date_value)-6,2,tmpline));
						year=atoi(right(date_value,4,tmpline));
					} else if (strcmp(value,"MM_dd_yyyy")==0) {
						month=atoi(mid(date_value,0,2,tmpline));
						day=atoi(mid(date_value,3,2,tmpline));
						year=atoi(right(date_value,4,tmpline));
					} else if (strcmp(value,"MMM_dd_yy")==0) {
						month = month_nb(mid(date_value, 0, strlen(date_value) - strlen(value) + 3, tmpline));
						day=atoi(mid(date_value,strlen(date_value)-5,2,tmpline));
						year=atoi(right(date_value,2,tmpline))+2000;
					} else if (strcmp(value,"yyyyMMMdd")==0) {
						year=atoi(mid(date_value,0,4,tmpline));				
						month = month_nb(mid(date_value, 4, strlen(date_value) - strlen(value) + 3, tmpline));
						day=atoi(right(date_value,2,tmpline));
					} else if (strcmp(value,"yyyy_MM_dd")==0) {
						year=atoi(mid(date_value,0,4,tmpline));				
						month=atoi(mid(date_value,5,2,tmpline));
						day=atoi(mid(date_value,8,2,tmpline));
					} else if (strcmp(value,"yy_MMM_dd")==0) {
						year=atoi(mid(date_value,0,2,tmpline))+2000;				
						month = month_nb(mid(date_value, 3, strlen(date_value) - strlen(value) + 3, tmpline));
						day=atoi(right(date_value,2,tmpline));
					} else if (strcmp(value,"dd_MMM_yyyy")==0) {
						day=atoi(mid(date_value,0,2,tmpline));
						month = month_nb(mid(date_value, 3, strlen(date_value) - strlen(value) + 3, tmpline));
						year=atoi(right(date_value,4,tmpline));
					} else if (strcmp(value,"MMM_dd_yyyy")==0) {
						month = month_nb(mid(date_value, 0, strlen(date_value) - strlen(value) + 3, tmpline));
						day=atoi(mid(date_value,strlen(date_value)-7,2,tmpline));
						year=atoi(right(date_value,4,tmpline));
					} else if (strcmp(value,"yyyy_MMM_dd")==0) {
						year=atoi(mid(date_value,0,4,tmpline));				
						month = month_nb(mid(date_value, 5, strlen(date_value) - strlen(value) + 3, tmpline));
						day=atoi(right(date_value,2,tmpline));
					}
					if (end_time_flag == 1) {
						day_end = day;
						month_end = month;
						year_end = year;
					}
				} else if (strcmp(fieldname,"Time_format")==0) {
					if ((strcmp(value,"HHmmss")==0) || (strcmp(value,"KKmmss")==0) || (strcmp(value,"KKmmss a")==0)) {
						hour=atoi(mid(start_value,0,2,tmpline));
						min=atoi(mid(start_value,2,2,tmpline));
						sec=strtod(mid(start_value,4,2,tmpline),NULL);
						hour_end=atoi(mid(end_value,0,2,tmpline));
						min_end=atoi(mid(end_value,2,2,tmpline));
						sec_end=strtod(mid(end_value,4,2,tmpline),NULL);
					} else if ((strcmp(value,"HH_mm_ss")==0) || (strcmp(value,"KK_mm_ss")==0) || (strcmp(value,"KK_mm_ss a")==0)) {
						hour=atoi(mid(start_value,0,2,tmpline));
						min=atoi(mid(start_value,3,2,tmpline));
						sec=strtod(mid(start_value,6,2,tmpline),NULL);
						hour_end=atoi(mid(end_value,0,2,tmpline));
						min_end=atoi(mid(end_value,3,2,tmpline));
						sec_end=strtod(mid(end_value,6,2,tmpline),NULL);
					}
					if (strcmp(value,"KKmmss a")==0) {
						if ((strcmp(mid(start_value,7,2,tmpline),"PM")==0) && (hour<12)) {
							hour+=12;
						}
						if ((strcmp(mid(end_value,7,2,tmpline),"PM")==0) && (hour_end<12)) {
							hour_end+=12;
						}
					} else if (strcmp(value,"KK_mm_ss a")==0) {
						if ((strcmp(mid(start_value,9,2,tmpline),"PM")==0) && (hour<12)) {
							hour+=12;
						}
						if ((strcmp(mid(end_value,9,2,tmpline),"PM")==0) && (hour_end<12)) {
							hour_end+=12;
						}
					}
					sec=sec+strtod(right(start_value,3,tmpline),NULL)/1000.0;
					sec_end=sec_end+strtod(right(end_value,3,tmpline),NULL)/1000.0;
					if (hour_end<hour) { /* Day change */
						day_end+=1;
					}
											if (debug_mode) { fprintf(stdout,"dtcGetInfoDatationFromLogFile: start time y m d h m s|%d %d %d %d %d %f|\n", year, month, day, hour, min,sec); }
											if (debug_mode) { fprintf(stdout,"dtcGetInfoDatationFromLogFile: end time   y m d h m s|%d %d %d %d %d %f|\n", year_end, month_end, day_end, hour_end, min_end, sec_end); }
					(*jd_start_time_loginfo)=gregorian_calendar_to_jd(year, month, day, hour, min, sec);
					(*jd_end_time_loginfo)=gregorian_calendar_to_jd(year_end, month_end, day_end, hour_end, min_end, sec_end);
					if (fabs((jd_log-(*jd_end_time_loginfo)-12.0/24.0))*ONE_DAY_SEC<=1.0) { /* = AM/PM not determined */
						(*jd_start_time_loginfo)=(*jd_start_time_loginfo)+12.0/24.0;
						(*jd_end_time_loginfo)=(*jd_end_time_loginfo)+12.0/24.0;
					}
					(*pDuration)=((*jd_end_time_loginfo)-(*jd_start_time_loginfo))*ONE_DAY_SEC;
											if (debug_mode) { fprintf(stdout,"dtcGetInfoDatationFromLogFile: Duration|%s,%f|\n", value,(*pDuration)); }
					jd_mid_time_loginfo=((*jd_end_time_loginfo)-(*jd_start_time_loginfo))/2;
				} else if (strcmp(fieldname,"LT")==0) { /*LT=UT-1h*/
					if (strlen(value)==2) {
						timezone=0;
						(*ptimetype_log) = UT; //new
					}
					else {
						timezone = atoi(mid(value, 2, strlen(value) - 3, tmpline));
						if ((software_version == 2.3) && (software_beta >= 16)) {
							timezone = -timezone;
						}
						if ((*ptimetype_log) != UT) {
							(*jd_start_time_loginfo) = (*jd_start_time_loginfo) - timezone / 24.0;
							(*jd_end_time_loginfo) = (*jd_end_time_loginfo) - timezone / 24.0;
							(*ptimetype_log) = UT;
						}
					}
				} else if (strcmp(fieldname,"Duration")==0) { 	/* Duration=135s */
					(*pDuration)=strtod(left(value,strlen(value) - 1,tmpline),NULL);
											if (debug_mode) { fprintf(stdout,"dtcGetInfoDatationFromLogFile: Duration|%s,%f|\n", value,(*pDuration)); }
/*					} else if (strcmp(fieldname,"Profile")==0) {
					strcpy(planet,value);*/
				} else if (((strcmp(fieldname,"FPS")==0) || (strcmp(fieldname,"FPS (avg.)")==0)) && ((*pfps)<FPS_MIN)) { 	/* FPS=49 */
					(*pfps)=atoi(value);
											if (debug_mode) { fprintf(stdout,"dtcGetInfoDatationFromLogFile: FPS|%s,%f|\n", value,(*pfps)); }
				}
				else if ((strcmp(fieldname, "Frames captured") == 0)) { 					// Frames captured=29248
					(*pnbframes) = strtol(value, NULL, 10);
				}
				else if (strcmp(left(fieldname, 5,tmpline), "Frame ") == 0) {				//  Frame 1:	UT 160627 210709.643
				}
				else if (strcmp(left(fieldname, 8, tmpline), "Observer") == 0) {			//	Observer=Marc Delcroix
					strcpy((pCaptureInfo->observer), value);
				}
				else if (strcmp(left(fieldname, 8, tmpline), "Location") == 0) {			//	Location=Tournefeuille
					strcpy((pCaptureInfo->location), value);
				}
				else if (strcmp(left(fieldname, 5, tmpline), "Scope") == 0) {				//	Scope=Newton 320mm
					strcpy((pCaptureInfo->scope), value);
				}
				else if (strcmp(left(fieldname, 6, tmpline), "Camera") == 0) {				//	Camera = ZWO ASI290MM 
					strcpy((pCaptureInfo->camera), value);
				}
				else if (strcmp(left(fieldname, 6, tmpline), "Filter") == 0) {				//	Filter=IR685
					strcpy((pCaptureInfo->filter), value);
				}
				else if (strcmp(left(fieldname, 7, tmpline), "Profile") == 0) {				//	Filter=IR685
					strcpy((pCaptureInfo->profile), value);
				}
				else if (strcmp(left(fieldname, 8, tmpline), "Diameter") == 0) {			//	Diameter=15.98"
					(pCaptureInfo->diameter_arcsec) = strtod(replace_str(value, "\"", ""), NULL);
				}
				else if (strcmp(left(fieldname, 9, tmpline), "Magnitude") == 0) {			//	Magnitude=-1.40
					(pCaptureInfo->magnitude) = strtod(value, NULL);
				}
				else if (strcmp(left(fieldname, 2, tmpline), "CM") == 0) {					//	CM=156.9�  (during mid of capture)
					strcpy((pCaptureInfo->centralmeridian), replace_str(value, "  (during mid of capture)", ""));
				}
				else if (strcmp(left(fieldname, 11, tmpline), "FocalLength") == 0) {		//	FocalLength=6300mm (F/19)
					pos = InStr(value, "mm");
					if (pos >0 ) (pCaptureInfo->focallength_mm) = atoi(left(value, pos, tmpline));
					else (pCaptureInfo->focallength_mm) = atoi(value);
				}
				else if (strcmp(left(fieldname, 10, tmpline), "Resolution") == 0) {			//	Resolution=0.10"
					(pCaptureInfo->resolution) = strtod(replace_str(value, "\"", ""), NULL);
				}
				else if (strcmp(left(fieldname, 7, tmpline), "Binning") == 0) {				//	Binning=no
					strcpy((pCaptureInfo->binning), value);
				}
				else if (strcmp(left(fieldname, 10, tmpline), "Bit depth") == 0) {			//	Bit depth=8bit
					(pCaptureInfo->bitdepth) = atoi(replace_str(value, "bit", ""));
				}
				else if (strcmp(left(fieldname, 7, tmpline), "Debayer") == 0) {				//	Debayer=no
					if (strcmp(value, "no") == 0)	(pCaptureInfo->debayer) = False;
					else if (strcmp(value, "yes") == 0) (pCaptureInfo->debayer) = True;
					else (pCaptureInfo->debayer) = NotSet;
				}
				else if (strcmp(left(fieldname, 10, tmpline), "Shutter") == 0) {			//	Shutter=4.100ms
					(pCaptureInfo->exposure_ms) = strtod(replace_str(value, "ms", ""), NULL);
				}
				else if (strcmp(left(fieldname, 4, tmpline), "Gain") == 0) {				//	Gain=250 (41%)
					pos = InStr(value, " (");
					if (pos>0) (pCaptureInfo->gain) = atoi(left(value, pos, tmpline));
					else (pCaptureInfo->gain) = atoi(value);
				}
					else if (strcmp(left(fieldname, 5, tmpline), "Gamma") == 0) {			//	Gamma=70 (off)
					if (InStr(value, "(off)") > 0) (pCaptureInfo->gamma) = -1;
					else (pCaptureInfo->gamma) = atoi(value);
				}
				else if (strcmp(left(fieldname, 12, tmpline), "AutoExposure") == 0) {		//	AutoExposure=off
					if (strcmp(value, "off") == 0)	(pCaptureInfo->autoexposure) = False;
					else if (strcmp(value, "on") == 0) (pCaptureInfo->autoexposure) = True;
					else (pCaptureInfo->autoexposure) = NotSet;
				}
				else if (strcmp(left(fieldname, 12, tmpline), "SoftwareGain") == 0) {		//	SoftwareGain=10 (off)
					if (InStr(value, "(off)") > 0) (pCaptureInfo->softwaregain) = -1;
					else (pCaptureInfo->softwaregain) = atoi(value);
				}
				else if (strcmp(left(fieldname, 9, tmpline), "AutoHisto") == 0) {			//	AutoHisto=75 (off)
					if (InStr(value, "(off)") > 0) (pCaptureInfo->autohisto) = -1;
					else (pCaptureInfo->autohisto) = atoi(value);
				}
				else if (strcmp(left(fieldname, 10, tmpline), "Brightness") == 0) {			//	Brightness=43 (off)
					if (InStr(value, "(off)") > 0) (pCaptureInfo->brightness) = -1;
					else (pCaptureInfo->brightness) = atoi(value);
				}
				else if (strcmp(left(fieldname, 12, tmpline), "AutoGain") == 0) {			//	AutoGain=off
				if (strcmp(value, "off") == 0)	(pCaptureInfo->autogain) = False;
				else if (strcmp(value, "on") == 0) (pCaptureInfo->autogain) = True;
				else (pCaptureInfo->autogain) = NotSet;
				}
				else if (strcmp(left(fieldname, 15, tmpline), "Histogramm(min)") == 0) {	//	Histogramm(min)=0
					(pCaptureInfo->histmin) = atoi(value);
				}
				else if (strcmp(left(fieldname, 15, tmpline), "Histogramm(max)") == 0) {	//	Histogramm(max)=167
					(pCaptureInfo->histmax) = atoi(value);
				}
				else if (strcmp(left(fieldname, 10, tmpline), "Histogramm") == 0) {			//	Histogramm=65%
					(pCaptureInfo->histavg_pc) = atoi(replace_str(value, "%", ""));
				}
				else if (strcmp(left(fieldname, 5, tmpline), "Noise") == 0) {				//	Noise(avg.deviation)=0.80
					(pCaptureInfo->noise) = strtod(left(value, strlen(value) - 1, tmpline), NULL);
				}
				else if (strcmp(left(fieldname, 7, tmpline), "PreFilter") == 0) {			//	PreFilter=none
					strcpy((pCaptureInfo->prefilter), value);
				}
				else if (strcmp(left(fieldname, 18, tmpline), "Sensor temperature") == 0) {	//	Sensor temperature=11.6�C
					if (InStr(value, "F") > 0) {
						(pCaptureInfo->temp_C) = (strtod(replace_str(value, "�F", ""), NULL) - 32.0) / 1.8;
					}
					else if (InStr(value, "C") > 0) {
						(pCaptureInfo->temp_C) = strtod(replace_str(value, "�C", ""), NULL);
					}
					else {
						(pCaptureInfo->temp_C) = -DBL_MAX;
					}
				}
				else if (strcmp(left(fieldname, 6, tmpline), "Target") == 0) {				//	Target=Mars, Date: 201122, Time: 225626 UT, Mag: -1.40, Dia: 15.98, Res: 0.10, Az: 228.00, Alt: 42.14, Phase: 0.94, CM: CM=156.9�, Camera: ZWO ASI290MM, Scope: Newton 320mm, FL: 6300mm, F-ratio: 19, Observer: Marc Delcroix, Location: Tournefeuille, Comment: , Seeing: 
					strcpy((pCaptureInfo->target), replace_str(value, ";", ","));
				}
/**************************************************************************************************************/
/* Genika Astro + Trigger	                                                                                  */
/**************************************************************************************************************/
			} else if (strcmp(software,"Genika")==0) {			
				if ((InStr(fieldname, "Plan�te") >= 0) || (InStr(fieldname, "Planet") >= 0)) { /* Plan�te = Jupiter */
					if (InStr(value, "Jupiter") >=0) *planet = Jupiter;
					else if (InStr(value, "Saturn") >=0) *planet = Saturn;
					else if (InStr(value, "Mercur")>=0) *planet = Mercury;
					else if (InStr(value, "Venus") >=0) *planet = Venus;
					else if (InStr(value, "Mars") >=0) *planet = Mars;
					else if (InStr(value, "Uranus") >=0) *planet = Uranus;
					else if (InStr(value, "Neptun") >=0) *planet = Neptun;
				}
				if (strcmp(right(fieldname, strlen("but de la capture"),tmpline),"but de la capture") == 0) {
					strcpy(fieldname,"D�but de la capture");
				}
				if ((strcmp(right(fieldname, strlen("e de capture (s)"),tmpline),"e de capture (s)") == 0) && (strcmp(left(fieldname,strlen("Dur"),tmpline),"Dur")==0)) {
					strcpy(fieldname,"Dur�e de capture (s)");
				}
				if        (strcmp(fieldname,"Genika Astro 64 bits release")==0) {				/* Genika Astro 64 bits release : 2.3.3.0 */
					strcpy(software_version_string, value);
					software_version_x86=64;
				} else if (strcmp(fieldname,"Genika Astro 32 bits release")==0) {
					strcpy(software_version_string, value);
					software_version_x86=32;
				} else if (strcmp(fieldname,"Genika Trigger 64 bits release")==0) {
					strcpy(software_version_string, "Trig. ");
					strcat(software_version_string, value);
					software_version_x86=64;
				} else if (strcmp(fieldname,"Genika Trigger 32 bits release")==0) {
					strcpy(software_version_string, "Trig. ");
					strcat(software_version_string, value);
					software_version_x86=32;
				} else if ((strcmp(fieldname,"D�but de la capture")==0) || (strcmp(fieldname,"Start Time")==0)) {	/*   1   5   9  2  5  8  1 */
					if (strcmp(mid(value,3,1,tmpline)," ")==0) {											/* : Fri Apr 29 22:47:57 2011 */
						mid(value,4,3,month_letter);
						month=month_nb(month_letter);
						year=atoi(mid(value,20,4,tmpline));
						day=atoi(mid(value,8,2,tmpline));
						hour=atoi(mid(value,11,2,tmpline));
/*									hour=hour - oShell.RegRead("HKEY_LOCAL_MACHINE\System\CurrentControlSet\Control\TimeZoneInformation\ActiveTimeBias")*-1/60 */
						min=atoi(mid(value,14,2,tmpline));
						sec=strtod(mid(value,17,2,tmpline),NULL);
					} else {																				/* 09/11/2013 05:58:55 */
						day=atoi(strtok(value,"/"));
						month=atoi(strtok(NULL,"/"));
						year=atoi(strtok(NULL," "));
						hour=atoi(strtok(NULL,":"));
						min=atoi(strtok(NULL,":"));					
						sec=strtod(strtok(NULL,":"),NULL);
					}
					(*jd_start_time_loginfo)=gregorian_calendar_to_jd(year, month, day, hour, min, sec);
					(*ptimetype_log)=LT;
											if (debug_mode) { fprintf(stdout,"dtcGetInfoDatationFromLogFile: Start time|y m d h m s|%d %d %d %d %d %f|%f JD|\n", year, month, day, hour, min,sec,(*jd_start_time_loginfo)); }
				} else if (strcmp(fieldname,"Start Time local time")==0) {	/*   1   5   9  2  5  8  1 */
					right(value,strlen(value)-InStr(value," : ")-3,value2);
					strcat(value2,"\0");
					month=atoi(strtok(value2,"/"));
					day=atoi(strtok(NULL,"/"));
					year=atoi(strtok(NULL," "));
					hour=atoi(strtok(NULL,":"));
					min=atoi(strtok(NULL,":"));					
					sec=strtod(strtok(NULL,":"),NULL);
					(*jd_start_time_loginfo)=gregorian_calendar_to_jd(year, month, day, hour, min, sec);
											if (debug_mode) { fprintf(stdout,"dtcGetInfoDatationFromLogFile: Start time|y m d h m s|%d %d %d %d %d %f|%f JD|\n", year, month, day, hour, min,sec,(*jd_start_time_loginfo)); }
					if ((strcmp(right(value,2,tmpline),"PM")==0) && (hour<12)) {
						(*jd_start_time_loginfo)=(*jd_start_time_loginfo)+0.5;
					}
					if ((strcmp(right(value, 2, tmpline), "AM") == 0) && (hour==12)) {
						(*jd_start_time_loginfo) = (*jd_start_time_loginfo) - 0.5;
					}
					(*ptimetype_log) = UT;
				} else if (strcmp(fieldname,"Fin de la capture")==0) {			/*   1   5   9  2  5  8  1 */
																				/* : Fri Apr 29 22:47:57 2011 */
					mid(value,4,3,month_letter);
					month_end=month_nb(month_letter);
					year_end=atoi(mid(value,20,4,tmpline));
					day_end=atoi(mid(value,8,2,tmpline));
					hour_end=atoi(mid(value,11,2,tmpline));
					(*ptimetype_log)=LT;
/*									hour=hour - oShell.RegRead("HKEY_LOCAL_MACHINE\System\CurrentControlSet\Control\TimeZoneInformation\ActiveTimeBias")*-1/60 */
					min_end=atoi(mid(value,14,2,tmpline));
					sec_end=strtod(mid(value,17,2,tmpline),NULL);
						(*jd_end_time_loginfo)=gregorian_calendar_to_jd(year_end, month_end, day_end, hour_end, min_end, sec_end);											if (debug_mode) { fprintf(stdout,"dtcGetInfoDatationFromLogFile: y m d h m s|%d %d %d %d %d %f|%f JD|\n", year_end, month_end, day_end, hour_end, min_end,sec_end,(*jd_end_time_loginfo)); }
				} else if (strcmp(fieldname,"End Time local time")==0) {	/*   1   5   9  2  5  8  1 */
					right(value,strlen(value)-InStr(value," : ")-3,value2);
					strcat(value2,"\0");
					month=atoi(strtok(value2,"/"));
					day=atoi(strtok(NULL,"/"));
					year=atoi(strtok(NULL," "));
					hour=atoi(strtok(NULL,":"));
					min=atoi(strtok(NULL,":"));					
					sec=strtod(strtok(NULL,":"),NULL);
					(*jd_end_time_loginfo)=gregorian_calendar_to_jd(year, month, day, hour, min, sec);
											if (debug_mode) { fprintf(stdout,"dtcGetInfoDatationFromLogFile: End time|y m d h m s|%d %d %d %d %d %f|%f JD|\n", year, month, day, hour, min,sec,(*jd_start_time_loginfo)); }
					if ((strcmp(right(value,2,tmpline),"PM")==0) && (hour<12)) {
						(*jd_end_time_loginfo)=(*jd_end_time_loginfo)+0.5;
					}
					if ((strcmp(right(value, 2, tmpline), "AM") == 0) && (hour == 12)) {
						(*jd_end_time_loginfo) = (*jd_end_time_loginfo) - 0.5;
					}
					(*ptimetype_log) = UT;
				}  else if ((strcmp(fieldname,"Instant de milieu de capture (GMT)  YYYY:(D)D:(M)M:(H)H:(M)M.")==0) || (strcmp(fieldname,"SER file mid acquisition time UTC (Winjupos hh:mm.mm)")==0))  {																																														
					if (strcmp(mid(value,4,1,tmpline),":")==0) {	/* 2012:8:4:4:24.2333 */
						year_mid=atoi(strtok(value,":"));
						month_mid=atoi(strtok(NULL,":"));
						day_mid=atoi(strtok(NULL,":"));
						hour_mid=atoi(strtok(NULL,":"));
						minsec_mid=strtod(strtok(NULL,":"),NULL);
					} else {								/* 2013/11/09 04:59.30 */
						year_mid=atoi(strtok(value,"/"));
						month_mid=atoi(strtok(NULL,"/"));
						day_mid=atoi(strtok(NULL," "));
						hour_mid=atoi(strtok(NULL,":"));
						minsec_mid=strtod(strtok(NULL,":"),NULL);					
					}
					jd_mid_time_loginfo=gregorian_calendar_to_jd(year_mid, month_mid, day_mid, hour_mid, 0, minsec_mid*60);
					timezone=(int) floor(0.5+24*((*jd_start_time_loginfo)-jd_mid_time_loginfo));
					(*jd_start_time_loginfo)=(*jd_start_time_loginfo)-timezone/24.0;
					(*jd_end_time_loginfo)=(*jd_end_time_loginfo)-timezone/24.0;
					(*ptimetype_log)=UT;
											if (debug_mode) { fprintf(stdout,"dtcGetInfoDatationFromLogFile: Mid time|y m d h m.m|%d %d %d %d %f|%f JD|\n", year_mid, month_mid, day_mid, hour_mid, minsec_mid,jd_mid_time_loginfo); }
				} else if (strcmp(fieldname,"Dur�e de capture (s)")==0)  {	/* : 181.12 */
					(*pDuration)=strtod(value,NULL);
											if (debug_mode) { fprintf(stdout,"dtcGetInfoDatationFromLogFile: Duration|%s,%f|\n", value,(*pDuration)); }
				} else if (strcmp(fieldname,"Acquisition Length in mS")==0)  {	/* : 181.12 */
					(*pDuration)=strtod(value,NULL)/1000.0;
											if (debug_mode) { fprintf(stdout,"dtcGetInfoDatationFromLogFile: Duration|%s,%f|\n", value,(*pDuration)); }
				} else if ((strcmp(fieldname,"FPS moyenne")==0) || (strcmp(fieldname,"Mean FPS")==0) || (strcmp(fieldname,"resulting FPS")==0)) { 	
					if (((*pfps)==0) && (strtod(value,NULL)>0)) {
						(*pfps)=strtod(value,NULL);
												if (debug_mode) { fprintf(stdout,"dtcGetInfoDatationFromLogFile: FPS|%s,%f|\n", value,(*pfps)); }
					}
				}
/**************************************************************************************************************/
/* Avi felopaul                                                                                               */
/**************************************************************************************************************/
			} else if (strcmp(software,"Avi Felopaul")==0) {			
				if (strcmp(fieldname,"Date")==0) {				/*   	1   5   9  2  5  8  1 */
																/* Date :	2011/04/16 */
					year=atoi(mid(value,0,4,tmpline));
					month=atoi(mid(value,5,2,tmpline));
					day=atoi(mid(value,8,2,tmpline));
				} else if (strcmp(fieldname,"Time")==0) { 	/* 				Time : 	23:08:42 (UHT)  01:08:42 (LOC) */
					hour=atoi(mid(value,0,2,tmpline));
					min=atoi(mid(value,3,2,tmpline));
					sec=strtod(mid(value,6,2,tmpline),NULL);
					(*ptimetype_log)=UT;
				}
				(*jd_start_time_loginfo)=gregorian_calendar_to_jd(year, month, day, hour, min, sec);
/**************************************************************************************************************/
/* Genicap                                                                                                    */
/**************************************************************************************************************/
			} else if (strcmp(software,"Genicap")==0) {			
				if (strcmp(fieldname,"Date")==0) {				/*   	1   5   9  2  5  8  1 */
																/* Date :	2011_02_11_185354 */
					year=atoi(mid(value,0,4,tmpline));
					month=atoi(mid(value,5,2,tmpline));
					day=atoi(mid(value,8,2,tmpline));
					hour=atoi(mid(value,11,2,tmpline));
					min=atoi(mid(value,13,2,tmpline));
					sec=strtod(mid(value,7,2,tmpline),NULL);
					(*ptimetype_log)=LT;
				}
				(*jd_start_time_loginfo)=gregorian_calendar_to_jd(year, month, day, hour, min, sec);
/**************************************************************************************************************/
/* Lucam Recorder                                                                                             */
/**************************************************************************************************************/
			} else if (strcmp(software,"Lucam Recorder")==0) {
				if ((strcmp(fieldname,"Capture start time")==0) || (strcmp(fieldname,"Start time of recording")==0)) {			/*   1   5   9  2  5  8  1 */
																																/*   2007-06-22, 22:46:42 UTC */
																																/*   2009-07-27, 02:41:12 = UTC */
																																/*   2009-08-15, 22:42:58 / UTC */
																															/*		2007-10-12, 19:50:44 = UTC +2 hours */
																																/* : Tuesday, 07 August 2012 03:45:27 / UTC */
																																/* : Saturday, 21 August 2010 05:55:10 / UTC +2 Hours */
					strcpy(value,replace_str(value," / "," "));		/* same UTC */
					strcpy(value,replace_str(value,"  :  "," "));

					if ((!strcmp(right(value,3,tmpline),"UTC") == 0) &&  (!strcmp(right(lcase(value, tmpline2),5,tmpline),"hours") == 0)) {
						(*ptimetype_log)=LT;
					} else {
						(*ptimetype_log) = UT;
						left(value, InStr(value, "UTC"), value);
					}
					if (strlen(value)>26) {
						strcpy(value,replace_str(value,"Monday, ",""));
						strcpy(value,replace_str(value,"Tuesday, ",""));
						strcpy(value,replace_str(value,"Wednesday, ",""));
						strcpy(value,replace_str(value,"Thursday, ",""));
						strcpy(value,replace_str(value,"Friday, ",""));
						strcpy(value,replace_str(value,"Saturday, ",""));
						strcpy(value,replace_str(value,"Sunday, ",""));
						strcpy(value,replace_str(value,"January","Jan"));
						strcpy(value,replace_str(value,"February","Feb"));
						strcpy(value,replace_str(value,"March","Mar"));
						strcpy(value,replace_str(value,"April","Apr"));
						strcpy(value,replace_str(value,"May","May"));
						strcpy(value,replace_str(value,"June","Jun"));
						strcpy(value,replace_str(value,"July","Jul"));
						strcpy(value,replace_str(value,"August","Aug"));
						strcpy(value,replace_str(value,"September","Sep"));
						strcpy(value,replace_str(value,"October","Oct"));
						strcpy(value,replace_str(value,"November","Nov"));
						strcpy(value,replace_str(value,"December","Dec"));
						mid(value,3,3,month_letter);
						month=month_nb(month_letter);
						year=atoi(mid(value,7,4,tmpline));
						day=atoi(mid(value,0,2,tmpline));
						hour=atoi(mid(value,12,2,tmpline));
						min=atoi(mid(value,15,2,tmpline));
						sec=strtod(mid(value,18,2,tmpline),NULL);
					} else { /* 2007-06-22, 22:46:42 */
						year=atoi(mid(value,0,4,tmpline));
						month=atoi(mid(value,5,4,tmpline));
						day=atoi(mid(value,8,2,tmpline));
						hour=atoi(mid(value,12,2,tmpline));
						min=atoi(mid(value,15,2,tmpline));					
						sec=strtod(mid(value,18,2,tmpline),NULL);
					}
//					if ((!strcmp(right(value,3,tmpline),"UTC") == 0) &&  (!strcmp(right(value,5,tmpline),"Hours") == 0)) {
//						(*ptimetype_log)=LT;
/*										hour=hour - oShell.RegRead("HKEY_LOCAL_MACHINE\System\CurrentControlSet\Control\TimeZoneInformation\ActiveTimeBias")*-1/60 */
//					} else {
//						(*ptimetype_log)=UT;
//					}
					(*jd_start_time_loginfo)=gregorian_calendar_to_jd(year, month, day, hour, min, sec);
											if (debug_mode) { fprintf(stdout,"dtcGetInfoDatationFromLogFile: y m d h m s|%d %d %d %d %d %f|\n", year, month, day, hour, min,sec); }
				} else if ((strcmp(fieldname,"Capture duration")==0) ||(strcmp(fieldname,"Recording duration")==0)) { 	/* : 181.12 */
					strcpy(value,replace_str(value, " Sec", ""));
					strcpy(value,replace_str(value, " sec", ""));
					strcpy(value,replace_str(value, "s", ""));
					strcpy(value,replace_str(value, ",", "."));
					(*pDuration)=strtod(value,NULL);
											if (debug_mode) { fprintf(stdout,"dtcGetInfoDatationFromLogFile: Duration|%s,%f|\n", value,(*pDuration)); }
				} else if ((strcmp(fieldname,"Capture frame speed")==0) || (strcmp(fieldname,"Camera stream rate")==0)) { 	/* : 1.4 */
					strcpy(value,replace_str(value, " Fps", ""));
					strcpy(value,replace_str(value, " fps", ""));
					(*pfps)=strtod(value,NULL);
											if (debug_mode) { fprintf(stdout,"dtcGetInfoDatationFromLogFile: FPS|%s,%f|\n", value,(*pfps)); }
				}
/**************************************************************************************************************/
/* PLxCapture                                                                                                 */
/**************************************************************************************************************/
			} else if (strcmp(software,"PLxCapture")==0) {			
				if        (strcmp(fieldname,"BeginRec")==0) {				/*   	1   5   9  2  5  8  1 */
																			/* Date :	2012-03-15T23:04:58.283 */
					year=atoi(mid(value,0,4,tmpline));
					month=atoi(mid(value,5,2,tmpline));
					day=atoi(mid(value,8,2,tmpline));
					hour=atoi(mid(value,11,2,tmpline));
					min=atoi(mid(value,14,2,tmpline));
					sec=strtod(mid(value,17,6,tmpline),NULL);
					if (strcmp(right(value,1,tmpline),"Z") == 0) (*ptimetype_log) = UT;
					else (*ptimetype_log)=LT;
					(*jd_start_time_loginfo)=gregorian_calendar_to_jd(year, month, day, hour, min, sec);
																/*      012345678901234567*/
																/* JLRGB20121031-010133061.plx*/				
		/* Gets UT from Filename */
					right(logfilename_rac,18,logfilename_tmp);
											if (debug_mode) { fprintf(stdout,"dtcGetInfoDatationFromLogFile: filename|%s|\n", logfilename_tmp); }
					year_tmp=atoi(mid(logfilename_tmp,0,4,tmpline));
					month_tmp=atoi(mid(logfilename_tmp,4,2,tmpline));
					day_tmp=atoi(mid(logfilename_tmp,6,2,tmpline));
					hour_tmp=atoi(mid(logfilename_tmp,9,2,tmpline));
					min_tmp=atoi(mid(logfilename_tmp,11,2,tmpline));
					sec_tmp=strtod(mid(logfilename_tmp,13,2,tmpline),NULL);
					timezone=(int) floor(0.5+((*jd_start_time_loginfo)-gregorian_calendar_to_jd(year_tmp, month_tmp, day_tmp, hour_tmp, min_tmp, sec_tmp))*24);
											if (debug_mode) { fprintf(stdout,"dtcGetInfoDatationFromLogFile: ymdhms=%d.%d.%d.%d.%d.%f timzeone=%d|\n", year_tmp,month_tmp,day_tmp,hour_tmp,min_tmp,sec_tmp,timezone); }
					if ((timezone>=-12) && (timezone<=12)) {
						(*ptimetype_log)=UT;
						(*jd_start_time_loginfo)-=timezone/24.0;
					} else {
						timezone=0;
					}
				} else if (strcmp(fieldname,"EndRec")==0) { 	/* 				Time : 	23:08:42 (UHT)  01:08:42 (LOC) */
					year_end=atoi(mid(value,0,4,tmpline));
					month_end=atoi(mid(value,5,2,tmpline));
					day_end=atoi(mid(value,8,2,tmpline));
					hour_end=atoi(mid(value,11,2,tmpline));
					min_end=atoi(mid(value,14,2,tmpline));
					sec_end=strtod(mid(value,17,6,tmpline),NULL);
/*					(*ptimetype_log)=LT;*/
					(*jd_end_time_loginfo)=gregorian_calendar_to_jd(year_end, month_end, day_end, hour_end, min_end, sec_end)-timezone/24.0;
				} else if (strcmp(fieldname,"Origin")==0) {		/*				Origin=PlxCapture 2.2.3.49 */
					if (strlen(value)>11) {
						right(value,strlen(value)-11,software_version_string);
					}
				}
/**************************************************************************************************************/
/* SharpCap                                                                                                   */
/**************************************************************************************************************/
			} else if (strcmp(software,"SharpCap")==0) {
				strcat(fieldname, "\0");
																																				//	0123456789012345678901234																				
				if ((strcmp(fieldname, "TimeStamp") == 0) || (strcmp(fieldname, "StartCapture") == 0)) {	//	2021-07-30T17:52:23.1234Z
					year = atoi(mid(value, 0, 4, tmpline));
					month = atoi(mid(value, 5, 2, tmpline));
					day = atoi(mid(value, 8, 2, tmpline));
					hour = atoi(mid(value, 11, 2, tmpline));
					min = atoi(mid(value, 14, 2, tmpline));
					sec = strtod(mid(value, 17, strlen(value)-17+1, tmpline), NULL);
					if (strcmp(right(value, 1,tmpline), "Z") == 0) (*ptimetype_log) = UT;
					else (*ptimetype_log) = LT;
					(*jd_start_time_loginfo) = gregorian_calendar_to_jd(year, month, day, hour, min, sec);
				}
				else if (strcmp(fieldname, "EndCapture") == 0) {																//	2021-07-30T17:52:23.1234Z
					year = atoi(mid(value, 0, 4, tmpline));
					month = atoi(mid(value, 5, 2, tmpline));
					day = atoi(mid(value, 8, 2, tmpline));
					hour = atoi(mid(value, 11, 2, tmpline));
					min = atoi(mid(value, 14, 2, tmpline));
					sec = strtod(mid(value, 17, strlen(value)-17+1, tmpline), NULL);
					if (strcmp(right(value, 1, tmpline), "Z") == 0) (*ptimetype_log) = UT;
					else (*ptimetype_log) = LT;
					(*jd_end_time_loginfo) = gregorian_calendar_to_jd(year, month, day, hour, min, sec);
				}
				else if ((strcmp(fieldname, "FrameCount") == 0)) { 					// Frames captured=29248
					(*pnbframes) = strtol(value, NULL, 10);
				}
				else if (strcmp(fieldname, "SharpCapVersion") == 0) {		// SharpCapVersion=3.2.6482.0
					software_version = strtod(left(value, 3, tmpline), NULL);
					if (debug_mode) { fprintf(stdout, "SharpCap v%1.1f\n", software_version); }
				} else if ((strncmp(fieldname, "[", 1) == 0) && (strcmp(right(fieldname, 1, tmpline), "]") == 0)) {	//	[ZWO ASI120MM-S]
					strcpy((pCaptureInfo->camera), right(left(fieldname, strlen(fieldname) - 1, tmpline), strlen(fieldname) - 2, tmpline2));
				}
				else if (strcmp(fieldname, "Binning") == 0) {					//	Binning=1
					if (strcmp(value,"1") ==0) strcpy((pCaptureInfo->binning), "1x1");
					else if (strcmp(value, "2") == 0) strcpy((pCaptureInfo->binning), "2x2");
					else strcpy((pCaptureInfo->binning), value);
				}
				else if ((strcmp(fieldname, "Colour Space") == 0) || (strcmp(fieldname, "ColourSpace") == 0)) {			//	Colour Space=MONO8, MONO16, RAW8, RAW16, RGB24
					if (strcmp(right(value, 2, tmpline), "16") == 0) (pCaptureInfo->bitdepth) = 16;
					else if (strcmp(right(value, 1, tmpline), "8") == 0) (pCaptureInfo->bitdepth) = 8;
					else if (strcmp(value, "RGB24") == 0) {
						(pCaptureInfo->bitdepth) = 9;
						(pCaptureInfo->debayer) = True;
					}
					if (strncmp(value, "RAW", 3) == 0) (pCaptureInfo->debayer) = False;
				}
				else if ((strcmp(fieldname, "Sensor Temp") == 0) || (strcmp(fieldname, "Temperature") == 0)) {			//	Sensor Temp=22,25
					(pCaptureInfo->temp_C) = strtod(replace_str(value, ",", "."), NULL);
				}
				else if (strcmp(fieldname, "Brightness") == 0) {				//	Brightness=0
					(pCaptureInfo->brightness) = atoi(value);
				}
				else if (strcmp(fieldname, "Gain") == 0) {					//	Gain=100
					(pCaptureInfo->gain) = atoi(value);
				}
				else if (strcmp(fieldname, "Gamma") == 0) {					//	Gamma=70 (off)
					(pCaptureInfo->gamma) = atoi(value);
				}
				else if (strcmp(fieldname, "Exposure") == 0) {			//	Shutter=4.100ms
					if ((InStr(value, "us") > 0) || (InStr(fieldname, "us") > 0))		(pCaptureInfo->exposure_ms) = strtod(replace_str(replace_str(value, ",", "."), "us", ""), NULL) / 1000.0;
					else if ((InStr(value, "ms") > 0) || (InStr(fieldname, "ms") > 0))	(pCaptureInfo->exposure_ms) = strtod(replace_str(replace_str(value, ",", "."), "ms", ""), NULL);
					else																(pCaptureInfo->exposure_ms) = strtod(replace_str(replace_str(value, ",", "."), "s", ""), NULL) * 1000.0;
				}
			}
/**************************************************************************************************************/
/* ASICap                                                                                                   */
/**************************************************************************************************************/

			else if (strcmp(software, "ASICap") == 0) {
				if ((strncmp(fieldname, "[", 1) == 0) && (strcmp(right(fieldname, 1, tmpline), "]") == 0)) {	//	[ZWO ASI120MM-S]
					strcpy((pCaptureInfo->camera), right(left(fieldname, strlen(fieldname) - 1, tmpline), strlen(fieldname) - 2, tmpline2));
				}
				else if (strcmp(fieldname, "StartCapture") == 0) {	//	2021-07-30T17:52:23.1234Z
					year = atoi(mid(value, 0, 4, tmpline));
					month = atoi(mid(value, 5, 2, tmpline));
					day = atoi(mid(value, 8, 2, tmpline));
					hour = atoi(mid(value, 11, 2, tmpline));
					min = atoi(mid(value, 14, 2, tmpline));
					sec = strtod(mid(value, 17, strlen(value) - 17 + 1, tmpline), NULL);
					(*ptimetype_log) = UT;
					(*jd_start_time_loginfo) = gregorian_calendar_to_jd(year, month, day, hour, min, sec);
				}
				else if (strcmp(fieldname, "EndCapture") == 0) {																//	2021-07-30T17:52:23.1234Z
					year = atoi(mid(value, 0, 4, tmpline));
					month = atoi(mid(value, 5, 2, tmpline));
					day = atoi(mid(value, 8, 2, tmpline));
					hour = atoi(mid(value, 11, 2, tmpline));
					min = atoi(mid(value, 14, 2, tmpline));
					sec = strtod(mid(value, 17, strlen(value) - 17 + 1, tmpline), NULL);
					(*ptimetype_log) = UT;
					(*jd_end_time_loginfo) = gregorian_calendar_to_jd(year, month, day, hour, min, sec);
				}
				else if ((strcmp(fieldname, "FrameCount") == 0)) { 					// Frames captured=29248
					(*pnbframes) = strtol(value, NULL, 10);
				}
				else if (strcmp(fieldname, "Bin") == 0) {					//	Binning=1
					if (strcmp(value, "1") == 0) strcpy((pCaptureInfo->binning), "1x1");
					else if (strcmp(value, "2") == 0) strcpy((pCaptureInfo->binning), "2x2");
					else strcpy((pCaptureInfo->binning), value);
				}
				else if ((strcmp(fieldname, "Colour Format") == 0)) {			//	Colour Space=MONO8, MONO16, RAW8, RAW16, RGB24
					if (strcmp(right(value, 2, tmpline), "16") == 0) (pCaptureInfo->bitdepth) = 16;
					else if (strcmp(right(value, 1, tmpline), "8") == 0) (pCaptureInfo->bitdepth) = 8;
					else if (strcmp(value, "RGB24") == 0) {
						(pCaptureInfo->bitdepth) = 9;
						(pCaptureInfo->debayer) = True;
					}
					if (strcmp(value, "Raw Format") == 0) if (strcmp(value, "ON") == 0) (pCaptureInfo->debayer) = False;
				}
				else if ((strcmp(fieldname, "Sensor Temp") == 0) || (strcmp(fieldname, "Temperature") == 0)) {			//	Sensor Temp=22,25
					(pCaptureInfo->temp_C) = strtod(replace_str(replace_str(value, " ",""), ",", "."), NULL);
				}
				else if (strcmp(fieldname, "Brightness") == 0) {				//	Brightness=0
					(pCaptureInfo->brightness) = atoi(value);
				}
				else if (strcmp(fieldname, "Gain") == 0) {					//	Gain=100
					(pCaptureInfo->gain) = atoi(value);
				}
				else if (strcmp(fieldname, "Gamma") == 0) {					//	Gamma=70 (off)
					(pCaptureInfo->gamma) = atoi(value);
				}
				else if (strcmp(fieldname, "Exposure") == 0) {			//	Shutter=4.100ms
					if		((InStr(value, "us") > 0) || (InStr(fieldname, "us") > 0))	(pCaptureInfo->exposure_ms) = strtod(replace_str(replace_str(value, ",", "."), "us", ""), NULL) / 1000.0;
					else if ((InStr(value, "ms") > 0) || (InStr(fieldname, "ms") > 0))	(pCaptureInfo->exposure_ms) = strtod(replace_str(replace_str(value, ",", "."), "ms", ""), NULL);
					else																(pCaptureInfo->exposure_ms) = strtod(replace_str(replace_str(value, ",", "."), "s", ""), NULL) * 1000.0;
					
				}
			}
		}
/**************************************************************************************************************/
/* End                                                                                                        */
/**************************************************************************************************************/
		if (fclose(logfile)!=0) {
			 char msgtext[MAX_STRING] = { 0 };										
			snprintf(msgtext, MAX_STRING, "cannot close file %s", logfilename);
			Warning(WARNING_MESSAGE_BOX, "cannot close file", "dtcGetInfoDatationFromLogFile()", msgtext);
			//exit(EXIT_FAILURE);
		}
		if ((*jd_end_time_loginfo)<(JD_init+1) && ((*jd_start_time_loginfo)>(JD_init+1)) && (IsDurationValid(*pDuration))) {
			(*jd_end_time_loginfo)=(*jd_start_time_loginfo)+(*pDuration)/ONE_DAY_SEC;
		} else if ((*jd_start_time_loginfo)<(JD_init+1) && ((*jd_end_time_loginfo)>(JD_init+1)) && (IsDurationValid(*pDuration))) {
			(*jd_start_time_loginfo)=(*jd_end_time_loginfo)-(*pDuration)/ONE_DAY_SEC;
		}
		
		strcpy(comment,software);
		if (software_version>0) {
			sprintf(tmpline," %1.1f",software_version);
			strcat(comment,tmpline);
			strcat(software, tmpline);
		}
		if (software_beta==0) {
			strcat(comment,"beta");
			strcat(software, tmpline);
		}
		if (software_beta>0) {
			sprintf(tmpline,"b%d",software_beta);
			strcat(comment,tmpline);
			strcat(software, tmpline);
		}
		if (strlen(software_version_string)>0) {
			sprintf(tmpline," %s",software_version_string);
			strcat(comment,tmpline);
			strcat(software, tmpline);
		}
		if (software_version_x86>0) {
			sprintf(tmpline," %2db",software_version_x86);
			strcat(comment,tmpline);
			strcat(software, tmpline);
		}

		return EXIT_SUCCESS;
	}
}

/*****************************************************************************************/
/******************************Accessory functions****************************************/
/*****************************************************************************************/

BOOL IsDateCorrect(int y, int m, int d, int hour, int min, double sec) {
	int month_nbdays[] = {0,31,29,31,30,31,30,31,31,30,31,30,31};

	if (y < 0) return FALSE;
	if ((m < 1) || (m > 12)) return FALSE;
	if ((d < 1) || (d > month_nbdays[m])) return FALSE;
	//in case of non leap year
	if ((m==2) && ((y/4 != (int) (y/4)) || (y / 400 == (int)(y / 400)))) if (d > (month_nbdays[m]-1)) return FALSE;
	if ((hour < 0) || (hour > 23)) return FALSE;
	if ((min < 0) || (min > 59)) return FALSE;
	if ((sec < 0.0) || (sec >= 60.0)) return FALSE;
	return TRUE;
}

/*****************Calendar to Julian Day***************************/
double gregorian_calendar_to_jd(int y, int m, int d, int hour, int min, double sec)
{
	double jj;
	
	y+=8000;
	if(m<3) { y--; m+=12; }
	
	jj=((sec/60+min)/60+(hour-12))/24;
	
	jj+=(y*365) +(double)(y/4) -(double)(y/100) +(double)(y/400) -1200820+(m*153+3)/5-92+d-1;
	
	return jj;
}

/*****************Julian Day to calendar***************************/
void jd_to_date(double jj, double *psec, int *pmin, int *phour, int *pday, int *pmonth, int *pyear)
{
	  int z;
	  int a;
	  int a2;
	  int b;
	  int c;
	  int d;
	  int e;
	  double f;
	  double jourhms;
	  double heurems;
	  double mins;
  	  double fracsec;
/*	  int precision;
	  
	  precision=1;*/
	  (*psec)=0;
	  (*pmin)=0;
	  (*phour)=0;
	  (*pday)=0;
	  (*pmonth)=0;
	  (*pyear)=0;
		  	  
	  fracsec=jj-(int)(jj);
/*	  jj=(int)(jj)+floor(0.5+fracsec*24*60*10^(precision))/24/60/10^(precision)+0.00000001;*/
	  
	  z=(int) (jj+0.5);
	  f=jj+0.5-z;
	  if (z<2299161) {
		a=z;
	  } else {
		a2=(int) ((z-1867216.25)/36524.25);
		a=z+1+a2-(int) (a2/4);
	  }
	  b=a+1524;
	  c=(int)((b-122.1)/365.25);
	  d=(int) (365.25*c);
	  e=(int) ((b-d)/30.6001);
	  jourhms=b-d-(int) (30.6001*e)+f;
	  (*pday)=(int) (jourhms);
	  heurems=(jourhms-(*pday))*24;
	  (*phour)=(int) (heurems);
	  mins=(heurems-(*phour))*60;
	  (*pmin)=(int) (mins);
	  (*psec)=(mins - (*pmin))*60;
	  if (e<13.5) {
		(*pmonth)=e-1;
	  } else {
		(*pmonth)=e-13;
	  }
	  if ((*pmonth)>2.5) {
		(*pyear)=c-4716;
	  } else {
		(*pyear)=c-4715;
	  }
}

double JD_from_time_t(const time_t time_t_value)
{
	struct tm *p_time_tm;
	double jd;
	
	p_time_tm=localtime(&time_t_value);
	jd=gregorian_calendar_to_jd(p_time_tm->tm_year+1900, p_time_tm->tm_mon+1, p_time_tm->tm_mday, p_time_tm->tm_hour, p_time_tm->tm_min, (double) (p_time_tm->tm_sec));
/*	free(p_time_tm); */
	
	return jd;
}

void fprint_jd(FILE *stream, const double jd)
{
	double sec;
	int min;
	int hour;
	int day;
	int month;
	int year;
	
	jd_to_date(jd, &sec, &min, &hour, &day, &month, &year);
	fprintf(stream,"%04d.%02d.%02d %02d:%02d:",year,month,day,hour,min);
	if (sec>9.9999) {
		fprintf(stream,"%06.4f",sec);
	} else {
		fprintf(stream,"0%06.4f",sec);
	}
}

void fprint_jd_wj(FILE *stream, const double jd)
{
	double sec;
	int min;
	int hour;
	int day;
	int month;
	int year;
	long min_frac;
	double jd_frac;
	double jd_wj;
	long precision = 10*10*10*10*10*10;

	
	jd_frac = jd - ceil(jd);
	jd_wj =ceil(jd) + floor(0.5+jd_frac*24*60*precision)/(24*60*precision)+0.0000000001;
	jd_to_date(jd_wj, &sec, &min, &hour, &day, &month, &year);

	min_frac=(long) ceil(floor(0.5+sec*precision/60.0));
	if (min_frac>=precision) {
		min_frac=(long) (precision-1);
	}
		
	fprintf(stream,"%04d/%02d/%02d %02d:%02d",year,month,day,hour,min);
	fprintf(stream,",%06d",(int) min_frac);
}

void fprint_timetype(FILE *stream, const TIME_TYPE timetype)
{
	switch (timetype) {
		case LT:
			fprintf(stream,"LT");
			break;
		case UT:
			fprintf(stream,"UT");
			break;
		default:
			fprintf(stream,"Undefined");
	}
}

int month_nb(char *month_letter)
{
	int month;
	/* J F M A M J J A S O N D */
	
	if ((strncmp(month_letter,"Jan",3)==0) || (strncmp(month_letter,"jan",3)==0)) {
			month=1;
	} else if ((strncmp(month_letter,"Feb",3)==0) || (strncmp(month_letter,"feb",3)==0) || (strncmp(month_letter,"F�v",3)==0) || (strncmp(month_letter,"f�v",3)==0) || (strncmp(month_letter,"F",1)==0) || (strncmp(month_letter,"f",1)==0)) {
			month=2;
	} else if ((strncmp(month_letter,"Mar",3)==0) || (strncmp(month_letter,"mar",3)==0)) {
			month=3;
	} else if ((strncmp(month_letter,"Apr",3)==0) || (strncmp(month_letter,"apr",3)==0) || (strncmp(month_letter,"Avr",3)==0) || (strncmp(month_letter,"avr",3)==0)) {
			month=4;
	} else if ((strncmp(month_letter,"May",3)==0) || (strncmp(month_letter,"may",3)==0) || (strncmp(month_letter,"Mai",3)==0) || (strncmp(month_letter,"mai",3)==0)){
			month=5;
	} else if ((strncmp(month_letter,"Jun",3)==0) ||( strncmp(month_letter,"jun",3)==0) || (strncmp(month_letter,"Juin",4)==0) || (strncmp(month_letter,"juin",4)==0)){
			month=6;
	} else if ((strncmp(month_letter,"Jul",3)==0) || (strncmp(month_letter,"jul",3)==0) || (strncmp(month_letter,"Juil",4)==0) || (strncmp(month_letter,"juil",4)==0) || (strncmp(month_letter,"Jui",3)==0) || (strncmp(month_letter,"jui",3)==0)) {
			month=7;
	} else if ((strncmp(month_letter,"Aug",3)==0) || (strncmp(month_letter,"aug",3)==0) || (strncmp(month_letter,"Ao�",3)==0) || (strncmp(month_letter,"ao�",3)==0) || (strncmp(month_letter,"A",1)==0) || (strncmp(month_letter,"a",1)==0)) {
			month=8;
	} else if ((strncmp(month_letter,"Sep",3)==0) || (strncmp(month_letter,"sep",3)==0) || (strncmp(month_letter,"S",1)==0) || (strncmp(month_letter,"s",1)==0)) {
			month=9;
	} else if ((strncmp(month_letter,"Oct",3)==0) || (strncmp(month_letter,"oct",3)==0) || (strncmp(month_letter,"O",1)==0) || (strncmp(month_letter,"o",1)==0)) {
			month=10;
	} else if ((strncmp(month_letter,"Nov",3)==0) || (strncmp(month_letter,"nov",3)==0) || (strncmp(month_letter,"N",1)==0) || (strncmp(month_letter,"n",1)==0)) {
			month=11;
	} else if ((strncmp(month_letter,"Dec",3)==0) || (strncmp(month_letter,"dec",3)==0) || (strncmp(month_letter,"D�c",3)==0) || (strncmp(month_letter,"d�c",3)==0) || (strncmp(month_letter,"D",1)==0) || (strncmp(month_letter,"d",1)==0)) {
			month=12;
	} else {
		month=0;
	}
	return month;
}

bool IsDateValid(double julianday) {
	double JD_min = gregorian_calendar_to_jd(1999, 1, 1, 0, 0, 0);
	//double JD_max = gregorian_calendar_to_jd(2080, 1, 1, 0, 0, 0);

	time_t now = time(NULL);
	struct tm* pnow_tm = localtime(&now);
	double JD_max = gregorian_calendar_to_jd(pnow_tm->tm_year + 1900, pnow_tm->tm_mon + 1, pnow_tm->tm_mday, pnow_tm->tm_hour, pnow_tm->tm_min, (double)(pnow_tm->tm_sec)) + 1;

	return ((julianday > JD_min) && (julianday < JD_max));
}

bool IsDurationValid(double duration) {
	return ((duration >= DURATION_MIN) && (duration <= DURATION_MAX));
}

bool IsFPSValid(double fps) {
	return ((fps >= FPS_MIN) && (fps <= FPS_MAX));
}

void CorrectDatationFromPIPP(int nbframes, double* pstart_time, double* pend_time, double* pduration, PIPPInfo* pipp_info, char* comment) {
	double delta_start			= 0;
	double duration_adjusted	= (*pduration);

	if (nbframes > 0) { // Checks if number of frames have been truncated
		if ((*pipp_info).centered_frames) {
			delta_start = (nbframes / 2.0) - MIN(((*pipp_info).max_nb_frames / 2.0), (nbframes / 2.0)) * (*pduration) / nbframes;
			duration_adjusted = MIN((*pipp_info).max_nb_frames, nbframes) * (*pduration) / nbframes;
			(*pstart_time) = ((*pstart_time)+(*pend_time))/2.0 - duration_adjusted / (2 * (ONE_DAY_SEC));;
			(*pend_time) = (*pstart_time) + duration_adjusted / (ONE_DAY_SEC);
		} else {
			if ((*pipp_info).start_frame > 1)	delta_start = (MIN(((*pipp_info).start_frame - 1), nbframes)) * (*pduration) / nbframes;
			else (*pipp_info).start_frame = 1;
			if ((*pipp_info).max_nb_frames > 0)	duration_adjusted = (MIN((*pipp_info).max_nb_frames, nbframes - ((*pipp_info).start_frame - 1))) * (*pduration) / nbframes;
			(*pstart_time) += delta_start / (ONE_DAY_SEC);
			(*pend_time) = (*pstart_time) + duration_adjusted / (ONE_DAY_SEC);
		}
		if ((delta_start > 0) || (duration_adjusted != (*pduration))) strcat(comment, ", pipp info");
		(*pduration) = duration_adjusted;
	}
}
