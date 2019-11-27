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

void RunTests()
{
	TestRemoveByPage();
	TestRemoveBlankPage();
	TestMergeTiffs();
}


std::string GetDirectory (const std::string& path)
{
    size_t found = path.find_last_of("/\\");
    return(path.substr(0, found));
}

int GetPageCount(std::string filename)
{
	TIFF* tif = TIFFOpen(filename.c_str(), "r");
    if (tif) {
	int dircount = 0;
	do {
	    dircount++;
	} while (TIFFReadDirectory(tif));
	if( verbose )
		printf("%d directories in %s\n", dircount, filename);
	TIFFClose(tif);
    }
    exit(0);
}

// caller must free all std::ostringstream * allocated in vector
std::vector<std::ostringstream *> GetPagesAsStreams(std::string tifFile)
{
	std::vector<std::ostringstream *> pages;
	TIFF* tif = TIFFOpen(tifFile.c_str(), "r");
	 if (tif) {
		 do {
			std::ostringstream *output_TIFF_stream = new std::ostringstream();
			TIFF* out = TIFFStreamOpen("MemTIFF", output_TIFF_stream);

			unsigned int width, height;
			// get the size of the tiff
			TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &width);
			TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &height);
			int sampleperpixel = 4;  
			TIFFGetField(tif, TIFFTAG_SAMPLESPERPIXEL, &sampleperpixel);
			int bitspersample = 8;
			TIFFGetField(tif, TIFFTAG_BITSPERSAMPLE, &bitspersample);
			int orientation = ORIENTATION_TOPLEFT;
			TIFFGetField(tif, TIFFTAG_ORIENTATION, &orientation);
			int planar = PLANARCONFIG_CONTIG;
			TIFFGetField(tif, TIFFTAG_PLANARCONFIG, &planar);
			int photo = PHOTOMETRIC_RGB;
			TIFFGetField(tif, TIFFTAG_PHOTOMETRIC, &photo);

			// write raster to a new file
			TIFFSetField (out, TIFFTAG_IMAGEWIDTH, width);  
			TIFFSetField(out, TIFFTAG_IMAGELENGTH, height); 
			TIFFSetField(out, TIFFTAG_SAMPLESPERPIXEL, sampleperpixel); 
			TIFFSetField(out, TIFFTAG_BITSPERSAMPLE, bitspersample);   
			TIFFSetField(out, TIFFTAG_ORIENTATION, orientation);    
			TIFFSetField(out, TIFFTAG_PLANARCONFIG, planar);
			TIFFSetField(out, TIFFTAG_PHOTOMETRIC, photo);

			tsize_t linebytes = sampleperpixel * width;  
			TIFFSetField(out, TIFFTAG_ROWSPERSTRIP, TIFFDefaultStripSize(out, width*sampleperpixel));

			uint32 imagelength;
			tdata_t in_buf;
			uint32 row;
			TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &imagelength);
			in_buf = _TIFFmalloc(TIFFScanlineSize(tif));
			for (row = 0; row < imagelength; row++)
			{
				TIFFReadScanline(tif, in_buf, row);
				TIFFWriteScanline(out, in_buf, row, 0);
			}

			TIFFWriteDirectory(out);
			pages.push_back(output_TIFF_stream);
			(void) TIFFClose(out);
			if (in_buf)
				_TIFFfree(in_buf);
			
		} while (TIFFReadDirectory(tif)); // get the next tif
			TIFFClose(tif); // close the tif file
		}
	return pages;
}

// caller must free all TIFF * allocated in vector
std::vector<TIFF *> GetPagesAsTiffs(std::string tifFile)
{
	std::vector<TIFF *> pages;
	TIFF* tif = TIFFOpen(tifFile.c_str(), "r");
	 if (tif) {
		 do {
			std::ostringstream output_TIFF_stream;
			TIFF* out = TIFFStreamOpen("MemTIFF", &output_TIFF_stream);

			unsigned int width, height;
			TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &width);
			TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &height);
			int sampleperpixel = 4;  
			TIFFGetField(tif, TIFFTAG_SAMPLESPERPIXEL, &sampleperpixel);
			int bitspersample = 8;
			TIFFGetField(tif, TIFFTAG_BITSPERSAMPLE, &bitspersample);
			int orientation = ORIENTATION_TOPLEFT;
			TIFFGetField(tif, TIFFTAG_ORIENTATION, &orientation);
			int planar = PLANARCONFIG_CONTIG;
			TIFFGetField(tif, TIFFTAG_PLANARCONFIG, &planar);
			int photo = PHOTOMETRIC_RGB;
			TIFFGetField(tif, TIFFTAG_PHOTOMETRIC, &photo);

			TIFFSetField (out, TIFFTAG_IMAGEWIDTH, width);  
			TIFFSetField(out, TIFFTAG_IMAGELENGTH, height); 
			TIFFSetField(out, TIFFTAG_SAMPLESPERPIXEL, sampleperpixel); 
			TIFFSetField(out, TIFFTAG_BITSPERSAMPLE, bitspersample);  
			TIFFSetField(out, TIFFTAG_ORIENTATION, orientation); 
			TIFFSetField(out, TIFFTAG_PLANARCONFIG, planar);
			TIFFSetField(out, TIFFTAG_PHOTOMETRIC, photo);
			tsize_t linebytes = sampleperpixel * width;   
			TIFFSetField(out, TIFFTAG_ROWSPERSTRIP, TIFFDefaultStripSize(out, width*sampleperpixel));

			uint32 imagelength;
			tdata_t in_buf;
			uint32 row;
			TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &imagelength);
			in_buf = _TIFFmalloc(TIFFScanlineSize(tif));
			for (row = 0; row < imagelength; row++)
			{
				TIFFReadScanline(tif, in_buf, row);
				TIFFWriteScanline(out, in_buf, row, 0);
			}

			TIFFWriteDirectory(out);
			pages.push_back(out);

			(void) TIFFClose(out);
			if (in_buf)
				_TIFFfree(in_buf);
			
		} while (TIFFReadDirectory(tif)); // get the next tif
			TIFFClose(tif); // close the tif file
		}
	return pages;
}

