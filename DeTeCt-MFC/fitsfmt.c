/********************************************************************************/
/*                                                                              */
/*	DTC	(c) Marc Delcroix (delcroix.marc@free.fr) 2012-							*/
/*                                                                              */
/*    FITSFMT: Handling of individual FITS files acquisitions functions			*/
/*                                                                              */
/********************************************************************************/
#include "common.h"

#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>

#include <opencv/highgui.h>

#include "fitsfmt.h"
#include "wrapper.h"
#include "dtc.h"
#include "datation.h"
#include "fitsfile_dtc.h"


/*****************Reads current image data***************************/		
/* FITS specific */		
size_t fitsImageRead(void *image, const size_t size, const size_t num, FILE *f)
{
	size_t bytesC;
	size_t bytesR = 0;
	char *ptr;
	char tmp;
	long int length;
	
	length=(long int)(size*num);
	ptr = image;
	while (bytesR < num * size)	{
		bytesC = fread(ptr, 1, length, f);
		if (!bytesC)
			return 0;		
		bytesR += bytesC;
		ptr += bytesR;

	}
/* Reverse bytes order for 16bits resolution */
	if (size==2) {
		ptr = image;
		for (long int i = 0; i<length; i = i + 2) {
			tmp=ptr[i];
			ptr[i]=ptr[i+1];
			ptr[i+1]=tmp;
		}
	}
	
	return bytesR;
}

void fitsGet_info(FileCapture *fc, const char *fname, double *date)
{
	char *buffer;
	int lhead=0;
	int nbhead=0;

	buffer=(char *) fitsrhead(fname, &lhead, &nbhead);
	hgeti4(buffer, "NAXIS1", &fc->ImageWidth);
	hgeti4 (buffer,"NAXIS2", &fc->ImageHeight);
	hgeti4 (buffer,"BITPIX", &fc->PixelDepth);
										if (debug_mode) { fprintf(stdout, "fitsGet_info: Header length %d, char %d, row %d, pixeldepth %d \n", nbhead,fc->ImageWidth, fc->ImageHeight, fc->PixelDepth); }
	if (fread(buffer, sizeof (char), nbhead, fc->fh) != (size_t) nbhead)
	{
		 char msgtext[MAX_STRING] = { 0 };										
		snprintf(msgtext, MAX_STRING, "wrong fits header size in %s", fname);
		ErrorExit(TRUE, "wrong fits header size", __func__, msgtext);
	}
	fc->header_size=nbhead;

	(*date)=fitsJD_date(buffer);
	free(buffer);
}

