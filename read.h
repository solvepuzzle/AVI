#ifndef _AVIAVEXTRACTOR_H_
#define _AVIAVEXTRACTOR_H_

#include<stdio.h>
#include<stdlib.h>
#include "type.h"

/*== pre-defined essential structue ==*/
struct AVIDATA{

	DWORD posOfFirstDataBlock;		/* position for first data block */
	DWORD posError;					/* position error for index */
	
	FOURCC vidsIDb;					/* FOURCC for vids(uncompressed) */
	FOURCC vidsIDc;					/* FOURCC for vids(compressed) */
	FOURCC audsID;						/* FOURCC for auds */
	
	FOURCC numOfIndexItem;			/* total index items */
	
	struct RIFFHEADER riffHeader;					/* RIFF structure */
	struct AVIMAINHEADER aviMainHeader;			/* AVIMainHeader */
		
	struct LIST list1;								/* Main list1 */
	struct LIST list2;								/* Main list2 */
	
	struct AVISTREAMHEADER aviStreamHeader[4];/* AVIStreamHeader */
	struct LIST subList[4];							/* ex. vids, auds, time code, odml respectively */
	
	struct BITMAPINFOHEADER bitmapInfoHeader;	/* Bitmap Header */
	struct WAVEFORMATEX waveFormatex;			/* wave format header */	
	
	struct AVIOLDINDEX avioldIndex;				/* index header */
	struct AVIOLDINDEX_ENTRY* indexEntry;		/* index entry */
};


void readFileInfo(FILE *fp, struct AVIDATA* aviData);
void extractVideo(FILE *fp, struct AVIDATA* aviData);
void extractAudio(FILE *fp, struct AVIDATA* aviData);

//void printFourcc(FOURCC );
//DWORD toFourcc(char *);

int matchStreamType(FOURCC sType);
	
#endif
