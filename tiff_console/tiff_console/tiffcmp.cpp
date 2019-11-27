#include "stdafx.h"
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include "tiffio.h"

// shameless copy of tiffcmp from tifflib distribution tools folder
// changes made:
//   support calling as function instead from main
//   change exits to return values
//   control printf using local _verbose flag

bool _verbose = false;

extern "C"
{
    int tiffcmp(TIFF* tif1, TIFF* tif2);
}

static	int stopondiff = 1;
static	int stoponfirsttag = 1;
static	uint16 bitspersample = 1;
static	uint16 samplesperpixel = 1;
static	uint16 sampleformat = SAMPLEFORMAT_UINT;
static	uint32 imagewidth;
static	uint32 imagelength;

static	void usage(void);
//static int tiffcmp(TIFF*, TIFF*);
static	int cmptags(TIFF*, TIFF*);
static	int ContigCompare(int, uint32, unsigned char*, unsigned char*, tsize_t);
static	int SeparateCompare(int, int, uint32, unsigned char*, unsigned char*);
static	int  PrintIntDiff(uint32, int, uint32, uint32, uint32);
static	int  PrintFloatDiff(uint32, int, uint32, double, double);

static	void leof(const char*, uint32, int);

#define	checkEOF(tif, row, sample) { \
	leof(TIFFFileName(tif), row, sample); \
	goto bad; \
}

static	int CheckShortTag(TIFF*, TIFF*, int, char*);
static	int CheckShort2Tag(TIFF*, TIFF*, int, char*);
static	int CheckShortArrayTag(TIFF*, TIFF*, int, char*);
static	int CheckLongTag(TIFF*, TIFF*, int, char*);
static	int CheckFloatTag(TIFF*, TIFF*, int, char*);
static	int CheckStringTag(TIFF*, TIFF*, int, char*);

 int tiffcmp(TIFF* tif1, TIFF* tif2)
{
	uint16 config1, config2;
	tsize_t size1;
	uint32 row;
	tsample_t s;
	unsigned char *buf1, *buf2;

	if (!CheckShortTag(tif1, tif2, TIFFTAG_BITSPERSAMPLE, "BitsPerSample"))
		return (0);
	if (!CheckShortTag(tif1, tif2, TIFFTAG_SAMPLESPERPIXEL, "SamplesPerPixel"))
		return (0);
	if (!CheckLongTag(tif1, tif2, TIFFTAG_IMAGEWIDTH, "ImageWidth"))
		return (0);
	if (!cmptags(tif1, tif2))
		return (1);
	(void) TIFFGetField(tif1, TIFFTAG_BITSPERSAMPLE, &bitspersample);
	(void) TIFFGetField(tif1, TIFFTAG_SAMPLESPERPIXEL, &samplesperpixel);
	(void) TIFFGetField(tif1, TIFFTAG_SAMPLEFORMAT, &sampleformat);
	(void) TIFFGetField(tif1, TIFFTAG_IMAGEWIDTH, &imagewidth);
	(void) TIFFGetField(tif1, TIFFTAG_IMAGELENGTH, &imagelength);
	(void) TIFFGetField(tif1, TIFFTAG_PLANARCONFIG, &config1);
	(void) TIFFGetField(tif2, TIFFTAG_PLANARCONFIG, &config2);
	buf1 = (unsigned char *)_TIFFmalloc(size1 = TIFFScanlineSize(tif1));
	buf2 = (unsigned char *)_TIFFmalloc(TIFFScanlineSize(tif2));
	if (buf1 == NULL || buf2 == NULL) {
		fprintf(stderr, "No space for scanline buffers\n");
		return(-1);
	}
	if (config1 != config2 && bitspersample != 8 && samplesperpixel > 1) {
		fprintf(stderr,
"Can't handle different planar configuration w/ different bits/sample\n");
		goto bad;
	}
#define	pack(a,b)	((a)<<8)|(b)
	switch (pack(config1, config2)) {
	case pack(PLANARCONFIG_SEPARATE, PLANARCONFIG_CONTIG):
		for (row = 0; row < imagelength; row++) {
			if (TIFFReadScanline(tif2, buf2, row, 0) < 0)
				checkEOF(tif2, row, -1)
			for (s = 0; s < samplesperpixel; s++) {
				if (TIFFReadScanline(tif1, buf1, row, s) < 0)
					checkEOF(tif1, row, s)
				if (SeparateCompare(1, s, row, buf2, buf1) < 0)
					goto bad1;
			}
		}
		break;
	case pack(PLANARCONFIG_CONTIG, PLANARCONFIG_SEPARATE):
		for (row = 0; row < imagelength; row++) {
			if (TIFFReadScanline(tif1, buf1, row, 0) < 0)
				checkEOF(tif1, row, -1)
			for (s = 0; s < samplesperpixel; s++) {
				if (TIFFReadScanline(tif2, buf2, row, s) < 0)
					checkEOF(tif2, row, s)
				if (SeparateCompare(0, s, row, buf1, buf2) < 0)
					goto bad1;
			}
		}
		break;
	case pack(PLANARCONFIG_SEPARATE, PLANARCONFIG_SEPARATE):
		for (s = 0; s < samplesperpixel; s++)
			for (row = 0; row < imagelength; row++) {
				if (TIFFReadScanline(tif1, buf1, row, s) < 0)
					checkEOF(tif1, row, s)
				if (TIFFReadScanline(tif2, buf2, row, s) < 0)
					checkEOF(tif2, row, s)
				if (ContigCompare(s, row, buf1, buf2, size1) < 0)
					goto bad1;
			}
		break;
	case pack(PLANARCONFIG_CONTIG, PLANARCONFIG_CONTIG):
		for (row = 0; row < imagelength; row++) {
			if (TIFFReadScanline(tif1, buf1, row, 0) < 0)
				checkEOF(tif1, row, -1)
			if (TIFFReadScanline(tif2, buf2, row, 0) < 0)
				checkEOF(tif2, row, -1)
			if (ContigCompare(-1, row, buf1, buf2, size1) < 0)
				goto bad1;
		}
		break;
	}
	if (buf1) _TIFFfree(buf1);
	if (buf2) _TIFFfree(buf2);
	return (2);
bad:
	if (stopondiff)
		return(1);
		//exit(1);
bad1:
	if (buf1) _TIFFfree(buf1);
	if (buf2) _TIFFfree(buf2);
	return (0);
}

