/********************************************************************************/
/*                                                                              */
/*	DTC	(c) Marc Delcroix (delcroix.marc@free.fr) 2012-							*/
/*                                                                              */
/*    FILEFMT: Handling of individual files acquisitions functions				*/
/*                                                                              */
/********************************************************************************/
#include "common.h"

#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <opencv/highgui.h>

#include "filefmt.h"
#include "wrapper.h"
#include "dtc.h"
#include "datation.h"
#include "fitsfmt.h"


FileCapture *FileCaptureFromFile(const char *fname, int *pframecount, const int capture_type)
{
	FileCapture *fc;
	int nChannels, depth;
	int frame_idx=0;
	char filename_tmp[MAX_STRING];
	char filename_root[MAX_STRING];
	char first_nb[MAX_STRING];
	struct stat teststat_start;
	struct stat teststat_end;
	int	i;
	int position_found;

/* Init */
	fc = calloc(sizeof (FileCapture), 1);
	assert(fc != NULL);	

	if (!(fc->fh = fopen(fname, "rb")))	{
		fprintf(stderr, "ERROR in FileCaptureFromFile opening first file %s\n", fname);
		fflush(stderr);
		free(fc);
		fc=NULL;
		exit(EXIT_FAILURE);
	}


	get_fileextension(fname, fc->filename_ext, EXT_MAX);
	get_folder(fname, fc->filename_folder);
	right(fname,strlen(fname)-strlen(fc->filename_folder)-1,filename_root);
										if (opts.debug) { fprintf(stderr, "FileCaptureFromFile: Folder %s file %s\n", fc->filename_folder, fname); }
	fc->FirstFileIdx=-1;
	fc->LastFileIdx=-1;
	fc->LeadingZeros=0;
	fc->StartTimeUTC_JD=gregorian_calendar_to_jd(1,1,1,0,0,0.0);
	fc->EndTimeUTC_JD=gregorian_calendar_to_jd(1,1,1,0,0,0.0);
	fc->StartTime_JD=gregorian_calendar_to_jd(1,1,1,0,0,0.0);
	fc->EndTime_JD=gregorian_calendar_to_jd(1,1,1,0,0,0.0);
	fc->FileType=capture_type;
	init_string(fc->filename_rac);
	init_string(fc->filename_head);
	init_string(fc->filename_trail);
	position_found=-1;

/* Look for number syntax */	
	if ((!(strrstr(fname,"1.")==NULL)) && (strlen(strrstr(fname,"1."))==(strlen("1.")+strlen(fc->filename_ext)))) {	/* *0.* */
		fc->FirstFileIdx=1;
		i=(strlen(fname)-strlen(strrstr(fname,"1.")-1));
		while ((fname[i]=='0') && (i>=0)) {
			fc->LeadingZeros++;
			i--;
		}
	} else if ((!(strrstr(fname,"0.")==NULL)) && (strlen(strrstr(fname,"0."))==(strlen("0.")+strlen(fc->filename_ext)))) {	/* *1.* */
		fc->FirstFileIdx=0;			
		i=(strlen(fname)-strlen(strrstr(fname,"0.")-1));
		while ((fname[i]=='0') && (i>=0)) {
			fc->LeadingZeros++;
			i--;
		}
	}
	if (fc->FirstFileIdx>=0) {
		if (fc->LeadingZeros>0) {
			for (i=0; i<fc->LeadingZeros; first_nb[i++]='0');
			first_nb[i]='\0';
			fc->NumberPos=InRstr(filename_root,first_nb)+strlen(fc->filename_folder);
		} else {
			if (fc->FirstFileIdx==0) {
				fc->NumberPos=InRstr(filename_root,"0")+1+strlen(fc->filename_folder);
			} else {
				fc->NumberPos=InRstr(filename_root,"1")+1+strlen(fc->filename_folder);
			}
		}
		strncpy(fc->filename_rac, fname, fc->NumberPos);
		strcat(fc->filename_rac, "\0");
		if (strlen(fc->filename_rac)>strlen(fc->filename_folder)) {
			right(fc->filename_rac,strlen(fc->filename_rac)-strlen(fc->filename_folder)-1,fc->filename_head);
		} else {
			strcat(fc->filename_head, "\0");
		}
		if (strcmp(fc->filename_folder,".")!=0) {
			fc->NumberPos=fc->NumberPos-strlen(fc->filename_folder)-1;
		}
		fileGet_filename(filename_tmp, fc, fc->FirstFileIdx+1);
		if (strlen(filename_tmp)>0) {
			position_found=0;
		}
	}
	if (position_found<0) {	/* *0* */
		fc->LeadingZeros=10;
		for (i=0; i<fc->LeadingZeros; first_nb[i++]='0');
		first_nb[i]='\0';
		while ((fc->LeadingZeros>0) && (InRstr(filename_root,first_nb)<0)) {
/*										if (opts.debug) { fprintf(stderr, "FileCaptureFromFile: Leading zeros %d first_nb %s\n", fc->LeadingZeros, first_nb); }*/
			fc->LeadingZeros--;
			first_nb[fc->LeadingZeros]='\0';
		}	
		first_nb[fc->LeadingZeros]='1';
		first_nb[fc->LeadingZeros+1]='\0';
		if (InRstr(filename_root,first_nb)>=0) { /* *01* */
			fc->FirstFileIdx=1;
		} else if (fc->LeadingZeros>0) {
			fc->FirstFileIdx=0;			
			first_nb[fc->LeadingZeros]='\0';
			fc->LeadingZeros--;
		}
		fc->NumberPos=InRstr(filename_root,first_nb)+strlen(fc->filename_folder);
		init_string(fc->filename_rac);
		init_string(fc->filename_head);
		init_string(fc->filename_trail);
		strncpy(fc->filename_rac, fname, fc->NumberPos+1);
		strcat(fc->filename_rac, "\0");
		if (strlen(fc->filename_rac)>strlen(fc->filename_folder)) {
			right(fc->filename_rac,strlen(fc->filename_rac)-strlen(fc->filename_folder)-1,fc->filename_head);
		} else {
			strcat(fc->filename_head, "\0");
		}
		mid(filename_root,strlen(fc->filename_head)+fc->LeadingZeros+1,strlen(filename_root)-1-strlen(fc->filename_ext)-fc->LeadingZeros-1-strlen(fc->filename_head),fc->filename_trail);
		if (strcmp(fc->filename_folder,".")!=0) {
			fc->NumberPos=fc->NumberPos-strlen(fc->filename_folder)-1;
		}
	}

	fc->LastFileIdx=fc->FirstFileIdx;
	if (fc->FirstFileIdx>=0) {
//									if (opts.debug) { fprintf(stderr, "FileCaptureFromFile: File syntax %s*%s.%s (%d vs %d vs %d), leading zeros %d\n", fc->filename_head, fc->filename_trail, fc->filename_ext, strlen(fname), strlen(fc->filename_rac), strlen(fc->filename_ext),fc->LeadingZeros); }
/* Look for last file */	
		fc->LastFileIdx=(*pframecount) -1 + fc->FirstFileIdx;
		if (fc->LastFileIdx<fc->FirstFileIdx) {
			frame_idx=fc->FirstFileIdx-1;
			do {
				frame_idx++;
				fileGet_filename(filename_tmp, fc, frame_idx);
/*										if (opts.debug) { fprintf(stderr,"FileCaptureFromFile: Checking frame %d\n",frame_idx); }*/
			} while (strlen(filename_tmp)>0);
			fc->LastFileIdx=frame_idx-1;
		}
	}
	fc->FrameCount=fc->LastFileIdx-fc->FirstFileIdx+1;
	fc->ValidFrameCount=fc->FrameCount;
	(*pframecount)=fc->FrameCount;
									if (opts.debug) { fprintf(stderr,"FileCaptureFromFile: First frame index %d, Last Frame index %d\n",fc->FirstFileIdx,fc->LastFileIdx); }
	if (fc->FrameCount<=0) {
		fprintf(stderr,"ERROR in FileCaptureFromFile: no frame number detected to process for file %s\n",fname);
		fclose(fc->fh);
		exit(EXIT_FAILURE);
	} else {
/* Reads information from first file */		
		switch (fc->FileType) {
			case CAPTURE_FITS:			
				fileGet_info(fc, fname, &fc->StartTimeUTC_JD);
				if (fc->StartTimeUTC_JD<gregorian_calendar_to_jd(1,1,1,0,0,1.0)) {
					if (stat(fname, &teststat_start)>=0) {
						fc->StartTime_JD=JD_from_time_t(teststat_start.st_mtime);
					}
				}
				nChannels = 1;
				depth     = fc->PixelDepth > 8 ? IPL_DEPTH_16U : IPL_DEPTH_8U;
				fc->image					= cvCreateImageHeader(cvSize(fc->ImageWidth, fc->ImageHeight), depth, nChannels);
				assert(fc->image != NULL);
				fc->image->imageData		= calloc(sizeof (char), fc->ImageBytes);
				assert(fc->image->imageData != NULL);
				fc->image->widthStep		= fc->ImageWidth * fc->BytesPerPixel * fc->image->nChannels;
				fc->image->imageDataOrigin	= fc->image->imageData;
				break;
			case CAPTURE_FILES:
				fileGet_info(fc, fname, &fc->StartTime_JD);
				if (stat(fname, &teststat_start)>=0) {
					fc->StartTime_JD=JD_from_time_t(teststat_start.st_mtime);
							if (opts.debug) { fprintf(stderr, "FileCaptureFromFile: StartTime_JD=%f\n", fc->StartTime_JD); }
				}
				break;
		}
		fc->frame = -1;
		if (fclose(fc->fh)!=0) {
			fprintf(stderr, "ERROR in FileCaptureFromFile closing capture file\n");
			exit(EXIT_FAILURE);
		}
											if (opts.debug) { fprintf(stderr, "FileCaptureFromFile: ImageWidth=%d, ImageHeight=%d, BytesPerPixel=%d, ImageBytes=%d\n", fc->ImageWidth, fc->ImageHeight, fc->BytesPerPixel,fc->ImageBytes); }
		if (fc->FrameCount>1) {
/* Reads information from last file */		
			fileGet_filename(filename_tmp, fc, fc->LastFileIdx);
			if (!(fc->fh = fopen(filename_tmp, "rb"))) {
				fprintf(stderr, "ERROR in FileCaptureFromFile opening %s file...\n", filename_tmp);
				exit(EXIT_FAILURE);
			}
												if (opts.debug) { fprintf(stderr, "FileCaptureFromFile: opening %s file\n", filename_tmp); }
			switch (fc->FileType) {
				case CAPTURE_FITS:			
					fileGet_info(fc, filename_tmp, &fc->EndTimeUTC_JD);
					if (fc->EndTimeUTC_JD<gregorian_calendar_to_jd(1,1,1,0,0,1.0)) {
						if (stat(filename_tmp, &teststat_end)>=0) {
							fc->EndTime_JD=JD_from_time_t(teststat_end.st_mtime);
						}
					}
					break;
				case CAPTURE_FILES:
/*				fileGet_info(fc, filename_tmp, &fc->EndTime_JD); */
					if (stat(filename_tmp, &teststat_end)>=0) {
						fc->EndTime_JD=JD_from_time_t(teststat_end.st_mtime);
						if (opts.debug) { fprintf(stderr, "FileCaptureFromFile: EndTime_JD=%f\n", fc->EndTime_JD); }
					}
					break;
			}
/* Cleaning */		
			if (fclose(fc->fh)!=0) {
				fprintf(stderr, "ERROR in FileCaptureFromFile closing capture file\n");
				exit(EXIT_FAILURE);
			}
		}
	}
	return fc;
}

