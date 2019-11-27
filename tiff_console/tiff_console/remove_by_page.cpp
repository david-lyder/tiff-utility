#include "stdafx.h"
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include "tiffio.h"
#include <string>
#include <vector>
#include <sstream> 
#include "tiff_console.h"

extern bool verbose;

// RemoveByPage, remove a page from a tiff file
// inFile: tiff to have page removed
// outFile: tiff without page
// pageNumber: the page number to remove
// Note: pageNumber is 1 based
bool RemoveByPage(std::string inFile, std::string outFile, int pageNumber)
{
	int currentPage = 0;
	bool removed = false;
	// TODO: catch file open exception
	TIFF* inTif = TIFFOpen(inFile.c_str(), "r");
	TIFF* outTif = TIFFOpen(outFile.c_str(), "w");

	if( inTif && outTif )
	{
		do {
			if(verbose)
				printf("current page:%d page to remove:%d\n", currentPage, pageNumber);
	
			if( ++currentPage == pageNumber )
			{
				removed = true;
				continue;
			}

			// get field values
			unsigned int width, height;
			TIFFGetField(inTif, TIFFTAG_IMAGEWIDTH, &width);
			TIFFGetField(inTif, TIFFTAG_IMAGELENGTH, &height);
			int sampleperpixel = 4;  
			TIFFGetField(inTif, TIFFTAG_SAMPLESPERPIXEL, &sampleperpixel);
			int bitspersample = 8;
			TIFFGetField(inTif, TIFFTAG_BITSPERSAMPLE, &bitspersample);
			int orientation = ORIENTATION_TOPLEFT;
			TIFFGetField(inTif, TIFFTAG_ORIENTATION, &orientation);
			int planar = PLANARCONFIG_CONTIG;
			TIFFGetField(inTif, TIFFTAG_PLANARCONFIG, &planar);
			int photo = PHOTOMETRIC_RGB;
			TIFFGetField(inTif, TIFFTAG_PHOTOMETRIC, &photo);

			// set field values
			TIFFSetField(outTif, TIFFTAG_IMAGEWIDTH, width);				
			TIFFSetField(outTif, TIFFTAG_IMAGELENGTH, height);				
			TIFFSetField(outTif, TIFFTAG_SAMPLESPERPIXEL, sampleperpixel);  
			TIFFSetField(outTif, TIFFTAG_BITSPERSAMPLE, bitspersample);		
			TIFFSetField(outTif, TIFFTAG_ORIENTATION, orientation);			
			TIFFSetField(outTif, TIFFTAG_PLANARCONFIG, planar);
			TIFFSetField(outTif, TIFFTAG_PHOTOMETRIC, photo);
			TIFFSetField(outTif, TIFFTAG_ROWSPERSTRIP, TIFFDefaultStripSize(outTif, width*sampleperpixel));

			uint32 imagelength;
			tdata_t in_buf;
			uint32 row;
			TIFFGetField(inTif, TIFFTAG_IMAGELENGTH, &imagelength);
			in_buf = _TIFFmalloc(TIFFScanlineSize(inTif));
			for (row = 0; row < imagelength; row++)
			{
				TIFFReadScanline(inTif, in_buf, row);
				TIFFWriteScanline(outTif, in_buf, row, 0);
			}

			TIFFWriteDirectory(outTif);
			if (in_buf)
				_TIFFfree(in_buf);
			
		} while (TIFFReadDirectory(inTif)); 
			
		TIFFClose(inTif); 
		TIFFClose(outTif);
	}
	else
	{
		// TODO add more info
		printf("Tiff File Error");
		return(false);
	}

	if( removed == false )
		printf("Warning:requested page number not removed:%d\n", pageNumber);

	return(true);
}