#define	CmpShortField(tag, name) \
	if (!CheckShortTag(tif1, tif2, tag, name) && stoponfirsttag) return (0)
#define	CmpShortField2(tag, name) \
	if (!CheckShort2Tag(tif1, tif2, tag, name) && stoponfirsttag) return (0)
#define	CmpLongField(tag, name) \
	if (!CheckLongTag(tif1, tif2, tag, name) && stoponfirsttag) return (0)
#define	CmpFloatField(tag, name) \
	if (!CheckFloatTag(tif1, tif2, tag, name) && stoponfirsttag) return (0)
#define	CmpStringField(tag, name) \
	if (!CheckStringTag(tif1, tif2, tag, name) && stoponfirsttag) return (0)
#define	CmpShortArrayField(tag, name) \
	if (!CheckShortArrayTag(tif1, tif2, tag, name) && stoponfirsttag) return (0)

static int
cmptags(TIFF* tif1, TIFF* tif2)
{
	uint16 compression1, compression2;
	CmpLongField(TIFFTAG_SUBFILETYPE,	"SubFileType");
	CmpLongField(TIFFTAG_IMAGEWIDTH,	"ImageWidth");
	CmpLongField(TIFFTAG_IMAGELENGTH,	"ImageLength");
	CmpShortField(TIFFTAG_BITSPERSAMPLE,	"BitsPerSample");
	CmpShortField(TIFFTAG_COMPRESSION,	"Compression");
	CmpShortField(TIFFTAG_PREDICTOR,	"Predictor");
	CmpShortField(TIFFTAG_PHOTOMETRIC,	"PhotometricInterpretation");
	CmpShortField(TIFFTAG_THRESHHOLDING,	"Thresholding");
	CmpShortField(TIFFTAG_FILLORDER,	"FillOrder");
	CmpShortField(TIFFTAG_ORIENTATION,	"Orientation");
	CmpShortField(TIFFTAG_SAMPLESPERPIXEL,	"SamplesPerPixel");
	CmpShortField(TIFFTAG_MINSAMPLEVALUE,	"MinSampleValue");
	CmpShortField(TIFFTAG_MAXSAMPLEVALUE,	"MaxSampleValue");
	CmpShortField(TIFFTAG_SAMPLEFORMAT,	"SampleFormat");
	CmpFloatField(TIFFTAG_XRESOLUTION,	"XResolution");
	CmpFloatField(TIFFTAG_YRESOLUTION,	"YResolution");
	if( TIFFGetField(tif1, TIFFTAG_COMPRESSION, &compression1) &&
		compression1 == COMPRESSION_CCITTFAX3 &&
		TIFFGetField(tif2, TIFFTAG_COMPRESSION, &compression2) &&
		compression2 == COMPRESSION_CCITTFAX3 )
	{
		CmpLongField(TIFFTAG_GROUP3OPTIONS,	"Group3Options");
	}
	if( TIFFGetField(tif1, TIFFTAG_COMPRESSION, &compression1) &&
		compression1 == COMPRESSION_CCITTFAX4 &&
		TIFFGetField(tif2, TIFFTAG_COMPRESSION, &compression2) &&
		compression2 == COMPRESSION_CCITTFAX4 )
	{
		CmpLongField(TIFFTAG_GROUP4OPTIONS,	"Group4Options");
	}
	CmpShortField(TIFFTAG_RESOLUTIONUNIT,	"ResolutionUnit");
	CmpShortField(TIFFTAG_PLANARCONFIG,	"PlanarConfiguration");
	CmpLongField(TIFFTAG_ROWSPERSTRIP,	"RowsPerStrip");
	CmpFloatField(TIFFTAG_XPOSITION,	"XPosition");
	CmpFloatField(TIFFTAG_YPOSITION,	"YPosition");
	CmpShortField(TIFFTAG_GRAYRESPONSEUNIT, "GrayResponseUnit");
	CmpShortField(TIFFTAG_COLORRESPONSEUNIT, "ColorResponseUnit");
#ifdef notdef
	{ uint16 *graycurve;
	  CmpField(TIFFTAG_GRAYRESPONSECURVE, graycurve);
	}
	{ uint16 *red, *green, *blue;
	  CmpField3(TIFFTAG_COLORRESPONSECURVE, red, green, blue);
	}
	{ uint16 *red, *green, *blue;
	  CmpField3(TIFFTAG_COLORMAP, red, green, blue);
	}
#endif
	CmpShortField2(TIFFTAG_PAGENUMBER,	"PageNumber");
	CmpStringField(TIFFTAG_ARTIST,		"Artist");
	CmpStringField(TIFFTAG_IMAGEDESCRIPTION,"ImageDescription");
	CmpStringField(TIFFTAG_MAKE,		"Make");
	CmpStringField(TIFFTAG_MODEL,		"Model");
	CmpStringField(TIFFTAG_SOFTWARE,	"Software");
	CmpStringField(TIFFTAG_DATETIME,	"DateTime");
	CmpStringField(TIFFTAG_HOSTCOMPUTER,	"HostComputer");
	CmpStringField(TIFFTAG_PAGENAME,	"PageName");
	CmpStringField(TIFFTAG_DOCUMENTNAME,	"DocumentName");
	CmpShortField(TIFFTAG_MATTEING,		"Matteing");
	CmpShortArrayField(TIFFTAG_EXTRASAMPLES,"ExtraSamples");
	return (1);
}