double fitsJD_date(char *buffer)
{
	double jd;
	double date;
	int delta_time_hour;
	int delta_time_min;
	char value[MAX_STRING]			= { 0 };
	char month_letter[MAX_STRING]	= { 0 };
	char tmpline[MAX_STRING]		= { 0 };
	delta_time_hour=0;
	delta_time_min=0;
	date = 0;
	
	jd=gregorian_calendar_to_jd(1,1,1,0,0,0.0);
	if (hgetdate (buffer,"DATE-OBS", &date)==1) {       /* '2012-08-04T06:23:14.618' */
														/* '2015-04-20T21:36:08' */
									if (debug_mode) { fprintf(stdout, "fitsJD_date: First DATE-OBS %f\n", date); }
		jd = gregorian_calendar_to_jd((int)fabs(date), 1, 1, 0, 0, 0.0) + (gregorian_calendar_to_jd((int)fabs(date) + 1, 1, 1, 0, 0, 0.0) - gregorian_calendar_to_jd((int)fabs(date), 1, 1, 0, 0, 0.0))*(date - (int)fabs(date));
	} else if (ksearch (buffer,"TIMESTMP")) {      /* 08/08/2012 04:49:17.751 */
		strcpy(value,hgetc (buffer,"TIMESTMP"));
		replace_str(value, "'", "");
		jd=gregorian_calendar_to_jd(atoi(mid(value,6,4,tmpline)),atoi(mid(value,3,3,tmpline)),atoi(mid(value,0,2,tmpline)),atoi(mid(value,11,2,tmpline)),atoi(mid(value,14,2,tmpline)),strtod(mid(value,17,6,tmpline),NULL));
									if (debug_mode) { fprintf(stdout, "fitsJD_date: First TIMESTMP %s\n", value); }
											
	} else if (ksearch (buffer,"SIMPLE")) {             /*  SIMPLE  =                    T / Java FITS: Thu Jun 03 05:51:20 CST 2010 */
														/*   1        0         0         0         012     890         01234     01  */
														/*  SIMPLE  =                    T / C# FITS: 09/11/2014 15:23:22 */
		left(ksearch (buffer,"SIMPLE"),73,value);
		if (strcmp(left(value,44,tmpline),"SIMPLE  =                    T / Java FITS: ")==0) {
			mid(value,48,3,month_letter);
			/*
			ACDT 	Australian Central Daylight Time 	UTC+10:30
			ACST 	Australian Central Standard Time 	UTC+09:30
			ACT 	ASEAN Common Time 	UTC+08
			ADT 	Atlantic Daylight Time 	UTC-03
			AEDT 	Australian Eastern Daylight Time 	UTC+11
			AEST 	Australian Eastern Standard Time 	UTC+10
			AFT 	Afghanistan Time 	UTC+04:30
			AKDT 	Alaska Daylight Time 	UTC-08
			AKST 	Alaska Standard Time 	UTC-09
			AMST 	Amazon Summer Time (Brazil)[1] 	UTC-03
			AMST 	Armenia Summer Time 	UTC+05
			AMT 	Amazon Time (Brazil)[2] 	UTC-04
			AMT 	Armenia Time 	UTC+04
			ART 	Argentina Time 	UTC-03
			AST 	Arabia Standard Time 	UTC+03
			AST 	Atlantic Standard Time 	UTC-04
			AWDT 	Australian Western Daylight Time 	UTC+09
			AWST 	Australian Western Standard Time 	UTC+08
			AZOST 	Azores Standard Time 	UTC-01
			AZT 	Azerbaijan Time 	UTC+04
			BDT 	Brunei Time 	UTC+08
			BIOT 	British Indian Ocean Time 	UTC+06
			BIT 	Baker Island Time 	UTC-12
			BOT 	Bolivia Time 	UTC-04
			BRT 	Brasilia Time 	UTC-03
			BST 	Bangladesh Standard Time 	UTC+06
			BST 	British Summer Time (British Standard Time from Feb 1968 to Oct 1971) 	UTC+01
			BTT 	Bhutan Time 	UTC+06
			CAT 	Central Africa Time 	UTC+02
			CCT 	Cocos Islands Time 	UTC+06:30
			CDT 	Central Daylight Time (North America) 	UTC-05
			CDT 	Cuba Daylight Time[3] 	UTC-04
			CEDT 	Central European Daylight Time 	UTC+02
			CEST 	Central European Summer Time (Cf. HAEC) 	UTC+02
			CET 	Central European Time 	UTC+01
			CHADT 	Chatham Daylight Time 	UTC+13:45
			CHAST 	Chatham Standard Time 	UTC+12:45
			CHOT 	Choibalsan 	UTC+08
			ChST 	Chamorro Standard Time 	UTC+10
			CHUT 	Chuuk Time 	UTC+10
			CIST 	Clipperton Island Standard Time 	UTC-08
			CIT 	Central Indonesia Time 	UTC+08
			CKT 	Cook Island Time 	UTC-10
			CLST 	Chile Summer Time 	UTC-03
			CLT 	Chile Standard Time 	UTC-04
			COST 	Colombia Summer Time 	UTC-04
			COT 	Colombia Time 	UTC-05
			CST 	Central Standard Time (North America) 	UTC-06
			CST 	China Standard Time 	UTC+08
			CST 	Central Standard Time (Australia) 	UTC+09:30
			CST 	Central Summer Time (Australia) 	UTC+10:30
			CST 	Cuba Standard Time 	UTC-05
			CT 	China time 	UTC+08
			CVT 	Cape Verde Time 	UTC-01
			CWST 	Central Western Standard Time (Australia) 	UTC+08:45
			CXT 	Christmas Island Time 	UTC+07
			DAVT 	Davis Time 	UTC+07
			DDUT 	Dumont d'Urville Time 	UTC+10
			DFT 	AIX specific equivalent of Central European Time[4] 	UTC+01
			EASST 	Easter Island Standard Summer Time 	UTC-05
			EAST 	Easter Island Standard Time 	UTC-06
			EAT 	East Africa Time 	UTC+03
			ECT 	Eastern Caribbean Time (does not recognise DST) 	UTC-04
			ECT 	Ecuador Time 	UTC-05
			EDT 	Eastern Daylight Time (North America) 	UTC-04
			EEDT 	Eastern European Daylight Time 	UTC+03
			EEST 	Eastern European Summer Time 	UTC+03
			EET 	Eastern European Time 	UTC+02
			EGST 	Eastern Greenland Summer Time 	UTC+00
			EGT 	Eastern Greenland Time 	UTC-01
			EIT 	Eastern Indonesian Time 	UTC+09
			EST 	Eastern Standard Time (North America) 	UTC-05
			EST 	Eastern Standard Time (Australia) 	UTC+10
			FET 	Further-eastern European Time 	UTC+03
			FJT 	Fiji Time 	UTC+12
			FKST 	Falkland Islands Standard Time 	UTC-03
			FKST 	Falkland Islands Summer Time 	UTC-03
			FKT 	Falkland Islands Time 	UTC-04
			FNT 	Fernando de Noronha Time 	UTC-02
			GALT 	Galapagos Time 	UTC-06
			GAMT 	Gambier Islands 	UTC-09
			GET 	Georgia Standard Time 	UTC+04
			GFT 	French Guiana Time 	UTC-03
			GILT 	Gilbert Island Time 	UTC+12
			GIT 	Gambier Island Time 	UTC-09
			GMT 	Greenwich Mean Time 	UTC
			GST 	South Georgia and the South Sandwich Islands 	UTC-02
			GST 	Gulf Standard Time 	UTC+04
			GYT 	Guyana Time 	UTC-04
			HADT 	Hawaii-Aleutian Daylight Time 	UTC-09
			HAEC 	Heure Avancée d'Europe Centrale francised name for CEST 	UTC+02
			HAST 	Hawaii-Aleutian Standard Time 	UTC-10
			HKT 	Hong Kong Time 	UTC+08
			HMT 	Heard and McDonald Islands Time 	UTC+05
			HOVT 	Khovd Time 	UTC+07
			HST 	Hawaii Standard Time 	UTC-10
			ICT 	Indochina Time 	UTC+07
			IDT 	Israel Daylight Time 	UTC+03
			IOT 	Indian Ocean Time 	UTC+03
			IRDT 	Iran Daylight Time 	UTC+08
			IRKT 	Irkutsk Time 	UTC+09
			IRST 	Iran Standard Time 	UTC+03:30
			IST 	Indian Standard Time 	UTC+05:30
			IST 	Irish Standard Time[5] 	UTC+01
			IST 	Israel Standard Time 	UTC+02
			JST 	Japan Standard Time 	UTC+09
			KGT 	Kyrgyzstan time 	UTC+06
			KOST 	Kosrae Time 	UTC+11
			KRAT 	Krasnoyarsk Time 	UTC+07
			KST 	Korea Standard Time 	UTC+09
			LHST 	Lord Howe Standard Time 	UTC+10:30
			LHST 	Lord Howe Summer Time 	UTC+11
			LINT 	Line Islands Time 	UTC+14
			MAGT 	Magadan Time 	UTC+12
			MART 	Marquesas Islands Time 	UTC-09:30
			MAWT 	Mawson Station Time 	UTC+05
			MDT 	Mountain Daylight Time (North America) 	UTC-06
			MET 	Middle European Time Same zone as CET 	UTC+01
			MEST 	Middle European Saving Time Same zone as CEST 	UTC+02
			MHT 	Marshall Islands 	UTC+12
			MIST 	Macquarie Island Station Time 	UTC+11
			MIT 	Marquesas Islands Time 	UTC-09:30
			MMT 	Myanmar Time 	UTC+06:30
			MSK 	Moscow Time 	UTC+04
			MST 	Malaysia Standard Time 	UTC+08
			MST 	Mountain Standard Time (North America) 	UTC-07
			MST 	Myanmar Standard Time 	UTC+06:30
			MUT 	Mauritius Time 	UTC+04
			MVT 	Maldives Time 	UTC+05
			MYT 	Malaysia Time 	UTC+08
			NCT 	New Caledonia Time 	UTC+11
			NDT 	Newfoundland Daylight Time 	UTC-02:30
			NFT 	Norfolk Time 	UTC+11:30
			NPT 	Nepal Time 	UTC+05:45
			NST 	Newfoundland Standard Time 	UTC-03:30
			NT 	Newfoundland Time 	UTC-03:30
			NUT 	Niue Time 	UTC-11
			NZDT 	New Zealand Daylight Time 	UTC+13
			NZST 	New Zealand Standard Time 	UTC+12
			OMST 	Omsk Time 	UTC+07
			ORAT 	Oral Time 	UTC+05
			PDT 	Pacific Daylight Time (North America) 	UTC-07
			PET 	Peru Time 	UTC-05
			PETT 	Kamchatka Time 	UTC+12
			PGT 	Papua New Guinea Time 	UTC+10
			PHOT 	Phoenix Island Time 	UTC+13
			PHT 	Philippine Time 	UTC+08
			PKT 	Pakistan Standard Time 	UTC+05
			PMDT 	Saint Pierre and Miquelon Daylight time 	UTC-02
			PMST 	Saint Pierre and Miquelon Standard Time 	UTC-03
			PONT 	Pohnpei Standard Time 	UTC+11
			PST 	Pacific Standard Time (North America) 	UTC-08
			PYST 	Paraguay Summer Time (Brazil)[6] 	UTC-03
			PYT 	Paraguay Time (Brazil)[7] 	UTC-04
			RET 	Réunion Time 	UTC+04
			ROTT 	Rothera Research Station Time 	UTC-03
			SAKT 	Sakhalin Island time 	UTC+11
			SAMT 	Samara Time 	UTC+04
			SAST 	South African Standard Time 	UTC+02
			SBT 	Solomon Islands Time 	UTC+11
			SCT 	Seychelles Time 	UTC+04
			SGT 	Singapore Time 	UTC+08
			SLST 	Sri Lanka Time 	UTC+05:30
			SRT 	Suriname Time 	UTC-03
			SST 	Samoa Standard Time 	UTC-11
			SST 	Singapore Standard Time 	UTC+08
			SYOT 	Showa Station Time 	UTC+03
			TAHT 	Tahiti Time 	UTC-10
			THA 	Thailand Standard Time 	UTC+07
			TFT 	Indian/Kerguelen 	UTC+05
			TJT 	Tajikistan Time 	UTC+05
			TKT 	Tokelau Time 	UTC+14
			TLT 	Timor Leste Time 	UTC+09
			TMT 	Turkmenistan Time 	UTC+05
			TOT 	Tonga Time 	UTC+13
			TVT 	Tuvalu Time 	UTC+12
			UCT 	Coordinated Universal Time 	UTC
			ULAT 	Ulaanbaatar Time 	UTC+08
			UTC 	Coordinated Universal Time 	UTC
			UYST 	Uruguay Summer Time 	UTC-02
			UYT 	Uruguay Standard Time 	UTC-03
			UZT 	Uzbekistan Time 	UTC+05
			VET 	Venezuelan Standard Time 	UTC-04:30
			VLAT 	Vladivostok Time 	UTC+10
			VOLT 	Volgograd Time 	UTC+04
			VOST 	Vostok Station Time 	UTC+06
			VUT 	Vanuatu Time 	UTC+11
			WAKT 	Wake Island Time 	UTC+12
			WAST 	West Africa Summer Time 	UTC+02
			WAT 	West Africa Time 	UTC+01
			WEDT 	Western European Daylight Time 	UTC+01
			WEST 	Western European Summer Time 	UTC+01
			WET 	Western European Time 	UTC
			WST 	Western Standard Time 	UTC+08
			YAKT 	Yakutsk Time 	UTC+10
			YEKT 	Yekaterinburg Time 	UTC+06
			Z 	Zulu Time (Coordinated Universal Time) 	UTC
			*/
			delta_time_min=0;
			if (strcmp(mid(value,64,3,tmpline),"ACD") == 0) {
				delta_time_hour=10;
				delta_time_min=30;
			} else if (strcmp(mid(value,64,3,tmpline),"ACS") == 0) {
				delta_time_hour=9;
				delta_time_min=30;
			} else if (strcmp(mid(value,64,3,tmpline),"ACT") == 0) {
				delta_time_hour=8;
			} else if (strcmp(mid(value,64,3,tmpline),"ADT") == 0) {
				delta_time_hour=-3;
			} else if (strcmp(mid(value,64,3,tmpline),"AED") == 0) {
				delta_time_hour=11;
			} else if (strcmp(mid(value,64,3,tmpline),"AES") == 0) {
				delta_time_hour=10;
			} else if (strcmp(mid(value,64,3,tmpline),"AFT") == 0) {
				delta_time_hour=4;
				delta_time_min=30;
			} else if (strcmp(mid(value,64,3,tmpline),"AKD") == 0) {
				delta_time_hour=-8;
			} else if (strcmp(mid(value,64,3,tmpline),"AKS") == 0) {
				delta_time_hour=-7;
			} else if (strcmp(mid(value,64,3,tmpline),"AMS") == 0) {
				delta_time_hour=-3; /* 5 */
			} else if (strcmp(mid(value,64,3,tmpline),"AMT") == 0) {
				delta_time_hour=-4; /* 4 */
			} else if (strcmp(mid(value,64,3,tmpline),"ART") == 0) {
				delta_time_hour=-3;
			} else if (strcmp(mid(value,64,3,tmpline),"AST") == 0) {
				delta_time_hour=3; /* -4 */
			} else if (strcmp(mid(value,64,3,tmpline),"AWD") == 0) {
				delta_time_hour=9;
			} else if (strcmp(mid(value,64,3,tmpline),"AWS") == 0) {
				delta_time_hour=8;
			} else if (strcmp(mid(value,64,3,tmpline),"AZO") == 0) {
				delta_time_hour=-1;
			} else if (strcmp(mid(value,64,3,tmpline),"AZT") == 0) {
				delta_time_hour=4;
			} else if (strcmp(mid(value,64,3,tmpline),"BDT") == 0) {
				delta_time_hour=8;
			} else if (strcmp(mid(value,64,3,tmpline),"BIO") == 0) {
				delta_time_hour=6;
			} else if (strcmp(mid(value,64,3,tmpline),"BIT") == 0) {
				delta_time_hour=-12;
			} else if (strcmp(mid(value,64,3,tmpline),"BOT") == 0) {
				delta_time_hour=-4;
			} else if (strcmp(mid(value,64,3,tmpline),"BRT") == 0) {
				delta_time_hour=-3;
			} else if (strcmp(mid(value,64,3,tmpline),"BST") == 0) {
				delta_time_hour=1; /* 6 */
			} else if (strcmp(mid(value,64,3,tmpline),"BTT") == 0) {
				delta_time_hour=6;
			} else if (strcmp(mid(value,64,3,tmpline),"CAT") == 0) {
				delta_time_hour=2;
			} else if (strcmp(mid(value,64,3,tmpline),"CCT") == 0) {
				delta_time_hour=6;
				delta_time_min=30;
			} else if (strcmp(mid(value,64,3,tmpline),"CDT") == 0) {
				delta_time_hour=-5; /* -4 */
			} else if (strcmp(mid(value,64,3,tmpline),"CED") == 0) {
				delta_time_hour=2;
			} else if (strcmp(mid(value,64,3,tmpline),"CES") == 0) {
				delta_time_hour=2;
			} else if (strcmp(mid(value,64,3,tmpline),"CET") == 0) {
				delta_time_hour=1;
			} else if (strcmp(mid(value,64,3,tmpline),"CHA") == 0) {
				delta_time_hour=13;
				delta_time_min=45;
			} else if (strcmp(mid(value,64,4,tmpline),"CHAS") == 0) {
				delta_time_hour=13; /* 12 */
				delta_time_min=45;
			} else if (strcmp(mid(value,64,4,tmpline),"CHAS") == 0) {
				delta_time_hour=12;
				delta_time_min=45;
			} else if (strcmp(mid(value,64,3,tmpline),"CHO") == 0) {
				delta_time_hour=8;
			} else if (strcmp(mid(value,64,3,tmpline),"CHS") == 0) {
				delta_time_hour=10;
			} else if (strcmp(mid(value,64,3,tmpline),"CHU") == 0) {
				delta_time_hour=10;
			} else if (strcmp(mid(value,64,3,tmpline),"CIS") == 0) {
				delta_time_hour=-8;
			} else if (strcmp(mid(value,64,3,tmpline),"CKT") == 0) {
				delta_time_hour=-10;
			} else if (strcmp(mid(value,64,3,tmpline),"CLS") == 0) {
				delta_time_hour=-3;
			} else if (strcmp(mid(value,64,3,tmpline),"COS") == 0) {
				delta_time_hour=-4;
			} else if (strcmp(mid(value,64,3,tmpline),"COT") == 0) {
				delta_time_hour=-5;
			} else if (strcmp(mid(value,64,3,tmpline),"CST") == 0) {
				delta_time_hour=-6; /* 8, 9:30, 10:30, -5 */
			} else if (strcmp(mid(value,64,2,tmpline),"CT") == 0) {
				delta_time_hour=8;
			} else if (strcmp(mid(value,64,3,tmpline),"CVT") == 0) {
				delta_time_hour=-1;
			} else if (strcmp(mid(value,64,3,tmpline),"CWS") == 0) {
				delta_time_hour=8;
				delta_time_min=45;
			} else if (strcmp(mid(value,64,3,tmpline),"CXT") == 0) {
				delta_time_hour=7;
			} else if (strcmp(mid(value,64,3,tmpline),"DAV") == 0) {
				delta_time_hour=7;
			} else if (strcmp(mid(value,64,3,tmpline),"DDU") == 0) {
				delta_time_hour=10;
			} else if (strcmp(mid(value,64,3,tmpline),"DFT") == 0) {
				delta_time_hour=1;
			} else if (strcmp(mid(value,64,3,tmpline),"EAS") == 0) {
				delta_time_hour=-5; /* -6 */
			} else if (strcmp(mid(value,64,4,tmpline),"EASS") == 0) {
				delta_time_hour=-5;
			} else if (strcmp(mid(value,64,4,tmpline),"EAST") == 0) {
				delta_time_hour=-6;
			} else if (strcmp(mid(value,64,3,tmpline),"EAT") == 0) {
				delta_time_hour=3;
			} else if (strcmp(mid(value,64,3,tmpline),"ECT") == 0) {
				delta_time_hour=-4; /* -5*/
			} else if (strcmp(mid(value,64,3,tmpline),"EDT") == 0) {
				delta_time_hour=-4;
			} else if (strcmp(mid(value,64,3,tmpline),"EED") == 0) {
				delta_time_hour=3;
			} else if (strcmp(mid(value,64,3,tmpline),"EES") == 0) {
				delta_time_hour=3;
			} else if (strcmp(mid(value,64,3,tmpline),"EET") == 0) {
				delta_time_hour=2;
			} else if (strcmp(mid(value,64,3,tmpline),"EGS") == 0) {
				delta_time_hour=0;
			} else if (strcmp(mid(value,64,3,tmpline),"EGT") == 0) {
				delta_time_hour=-1;
			} else if (strcmp(mid(value,64,3,tmpline),"EIT") == 0) {
				delta_time_hour=9;
			} else if (strcmp(mid(value,64,3,tmpline),"EST") == 0) {
				delta_time_hour=-5; /* 10 */
			} else if (strcmp(mid(value,64,3,tmpline),"FET") == 0) {
				delta_time_hour=3;
			} else if (strcmp(mid(value,64,3,tmpline),"FJT") == 0) {
				delta_time_hour=12;
			} else if (strcmp(mid(value,64,3,tmpline),"FKS") == 0) {
				delta_time_hour=-3;
			} else if (strcmp(mid(value,64,3,tmpline),"FKT") == 0) {
				delta_time_hour=-4;
			} else if (strcmp(mid(value,64,3,tmpline),"FNT") == 0) {
				delta_time_hour=-2;
			} else if (strcmp(mid(value,64,3,tmpline),"GAL") == 0) {
				delta_time_hour=-6;
			} else if (strcmp(mid(value,64,3,tmpline),"GAM") == 0) {
				delta_time_hour=-9;
			} else if (strcmp(mid(value,64,3,tmpline),"GET") == 0) {
				delta_time_hour=4;
			} else if (strcmp(mid(value,64,3,tmpline),"GFT") == 0) {
				delta_time_hour=-3;
			} else if (strcmp(mid(value,64,3,tmpline),"GIL") == 0) {
				delta_time_hour=12;
			} else if (strcmp(mid(value,64,3,tmpline),"GIT") == 0) {
				delta_time_hour=-9;
			} else if (strcmp(mid(value,64,3,tmpline),"GMT") == 0) {
				delta_time_hour=0;
			} else if (strcmp(mid(value,64,3,tmpline),"GST") == 0) {
				delta_time_hour=4; /* -2 */
			} else if (strcmp(mid(value,64,3,tmpline),"GYT") == 0) {
				delta_time_hour=-4;
			} else if (strcmp(mid(value,64,3,tmpline),"HAD") == 0) {
				delta_time_hour=-9;
			} else if (strcmp(mid(value,64,3,tmpline),"HAE") == 0) {
				delta_time_hour=2;
			} else if (strcmp(mid(value,64,3,tmpline),"HAS") == 0) {
				delta_time_hour=-10;
			} else if (strcmp(mid(value,64,3,tmpline),"HKT") == 0) {
				delta_time_hour=8;
			} else if (strcmp(mid(value,64,3,tmpline),"HMT") == 0) {
				delta_time_hour=5;
			} else if (strcmp(mid(value,64,3,tmpline),"HOV") == 0) {
				delta_time_hour=7;
			} else if (strcmp(mid(value,64,3,tmpline),"HST") == 0) {
				delta_time_hour=-10;
			} else if (strcmp(mid(value,64,3,tmpline),"ICT") == 0) {
				delta_time_hour=7;
			} else if (strcmp(mid(value,64,3,tmpline),"IDT") == 0) {
				delta_time_hour=3;
			} else if (strcmp(mid(value,64,3,tmpline),"IOT") == 0) {
				delta_time_hour=3;
			} else if (strcmp(mid(value,64,3,tmpline),"IRD") == 0) {
				delta_time_hour=8;
			} else if (strcmp(mid(value,64,3,tmpline),"IRK") == 0) {
				delta_time_hour=9;
			} else if (strcmp(mid(value,64,3,tmpline),"IRS") == 0) {
				delta_time_hour=3;
				delta_time_min=30;
			} else if (strcmp(mid(value,64,3,tmpline),"IST") == 0) {
				delta_time_hour=1; /* 2, 5:30 */
			} else if (strcmp(mid(value,64,3,tmpline),"JST") == 0) {
				delta_time_hour=9;
			} else if (strcmp(mid(value,64,3,tmpline),"KGT") == 0) {
				delta_time_hour=6;
			} else if (strcmp(mid(value,64,3,tmpline),"KOS") == 0) {
				delta_time_hour=11;
			} else if (strcmp(mid(value,64,3,tmpline),"KRA") == 0) {
				delta_time_hour=7;
			} else if (strcmp(mid(value,64,3,tmpline),"KST") == 0) {
				delta_time_hour=9;
			} else if (strcmp(mid(value,64,3,tmpline),"KUY") == 0) {
				delta_time_hour=4;
			} else if (strcmp(mid(value,64,3,tmpline),"LHS") == 0) {
				delta_time_hour=11; /* 10:30 */
			} else if (strcmp(mid(value,64,3,tmpline),"LIN") == 0) {
				delta_time_hour=14;
			} else if (strcmp(mid(value,64,3,tmpline),"MAG") == 0) {
				delta_time_hour=12;
			} else if (strcmp(mid(value,64,3,tmpline),"MAR") == 0) {
				delta_time_hour=-9;
				delta_time_min=-30;
			} else if (strcmp(mid(value,64,3,tmpline),"MAW") == 0) {
				delta_time_hour=5;
			} else if (strcmp(mid(value,64,3,tmpline),"MDT") == 0) {
				delta_time_hour=-6;
			} else if (strcmp(mid(value,64,3,tmpline),"MES") == 0) {
				delta_time_hour=2;
			} else if (strcmp(mid(value,64,3,tmpline),"MET") == 0) {
				delta_time_hour=1;
			} else if (strcmp(mid(value,64,3,tmpline),"MHT") == 0) {
				delta_time_hour=12;
			} else if (strcmp(mid(value,64,3,tmpline),"MIS") == 0) {
				delta_time_hour=11;
			} else if (strcmp(mid(value,64,3,tmpline),"MIT") == 0) {
				delta_time_hour=-9;
				delta_time_min=-30;
			} else if (strcmp(mid(value,64,3,tmpline),"MMT") == 0) {
				delta_time_hour=6;
				delta_time_min=30;
			} else if (strcmp(mid(value,64,3,tmpline),"MSD") == 0) {
				delta_time_hour=4;
			} else if (strcmp(mid(value,64,3,tmpline),"MSK") == 0) {
				delta_time_hour=4;
			} else if (strcmp(mid(value,64,3,tmpline),"MST") == 0) {
				delta_time_hour=-7; /* 8, 6:30 */
			} else if (strcmp(mid(value,64,3,tmpline),"MUT") == 0) {
				delta_time_hour=4;
			} else if (strcmp(mid(value,64,3,tmpline),"MVT") == 0) {
				delta_time_hour=5;
			} else if (strcmp(mid(value,64,3,tmpline),"MYT") == 0) {
				delta_time_hour=8;
			} else if (strcmp(mid(value,64,3,tmpline),"NCT") == 0) {
				delta_time_hour=11;
			} else if (strcmp(mid(value,64,3,tmpline),"NDT") == 0) {
				delta_time_hour=-2;
				delta_time_min=-30;
			} else if (strcmp(mid(value,64,3,tmpline),"NFT") == 0) {
				delta_time_hour=11;
				delta_time_min=30;
			} else if (strcmp(mid(value,64,3,tmpline),"NPT") == 0) {
				delta_time_hour=5;
				delta_time_min=45;
			} else if (strcmp(mid(value,64,3,tmpline),"NST") == 0) {
				delta_time_hour=-3;
				delta_time_min=-30;
			} else if (strcmp(mid(value,64,2,tmpline),"NT") == 0) {
				delta_time_hour=-3;
				delta_time_min=-30;
			} else if (strcmp(mid(value,64,3,tmpline),"NUT") == 0) {
				delta_time_hour=-11;
			} else if (strcmp(mid(value,64,3,tmpline),"NZD") == 0) {
				delta_time_hour=13;
			} else if (strcmp(mid(value,64,3,tmpline),"NZS") == 0) {
				delta_time_hour=12;
			} else if (strcmp(mid(value,64,3,tmpline),"OMS") == 0) {
				delta_time_hour=7;
			} else if (strcmp(mid(value,64,3,tmpline),"ORA") == 0) {
				delta_time_hour=5;
			} else if (strcmp(mid(value,64,3,tmpline),"PDT") == 0) {
				delta_time_hour=-7;
			} else if (strcmp(mid(value,64,3,tmpline),"PET") == 0) {
				delta_time_hour=-5;
			} else if (strcmp(mid(value,64,4,tmpline),"PETT") == 0) {
				delta_time_hour=12;
			} else if (strcmp(mid(value,64,3,tmpline),"PGT") == 0) {
				delta_time_hour=10;
			} else if (strcmp(mid(value,64,3,tmpline),"PHO") == 0) {
				delta_time_hour=13;
			} else if (strcmp(mid(value,64,3,tmpline),"PHT") == 0) {
				delta_time_hour=8;
			} else if (strcmp(mid(value,64,3,tmpline),"PKT") == 0) {
				delta_time_hour=5;
			} else if (strcmp(mid(value,64,3,tmpline),"PMD") == 0) {
				delta_time_hour=-2;
			} else if (strcmp(mid(value,64,3,tmpline),"PMS") == 0) {
				delta_time_hour=-3;
			} else if (strcmp(mid(value,64,3,tmpline),"PON") == 0) {
				delta_time_hour=11;
			} else if (strcmp(mid(value,64,3,tmpline),"PST") == 0) {
				delta_time_hour=-8;
			} else if (strcmp(mid(value,64,3,tmpline),"PYS") == 0) {
				delta_time_hour=-3;
			} else if (strcmp(mid(value,64,3,tmpline),"PYT") == 0) {
				delta_time_hour=-4;
			} else if (strcmp(mid(value,64,3,tmpline),"RET") == 0) {
				delta_time_hour=4;
			} else if (strcmp(mid(value,64,3,tmpline),"ROT") == 0) {
				delta_time_hour=-3;
			} else if (strcmp(mid(value,64,3,tmpline),"SAK") == 0) {
				delta_time_hour=11;
			} else if (strcmp(mid(value,64,3,tmpline),"SAM") == 0) {
				delta_time_hour=4;
			} else if (strcmp(mid(value,64,3,tmpline),"SAS") == 0) {
				delta_time_hour=2;
			} else if (strcmp(mid(value,64,3,tmpline),"SBT") == 0) {
				delta_time_hour=11;
			} else if (strcmp(mid(value,64,3,tmpline),"SCT") == 0) {
				delta_time_hour=4;
			} else if (strcmp(mid(value,64,3,tmpline),"SGT") == 0) {
				delta_time_hour=8;
			} else if (strcmp(mid(value,64,3,tmpline),"SLS") == 0) {
				delta_time_hour=5;
				delta_time_min=30;
			} else if (strcmp(mid(value,64,3,tmpline),"SRT") == 0) {
				delta_time_hour=-3;
			} else if (strcmp(mid(value,64,3,tmpline),"SST") == 0) {
				delta_time_hour=8; /* -11 */
			} else if (strcmp(mid(value,64,3,tmpline),"SYO") == 0) {
				delta_time_hour=3;
			} else if (strcmp(mid(value,64,3,tmpline),"TAH") == 0) {
				delta_time_hour=-10;
			} else if (strcmp(mid(value,64,3,tmpline),"THA") == 0) {
				delta_time_hour=7;
			} else if (strcmp(mid(value,64,3,tmpline),"TFT") == 0) {
				delta_time_hour=5;
			} else if (strcmp(mid(value,64,3,tmpline),"TJT") == 0) {
				delta_time_hour=5;
			} else if (strcmp(mid(value,64,3,tmpline),"TKT") == 0) {
				delta_time_hour=14;
			} else if (strcmp(mid(value,64,3,tmpline),"TLT") == 0) {
				delta_time_hour=9;
			} else if (strcmp(mid(value,64,3,tmpline),"TMT") == 0) {
				delta_time_hour=5;
			} else if (strcmp(mid(value,64,3,tmpline),"TOT") == 0) {
				delta_time_hour=13;
			} else if (strcmp(mid(value,64,3,tmpline),"TVT") == 0) {
				delta_time_hour=12;
			} else if (strcmp(mid(value,64,3,tmpline),"UCT") == 0) {
				delta_time_hour=0;
			} else if (strcmp(mid(value,64,3,tmpline),"ULA") == 0) {
				delta_time_hour=8;
			} else if (strcmp(mid(value,64,3,tmpline),"UTC") == 0) {
				delta_time_hour=0;
			} else if (strcmp(mid(value,64,3,tmpline),"UYS") == 0) {
				delta_time_hour=-2;
			} else if (strcmp(mid(value,64,3,tmpline),"UYT") == 0) {
				delta_time_hour=-3;
			} else if (strcmp(mid(value,64,3,tmpline),"UZT") == 0) {
				delta_time_hour=5;
			} else if (strcmp(mid(value,64,3,tmpline),"VET") == 0) {
				delta_time_hour=-4;
				delta_time_min=-30;
			} else if (strcmp(mid(value,64,3,tmpline),"VLA") == 0) {
				delta_time_hour=10;
			} else if (strcmp(mid(value,64,3,tmpline),"VOL") == 0) {
				delta_time_hour=4;
			} else if (strcmp(mid(value,64,3,tmpline),"VOS") == 0) {
				delta_time_hour=6;
			} else if (strcmp(mid(value,64,3,tmpline),"VUT") == 0) {
				delta_time_hour=11;
			} else if (strcmp(mid(value,64,3,tmpline),"WAK") == 0) {
				delta_time_hour=12;
			} else if (strcmp(mid(value,64,3,tmpline),"WAS") == 0) {
				delta_time_hour=2;
			} else if (strcmp(mid(value,64,3,tmpline),"WAT") == 0) {
				delta_time_hour=1;
			} else if (strcmp(mid(value,64,3,tmpline),"WDT") == 0) {
				delta_time_hour=9;
			} else if (strcmp(mid(value,64,3,tmpline),"WED") == 0) {
				delta_time_hour=1;
			} else if (strcmp(mid(value,64,3,tmpline),"WES") == 0) {
				delta_time_hour=1;
			} else if (strcmp(mid(value,64,3,tmpline),"WET") == 0) {
				delta_time_hour=0;
			} else if (strcmp(mid(value,64,3,tmpline),"WEZ") == 0) {
				delta_time_hour=0;
			} else if (strcmp(mid(value,64,3,tmpline),"WST") == 0) {
				delta_time_hour=8;
			} else if (strcmp(mid(value,64,3,tmpline),"YAK") == 0) {
				delta_time_hour=10;
			} else if (strcmp(mid(value,64,3,tmpline),"YEK") == 0) {
				delta_time_hour=6;
			} else if (strcmp(mid(value,64,1,tmpline),"Z") == 0) {
				delta_time_hour=0;
			}
			if ((isnum(mid(value,68,4,tmpline))) & (month_nb(month_letter)>0) & (isnum(mid(value,52,2,tmpline))) & (isnum(mid(value,55,2,tmpline))) & (isnum(mid(value,58,2,tmpline))) & (isnum(mid(value,61,2,tmpline)))) {
				jd=gregorian_calendar_to_jd(atoi(mid(value,68,4,tmpline)),month_nb(month_letter),atoi(mid(value,52,2,tmpline)),atoi(mid(value,55,2,tmpline))-delta_time_hour,atoi(mid(value,58,2,tmpline))-delta_time_min,strtod(mid(value,61,2,tmpline),NULL));
			}
										if (debug_mode) { fprintf(stdout, "fitsJD_date: First SIMPLE = %s\n", value); }
		} else if (strcmp(left(value,42,tmpline),"SIMPLE  =                    T / C# FITS: ")==0) { 														
																											/*  SIMPLE  =                    T / C# FITS: 09/11/2014 15:23:22 */
																											/*   1        0         0         0         012  5  8 0  3  6  90 */
			if ((isnum(mid(value,48,4,tmpline))) & (isnum(mid(value,45,2,tmpline))) & (isnum(mid(value,42,2,tmpline))) & (isnum(mid(value,53,2,tmpline))) & (isnum(mid(value,56,2,tmpline))) & (isnum(mid(value,59,2,tmpline)))) {
				jd=gregorian_calendar_to_jd(atoi(mid(value,48,4,tmpline)),atoi(mid(value,45,2,tmpline)),atoi(mid(value,42,2,tmpline)),atoi(mid(value,53,2,tmpline)),atoi(mid(value,56,2,tmpline)),strtod(mid(value,59,2,tmpline),NULL));
			}
		} else if (hgetdate(buffer, "DATE", &date) == 1) {       /* '2012-08-04T06:23:14.618' */
			/* '2015-04-20T21:36:08' */
			if (debug_mode) { fprintf(stdout, "fitsJD_date: First DATE %f\n", date); }
			jd = gregorian_calendar_to_jd((int)fabs(date), 1, 1, 0, 0, 0.0) + (gregorian_calendar_to_jd((int)fabs(date) + 1, 1, 1, 0, 0, 0.0) - gregorian_calendar_to_jd((int)fabs(date), 1, 1, 0, 0, 0.0))*(date - (int)fabs(date));
		}
	}
	
	return jd;
}