void fileReinitCaptureRead(FileCapture *fc,const char *fname)
{
	fc->frame = -1;
	if (!(fseek(fc->fh, SER_HEADER_SIZE, SEEK_SET)))
	{
		fprintf(stderr, "ERROR in fileReinitCaptureRead reinitializing %s file\n", fname);
		exit(EXIT_FAILURE);
	}
}

/*****************Reads current frame***************************/			
IplImage *fileQueryFrame(FileCapture *fc, const int ignore, int *perror)
{
	IplImage *old_image;
	char filename[MAX_STRING];
	char *header;
	size_t bytesR;
	
	(*perror)=0;
	if (fc->frame<0) {
		fc->frame=fc->FirstFileIdx;
	} else {
		fc->frame++;
	}
	if (fc->frame > fc->LastFileIdx) {
		return NULL;
	}

	fileGet_filename(filename, fc, fc->frame);

	if (!(fc->fh = fopen(filename, "rb"))) {
		fprintf(stderr, "ERROR in fileQueryFrame opening %s file (frame %d/%d)\n", filename, fc->frame,fc->LastFileIdx);
		exit(EXIT_FAILURE);
	}
	switch (fc->FileType) {
		case CAPTURE_FITS:
			header=malloc(sizeof *header * fc->header_size);
			if (header == NULL) {
				assert(header != NULL);
			}
			else {
				if (fread(header, sizeof(char), fc->header_size, fc->fh) != fc->header_size) {
					if (fclose(fc->fh) != 0) {
						fprintf(stderr, "ERROR in fileQueryFrame closing capture file\n");
						exit(EXIT_FAILURE);
					}
					free(header);
					if (!ignore) {
						fprintf(stderr, "ERROR in fileQueryFrame reading fits header frame %d (Header size different from %d)\n", fc->frame, fc->header_size);
						exit(EXIT_FAILURE);
					}
					else {
						fprintf(stderr, "WARNING in fileQueryFrame: ignoring error reading fits header frame #%d (Header size different from %d)\n", fc->frame, fc->header_size);
						(*perror) = 1;
						return fc->image;
					}
				}
				else {
					if (!(bytesR = fitsImageRead(fc->image->imageData, sizeof(char)*fc->BytesPerPixel, fc->ImageBytes / fc->BytesPerPixel, fc->fh)))	{
						if (fclose(fc->fh) != 0) {
							fprintf(stderr, "ERROR in fileQueryFrame closing capture file\n");
							exit(EXIT_FAILURE);
						}
						free(header);
						if (!ignore) {
							fprintf(stderr, "ERROR in fileQueryFrame reading fits frame %d\n", fc->frame);
							exit(EXIT_FAILURE);
						}
						else {
							fc->ValidFrameCount--;
							fprintf(stderr, "WARNING in fileQueryFrame: ignoring error reading fits frame #%d (%d missing till frame #%d)\n", fc->frame, fc->FrameCount - fc->ValidFrameCount, fc->FrameCount);
							(*perror) = 1;
							return fc->image;
						}
					}
				}
			}
			break;
		case CAPTURE_FILES:
			old_image=fc->image;
/*			fprintf(stderr, "fileQueryFrame: reading frame %d\n", fc->frame);*/
			if (!(fc->image=cvLoadImage(filename,CV_LOAD_IMAGE_ANYDEPTH))) {
				if (!ignore) {
					fprintf(stderr, "ERROR in fileQueryFrame reading frame %d\n", fc->frame);
					cvReleaseImage(&old_image);
					exit(EXIT_FAILURE);
				} else {
					fc->ValidFrameCount--;
					fprintf(stderr, "WARNING in fileQueryFrame: ignoring error reading frame #%d (%d missing till frame #%d)\n", fc->frame, fc->FrameCount-fc->ValidFrameCount, fc->FrameCount);
					(*perror)=1;
					fc->image=old_image;
					return fc->image;
				}
			} else {
				if (old_image!=NULL) {
					cvReleaseImage(&old_image);
				}
			}
			break;
	}
	fc->LastValidFileIdx=fc->frame;
	if (fclose(fc->fh)!=0) {
		fprintf(stderr, "ERROR in fileQueryFrame closing capture file\n");
		exit(EXIT_FAILURE);
	}
		
	return fc->image;
}