static int
ContigCompare(int sample, uint32 row,
	      unsigned char* p1, unsigned char* p2, tsize_t size)
{
	int rv = 0;
    uint32 pix;
    int ppb = 8 / bitspersample;
    int	 samples_to_test;

    if (memcmp(p1, p2, size) == 0)
        return 0;

    samples_to_test = (sample == -1) ? samplesperpixel : 1;

    switch (bitspersample) {
      case 1: case 2: case 4: case 8: 
      {
          unsigned char *pix1 = p1, *pix2 = p2;

          for (pix = 0; pix < imagewidth; pix += ppb) {
              int		s;

              for(s = 0; s < samples_to_test; s++) {
                  if (*pix1 != *pix2) {
                      if( sample == -1 )
                          rv = PrintIntDiff(row, s, pix, *pix1, *pix2);
                      else
                          rv = PrintIntDiff(row, sample, pix, *pix1, *pix2);
                  }
				  if( rv < 0 )
					  return(rv);
                  pix1++;
                  pix2++;
              }
          }
          break;
      }
      case 16: 
      {
          uint16 *pix1 = (uint16 *)p1, *pix2 = (uint16 *)p2;

          for (pix = 0; pix < imagewidth; pix++) {
              int	s;

              for(s = 0; s < samples_to_test; s++) {
                  if (*pix1 != *pix2)
                      rv = PrintIntDiff(row, sample, pix, *pix1, *pix2);
                   if( rv < 0 )
					  return(rv);      
                  pix1++;
                  pix2++;
              }
          }
          break;
      }
      case 32: 
	if (sampleformat == SAMPLEFORMAT_UINT
	    || sampleformat == SAMPLEFORMAT_INT) {
		uint32 *pix1 = (uint32 *)p1, *pix2 = (uint32 *)p2;

		for (pix = 0; pix < imagewidth; pix++) {
			int	s;

			for(s = 0; s < samples_to_test; s++) {
				if (*pix1 != *pix2) {
					rv = PrintIntDiff(row, sample, pix,
						     *pix1, *pix2);
					 if( rv < 0 )
					  return(rv);
				}
                        
				pix1++;
				pix2++;
			}
		}
	} else if (sampleformat == SAMPLEFORMAT_IEEEFP) {
		float *pix1 = (float *)p1, *pix2 = (float *)p2;

		for (pix = 0; pix < imagewidth; pix++) {
			int	s;

			for(s = 0; s < samples_to_test; s++) {
				if (fabs(*pix1 - *pix2) < 0.000000000001) {
					rv = PrintFloatDiff(row, sample, pix,
						       *pix1, *pix2);

					 if( rv < 0 )
					  return(rv);
				}
                        
				pix1++;
				pix2++;
			}
		}
	} else {
		  fprintf(stderr, "Sample format %d is not supported.\n",
			  sampleformat);
		  return -1;
	}
        break;
      default:
	fprintf(stderr, "Bit depth %d is not supported.\n", bitspersample);
	return -1;
    }

    return 0;
}