bool CopyTiffs(std::string inFile, std::string outPath)
{
	int page = 0;
	TIFF *inTif = TIFFOpen(inFile.c_str(), "r");
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
			
			std::string outFile(outPath);	
			outFile += "page";
			outFile += std::to_string(page);
			outFile += ".tif";
			TIFF* outTif = TIFFOpen (outFile.c_str(), "w");

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
		return(false);
	}
	return(true);
}

int GetBlankPageInMemory(std::string inFile)
{
	int page = 0, blankPage = -1;
	TIFF *inTif = TIFFOpen(inFile.c_str(), "r");
	TIFF *tifBlank = TIFFOpen("..\\..\\tiff_console\\input\\blank.tif", "r");
	std::ostringstream blank_TIFF_stream;
	TIFF* tifBlankStream = TIFFStreamOpen("blank", &blank_TIFF_stream);

	if( tifBlank && tifBlankStream )
	{
		do
		{
			page++;
			unsigned int width, height;
			TIFFGetField(tifBlank, TIFFTAG_IMAGEWIDTH, &width);
			TIFFGetField(tifBlank, TIFFTAG_IMAGELENGTH, &height);
			int sampleperpixel = 4;  
			TIFFGetField(tifBlank, TIFFTAG_SAMPLESPERPIXEL, &sampleperpixel);
			unsigned short bitspersample = 8;
			TIFFGetField(tifBlank, TIFFTAG_BITSPERSAMPLE, &bitspersample);
			int orientation = ORIENTATION_TOPLEFT;
			TIFFGetField(tifBlank, TIFFTAG_ORIENTATION, &orientation);
			int planar = PLANARCONFIG_CONTIG;
			TIFFGetField(tifBlank, TIFFTAG_PLANARCONFIG, &planar);
			int photo = PHOTOMETRIC_RGB;
			TIFFGetField(tifBlank, TIFFTAG_PHOTOMETRIC, &photo);
			
			
			if( tifBlankStream )
			{
				TIFFSetField(tifBlankStream, TIFFTAG_IMAGEWIDTH, width);  
				TIFFSetField(tifBlankStream, TIFFTAG_IMAGELENGTH, height);
				TIFFSetField(tifBlankStream, TIFFTAG_SAMPLESPERPIXEL, sampleperpixel); 
				TIFFSetField(tifBlankStream, TIFFTAG_BITSPERSAMPLE, bitspersample);  
				TIFFSetField(tifBlankStream, TIFFTAG_ORIENTATION, orientation);   
				TIFFSetField(tifBlankStream, TIFFTAG_PLANARCONFIG, planar);
				TIFFSetField(tifBlankStream, TIFFTAG_PHOTOMETRIC, photo);
				tsize_t linebytes = sampleperpixel * width; 
				TIFFSetField(tifBlankStream, TIFFTAG_ROWSPERSTRIP, TIFFDefaultStripSize(tifBlankStream, width*sampleperpixel));

				uint32 imagelength;
				tdata_t in_buf;
				uint32 row;
				TIFFGetField(tifBlank, TIFFTAG_IMAGELENGTH, &imagelength);
				in_buf = _TIFFmalloc(TIFFScanlineSize(tifBlank));

				for (row = 0; row < imagelength; row++)
				{
					TIFFReadScanline(tifBlank, in_buf, row);
					TIFFWriteScanline(tifBlankStream, in_buf, row, 0);
				}
				TIFFWriteDirectory(tifBlankStream);

				if (in_buf)
					_TIFFfree(in_buf);
			}
		} while(TIFFReadDirectory(tifBlank));
	}


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
			unsigned short bitspersample = 8;
			TIFFGetField(inTif, TIFFTAG_BITSPERSAMPLE, &bitspersample);
			int orientation = ORIENTATION_TOPLEFT;
			TIFFGetField(inTif, TIFFTAG_ORIENTATION, &orientation);
			int planar = PLANARCONFIG_CONTIG;
			TIFFGetField(inTif, TIFFTAG_PLANARCONFIG, &planar);
			int photo = PHOTOMETRIC_RGB;
			TIFFGetField(inTif, TIFFTAG_PHOTOMETRIC, &photo);
			
			std::string memname = "memTIFF";
			memname += std::to_string(page);
			const char* cname = memname.c_str();

			std::ostringstream output_TIFF_stream;
			TIFF* outTif = TIFFStreamOpen(cname, &output_TIFF_stream);

			if( outTif && tifBlankStream )
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
				TIFFSetField(outTif, TIFFTAG_BITSPERSAMPLE, bitspersample); 

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

				if( outTif )
				{
					if( 2 == tiffcmp(outTif, tifBlankStream) )
					//if( 2 == tiffcmp(tifBlankStream, tifBlankStream) )
						blankPage = page;
				}
				else
				{
					printf("tiff error");
				}


				if (in_buf)
					_TIFFfree(in_buf);

				//TIFFClose(outTif);
				TIFFClose(tifBlankStream);
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

	return(blankPage);
}