/*****************Gets general file info***************************/			
void fileGet_info(FileCapture *fc, const char *fname, double *date)
{
	(*date)=gregorian_calendar_to_jd(1,1,1,0,0,0.0);
	switch (fc->FileType) {
		case CAPTURE_FITS:
			fitsGet_info(fc, fname, date);
			break;
		case CAPTURE_FILES:
			fc->image		= cvLoadImage(fname,CV_LOAD_IMAGE_ANYDEPTH);
			fc->ImageWidth	= fc->image->width;
			fc->ImageHeight	= fc->image->height;
			fc->PixelDepth	= fc->image->depth;
			fc->ColorID		= fc->image->nChannels;
			fc->image->widthStep		= fc->ImageWidth * fc->BytesPerPixel * fc->image->nChannels;
			fc->image->imageDataOrigin	= fc->image->imageData;
			cvReleaseImage(&fc->image);
			break;
	}
	fc->BytesPerPixel = fc->PixelDepth > 8 ? 2 : 1;
	fc->ImageBytes    = fc->ImageWidth * fc->ImageHeight * fc->BytesPerPixel;
}

/*****************Cleans capture***************************/			
void fileReleaseCapture(FileCapture *fc)
{
/*	if (fclose(fc->fh)!=0) {
		fprintf(stderr, "ERROR in fileReleaseCapture: closing capture file\n");
		exit(EXIT_FAILURE);
	}*/
	switch (fc->FileType) {
		case CAPTURE_FITS:
			free(fc->image->imageData);
			cvReleaseImageHeader(&fc->image);
			break;
		case CAPTURE_FILES:
			cvReleaseImage(&fc->image);
			break;
	}
	free(fc);
}
void fileGenerate_filename(char *dest, FileCapture *fc, int nb)
{
	char nbstring[MAX_STRING];
	int max;
	
	dest=strcpy(dest,fc->filename_rac);
	max = fc->LeadingZeros;
	for (int i=1; i<=max; i++) {
		dest=strcat(dest,"0");
	}
	sprintf(nbstring, "%d%s.%s", nb,fc->filename_trail,fc->filename_ext);
	strcat(dest, nbstring);
}

