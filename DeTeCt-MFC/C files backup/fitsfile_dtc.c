/*** File libwcs/fitsfile.c
 *** September 15, 2011
 *** By Jessica Mink, jmink@cfa.harvard.edu
 *** Harvard-Smithsonian Center for Astrophysics
 *** Copyright (C) 1996-2011
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
#include "common.h"

#include <stdlib.h>
#ifndef VMS
	#if !defined(_MSC_VER)
		#include <unistd.h>
	#endif
#endif
#include <stdio.h>
#include <fcntl.h>
#if defined(_MSC_VER)
	#include <io.h>
#else
	#include <sys/file.h>
#endif
#include <errno.h>
#include <string.h>

#include "fitsfile_dtc.h"

static int verbose=0;		/* Print diagnostics */
static char fitserrmsg[80];
static int fitsinherit = 1;	/* Append primary header to extension header */
void
setfitsinherit (int inh)
{fitsinherit = inh; return;}

static off_t ibhead = 0;	/* Number of bytes read before header starts */

off_t
getfitsskip()
{return (ibhead);}

/* FITSRHEAD -- Read a FITS header */

char *
fitsrhead (filename, lhead, nbhead)

const char	*filename;	/* Name of FITS image file */
int	*lhead;		/* Allocated length of FITS header in bytes (returned) */
int	*nbhead;	/* Number of bytes before start of data (returned) */
			/* This includes all skipped image extensions */

