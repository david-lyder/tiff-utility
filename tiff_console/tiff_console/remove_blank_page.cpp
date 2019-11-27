#include "stdafx.h"
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include "tiffio.h"
#include <string>
#include <vector>
#include <sstream> 
#include "tiff_console.h"
#include "tiffio.hxx"

extern bool verbose;

// RemoveBlankPage,  Remove the last blank page found in inFile
// inFile: tiff to have blank page removed
// outFile: tiff without last blank page

bool RemoveBlankPage(std::string inFile, std::string outPath, std::string outFile)
{
	int page = GetBlankPageNumber(inFile, outPath);
	if( page < 0 )
	{
		printf("No blank page found in: %s\n", inFile);
		return(false);
	}
	return RemoveByPage(inFile, outFile, page);
}


// GetBlankPageNumber: return the page number of last blank page
// inFile: the tiff to find the blank page in
// outPath: a read\write-able temp folder
// Note: uses file blank.tif from file system

int GetBlankPageNumber(std::string inFile, std::string outPath)
{
	int page = 0;
	TIFF *inTif = TIFFOpen(inFile.c_str(), "r");
	TIFF *tifBlank = TIFFOpen("..\\..\\tiff_console\\input\\blank.tif", "r");
	std::vector<std::string> filenames;

	if( inTif )
	{
		do
		{
			page++;
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
			
			// build unique file name to write this page to, and add name to filenames list
			std::string outFile(outPath);	
			outFile += "page";
			outFile += std::to_string(page);
			outFile += ".tif";
			TIFF* outTif = TIFFOpen (outFile.c_str(), "w");
			filenames.push_back(outFile);

			if( outTif )
			{
				TIFFSetField(outTif, TIFFTAG_IMAGEWIDTH, width);  
				TIFFSetField(outTif, TIFFTAG_IMAGELENGTH, height);
				TIFFSetField(outTif, TIFFTAG_SAMPLESPERPIXEL, sampleperpixel); 
				TIFFSetField(outTif, TIFFTAG_BITSPERSAMPLE, bitspersample);  
				TIFFSetField(outTif, TIFFTAG_ORIENTATION, orientation);   
				TIFFSetField(outTif, TIFFTAG_PLANARCONFIG, planar);
				TIFFSetField(outTif, TIFFTAG_PHOTOMETRIC, photo);
				tsize_t linebytes = sampleperpixel * width; 
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

				TIFFClose(outTif);
			}
		} while(TIFFReadDirectory(inTif)); // get the next tif

		TIFFClose(inTif); 
	}
	else
	{
		// TODO add more info
		printf("Tiff File Error");
		return(-1);
	}

	// use tiffcmp to compare each page to a known blank tiff
	// if found, save page number (continuing this way will always get the last blank page)
	// clean up resources: close and remove temp files

	int blankPage = -1, rv = 0;
	page = 1;

	for(std::string file : filenames)
	{
		const char* cStr1 = file.c_str();
		TIFF* tif = TIFFOpen (cStr1, "r");
		if( tif )
		{
			if( 2 == tiffcmp(tif, tifBlank))
				blankPage = page;
			TIFFClose(tif);
			std::remove(file.c_str());
			page++;
		}
	}

	TIFFClose(tifBlank);
	return(blankPage);
}




