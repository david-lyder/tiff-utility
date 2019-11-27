#ifndef _TIF_CONSOLE_H_
#define _TIF_CONSOLE_H_

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

// MergeTiffs
bool MergeTiffs(std::vector<std::string> inFiles, std::string outFile);
void TestMergeTiffs();

// RemoveByPage
bool RemoveByPage(std::string inFile, std::string outFile, int pageNumber);
void TestRemoveByPage();
void TestRemoveByPage(int page);
void TestRemoveByPageTooSmall();
void TestRemoveByPageTooLarge();

// RemoveBlankPage
bool RemoveBlankPage(std::string inFile, std::string outPath, std::string outFile);
int GetBlankPageNumber(std::string inFile, std::string outPath);
void TestRemoveBlankPage();

// utils
std::string GetDirectory (const std::string& path);
int GetPageCount(std::string filename);
std::vector<std::ostringstream *> GetPagesAsStreams(std::string tifFile);
std::vector<TIFF *> GetPagesAsTiffs(std::string tifFile);
bool CopyTiffs(std::string inFile, std::string outPath);
bool IsBlankTiff(std::vector<std::string> inFiles);
void RunTests();

extern "C"
{
    int tiffcmp(TIFF* tif1, TIFF* tif2);
}


#endif