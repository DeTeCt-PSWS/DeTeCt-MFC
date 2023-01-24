/*** File fitsfile.h  FITS and IRAF file access subroutines
 *** September 25, 2009
 *** By Jessica Mink, jmink@cfa.harvard.edu
 *** Harvard-Smithsonian Center for Astrophysics
 *** Copyright (C) 1996-2009
 *** Smithsonian Astrophysical Observatory, Cambridge, MA, USA

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.
    
    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

    Correspondence concerning WCSTools should be addressed as follows:
           Internet email: jmink@cfa.harvard.edu
           Postal address: Jessica Mink
                           Smithsonian Astrophysical Observatory
                           60 Garden St.
                           Cambridge, MA 02138 USA

*** Modified by Marc Delcroix for dtc November 2012
***
*** Only 2 functions left (fitrsopen and fitsrhead)
*** parameter type for inpath and filename is const
***

 * Module:      fitsfile_dtc.c (FITS file reading)
 * Purpose:     Read FITS image and table files
 * fitsropen (inpath)
 *		Open a FITS file for reading, returning a FILE pointer
 * fitsrhead (filename, lhead, nbhead)
 *		Read FITS header and return it
 */
 
#ifndef fitsfile_h_
#define fitsfile_h_
#include "common.h"
#include "fits\fitshead.h"

/* Declarations for subroutines in fitsfile.c, imhfile.c, imio.c,
 * fileutil.c, and dateutil.c */

#define FITSBLOCK 2880

/* FITS table keyword structure */
struct Keyword {
    char kname[10];	/* Keyword for table entry */
    int lname;		/* Length of keyword name */
    int kn;		/* Index of entry on line */
    int kf;		/* Index in line of first character of entry */
    int kl;		/* Length of entry value */
    char kform[8];	/* Format for this value */
};

/* Structure for access to tokens within a string */
#define MAXTOKENS 1000    /* Maximum number of tokens to parse */
#define MAXWHITE 20     /* Maximum number of different whitespace characters */
struct Tokens {
    char *line;		/* Line which has been parsed */
    int lline;		/* Number of characters in line */
    int ntok;		/* Number of tokens on line */
    int nwhite;		/* Number of whitespace characters */
    char white[MAXWHITE]; /* Whitespace (separator) characters */
    char *tok1[MAXTOKENS]; /* Pointers to start of tokens */
    int ltok[MAXTOKENS]; /* Lengths of tokens */
    int itok;		/* Current token number */
};

#ifdef __cplusplus /* C++ prototypes */
extern "C" {
#endif


//#ifdef __STDC__   /* Full ANSI prototypes */

/* Declarations for subroutines in fitsfile.c, imhfile.c, imio.c,
 * fileutil.c, and dateutil.c */

/* FITS file access subroutines in fitsfile.c */
    extern int fitsropen(	/* Open a FITS file for reading, returning a FILE pointer */    // test OpenCV 4.7.0 - extern added
	const char *inpath);	/* Pathname for FITS tables file to read */
	
    extern char *fitsrhead(	/* Read a FITS header */    // test OpenCV 4.7.0 extern added
	const char *filename,	/* Name of FITS image file */
	int *lhead,	/* Allocated length of FITS header in bytes (returned) */
	int *nbhead);	/* Number of bytes before start of data (returned) */

/* FITS file access subroutines in fitsfile.c */
//extern int fitsropen();           // test OpenCV 4.7.0 
//extern char *fitsrhead();// test OpenCV 4.7.0 

//#endif  /* __STDC__ */

#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif /* fitsfile_h_ */

/* May 31 1996	Use stream I/O for reading as well as writing
 * Jun 12 1996	Add byte-swapping subroutines
 * Jul 10 1996	FITS header now allocated in subroutines
 * Jul 17 1996	Add FITS table column extraction subroutines
 * Aug  6 1996	Add MOVEPIX, HDEL and HCHANGE declarations
 *
 * Oct 10 1997	FITS file opening subroutines now return int instead of FILE *
 *
 * May 27 1998	Split off fitsio and imhio subroutines to fitsio.h
 * Jun  4 1998	Change fits2iraf from int to int *
 * Jul 24 1998	Make IRAF header char instead of int
 * Aug 18 1998	Change name to fitsfile.h from fitsio.h
 * Oct  5 1998	Add isiraf() and isfits()
 * Oct  7 1998	Note separation of imhfile.c into two files
 *
 * Jul 15 1999	Add fileutil.c subroutines
 * Sep 28 1999	Add (1,1)-based image access subroutines
 * Oct 21 1999	Add fitswhead()
 * Nov  2 1999	Add date utilities from wcscat.h
 * Nov 23 1999	Add fitscimage()
 * Dec 15 1999	Fix misdeclaration of *2fd() subroutines, add fd2i(), dt2i()
 * Dec 20 1999	Add isdate()
 *
 * Jan 20 2000	Add conversions to and from Besselian and Julian epochs
 * Jan 21 2000	Add conversions to old FITS date and time
 * Jan 26 2000	Add conversion to modified Julian date (JD - 2400000.5
 * Mar 22 2000  Add lt2* and ut2* to get current time as local and UT
 * Mar 24 2000	Add tsi2* and tsu2* to convert IRAF and Unix seconds
 * Sep  8 2000	Improve comments
 *
 * Apr 24 2001	Add length of column name to column data structure
 * May 22 2001	Add day of year date conversion subroutines
 * Sep 25 2001	Add isfilelist() and isfile()
 *
 * Jan  8 2002	Add sts2c() and stc2s()
 * Apr  8 2002	Change all long declarations to time_t for compatibility
 * Jun 18 2002	Add fitserr() to print error messages
 * Aug 30 2002	Add Ephemeris Time date conversions
 * Sep 10 2002	Add Sidereal Time conversions
 * Oct 21 2002	Add fitsrsect() to read sections of FITS images
 *
 * Mar  5 2003	Add isimlistd() to check image lists with root directory
 * Aug 20 2003	Add fitsrfull() to read n-dimensional simple FITS images
 *
 * Feb 27 2004  Add fillvec() and fillvec1()
 * May  3 2004	Add setfitsinherit()
 * May  6 2004	Add fitswexhead()
 * Aug 27 2004	Add fitsheadsize()
 *
 * Oct 14 2005	Add tsd2fd(), tsd2dt(), epj2ep(), epb2ep(), tsi2dt()
 *
 * Feb 23 2006	Add fitsrtail() to read appended FITS header
 * Feb 23 2006	Add istiff(), isjpeg(), isgif() to check TIFF, JPEG, GIF files
 * Sep  6 2006	Add heliocentric time conversions
 * Oct  5 2006	Add local sidereal time conversions
 *
 * Jan  9 2007	Add ANSI prototypes
 * Jan 11 2007	Add token subroutines from catutil.c/wcscat.h to fileutil.c
 * Jun 11 2007	Add minvec() subroutine in imio.c
 * Nov 28 2007	Add kform format to FITS table keyword data structure
 *
 * Sep  8 2008	Add ag2hr(), ang2deg(), deg2ang(), and hr2ang()
 *
 * Sep 25 2009	Add moveb()
 */