void fileGet_filename(char *dest, FileCapture *fc, int nb)
{
	char tmp_string[MAX_STRING];
	char tmp_string2[MAX_STRING];
	char filename_target[MAX_STRING];
	int found;
	struct dirent *dent;
	DIR *dir;
	
	found=0;
	fileGenerate_number(tmp_string, fc, nb);
	sprintf(filename_target, "%s%s%s.%s", fc->filename_head,tmp_string,fc->filename_trail,fc->filename_ext);
	strcat(filename_target,"\0");
//						if (opts.debug) { fprintf(stderr, "fileGet_filename: Checking filename=%s\n", filename_target); }
	dir=opendir(fc->filename_folder);
	if ((dir)!=NULL) {
		while (((dent=readdir(dir))!=NULL) &&  (found==0)) {
/*					if (opts.debug) { fprintf(stderr, "fileGet_filename: Checking file =%s vs |%s|%s%s.%s|,%s,%s\n", dent->d_name, filename_target,fc->filename_rac,tmp_string,fc->filename_ext,fc->filename_trail,fc->filename_folder); }	 */
/*		if (InRstr(dent->d_name,tmp_string)==fc->NumberPos) {*/
/*					if (opts.debug) { fprintf(stderr, "fileGet_filename: Checking directory filename=%s\n", dent->d_name); }*/
		if ((strncmp(dent->d_name,filename_target,strlen(filename_target))==0)
			|| ((strlen(dent->d_name)==strlen(filename_target))
			&& (strcmp(left(dent->d_name,strlen(fc->filename_head),tmp_string),left(filename_target,strlen(fc->filename_head),tmp_string2))==0)
			&& (strcmp(mid(dent->d_name,strlen(fc->filename_head),fc->LeadingZeros+1,tmp_string),mid(filename_target,strlen(fc->filename_head),fc->LeadingZeros+1,tmp_string2))==0)
			&& (strcmp(right(dent->d_name,strlen(fc->filename_ext),tmp_string),right(filename_target,strlen(fc->filename_ext),tmp_string2))==0))) {		
				found=1;
				strcpy(tmp_string,dent->d_name);
			}
		}
	//	close((int) dir);
		if (closedir(dir)!=0) {
			fprintf(stderr, "ERROR in fileGet_filename closing directory=%s\n", fc->filename_folder);
			exit(EXIT_FAILURE);
		}
	} else {
		fprintf(stderr, "ERROR in fileGet_filename opening directory=%s\n", fc->filename_folder);
	}
	if (found!=1) {
		strcpy(dest,"");
	} else {
		strcpy(dest,fc->filename_folder);
		strcat(dest,"\\");
		strcat(dest,tmp_string);
	}
}

void fileGenerate_number(char *dest, FileCapture *fc, int nb)
{
	int max;
	char nbstring[MAX_STRING];
	char tmpstring[MAX_STRING];
	
	strcpy(tmpstring,"");
	max = fc->LeadingZeros;
	for (int i=1; i<=max; i++) {
		strcat(tmpstring,"0");
	}
	sprintf(nbstring, "%d", nb);
	strcat(tmpstring, nbstring);
	if (fc->LeadingZeros>0) {
		right(tmpstring,fc->LeadingZeros+1,dest);
	} else {
		strcpy(dest,tmpstring);
	}
}