{
    int fd;
    char *header;	/* FITS image header (filled) */
    int extend;
    int nbytes,naxis, i;
    int ntry,nbr,irec,nrec, nbh, ipos, npos, nbprim, lprim, lext;
    int nax1, nax2, nax3, nax4, nbpix, ibpix, nblock, nbskip;
    char fitsbuf[2884];
    char *headend;	/* Pointer to last line of header */
    char *headnext;	/* Pointer to next line of header to be added */
    int hdu;		/* header/data unit counter */
    int extnum;		/* desired header data number
			   (0=primary -1=first with data -2=use EXTNAME) */
    char extname[32];	/* FITS extension name */
    char extnam[32];	/* Desired FITS extension name */
    char *ext;		/* FITS extension name or number in header, if any */
    char *pheader;	/* Primary header (naxis is 0) */
    char cext = 0;
    char *rbrac;	/* Pointer to right bracket if present in file name */
    char *mwcs;		/* Pointer to WCS name separated by % */
    char *newhead;	/* New larger header */
    int nbh0;		/* Length of old too small header */
    char *pheadend;
    int inherit = 1;	/* Value of INHERIT keyword in FITS extension header */
    int extfound = 0;	/* Set to one if desired FITS extension is found */
    int npcount;

    pheader = NULL;
    lprim = 0;
    header = NULL;
	nbr = 0;
	nbprim=0;
	strcpy(extnam, "");

    /* Check for FITS WCS specification and ignore for file opening */
    mwcs = strchr (filename, '%');
    if (mwcs != NULL)
	*mwcs = (char) 0;

    /* Check for FITS extension and ignore for file opening */
    rbrac = NULL;
    ext = strchr (filename, ',');
    if (ext == NULL) {
	ext = strchr (filename, '[');
	if (ext != NULL) {
	    rbrac = strchr (filename, ']');
	    if (rbrac != NULL)
		*rbrac = (char) 0;
	    }
	}
    if (ext != NULL) {
	cext = *ext;
	*ext = (char) 0;
	}

    /* Open the image file and read the header */
    if (strncasecmp (filename,"stdin",5)) {
	fd = -1;
	fd = fitsropen (filename);
	}
#ifndef VMS
    else {
	fd = STDIN_FILENO;
	extnum = -1;
	}
#endif

    if (ext != NULL) {
	if (isnum (ext+1))
	    extnum = atoi (ext+1);
	else {
	    extnum = -2;
	    strcpy (extnam, ext+1);
	    }
	}
    else
	extnum = -1;

    /* Repair the damage done to the file-name string during parsing */
    if (ext != NULL)
	*ext = cext;
    if (rbrac != NULL)
	*rbrac = ']';
    if (mwcs != NULL)
	*mwcs = '%';

    if (fd < 0) {
	fprintf (stderr,"FITSRHEAD:  cannot read file %s\n", filename);
	return (NULL);
	}

    nbytes = FITSBLOCK;
    *nbhead = 0;
    headend = NULL;
    nbh = FITSBLOCK * 20 + 4;
    header = (char *) calloc ((unsigned int) nbh, 1);
    (void) hlength (header, nbh);
    headnext = header;
    nrec = 1;
    hdu = 0;
    ibhead = 0;

    /* Read FITS header from input file one FITS block at a time */
    irec = 0;
    ibhead = 0;
    while (irec < 200) {
	nbytes = FITSBLOCK;
	for (ntry = 0; ntry < 10; ntry++) {
	    for (i = 0; i < 2884; i++) fitsbuf[i] = 0;
	    nbr = read (fd, fitsbuf, nbytes);

	    /* Short records allowed only if they have the last header line */
	    if (nbr < nbytes) {
		headend = ksearch (fitsbuf,"END");
		if (headend == NULL) {
		    if (ntry < 9) {
			if (verbose)
			    fprintf (stderr,"FITSRHEAD: %d / %d bytes read %d\n",
				     nbr,nbytes,ntry);
			}
		    else {
			snprintf(fitserrmsg,79,"FITSRHEAD: '%d / %d bytes of header read from %s\n"
				,nbr,nbytes,filename);
#ifndef VMS
			if (fd != STDIN_FILENO)
#endif
			    (void)close (fd);
			free (header);
			/* if (pheader != NULL)
			    return (pheader); */
    			if (extnum != -1 && !extfound) {
			    *ext = (char) 0;
			    if (extnum < 0) {
	    			snprintf (fitserrmsg,79,
				    "FITSRHEAD: Extension %s not found in file %s",
				    extnam, filename);
				}
			    else {
	    			snprintf (fitserrmsg,79,
				    "FITSRHEAD: Extension %d not found in file %s",
				    extnum, filename);
				}
			    *ext = cext;
			    }
			else if (hdu > 0) {
	    		    snprintf (fitserrmsg,79,
				"FITSRHEAD: No extensions found in file %s", filename);
			    hdu = 0;
			    if (pheader != NULL) {
				*lhead = nbprim;
				*nbhead = nbprim;
				return (pheader);
				}
			    break;
			    }
			else {
	    		    snprintf (fitserrmsg,79,
				"FITSRHEAD: No header found in file %s", filename);
			    }
			return (NULL);
			}
		    }
		else
		    break;
		}
	    else
		break;
	    }

	/* Move current FITS record into header string */
	for (i = 0; i < 2880; i++)
	    if (fitsbuf[i] < 32 || i > nbr) fitsbuf[i] = 32;
	if (nbr < 2880)
	    nbr = 2880;
	strncpy (headnext, fitsbuf, nbr);
	*nbhead = *nbhead + nbr;
	nrec = nrec + 1;
	*(headnext+nbr) = 0;
	ibhead = ibhead + 2880;

	/* Check to see if this is the final record in this header */
	headend = ksearch (fitsbuf,"END");
	if (headend == NULL) {

	    /* Increase size of header buffer by 4 blocks if too small */
	    if (nrec * FITSBLOCK > nbh) {
		nbh0 = nbh;
		nbh = (nrec + 4) * FITSBLOCK + 4;
		newhead = (char *) calloc (1,(unsigned int) nbh);
		for (i = 0; i < nbh0; i++)
		    newhead[i] = header[i];
		free (header);
		header = newhead;
		(void) hlength (header, nbh);
		headnext = header + *nbhead - FITSBLOCK;
		}
	    headnext = headnext + FITSBLOCK;
	    }

	else {
	    naxis = 0;
	    hgeti4 (header,"NAXIS",&naxis);

	    /* If header has no data, save it for appending to desired header */
	    if (naxis < 1) {
		nbprim = nrec * FITSBLOCK;
		headend = ksearch (header,"END");
		lprim = headend + 80 - header;
		pheader = (char *) calloc ((unsigned int) nbprim, 1);
		if (pheader == NULL) {
			assert(pheader != NULL);
		} else {
			if (header == NULL) {
				assert(header != NULL);
			} else {
				for (i = 0; i < lprim; i++)
					pheader[i] = header[i];
				strncpy(pheader, header, lprim);
			}
		}
		}

	    /* If header has no data, start with the next record */
	    if (naxis < 1 && extnum == -1) {
		extend = 0;
		hgetl (header,"EXTEND",&extend);
		if (naxis == 0 && extend) {
		    headnext = header;
		    *headend = ' ';
		    headend = NULL;
		    nrec = 1;
		    hdu = hdu + 1;
		    }
		else {
		    break;
		    }
		}

	    /* If this is the desired header data unit, keep it */
	    else if (extnum != -1) {
		if (extnum > -1 && hdu == extnum) {
		    extfound = 1;
		    break;
		    }
		else if (extnum < 0) {
		    extname[0] = 0;
		    hgets (header, "EXTNAME", 32, extname);
		    if (!strcmp (extnam,extname)) {
			extfound = 1;
			break;
			}
		    }

		/* If this is not desired header data unit, skip over data */
		hdu = hdu + 1;
		nblock = 0;
		ibhead = 0;
		if (naxis > 0) {
		    ibpix = 0;
		    hgeti4 (header,"BITPIX",&ibpix);
		    if (ibpix < 0) {
			nbpix = -ibpix / 8;
			}
		    else {
			nbpix = ibpix / 8;
			}
		    nax1 = 1;
		    hgeti4 (header,"NAXIS1",&nax1);
		    nax2 = 1;
		    if (naxis > 1) {
			hgeti4 (header,"NAXIS2",&nax2);
			}
		    nax3 = 1;
		    if (naxis > 2) {
			hgeti4 (header,"NAXIS3",&nax3);
			}
		    nax4 = 1;
		    if (naxis > 3) {
			hgeti4 (header,"NAXIS4",&nax4);
			}
		    nbskip = nax1 * nax2 * nax3 * nax4 * nbpix;
		    nblock = nbskip / 2880;
		    if (nblock*2880 < nbskip) {
			nblock = nblock + 1;
			}
		    npcount = 0;
		    hgeti4 (header,"PCOUNT", &npcount);
		    if (npcount > 0) {
			nbskip = nbskip + npcount;
			nblock = nbskip / 2880;
			if (nblock*2880 < nbskip)
			    nblock = nblock + 1;
			}
		    }
		else {
		    nblock = 0;
		    }
		*nbhead = *nbhead + (nblock * 2880);

		/* Set file pointer to beginning of next header/data unit */
		if (nblock > 0) {
#ifndef VMS
		    if (fd != STDIN_FILENO) {
			ipos = lseek (fd, *nbhead, SEEK_SET);
			npos = *nbhead;
			}
		    else {
#else
			{
#endif
			ipos = 0;
			for (i = 0; i < nblock; i++) {
			    nbytes = FITSBLOCK;
			    nbr = read (fd, fitsbuf, nbytes);
			    if (nbr < nbytes) {
				ipos = ipos + nbr;
				break;
				}
			    else {
				ipos = ipos + nbytes;
				}
			    }
			npos = nblock * 2880;
			}
		    if (ipos < npos) {
			snprintf (fitserrmsg,79,"FITSRHEAD: %d / %d bytes skipped\n",
				 ipos,npos);
			extfound = 0;
			break;
			}
		    }
		headnext = header;
		headend = NULL;
		nrec = 1;
		}
	    else {
		break;
		}
	    }
	}

#ifndef VMS
    if (fd != STDIN_FILENO)
	(void)close (fd);
#endif

/* Print error message and return null if extension not found */
    if (extnum != -1 && !extfound) {
	if (extnum < 0)
	    fprintf (stderr, "FITSRHEAD: Extension %s not found in file %s\n",extnam, filename);
	else
	    fprintf (stderr, "FITSRHEAD: Extension %d not found in file %s\n",extnum, filename);
	if (pheader != NULL) {
	    free (pheader);
	    pheader = NULL;
	    }
	return (NULL);
	}

    /* Allocate an extra block for good measure */
    *lhead = (nrec + 1) * FITSBLOCK;
    if (*lhead > nbh) {
	newhead = (char *) calloc (1,(unsigned int) *lhead);
	for (i = 0; i < nbh; i++)
	    newhead[i] = header[i];
	free (header);
	header = newhead;
	(void) hlength (header, *lhead);
	}
    else
	*lhead = nbh;

    /* If INHERIT keyword is FALSE, never append primary header */
    if (hgetl (header, "INHERIT", &inherit)) {
	if (!inherit && fitsinherit)
	    fitsinherit = 0;
	}

    /* Append primary data header to extension header */
    if (pheader != NULL && extnum != 0 && fitsinherit && hdu > 0) {
	extname[0] = 0;
	hgets (header, "XTENSION", 32, extname);
	if (!strcmp (extname,"IMAGE")) {
	    strncpy (header, "SIMPLE  ", 8);
	    hputl (header, "SIMPLE", 1);
	    }
	headend = blsearch (header,"END");
	if (headend == NULL)
	    headend = ksearch (header, "END");
	lext = headend - header;

	/* Update primary header for inclusion at end of extension header */
	hchange (pheader, "SIMPLE", "ROOTHEAD");
	hchange (pheader, "NEXTEND", "NUMEXT");
	hdel (pheader, "BITPIX");
	hdel (pheader, "NAXIS");
	hdel (pheader, "EXTEND");
	hputl (pheader, "ROOTEND",1);
	pheadend = ksearch (pheader,"END");
	lprim = pheadend + 320 - pheader;
	if (lext + lprim > nbh) {
	    nrec = (lext + lprim) / FITSBLOCK;
	    if (FITSBLOCK*nrec < lext+lprim)
		nrec = nrec + 1;
	    *lhead = (nrec+1) * FITSBLOCK;
	    newhead = (char *) calloc (1,(unsigned int) *lhead);
		if (newhead == NULL) {
			assert(newhead != NULL);
		}
		else {
			for (i = 0; i < nbh; i++)
				newhead[i] = header[i];
			free(header);
			header = newhead;
			headend = header + lext;
			(void)hlength(header, *lhead);
		}
	    }
	hputs (header,"COMMENT","-------------------------------------------");
	hputs (header,"COMMENT","Information from Primary Header");
	hputs (header,"COMMENT","-------------------------------------------");
	headend = blsearch (header,"END");
	if (headend == NULL)
	    headend = ksearch (header, "END");
	pheader[lprim] = 0;
	strncpy (headend, pheader, lprim);
	if (pheader != NULL) {
	    free (pheader);
	    pheader = NULL;
	    }
	}

    ibhead = *nbhead - ibhead;

    return (header);
}

/* FITSROPEN -- Open a FITS file, returning the file descriptor */

int
fitsropen (inpath)

const char	*inpath;	/* Pathname for FITS tables file to read */

{
    int ntry;
    int fd = 0;		/* file descriptor for FITS tables file (returned) */
    char *ext;		/* extension name or number */
    char cext = 0;
    char *rbrac;
    char *mwcs;		/* Pointer to WCS name separated by % */

/* Check for FITS WCS specification and ignore for file opening */
    mwcs = strchr (inpath, '%');

/* Check for FITS extension and ignore for file opening */
    ext = strchr (inpath, ',');
    rbrac = NULL;
    if (ext == NULL) {
	ext = strchr (inpath, '[');
	if (ext != NULL) {
	    rbrac = strchr (inpath, ']');
	    }
	}

/* Open input file */
    for (ntry = 0; ntry < 3; ntry++) {
	if (ext != NULL) {
	    cext = *ext;
	    *ext = 0;
	    }
	if (rbrac != NULL)
	    *rbrac = (char) 0;
	if (mwcs != NULL)
	    *mwcs = (char) 0;
	fd = open (inpath, O_RDONLY);
	if (ext != NULL)
	    *ext = cext;
	if (rbrac != NULL)
	    *rbrac = ']';
	if (mwcs != NULL)
	    *mwcs = '%';
	if (fd >= 0)
	    break;
	else if (ntry == 2) {
	    snprintf (fitserrmsg,79, "FITSROPEN:  cannot read file %s\n", inpath);
	    return (-1);
	    }
	}

    if (verbose)
	fprintf (stderr,"FITSROPEN:  input file %s opened\n",inpath);

    return (fd);
}