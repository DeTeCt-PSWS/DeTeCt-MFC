#ifndef __DATATION_H__
#define __DATATION_H__

//#include <time.h>
//#include "common.h"
#include "wrapper.h"


#define ONE_DAY_SEC	86400.0	/*86400= 24.0*60.0*60.0 */

	enum _TIME_TYPE { LT, UT, Unknown };
	typedef enum _TIME_TYPE TIME_TYPE;

#ifdef __cplusplus 
	extern "C" {
#endif

	/****************************************************************************************************/
	/*									Procedures and functions										*/
	/****************************************************************************************************/

	void 	dtcGetDatation(DtcCapture *capture, char *filename, int nbframes, double *pstart_time, double *pend_time, double *pDuration, double *pfps, TIME_TYPE *ptimetype, char *comment);
	void 	dtcCorrectDatation(DtcCapture *capture, double *pstart_time, double *pend_time, double *pDuration, double *pfps, TIME_TYPE *ptimetype, char *comment);

	//void 	dtcWriteLog(const char *dtcexename, const double start_time, const double end_time, const double duration, const double fps, const TIME_TYPE timetype, const char *filename, const char *comment, const int nb_impact, const int print);

	void 	dtcGetDatationFromFileInfo(DtcCapture *capture, const char *filename, const int nbframes, double *pstart_time, double *pend_time, double *pDuration, double *pfps);
	int		dtcGetDatationFromFilename(const char *filename, double *pstart_time, double *pmid_time);
	int 	dtcGetDatationFromLogFile(const char *filename, double *jd_start_time_loginfo, double *jd_end_time_loginfo, double *pDuration, double *pfps, long *pnbframes, TIME_TYPE *plogtimezone, char *comment);

	double 	gregorian_calendar_to_jd(int y, int m, int d, int hour, int min, double sec);
	void 	jd_to_date(double jj, double *psec, int *pmin, int *phour, int *pday, int *pmonth, int *pyear);

	int	IsDateCorrect(int y, int m, int d, int hour, int min, double sec);
	void 	fprint_jd(FILE *stream, const double jd);
	void 	fprint_jd_wj(FILE *stream, const double jd);
	void 	fprint_timetype(FILE *stream, const TIME_TYPE timetype);
	double 	JD_from_time_t(const time_t time_t_value);
	int 	month_nb(char *month_letter);

#ifdef __cplusplus 
}
#endif
#endif /* __DATATION_H__ */
