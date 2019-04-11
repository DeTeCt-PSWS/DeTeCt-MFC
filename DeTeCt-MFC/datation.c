/********************************************************************************/
/*                                                                              */
/*	DTC	(c) Marc Delcroix (delcroix.marc@free.fr) 2012-							*/
/*                                                                              */
/*    DATATION: Detection of datation information functions						*/
/*                                                                              */
/********************************************************************************/

#include "common.h"
#include <time.h>
#include <stdio.h>
#include <sys/stat.h>

#include "serfmt.h"
#include "datation.h"
#include "wrapper.h"
#include "dtc.h"

const double FPS_MIN		=	0.01;
const double FPS_MAX		=	2000.0;
const double DURATION_MIN	=	0.0005;				/* 1.0/FPS_MAX; */
const double DURATION_MAX	=	ONE_DAY_SEC;

/*****************************************************************************************/
/*******************MAIN FUNCTION to get datation of capture******************************/
/*****************************************************************************************/

void dtcGetDatation(DtcCapture *capture, char *filename, int nbframes, double *pstart_time, double *pend_time, double *pduration, double *pfps, TIME_TYPE *ptimetype, char *comment)
{
	double JD_init			= gregorian_calendar_to_jd(1,1,1,0,0,0);
	double JD_test_min		= gregorian_calendar_to_jd(1980,1,1,0,0,0);
	double JD_test_max		= gregorian_calendar_to_jd(2080,1,1,0,0,0);
/*	time_t now;
	struct tm *pnow_tm=malloc(sizeof *pnow_tm);*/
	
	double start_time_file	= JD_init;
	double end_time_file	= JD_init;
	TIME_TYPE timetype_file	= Unknown;
	double duration_file	= 0.0;
	double fps_file			= 0.0;

	double start_time_ser	= JD_init;
	double end_time_ser		= JD_init;
	TIME_TYPE timetype_ser	= Unknown;
	double duration_ser		= 0.0;
	double fps_ser			= 0.0;

	double start_time_fits	= JD_init;
	double end_time_fits	= JD_init;
	TIME_TYPE timetype_fits	= Unknown;
	double duration_fits	= 0.0;
	double fps_fits			= 0.0;

	double start_time_log	= JD_init;
	double end_time_log		= JD_init;
	TIME_TYPE timetype_log	= Unknown;
	double duration_log		= 0.0;
	double fps_log			= 0.0;
	long nbframes_log		= 0;
	int timezone			= -24;
	
	double time_tmp;
	char comment2[MAX_STRING];	
	
/********** Init **********/	
											if (opts.debug) { fprintf(stderr,"dtcGetDatation: Initializing\n"); }
	(*pstart_time)=JD_init;
	(*pend_time)=JD_init;
	(*ptimetype)=Unknown;
	(*pduration)=0.0;
	(*pfps)=0.0;
	init_string(comment2);
/*	now=time(NULL);
	pnow_tm=localtime(&now);
	JD_test_max=gregorian_calendar_to_jd(pnow_tm->tm_year+1900, pnow_tm->tm_mon+1, pnow_tm->tm_mday, pnow_tm->tm_hour, pnow_tm->tm_min, (double) (pnow_tm->tm_sec))+1;*/
/*fprintf(stderr,"dtcGetDatation: JD_max = %f\n",JD_test_max);*/

/********** Date from fileinfo **********/	
	switch (capture->type)
	{
		case CAPTURE_SER:
		case CAPTURE_CV:
													if (opts.debug) { fprintf(stderr,"dtcGetDatation: Reading information from file\n"); }
			dtcGetDatationFromFileInfo(capture, filename, nbframes, &start_time_file, &end_time_file, &duration_file, &fps_file);
			timetype_file=LT;
													if (opts.debug) {
														fprintf(stderr,"dtcGetDatation: FILE Start    = %f (", start_time_file);
														fprint_jd(stderr, start_time_file);
														fprintf(stderr,")\n");
														fprintf(stderr,"dtcGetDatation: FILE End      = %f (", end_time_file);
														fprint_jd(stderr, end_time_file);
														fprintf(stderr,")\n");
														fprintf(stderr,"dtcGetDatation: FILE Time     = ");
														fprint_timetype(stderr, timetype_file);
														fprintf(stderr,"\n");
														fprintf(stderr,"dtcGetDatation: FILE Duration = %lf\n", duration_file);
														fprintf(stderr,"dtcGetDatation: FILE fps      = %lf\n\n",fps_file);
													}
			break;
		case CAPTURE_FILES:
		case CAPTURE_FITS:
		default:
													if (opts.debug) {
														fprintf(stderr,"dtcGetDatation: FILES/FITS Start    = %fUT, %f (", capture->u.filecapture->StartTimeUTC_JD,capture->u.filecapture->StartTime_JD);
														fprint_jd(stderr, capture->u.filecapture->StartTimeUTC_JD);
														fprintf(stderr,", ");
														fprint_jd(stderr, capture->u.filecapture->StartTime_JD);
														fprintf(stderr,")\n");
														fprintf(stderr,"dtcGetDatation: FILES/FITS End      = %fUT, %f (", capture->u.filecapture->EndTimeUTC_JD,capture->u.filecapture->EndTime_JD);
														fprint_jd(stderr, capture->u.filecapture->EndTimeUTC_JD);
														fprintf(stderr,", ");
														fprint_jd(stderr, capture->u.filecapture->EndTime_JD);
														fprintf(stderr,")\n");
														fprintf(stderr,"dtcGetDatation: FILES/FITS Time     = ");
														fprint_timetype(stderr, timetype_file);
														fprintf(stderr,"\n");
														fprintf(stderr,"dtcGetDatation: FILES/FITS Duration = %lf\n", duration_file);
														fprintf(stderr,"dtcGetDatation: FILES/FITS fps      = %lf\n\n",fps_file);
													}
			break;
	}
	switch (capture->type)
	{
		case CAPTURE_SER:
/********** Date from ser file **********/	
/* Attempting date from SER file */
			start_time_ser=capture->u.sercapture->StartTimeUTC_JD;
			timetype_ser=UT;
			if ((capture->u.sercapture->StartTimeUTC_JD>JD_test_min) && (fabs(timezone)>12) && (fabs(floor(0.5+capture->u.sercapture->StartTime_JD-capture->u.sercapture->StartTimeUTC_JD)*24)<=12)) {
				timezone=(int) floor(0.5+(capture->u.sercapture->StartTime_JD-capture->u.sercapture->StartTimeUTC_JD)*24);
			}
												if (opts.debug) { fprintf(stderr,"dtcGetDatation: Reading information from ser file\n"); }
			serReadTimeStamps(capture->u.sercapture);
			if (capture->u.sercapture->TimeStampExists) {
/* End date available from SER file */
				timetype_ser=UT;
				duration_ser=(capture->u.sercapture->EndTimeUTC_JD - capture->u.sercapture->StartTimeUTC_JD)*ONE_DAY_SEC;
				if ((duration_ser<0) || (duration_ser>DURATION_MAX)) {
					duration_ser=0;
				}
				if (duration_ser==0) {
					fps_ser=0;
					end_time_ser=capture->u.sercapture->EndTimeUTC_JD;
				} else {
					fps_ser=(capture->u.sercapture->header.FrameCount-1)/duration_ser;
					duration_ser=duration_ser+1/fps_ser;
					end_time_ser=capture->u.sercapture->EndTimeUTC_JD+1/fps_ser/ONE_DAY_SEC;
				}
			} else {
				end_time_ser=capture->u.sercapture->EndTimeUTC_JD;
				if (end_time_ser>start_time_ser) {
					timetype_ser=UT;
				}
			}
			if ((capture->u.sercapture->EndTimeUTC_JD>JD_test_min) && (abs(timezone)>12) && (fabs(floor(0.5+capture->u.sercapture->EndTime_JD-capture->u.sercapture->EndTimeUTC_JD)*24)<=12)) {
				timezone=(int) floor(0.5+(capture->u.sercapture->EndTime_JD-capture->u.sercapture->EndTimeUTC_JD)*24);
			}

												if (opts.debug) {
													fprintf(stderr,"dtcGetDatation: SER  Start    = %f (", start_time_ser);
													fprint_jd(stderr, start_time_ser);
													fprintf(stderr,")\n");
													fprintf(stderr,"dtcGetDatation: SER  End      = %f (", end_time_ser);
													fprint_jd(stderr, end_time_ser);
													fprintf(stderr,")\n");
													fprintf(stderr,"dtcGetDatation: SER  Time     = ");
													fprint_timetype(stderr, timetype_ser);
													fprintf(stderr,"\n");
													if (abs(timezone)<=12) {
														fprintf(stderr,"dtcGetDatation: SER  timezone = %d\n",timezone);
													}
													fprintf(stderr,"dtcGetDatation: SER  Duration = %lf\n", duration_ser);
													fprintf(stderr,"dtcGetDatation: SER  fps      = %lf\n\n",fps_ser);
												}			
			if ((end_time_ser>JD_test_min) && (end_time_ser<JD_test_max)) {
				strcpy(comment,"ser file");
				if ((duration_ser>DURATION_MIN) && (duration_ser<=DURATION_MAX)) {
					(*pduration)=duration_ser;
				} else if (((*pfps)>FPS_MIN) && ((*pfps)<=FPS_MAX)) {
					(*pduration)=nbframes/(*pfps);
					strcat(comment,", duration estimated");
				} else if ((duration_file>DURATION_MIN) && (duration_file<=DURATION_MAX)) {				/* from file */
					(*pduration)=duration_file;
				}
				(*ptimetype)=timetype_ser;
				(*pfps)=fps_ser;
				(*pend_time)=end_time_ser;
				if ((start_time_ser>JD_test_min) && (start_time_ser<JD_test_max)) {
					(*pstart_time)=start_time_ser;
				} else {
					(*pstart_time)=(*pend_time)-(*pduration)/ONE_DAY_SEC;
					strcat(comment,", start date estimated");
				}
			}
			if ((start_time_ser>JD_test_min) && (start_time_ser<JD_test_max)) {
				strcpy(comment,"ser file");
				if ((duration_ser>DURATION_MIN) && (duration_ser<=DURATION_MAX)) {
					(*pduration)=duration_ser;
				} else if (((*pfps)>FPS_MIN) && ((*pfps)<=FPS_MAX)) {
					(*pduration)=nbframes/(*pfps);
					strcat(comment,", duration estimated");
				} else if ((duration_file>DURATION_MIN) && (duration_file<=DURATION_MAX)) {				/* from file */
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
			if ((duration_fits<0) || (duration_fits>DURATION_MAX)) {
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
												if (opts.debug) {
													fprintf(stderr,"dtcGetDatation: FILES Start    = %f (", start_time_fits);
													fprint_jd(stderr, start_time_fits);
													fprintf(stderr,")\n");
													fprintf(stderr,"dtcGetDatation: FILES End      = %f (", end_time_fits);
													fprint_jd(stderr, end_time_fits);
													fprintf(stderr,")\n");
													fprintf(stderr,"dtcGetDatation: FILES Time     = ");
													fprint_timetype(stderr, timetype_fits);
													fprintf(stderr,"\n");
													fprintf(stderr,"dtcGetDatation: FILES Duration = %lf\n", duration_fits);
													fprintf(stderr,"dtcGetDatation: FILES fps      = %lf\n\n",fps_fits);
												}
			if ((start_time_fits>JD_test_min) && (start_time_fits<JD_test_max)) {
				strcpy(comment,"file info");
				(*pfps)=fps_fits;
				if ((duration_fits>DURATION_MIN) && (duration_fits<=DURATION_MAX)) {
					(*pduration)=duration_fits;
				} else if (((*pfps)>FPS_MIN) && ((*pfps)<=FPS_MAX)) {
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
		case CAPTURE_FITS:
/********** Date from FITS file **********/	
			start_time_fits=capture->u.filecapture->StartTimeUTC_JD;
			end_time_fits=capture->u.filecapture->EndTimeUTC_JD;
			timetype_fits=UT;
			duration_fits=(capture->u.filecapture->EndTimeUTC_JD - capture->u.filecapture->StartTimeUTC_JD)*ONE_DAY_SEC;
			if ((duration_fits<0) || (duration_fits>DURATION_MAX)) {
					duration_fits=0;
			}
			if (fabs(duration_fits)<DURATION_MIN) {
				fps_fits=0;
				end_time_fits=JD_init;
			} else {
				fps_fits=(capture->u.filecapture->LastFileIdx-capture->u.filecapture->FirstFileIdx+1)/duration_fits;
				duration_fits=duration_fits+1/fps_fits;
				end_time_fits=capture->u.filecapture->EndTimeUTC_JD+1/fps_fits/ONE_DAY_SEC;
			}	
												if (opts.debug) {
													fprintf(stderr,"dtcGetDatation: FITS Start    = %f (", start_time_fits);
													fprint_jd(stderr, start_time_fits);
													fprintf(stderr,")\n");
													fprintf(stderr,"dtcGetDatation: FITS End      = %f (", end_time_fits);
													fprint_jd(stderr, end_time_fits);
													fprintf(stderr,")\n");
													fprintf(stderr,"dtcGetDatation: FITS Time     = ");
													fprint_timetype(stderr, timetype_fits);
													fprintf(stderr,"\n");
													fprintf(stderr,"dtcGetDatation: FITS Duration = %lf\n", duration_fits);
													fprintf(stderr,"dtcGetDatation: FITS fps      = %lf\n\n",fps_fits);
												}
			if ((start_time_fits>JD_test_min) && (start_time_fits<JD_test_max)) {
				strcpy(comment,"FITS info");
				(*pfps)=fps_fits;
				if ((duration_fits>DURATION_MIN) && (duration_fits<=DURATION_MAX)) {
					(*pduration)=duration_fits;
				} else if (((*pfps)>FPS_MIN) && ((*pfps)<=FPS_MAX)) {
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
	if (((*pend_time)>JD_test_min) && ((*pend_time)<JD_test_max)) {
		timezone=(int) floor(0.5+(end_time_file-(*pend_time))*24);
		if (fabs(timezone)>12) {
			timezone=-24;
		}
	} else if (((*pstart_time)>JD_test_min) && ((*pstart_time)<JD_test_max)) {
		timezone=(int) floor(0.5+(end_time_file-(*pstart_time))*24);
		if (fabs(timezone)>13) {
			timezone=-24;
		}
	}
}
/********** Date from log file **********/	
											if (opts.debug) { fprintf(stderr,"dtcGetDatation: Reading information from log file\n"); }
	dtcGetDatationFromLogFile(filename, &start_time_log, &end_time_log, &duration_log, &fps_log, &nbframes_log, &timetype_log, comment2);
	if ((duration_log<DURATION_MIN) && ((end_time_log-start_time_log)*ONE_DAY_SEC>DURATION_MIN) && ((end_time_log-start_time_log)*ONE_DAY_SEC<DURATION_MAX)) {
		duration_log=(end_time_log-start_time_log)*ONE_DAY_SEC;
	}
											if (opts.debug) {
												fprintf(stderr,"dtcGetDatation: LOG  Start    = %f (", start_time_log);
												fprint_jd(stderr, start_time_log);
												fprintf(stderr,")\n");
												fprintf(stderr,"dtcGetDatation: LOG  End      = %f (", end_time_log);
												fprint_jd(stderr, end_time_log);
												fprintf(stderr,")\n");
												fprintf(stderr,"dtcGetDatation: LOG  Time     = ");
												fprint_timetype(stderr, timetype_log);
												fprintf(stderr,"\n");
												fprintf(stderr,"dtcGetDatation: LOG  Duration = %lf\n", duration_log);
												fprintf(stderr,"dtcGetDatation: LOG  fps      = %lf\n\n",fps_log);
												fprintf(stderr,"dtcGetDatation: Comment       = %s\n\n",comment2);
												fprintf(stderr, "dtcGetDatation: LOG nframe      = %d\n\n", nbframes_log);
											}
	if (nbframes_log != nbframes) { fprintf(stderr, "WARNING: real number of frames %ld differs from theorical number of frames %ld, using real number.\n", nbframes, nbframes_log); }
/********** Use log file information if available **********/	
	if ((start_time_log>JD_test_min) && (start_time_log<JD_test_max)) {
		strcpy(comment,comment2);
		(*ptimetype)=timetype_log;
		(*pstart_time)=start_time_log;
		if ((fps_log>FPS_MIN) && (fps_log<=FPS_MAX)) { 
			(*pfps)=fps_log;
		}
		if ((duration_log>DURATION_MIN) && (duration_log<=DURATION_MAX)) {
			(*pduration)=duration_log;
		} else if ((*pduration)<DURATION_MIN) {
			if (((*pfps)>FPS_MIN) && ((*pfps)<=FPS_MAX)) {
				(*pduration)=nbframes/(*pfps);
				strcat(comment,", duration calculated");
			} else if ((duration_file>DURATION_MIN) && (duration_file<=DURATION_MAX)) {
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
	if (((*pstart_time)<(JD_test_min+1)) || ((*pstart_time)>JD_test_max)) {
		strcpy(comment,"file info");
		if ((duration_log>DURATION_MIN) && (duration_log<=DURATION_MAX)) {
			(*pduration)=duration_log;
			strcat(comment,", ");
			strcat(comment,comment2);
		} else if (((*pduration)<DURATION_MIN) && (duration_file>DURATION_MIN) && (duration_file<=DURATION_MAX)) {
			(*pduration)=duration_file;
		}
		if ((fps_log>FPS_MIN) && (fps_log<=FPS_MAX)) {
			(*pfps)=fps_log;
		} else if (duration_file>0) {
			(*pfps)=fps_file;
		}
		if (((*pduration)<DURATION_MIN) && ((*pfps)>FPS_MIN) && ((*pfps)<=FPS_MAX)) {
			(*pduration)=nbframes/(*pfps);
			strcat(comment,", duration estimated");
		}
		if (((*pend_time)>JD_test_min) && ((*pend_time)<JD_test_max)) {
			(*pstart_time)=(*pend_time)-(*pduration)/ONE_DAY_SEC;
			strcat(comment,", start date estimated");
		} else {
			(*ptimetype)=timetype_file;
			(*pend_time)=end_time_file;
			if (((*pend_time)>start_time_file) && (((*pend_time)-start_time_file)<ONE_DAY_SEC/2.0)) {
				(*pstart_time)=start_time_file;
			} else {
				(*pstart_time)=(*pend_time)-(*pduration)/ONE_DAY_SEC;
				strcat(comment,", start date estimated");
			}
		}
	}
	if (((*pend_time)<(JD_test_min+1)) || ((*pend_time)>JD_test_max)) {
		strcpy(comment,"file info");
		if ((duration_log>DURATION_MIN) && (duration_log<=DURATION_MAX)) {
			(*pduration)=duration_log;
			strcat(comment,", ");
			strcat(comment,comment2);
		} else if ((duration_file>DURATION_MIN) && (duration_file<=DURATION_MAX)) {
			(*pduration)=duration_file;
		}
		if ((fps_log>FPS_MIN) && (fps_log<=FPS_MAX)) {
			(*pfps)=fps_log;
		} else if (duration_file>0) {
			(*pfps)=fps_file;
		}
		if (((*pduration)<DURATION_MIN) && ((*pfps)>FPS_MIN) && ((*pfps)<=FPS_MAX)) {
			(*pduration)=nbframes/(*pfps);
			strcat(comment,", duration estimated");
		}		
		if (((*pstart_time)>JD_test_min) && ((*pstart_time)<JD_test_max)) {
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
	if (!((*ptimetype)==UT) && (abs(timezone)<=12)) {
		(*ptimetype)=UT;
		(*pstart_time)=(*pstart_time)-timezone/24.0;
		(*pend_time)=(*pend_time)-timezone/24.0;
	}
/********** Derive LT or UT from file info **********/	
	if ((*ptimetype)==Unknown) {
		if (fabs(end_time_file-(*pend_time))*24.0<1.0/60.0) {
			(*ptimetype)=LT;
		} else {
			timezone=(int) floor(0.5+((end_time_file)-(*pend_time))*24.0);
			if (fabs(((end_time_file)-(*pend_time))*24.0-timezone)<0.5/60.0) {
				(*ptimetype)=UT;
			}
		}
	}
/********** No duration => derive end time from file **********/	
	if ((fabs(((*pend_time)-(*pstart_time))*(ONE_DAY_SEC))<0.1) && (fabs(end_time_file-(*pstart_time))<13.0/24.0) && ((*ptimetype)==UT)) {
		(*pduration)=(end_time_file-floor(0.5+(end_time_file-(*pstart_time))*24)/24.0-(*pstart_time))*ONE_DAY_SEC;
		(*pend_time)=(*pstart_time)+(*pduration)/ONE_DAY_SEC;
	}

/********** Calculates fps if necessary **********/	
	if ((((*pfps)<FPS_MIN) || ((*pfps)>FPS_MAX)) && ((*pduration)>DURATION_MIN) && ((*pduration)<=DURATION_MAX)) {
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
	if (((*pfps)<FPS_MIN) || ((*pfps)>FPS_MAX)) {
		(*pfps)=0.0;
	}
											if (opts.debug) {
												fprintf(stderr,"dtcGetDatation: FINAL Start    = %f (", (*pstart_time));
												fprint_jd(stderr, (*pstart_time));
												fprintf(stderr,")\n");
												fprintf(stderr,"dtcGetDatation: FINAL End      = %f (", (*pend_time));
												fprint_jd(stderr,(*pend_time));
												fprintf(stderr,")\n");
												fprintf(stderr,"dtcGetDatation: FINAL Time     = ");
												fprint_timetype(stderr,(*ptimetype));
												fprintf(stderr,"\n");
												fprintf(stderr,"dtcGetDatation: FINAL Duration = %lf\n", (*pduration));
												fprintf(stderr,"dtcGetDatation: FINAL fps      = %lf\n\n",(*pfps));
											}
/*	free(pnow_tm);*/
}
/*****************************************************************************************/
/*************************Correct datation after scan ************************************/
/*****************************************************************************************/

void dtcCorrectDatation(DtcCapture *capture, double *pstart_time, double *pend_time, double *pduration, double *pfps, TIME_TYPE *ptimetype, char *comment)
{
	char comment2[MAX_STRING];

/**** Correction of end date/duration if frames invalid ****/
	init_string(comment2);
	if (((*pduration)>DURATION_MIN) && ((*pduration)<=DURATION_MAX)) {
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
											if (opts.debug) {
												fprintf(stderr,"dtcGetDatation: Start    = %f (", (*pstart_time));
												fprint_jd(stderr, (*pstart_time));
												fprintf(stderr,")\n");
												fprintf(stderr,"dtcGetDatation: Corr. End = %f (", (*pend_time));
												fprint_jd(stderr,(*pend_time));
												fprintf(stderr,")\n");
												fprintf(stderr,"dtcGetDatation: Time     = ");
												fprint_timetype(stderr,(*ptimetype));
												fprintf(stderr,"\n");
												fprintf(stderr,"dtcGetDatation: Corr.Dur = %lf\n", (*pduration));
												fprintf(stderr,"dtcGetDatation: fps      = %lf\n\n",(*pfps));
											}
}

/*****************************************************************************************/
/*****************Logs capture scan date information in dtc.log***************************/
/*****************************************************************************************/

void dtcWriteLog(const char *dtcexename, const double start_time, const double end_time, const double duration, const double fps, const TIME_TYPE timetype, const char *filename, const char *comment, const int nb_impact, const int print)
{
	FILE *dtclogfile;
	char dtclogfilename[MAX_STRING];
	char *pext;
	
	init_string(dtclogfilename);
	strcpy(dtclogfilename, dtcexename);
	pext = strstr(dtclogfilename, ".exe");
	if (pext!=NULL) {
		strncpy(pext, ".log", strlen(".log"));
	} else {
		strcat(dtclogfilename,".log");
	}
	
	dtclogfile=fopen(dtclogfilename,"r");
	if (dtclogfile==NULL) {
		dtclogfile=fopen(dtclogfilename,"a");
		fprintf(dtclogfile,"DeTeCt; jovian impact detection software\n");
		fprintf(dtclogfile,"PLEASE SEND THIS FILE to Marc Delcroix - delcroix.marc@free.fr - for work on impact frequency (participants will be named if work is published) - NO DETECTION MATTERS!\n");
		fprintf(dtclogfile,"Rating;    Start;                      End;                        Mid;                       Duration (s);  fps (fr/s);  File;                        DeTeCt version and comment\n");
	} else {
		dtclogfile=fopen(dtclogfilename,"a");
	}
	if (nb_impact>0) {
		fprintf(dtclogfile,"%3d       ; ",nb_impact);
	} else if (nb_impact==0) {
		fprintf(dtclogfile,"%3d       ; ",0);
	} else {
		fprintf(dtclogfile,"Not known; ");
	}
	
	fprint_jd_wj(dtclogfile, start_time);
	switch (timetype){
		case UT:
			fprintf(dtclogfile," UT; ");
			break;
		case LT:
			fprintf(dtclogfile," LT; ");
			break;
		default:
			fprintf(dtclogfile," xx; ");
	}	
	fprint_jd_wj(dtclogfile, end_time);
	switch (timetype){
		case UT:
			fprintf(dtclogfile," UT; ");
			break;
		case LT:
			fprintf(dtclogfile," LT; ");
			break;
		default:
			fprintf(dtclogfile," xx; ");
	}	
	fprint_jd_wj(dtclogfile, (end_time+start_time)/2);
	switch (timetype){
		case UT:
			fprintf(dtclogfile," UT; ");
			break;
		case LT:
			fprintf(dtclogfile," LT; ");
			break;
		default:
			fprintf(dtclogfile," xx; ");
	}	
	fprintf(dtclogfile,"%9.4lf s; ", duration);
	fprintf(dtclogfile,"%7.3lf fr/s; ",fps);
	fprintf(dtclogfile,"%s; ", filename);
	fprintf(dtclogfile,"%s v%s%s%s (%s)\n",PROGNAME, VERSION_NB,VERSION_MSVC,VERSION_DATE,comment);
	if (fclose(dtclogfile)!=0) {
		fprintf(stderr, "ERROR in dtcWriteLog closing file %s\n", dtclogfilename);
		exit(EXIT_FAILURE);
	}
	
	if (print) {
		printf("[");
		fprint_jd_wj(stdout, start_time);
		printf("-");
		fprint_jd_wj(stdout, end_time);
		printf(" ");
		switch (timetype){
			case UT:
				printf("UT");
				break;
			case LT:
				printf("LT");
				break;
			default:
				printf("xx");
		}
		printf("], ");
		if (((end_time-start_time)>=0.0) && ((end_time-start_time)<12.0/24.0)) {
			printf("mid ");
			fprint_jd_wj(stdout, (start_time+end_time)/2);
			printf(" ");
			switch (timetype){
				case UT:
					printf("UT, ");
					break;
				case LT:
					printf("LT, ");
					break;
				default:
					printf("xx, ");
			}
		}
		printf("%9.4lf s, ", duration);
		printf("%7.3lf fr/s, ",fps);
		printf("(%s)\n", comment);
	}
}

/*****************************************************************************************/
/*******************Gets datation from file system information****************************/
/*****************************************************************************************/

void dtcGetDatationFromFileInfo(DtcCapture *capture, const char *filename, const int nbframes, double *pstart_time, double *pend_time, double *pDuration, double *pfps)
{
	time_t	start_time_t;
	time_t	end_time_t;
	struct tm *pstart_time_tm;
	struct tm *pend_time_tm;
	struct stat videofile_info;
	double	duration_tmp;
	int		nbframes_opencv;
	
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
	}	

	if (InStr(filename,"-DeRot.")<0) {
		stat(filename, &videofile_info);
		start_time_t=videofile_info.st_ctime;
		end_time_t=videofile_info.st_mtime;
		pend_time_tm=localtime(&end_time_t);
		(*pend_time)=gregorian_calendar_to_jd(pend_time_tm->tm_year+1900, pend_time_tm->tm_mon+1, pend_time_tm->tm_mday, pend_time_tm->tm_hour, pend_time_tm->tm_min, (double) (pend_time_tm->tm_sec));
//		duration_tmp=(double) (difftime(videofile_info.st_mtime,videofile_info.st_ctime));
		duration_tmp=(double) (videofile_info.st_mtime-videofile_info.st_ctime);
		if ((duration_tmp<12.0*60.0*60.0) && (duration_tmp>DURATION_MIN))  {
			pstart_time_tm=localtime(&start_time_t);
			(*pstart_time)=gregorian_calendar_to_jd(pstart_time_tm->tm_year+1900, pstart_time_tm->tm_mon+1, pstart_time_tm->tm_mday, pstart_time_tm->tm_hour, pstart_time_tm->tm_min, (double) (pstart_time_tm->tm_sec));
			(*pDuration)=duration_tmp;
		} else if ((*pfps)>FPS_MIN) {
			if (nbframes>nbframes_opencv) {
				(*pDuration)=nbframes/(*pfps);
			} else {
				(*pDuration)=nbframes_opencv/(*pfps);
			}
			(*pstart_time)=(*pend_time)-(*pDuration)/ONE_DAY_SEC;
		} else  {
			(*pstart_time)=gregorian_calendar_to_jd(1,1,1,0,0,0);
		}
//(*pstart_time)=gregorian_calendar_to_jd(pstart_time_tm->tm_year+1900, pstart_time_tm->tm_mon+1, pstart_time_tm->tm_mday, pstart_time_tm->tm_hour, pstart_time_tm->tm_min, (double) (pstart_time_tm->tm_sec));
	}
}

/*****************************************************************************************/
/***************Gets datation from acquisition software log files*************************/
/*****************************************************************************************/

int dtcGetDatationFromLogFile(const char *filename, double *jd_start_time_loginfo, double *jd_end_time_loginfo, double *pDuration, double *pfps, long *pnbframes, TIME_TYPE *plogtimezone, char *comment)
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
	
	char line[MAX_STRING];
	char value[MAX_STRING];
	char value2[MAX_STRING];
	char fieldname[MAX_STRING];
	char software[MAX_STRING];
	char tmpline[MAX_STRING];
	char logfilename[MAX_STRING];
	char logfilename_rac[MAX_STRING];
	char logfilename_dir[MAX_STRING];
	char logfilename_short[MAX_STRING];
	char logfilename_tmp[MAX_STRING];
	char token[MAX_STRING];
	char month_letter[4];
	char Date_format[MAX_STRING];
	char Time_format[MAX_STRING];
	char software_version_string[MAX_STRING];
	int software_version_x86;
	double software_version;
	int software_beta;
	int year;
	int month;
	int day;
	int hour;
	int min;
	double sec;
	int year_tmp;
	int month_tmp;
	int day_tmp;
	int hour_tmp;
	int min_tmp;
	double sec_tmp;
	FILE *logfile;
	int end_time_flag;
	int year_end;
	int month_end;
	int day_end;
	int hour_end;
	int min_end;
	double sec_end;
	int year_mid;
	int month_mid;
	int day_mid;
	int hour_mid;
	double minsec_mid;
	double jd_mid_time_loginfo;
	int timezone;
	double JD_init;
	char date_value[MAX_STRING];
	char start_value[MAX_STRING];
	char mid_value[MAX_STRING];
	char end_value[MAX_STRING];
	struct dirent *pDirent;
	DIR *pDir;


	JD_init						= gregorian_calendar_to_jd(1,1,1,0,0,0);
	(*jd_start_time_loginfo)	= JD_init;
	(*jd_end_time_loginfo)		= (*jd_start_time_loginfo);
	(*pDuration)				= 0.0;
	(*pfps)						= 0.0;
	(*plogtimezone)				= Unknown;
	timezone					= 0;
	software_version			= 0.0;
	software_beta				= -1;
	software_version_x86		= 0;
	end_time_flag				= 0;
	year_end					= 0;
	month_end					= 0;
	day_end					= 0;
	hour_end					= 0;
	min_end					= 0;
	sec_end					= 0;
	
	init_string(line);
	init_string(value);
	init_string(value2);
	init_string(fieldname);
	init_string(software);
	init_string(tmpline);
	init_string(logfilename);
	init_string(logfilename_rac);
	init_string(logfilename_dir);
	init_string(logfilename_short);
	init_string(logfilename_tmp);
	init_string(token);
	init_string(Date_format);
	init_string(Time_format);
	init_string(date_value);
	init_string(start_value);
	init_string(mid_value);
	init_string(end_value);
	init_string(software_version_string);

	get_fileextension(filename,tmpline,EXT_MAX);
	left(filename,strlen(filename)-strlen(tmpline)-1,logfilename_rac);
	strcpy(logfilename_rac,replace_str(logfilename_rac,"_pipp",""));

	if (!(strrchr(filename, '\\')==NULL)) {
		left(filename, strlen(filename)-strlen(strrchr(filename, '\\'))+1,logfilename_dir);
		right(logfilename_rac, strlen(logfilename_rac) - strlen(logfilename_dir), logfilename_short);
	}
	else {
		strcpy(logfilename_dir, ".\\");
		strcpy(logfilename_short, logfilename_rac);
	}
										if (opts.debug) {fprintf(stderr, "dtcGetDatationFromLogFile: Len filename %s=%zd\n", filename ,strlen(filename));}
/* Firecapture log */	
	strcpy(logfilename, logfilename_rac);
	strncat(logfilename, ".txt", strlen(".txt"));
	logfile=fopen(logfilename,"r");
										if (opts.debug) {fprintf(stderr, "dtcGetDatationFromLogFile: Testing file %s\n", logfilename);}
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

			timezone=abs(round((jd_log-(*jd_start_time_loginfo))*24));
			if ((timezone>=1) && (timezone<=12)) { 
				(*plogtimezone)=UT;
				timezone=0;
			} else {
				(*plogtimezone)=LT;
				timezone=0;
			}
											if (opts.debug) {fprintf(stderr, "dtcGetDatationFromLogFile: Testing file %s\n", logfilename);}
/* Genika log */	
			} else {
				strcpy(logfilename, filename);
				strncat(logfilename, ".txt", strlen(".txt"));
				logfile=fopen(logfilename,"r");
											if (opts.debug) {fprintf(stderr, "dtcGetDatationFromLogFile: Testing file %s\n", logfilename);}
				if (logfile==NULL) {
/* Marc Delcroix's LucamRecorder log */	
					strcpy(logfilename, logfilename_rac);
					strncat(logfilename, "-Ser-Stream_info.Log", strlen("-Ser-Stream_info.Log"));
											if (opts.debug) {fprintf(stderr, "dtcGetDatationFromLogFile: Testing file %s\n", logfilename);}
					logfile=fopen(logfilename,"r");
					if (logfile!=NULL) {
						strcpy(software,"Lucam Recorder");
					} else {
/* LucamRecorder log */	
						strcpy(logfilename, logfilename_rac);
						strncat(logfilename, "-Ser-Stream.Log", strlen("-Ser-Stream.Log"));
											if (opts.debug) {fprintf(stderr, "dtcGetDatationFromLogFile: Testing file %s\n", logfilename);}
						logfile=fopen(logfilename,"r");
						if (logfile!=NULL) {
							strcpy(software,"Lucam Recorder");
						} else {
/* Marc Delcroix's LucamRecorder log fixed name */	
							strcpy(logfilename, logfilename_dir);
							strcat(logfilename, "stream_info.log");
													if (opts.debug) {fprintf(stderr, "dtcGetDatationFromLogFile: Testing file %s\n", logfilename);}
							logfile=fopen(logfilename,"r");
							if (logfile!=NULL) {
								strcpy(software,"Lucam Recorder");
							} else {
/* LucamRecorder log fixed name */	
								strcpy(logfilename, logfilename_dir);
								strcat(logfilename, "Stream.Log");
													if (opts.debug) {fprintf(stderr, "dtcGetDatationFromLogFile: Testing file %s\n", logfilename);}
								logfile=fopen(logfilename,"r");
								if (logfile!=NULL) {
									strcpy(software,"Lucam Recorder");
								} else {
/* PLxCapture log */	
									strcpy(logfilename, logfilename_rac);
									strncat(logfilename, ".plx", strlen(".plx"));
													if (opts.debug) {fprintf(stderr, "dtcGetDatationFromLogFile: Testing file %s\n", logfilename);}
									logfile=fopen(logfilename,"r");
									if (logfile!=NULL) {
										strcpy(software,"PLxCapture");
									} else { /* PLX log filename with info at the end */
										pDir = opendir(logfilename_dir);
										if (pDir == NULL) {
											printf("ERROR in dtcGetDatationFromLogFile: Cannot open directory '%s'\n", logfilename_dir);
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
										if (opts.debug) {fprintf(stderr, "dtcGetDatationFromLogFile: Processing file %s\n", logfilename);}
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
			strcpy(line,replace_str(line,"="," : "));

			strcpy(line,replace_str(line,"mS:","mS : "));
										if (opts.debug) {fprintf(stderr, "dtcGetDatationFromLogFile: Line |%s|, ", line);}		
			if (strstr(line," : ") == NULL) {
				strcpy(fieldname,line);
			} else {
				strncpy(fieldname,line,InStr(line," : "));
			}
			while ((fieldname[strlen(fieldname)-1] == ' ') && (strlen(fieldname)>0)) {
				strcpy(fieldname,left(fieldname,strlen(fieldname)-1,tmpline));
			}
			strcat(fieldname,"\0");
										if (opts.debug) {fprintf(stderr, "Field|%s|, ", fieldname);}												

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
									if (opts.debug) {fprintf(stderr, "Value|%s|\n", value);}		

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
					if (opts.debug) {fprintf(stderr, "Firecapture v%1.1f beta %d\n",software_version, software_beta);}
					init_string(tmpline);
				} else if (strcmp(fieldname,"Début de la capture")==0) {
					strcpy(software,"Genika");
				} else if (strcmp(fieldname,"___________________________________________________________________________")==0) {
					strcpy(software,"Genika");
				} else if (strncmp(fieldname,"Mod",3)==0) {
					strcpy(software,"Genika");
				} else if (strcmp(fieldname,"***********************  GENIKA ASTRO CAPTURE LOG FILE ************************************")==0) {
					strcpy(software,"Genika");
				} else if (strcmp(fieldname,"***********************  GENIKA TRIGGER CAPTURE LOG FILE ************************************")==0) {
					strcpy(software,"Genika");
				} else if (strcmp(fieldname,"Date")==0) {
					if (strlen(value)<=10) {
						strcpy(software,"Avi Felopaul");
					} else {
						strcpy(software,"Genicap");
					}
				} else if ((strcmp(fieldname,"Capture start time")==0) || (strcmp(fieldname,"Start time of recording")==0)) {
					strcpy(software,"Lucam Recorder");
				}
											if (opts.debug) {fprintf(stderr, "dtcGetDatationFromLogFile: Software detected %s\n\n", software);}		
			}
/**************************************************************************************************************/
/* Firecapture                                                                                                */
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

				if (strcmp(fieldname,"Date")==0)  { 	/* Date=11.03.2011 */
					(*plogtimezone)=LT;
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
										if (opts.debug) {fprintf(stderr, "ERROR in dtcGetDatationFromLogFile: Firecapture date format not detected for value %s\n", value); }
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
										if (opts.debug) {fprintf(stderr, "ERROR in dtcGetDatationFromLogFile: Firecapture date format not detected for value %s\n", value); }
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
									if (opts.debug) {fprintf(stderr, "ERROR in dtcGetDatationFromLogFile: Firecapture date format not detected for value %s\n", value); }
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
										if (opts.debug) {fprintf(stderr, "ERROR in dtcGetDatationFromLogFile: Firecapture date format not detected for value %s\n", value); }
									}
								} else {							
									if (opts.debug) {fprintf(stderr, "ERROR in dtcGetDatationFromLogFile: Firecapture date format not detected for value %s\n", value); }
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
										if (opts.debug) {fprintf(stderr, "ERROR in dtcGetDatationFromLogFile: Firecapture date format not detected for value %s\n", value); }
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
										if (opts.debug) {fprintf(stderr, "ERROR in dtcGetDatationFromLogFile: Firecapture date format not detected for value %s\n", value); }
									}
								} else {							
									if (opts.debug) {fprintf(stderr, "ERROR in dtcGetDatationFromLogFile: Firecapture date format not detected for value %s\n", value); }
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
									if (opts.debug) {fprintf(stderr, "ERROR in dtcGetDatationFromLogFile: Firecapture date format not detected for value %s\n", value); }
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
									if (opts.debug) {fprintf(stderr, "ERROR in dtcGetDatationFromLogFile: Firecapture date format not detected for value %s\n", value); }
								}
								break;
							default:
								if (opts.debug) {fprintf(stderr, "ERROR in dtcGetDatationFromLogFile: Firecapture date format not detected for value %s\n", value); }
						}
												if (opts.debug) { fprintf(stderr,"dtcGetDatationFromLogFile: y m d h m s|%d %d %d %d %d %f|\n", year, month, day, hour, min,sec); }
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
						(*plogtimezone) = UT;
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
												if (opts.debug) { fprintf(stderr,"dtcGetDatationFromLogFile: y m d h m s|%d %d %d %d %d %f|\n", year, month, day, hour, min,sec); }
					}
				} else if ((strcmp(fieldname,"End")==0) || (strcmp(fieldname,"End(UT)")==0)) { 	/* End=01:01:47 */
					end_time_flag = 1;
					if (strcmp(fieldname, "End(UT)") == 0) {
						(*plogtimezone) = UT;
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
											if (opts.debug) { fprintf(stderr,"dtcGetDatationFromLogFile: start time y m d h m s|%d %d %d %d %d %f|\n", year, month, day, hour, min,sec); }
											if (opts.debug) { fprintf(stderr,"dtcGetDatationFromLogFile: end time   y m d h m s|%d %d %d %d %d %f|\n", year_end, month_end, day_end, hour_end, min_end, sec_end); }
					(*jd_start_time_loginfo)=gregorian_calendar_to_jd(year, month, day, hour, min, sec);
					(*jd_end_time_loginfo)=gregorian_calendar_to_jd(year_end, month_end, day_end, hour_end, min_end, sec_end);
					if (fabs((jd_log-(*jd_end_time_loginfo)-12.0/24.0))*ONE_DAY_SEC<=1.0) { /* = AM/PM not determined */
						(*jd_start_time_loginfo)=(*jd_start_time_loginfo)+12.0/24.0;
						(*jd_end_time_loginfo)=(*jd_end_time_loginfo)+12.0/24.0;
					}
					(*pDuration)=((*jd_end_time_loginfo)-(*jd_start_time_loginfo))*ONE_DAY_SEC;
											if (opts.debug) { fprintf(stderr,"dtcGetDatationFromLogFile: Duration|%s,%f|\n", value,(*pDuration)); }
					jd_mid_time_loginfo=((*jd_end_time_loginfo)-(*jd_start_time_loginfo))/2;
				} else if (strcmp(fieldname,"LT")==0) { /*LT=UT-1h*/
					if (strlen(value)==2) {
						timezone=0;
					}
					else {
						timezone = atoi(mid(value, 2, strlen(value) - 3, tmpline));
						if ((software_version == 2.3) && (software_beta >= 16)) {
							timezone = -timezone;
						}
						if ((*plogtimezone) != UT) {
							(*jd_start_time_loginfo) = (*jd_start_time_loginfo) - timezone / 24.0;
							(*jd_end_time_loginfo) = (*jd_end_time_loginfo) - timezone / 24.0;
							(*plogtimezone) = UT;
						}
					}
				} else if (strcmp(fieldname,"Duration")==0) { 	/* Duration=135s */
					(*pDuration)=strtod(left(value,strlen(value) - 1,tmpline),NULL);
											if (opts.debug) { fprintf(stderr,"dtcGetDatationFromLogFile: Duration|%s,%f|\n", value,(*pDuration)); }
/*					} else if (strcmp(fieldname,"Profile")==0) {
					strcpy(planet,value);*/
				} else if (((strcmp(fieldname,"FPS")==0) || (strcmp(fieldname,"FPS (avg.)")==0)) && ((*pfps)<FPS_MIN)) { 	/* FPS=49 */
					(*pfps)=atoi(value);
											if (opts.debug) { fprintf(stderr,"dtcGetDatationFromLogFile: FPS|%s,%f|\n", value,(*pfps)); }
				}
				else if ((strcmp(fieldname, "Frames captured") == 0)) { 	/* Frames captured=29248 */
					(*pnbframes) = strtol(value, NULL, 10);
				}
				else if (strcmp(left(fieldname, 5,tmpline), "Frame ") == 0) {		/*	Frame 1:	UT 160627 210709.643 */
				}
/**************************************************************************************************************/
/* Genika Astro + Trigger	                                                                                  */
/**************************************************************************************************************/
			} else if (strcmp(software,"Genika")==0) {			
				if (strcmp(right(fieldname, strlen("but de la capture"),tmpline),"but de la capture") == 0) {
					strcpy(fieldname,"Début de la capture");
				}
				if ((strcmp(right(fieldname, strlen("e de capture (s)"),tmpline),"e de capture (s)") == 0) && (strcmp(left(fieldname,strlen("Dur"),tmpline),"Dur")==0)) {
					strcpy(fieldname,"Durée de capture (s)");
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
				} else if ((strcmp(fieldname,"Début de la capture")==0) || (strcmp(fieldname,"Start Time")==0)) {	/*   1   5   9  2  5  8  1 */
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
					(*plogtimezone)=LT;
											if (opts.debug) { fprintf(stderr,"dtcGetDatationFromLogFile: Start time|y m d h m s|%d %d %d %d %d %f|%f JD|\n", year, month, day, hour, min,sec,(*jd_start_time_loginfo)); }
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
											if (opts.debug) { fprintf(stderr,"dtcGetDatationFromLogFile: Start time|y m d h m s|%d %d %d %d %d %f|%f JD|\n", year, month, day, hour, min,sec,(*jd_start_time_loginfo)); }
					if ((strcmp(right(value,2,tmpline),"PM")==0) && (hour<12)) {
						(*jd_start_time_loginfo)=(*jd_start_time_loginfo)+0.5;
					}
					if ((strcmp(right(value, 2, tmpline), "AM") == 0) && (hour==12)) {
						(*jd_start_time_loginfo) = (*jd_start_time_loginfo) - 0.5;
					}
					(*plogtimezone) = UT;
				} else if (strcmp(fieldname,"Fin de la capture")==0) {			/*   1   5   9  2  5  8  1 */
																				/* : Fri Apr 29 22:47:57 2011 */
					mid(value,4,3,month_letter);
					month_end=month_nb(month_letter);
					year_end=atoi(mid(value,20,4,tmpline));
					day_end=atoi(mid(value,8,2,tmpline));
					hour_end=atoi(mid(value,11,2,tmpline));
					(*plogtimezone)=LT;
/*									hour=hour - oShell.RegRead("HKEY_LOCAL_MACHINE\System\CurrentControlSet\Control\TimeZoneInformation\ActiveTimeBias")*-1/60 */
					min_end=atoi(mid(value,14,2,tmpline));
					sec_end=strtod(mid(value,17,2,tmpline),NULL);
						(*jd_end_time_loginfo)=gregorian_calendar_to_jd(year_end, month_end, day_end, hour_end, min_end, sec_end);											if (opts.debug) { fprintf(stderr,"dtcGetDatationFromLogFile: y m d h m s|%d %d %d %d %d %f|%f JD|\n", year_end, month_end, day_end, hour_end, min_end,sec_end,(*jd_end_time_loginfo)); }
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
											if (opts.debug) { fprintf(stderr,"dtcGetDatationFromLogFile: End time|y m d h m s|%d %d %d %d %d %f|%f JD|\n", year, month, day, hour, min,sec,(*jd_start_time_loginfo)); }
					if ((strcmp(right(value,2,tmpline),"PM")==0) && (hour<12)) {
						(*jd_end_time_loginfo)=(*jd_end_time_loginfo)+0.5;
					}
					if ((strcmp(right(value, 2, tmpline), "AM") == 0) && (hour == 12)) {
						(*jd_end_time_loginfo) = (*jd_end_time_loginfo) - 0.5;
					}
					(*plogtimezone) = UT;
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
					(*plogtimezone)=UT;
											if (opts.debug) { fprintf(stderr,"dtcGetDatationFromLogFile: Mid time|y m d h m.m|%d %d %d %d %f|%f JD|\n", year_mid, month_mid, day_mid, hour_mid, minsec_mid,jd_mid_time_loginfo); }
				} else if (strcmp(fieldname,"Durée de capture (s)")==0)  {	/* : 181.12 */
					(*pDuration)=strtod(value,NULL);
											if (opts.debug) { fprintf(stderr,"dtcGetDatationFromLogFile: Duration|%s,%f|\n", value,(*pDuration)); }
				} else if (strcmp(fieldname,"Acquisition Length in mS")==0)  {	/* : 181.12 */
					(*pDuration)=strtod(value,NULL)/1000.0;
											if (opts.debug) { fprintf(stderr,"dtcGetDatationFromLogFile: Duration|%s,%f|\n", value,(*pDuration)); }
				} else if ((strcmp(fieldname,"FPS moyenne")==0) || (strcmp(fieldname,"Mean FPS")==0) || (strcmp(fieldname,"resulting FPS")==0)) { 	
					if (((*pfps)==0) && (strtod(value,NULL)>0)) {
						(*pfps)=strtod(value,NULL);
												if (opts.debug) { fprintf(stderr,"dtcGetDatationFromLogFile: FPS|%s,%f|\n", value,(*pfps)); }
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
					(*plogtimezone)=UT;
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
					(*plogtimezone)=LT;
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
																																/* : Tuesday, 07 August 2012 03:45:27 / UTC */
																																/* : Saturday, 21 August 2010 05:55:10 / UTC +2 Hours */
					strcpy(value,replace_str(value," / "," "));		/* same UTC */
					strcpy(value,replace_str(value,"  :  "," "));

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
					if ((!strcmp(right(value,3,tmpline),"UTC") == 0) &&  (!strcmp(right(value,5,tmpline),"Hours") == 0)) {
						(*plogtimezone)=LT;
/*										hour=hour - oShell.RegRead("HKEY_LOCAL_MACHINE\System\CurrentControlSet\Control\TimeZoneInformation\ActiveTimeBias")*-1/60 */
					} else {
						(*plogtimezone)=UT;
					}
					(*jd_start_time_loginfo)=gregorian_calendar_to_jd(year, month, day, hour, min, sec);
											if (opts.debug) { fprintf(stderr,"dtcGetDatationFromLogFile: y m d h m s|%d %d %d %d %d %f|\n", year, month, day, hour, min,sec); }
				} else if ((strcmp(fieldname,"Capture duration")==0) ||(strcmp(fieldname,"Recording duration")==0)) { 	/* : 181.12 */
					strcpy(value,replace_str(value, " Sec", ""));
					strcpy(value,replace_str(value, " sec", ""));
					strcpy(value,replace_str(value, "s", ""));
					strcpy(value,replace_str(value, ",", "."));
					(*pDuration)=strtod(value,NULL);
											if (opts.debug) { fprintf(stderr,"dtcGetDatationFromLogFile: Duration|%s,%f|\n", value,(*pDuration)); }
				} else if ((strcmp(fieldname,"Capture frame speed")==0) || (strcmp(fieldname,"Camera stream rate")==0)) { 	/* : 1.4 */
					strcpy(value,replace_str(value, " Fps", ""));
					strcpy(value,replace_str(value, " fps", ""));
					(*pfps)=strtod(value,NULL);
											if (opts.debug) { fprintf(stderr,"dtcGetDatationFromLogFile: FPS|%s,%f|\n", value,(*pfps)); }
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
					(*plogtimezone)=LT;
					(*jd_start_time_loginfo)=gregorian_calendar_to_jd(year, month, day, hour, min, sec);
																/*      012345678901234567*/
																/* JLRGB20121031-010133061.plx*/				
		/* Gets UT from Filename */
					right(logfilename_rac,18,logfilename_tmp);
											if (opts.debug) { fprintf(stderr,"dtcGetDatationFromLogFile: filename|%s|\n", logfilename_tmp); }
					year_tmp=atoi(mid(logfilename_tmp,0,4,tmpline));
					month_tmp=atoi(mid(logfilename_tmp,4,2,tmpline));
					day_tmp=atoi(mid(logfilename_tmp,6,2,tmpline));
					hour_tmp=atoi(mid(logfilename_tmp,9,2,tmpline));
					min_tmp=atoi(mid(logfilename_tmp,11,2,tmpline));
					sec_tmp=strtod(mid(logfilename_tmp,13,2,tmpline),NULL);
					timezone=(int) floor(0.5+((*jd_start_time_loginfo)-gregorian_calendar_to_jd(year_tmp, month_tmp, day_tmp, hour_tmp, min_tmp, sec_tmp))*24);
											if (opts.debug) { fprintf(stderr,"dtcGetDatationFromLogFile: ymdhms=%d.%d.%d.%d.%d.%f timzeone=%d|\n", year_tmp,month_tmp,day_tmp,hour_tmp,min_tmp,sec_tmp,timezone); }
					if ((timezone>=-12) && (timezone<=12)) {
						(*plogtimezone)=UT;
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
/*					(*plogtimezone)=LT;*/
					(*jd_end_time_loginfo)=gregorian_calendar_to_jd(year_end, month_end, day_end, hour_end, min_end, sec_end)-timezone/24.0;
				} else if (strcmp(fieldname,"Origin")==0) {		/*				Origin=PlxCapture 2.2.3.49 */
					if (strlen(value)>11) {
						right(value,strlen(value)-11,software_version_string);
					}
				}
/**************************************************************************************************************/
/* SharpCap                                                                                                 */
/**************************************************************************************************************/
			} else if (strcmp(software,"SharpCap")==0) {			
			}
		}
/**************************************************************************************************************/
/* End                                                                                                        */
/**************************************************************************************************************/
		if (fclose(logfile)!=0) {
			fprintf(stderr, "ERROR in dtcGetDatationFromLogFile closing file %s\n", logfilename);
			exit(EXIT_FAILURE);
		}
		if ((*jd_end_time_loginfo)<(JD_init+1) && ((*jd_start_time_loginfo)>(JD_init+1)) && ((*pDuration)>DURATION_MIN) && ((*pDuration)<=DURATION_MAX)) {
			(*jd_end_time_loginfo)=(*jd_start_time_loginfo)+(*pDuration)/ONE_DAY_SEC;
		} else if ((*jd_start_time_loginfo)<(JD_init+1) && ((*jd_end_time_loginfo)>(JD_init+1)) && ((*pDuration)>DURATION_MIN) && ((*pDuration)<=DURATION_MAX)) {
			(*jd_start_time_loginfo)=(*jd_end_time_loginfo)-(*pDuration)/ONE_DAY_SEC;
		}
		
		strcpy(comment,software);
		if (software_version>0) {
			sprintf(tmpline," %1.1f",software_version);
			strcat(comment,tmpline);
		}
		if (software_beta==0) {
			strcat(comment,"beta");
		}
		if (software_beta>0) {
			sprintf(tmpline,"b%d",software_beta);
			strcat(comment,tmpline);
		}
		if (strlen(software_version_string)>0) {
			sprintf(tmpline," %s",software_version_string);
			strcat(comment,tmpline);
		}
		if (software_version_x86>0) {
			sprintf(tmpline," %2db",software_version_x86);
			strcat(comment,tmpline);
		}

		return EXIT_SUCCESS;
	}
}

/*****************************************************************************************/
/******************************Accessory functions****************************************/
/*****************************************************************************************/

/*****************Calendar to Julian Day***************************/
double gregorian_calendar_to_jd(int y, int m, int d, int hour, int min, double sec)
{
	double jj;
	
	y+=8000;
	if(m<3) { y--; m+=12; }
	
	jj=((sec/60+min)/60+(hour-12))/24;
	
	jj+=(y*365) +(y/4) -(y/100) +(y/400) -1200820+(m*153+3)/5-92+d-1;
	
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
			fprintf(stream,"Unknown");
	}
}

int month_nb(char *month_letter)
{
	int month;
	/* J F M A M J J A S O N D */
	
	if ((strncmp(month_letter,"Jan",3)==0) || (strncmp(month_letter,"jan",3)==0)) {
			month=1;
	} else if ((strncmp(month_letter,"Feb",3)==0) || (strncmp(month_letter,"feb",3)==0) || (strncmp(month_letter,"Fév",3)==0) || (strncmp(month_letter,"fév",3)==0) || (strncmp(month_letter,"F",1)==0) || (strncmp(month_letter,"f",1)==0)) {
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
	} else if ((strncmp(month_letter,"Aug",3)==0) || (strncmp(month_letter,"aug",3)==0) || (strncmp(month_letter,"Aoû",3)==0) || (strncmp(month_letter,"aoû",3)==0) || (strncmp(month_letter,"A",1)==0) || (strncmp(month_letter,"a",1)==0)) {
			month=8;
	} else if ((strncmp(month_letter,"Sep",3)==0) || (strncmp(month_letter,"sep",3)==0) || (strncmp(month_letter,"S",1)==0) || (strncmp(month_letter,"s",1)==0)) {
			month=9;
	} else if ((strncmp(month_letter,"Oct",3)==0) || (strncmp(month_letter,"oct",3)==0) || (strncmp(month_letter,"O",1)==0) || (strncmp(month_letter,"o",1)==0)) {
			month=10;
	} else if ((strncmp(month_letter,"Nov",3)==0) || (strncmp(month_letter,"nov",3)==0) || (strncmp(month_letter,"N",1)==0) || (strncmp(month_letter,"n",1)==0)) {
			month=11;
	} else if ((strncmp(month_letter,"Dec",3)==0) || (strncmp(month_letter,"dec",3)==0) || (strncmp(month_letter,"Déc",3)==0) || (strncmp(month_letter,"déc",3)==0) || (strncmp(month_letter,"D",1)==0) || (strncmp(month_letter,"d",1)==0)) {
			month=12;
	} else {
		month=0;
	}
	return month;
}