int 
PrintIntDiff(uint32 row, int sample, uint32 pix, uint32 w1, uint32 w2)
{
	//if( _verbose == false )
		//return;


	if (sample < 0)
		sample = 0;
	switch (bitspersample) {
	case 1:
	case 2:
	case 4:
	    {
		int32 mask1, mask2, s;

        /* mask1 should have the n lowest bits set, where n == bitspersample */
        mask1 = ((int32)1 << bitspersample) - 1;
		s = (8 - bitspersample);
		mask2 = mask1 << s;
		for (; mask2 && pix < imagewidth;
		     mask2 >>= bitspersample, s -= bitspersample, pix++) {
			if ((w1 & mask2) ^ (w2 & mask2)) {
				if( _verbose == true )
				printf(
			"Scanline %lu, pixel %lu, sample %d: %01x %01x\n",
	    				(unsigned long) row,
					(unsigned long) pix,
					sample,
					(unsigned int)((w1 >> s) & mask1),
					(unsigned int)((w2 >> s) & mask1));
				if (--stopondiff <= 0)
					return(-1);
					//exit(1);
			}
		}
		break;
	    }
	case 8: 
		if( _verbose == true )
		printf("Scanline %lu, pixel %lu, sample %d: %02x %02x\n",
		       (unsigned long) row, (unsigned long) pix, sample,
		       (unsigned int) w1, (unsigned int) w2);
		if (--stopondiff <= 0)
			//printf("return 1\n");
			return(-1);
			//exit(1);
		break;
	case 16:
		if( _verbose == true )
		printf("Scanline %lu, pixel %lu, sample %d: %04x %04x\n",
		    (unsigned long) row, (unsigned long) pix, sample,
		    (unsigned int) w1, (unsigned int) w2);
		if (--stopondiff <= 0)
			//printf("return 1\n");
			return(-1);
			//exit(1);
		break;
	case 32:
		if( _verbose == true )
		printf("Scanline %lu, pixel %lu, sample %d: %08x %08x\n",
		    (unsigned long) row, (unsigned long) pix, sample,
		    (unsigned int) w1, (unsigned int) w2);
		if (--stopondiff <= 0)
			//printf("return 1\n");
			return(-1);
			//exit(1);
		break;
	default:
		break;
	}

	return(0);
}

static int
PrintFloatDiff(uint32 row, int sample, uint32 pix, double w1, double w2)
{
	if (sample < 0)
		sample = 0;
	switch (bitspersample) {
	case 32: 
		if( _verbose == true )
		printf("Scanline %lu, pixel %lu, sample %d: %g %g\n",
		    (long) row, (long) pix, sample, w1, w2);
		if (--stopondiff <= 0)
			//printf("return 1\n");
			return(-1);
			//exit(1);
		break;
	default:
		break;
	}
	return(0);
}

static int
SeparateCompare(int reversed, int sample, uint32 row,
		unsigned char* cp1, unsigned char* p2)
{
	uint32 npixels = imagewidth;
	int pixel;

	cp1 += sample;
	for (pixel = 0; npixels-- > 0; pixel++, cp1 += samplesperpixel, p2++) {
		if (*cp1 != *p2) {
			if( _verbose == true )
			printf("SeparateCompare Scanline %lu, pixel %lu, sample %ld: ", (long) row, (long) pixel, (long) sample);
			if (reversed)
				if( _verbose == true )
				printf("%02x %02x\n", *p2, *cp1);
			else
				if( _verbose == true )
				printf("%02x %02x\n", *cp1, *p2);
			if (--stopondiff <= 0)
				return(1);
			//exit(1);
		}
	}

	return 0;
}

