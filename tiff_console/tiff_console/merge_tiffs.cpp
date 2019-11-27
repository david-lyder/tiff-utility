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

// MergeTiffs, concatenate list of files
// inFiles: tiffs to be concatenated
// outFile: concatenated tiff 

bool MergeTiffs(std::vector<std::string> inFiles, std::string outFile)
{
	TIFF* outTif = TIFFOpen (outFile.c_str(), "w");

	for(std::string inFile : inFiles) 
	{
		TIFF *inTif = TIFFOpen(inFile.c_str(), "r");
		if( inTif && outTif )
		{
			do
			{
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

			} while(TIFFReadDirectory(inTif)); // get the next tif

			TIFFClose(inTif); 
		}
		else
		{
			// TODO add more info
			printf("Tiff File Error");
			return(false);
		}
	}
	return(true);
}