static int
checkTag(TIFF* tif1, TIFF* tif2, int tag, char* name, void* p1, void* p2)
{

	if (TIFFGetField(tif1, tag, p1)) {
		if (!TIFFGetField(tif2, tag, p2)) {
			printf("%s tag appears only in %s\n",
			    name, TIFFFileName(tif1));
			return (0);
		}
		return (1);
	} else if (TIFFGetField(tif2, tag, p2)) {
		printf("%s tag appears only in %s\n", name, TIFFFileName(tif2));
		return (0);
	}
	return (-1);
}

#define	CHECK(cmp, fmt) {				\
	switch (checkTag(tif1,tif2,tag,name,&v1,&v2)) {	\
	case 1:	if (cmp)				\
	case -1:	return (1);			\
		printf(fmt, name, v1, v2);		\
	}						\
	return (0);					\
}

static int
CheckShortTag(TIFF* tif1, TIFF* tif2, int tag, char* name)
{
	uint16 v1, v2;
	CHECK(v1 == v2, "%s: %u %u\n");
}

static int
CheckShort2Tag(TIFF* tif1, TIFF* tif2, int tag, char* name)
{
	uint16 v11, v12, v21, v22;

	if (TIFFGetField(tif1, tag, &v11, &v12)) {
		if (!TIFFGetField(tif2, tag, &v21, &v22)) {
			printf("%s tag appears only in %s\n",
			    name, TIFFFileName(tif1));
			return (0);
		}
		if (v11 == v21 && v12 == v22)
			return (1);
		printf("%s: <%u,%u> <%u,%u>\n", name, v11, v12, v21, v22);
	} else if (TIFFGetField(tif2, tag, &v21, &v22))
		printf("%s tag appears only in %s\n", name, TIFFFileName(tif2));
	else
		return (1);
	return (0);
}

static int
CheckShortArrayTag(TIFF* tif1, TIFF* tif2, int tag, char* name)
{
	uint16 n1, *a1;
	uint16 n2, *a2;

	if (TIFFGetField(tif1, tag, &n1, &a1)) {
		if (!TIFFGetField(tif2, tag, &n2, &a2)) {
			printf("%s tag appears only in %s\n",
			    name, TIFFFileName(tif1));
			return (0);
		}
		if (n1 == n2) {
			char* sep;
			uint16 i;

			if (memcmp(a1, a2, n1 * sizeof(uint16)) == 0)
				return (1);
			printf("%s: value mismatch, <%u:", name, n1);
			sep = "";
			for (i = 0; i < n1; i++)
				printf("%s%u", sep, a1[i]), sep = ",";
			printf("> and <%u: ", n2);
			sep = "";
			for (i = 0; i < n2; i++)
				printf("%s%u", sep, a2[i]), sep = ",";
			printf(">\n");
		} else
			printf("%s: %u items in %s, %u items in %s", name,
			    n1, TIFFFileName(tif1),
			    n2, TIFFFileName(tif2)
			);
	} else if (TIFFGetField(tif2, tag, &n2, &a2))
		printf("%s tag appears only in %s\n", name, TIFFFileName(tif2));
	else
		return (1);
	return (0);
}

static int
CheckLongTag(TIFF* tif1, TIFF* tif2, int tag, char* name)
{
	uint32 v1, v2;
	CHECK(v1 == v2, "%s: %u %u\n");
}

static int
CheckFloatTag(TIFF* tif1, TIFF* tif2, int tag, char* name)
{
	float v1, v2;
	CHECK(v1 == v2, "%s: %g %g\n");
}

static int
CheckStringTag(TIFF* tif1, TIFF* tif2, int tag, char* name)
{
	char *v1, *v2;
	CHECK(strcmp(v1, v2) == 0, "%s: \"%s\" \"%s\"\n");
}

static void
leof(const char* name, uint32 row, int s)
{

	printf("%s: EOF at scanline %lu", name, (unsigned long)row);
	if (s >= 0)
		printf(", sample %d", s);
	printf("\n");
}

/* vim: set ts=8 sts=8 sw=8 noet: */
/*
 * Local Variables:
 * mode: c
 * c-basic-offset: 8
 * fill-column: 78
 * End:
 */

