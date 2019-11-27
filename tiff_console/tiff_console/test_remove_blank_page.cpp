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

void TestRemoveBlankPage()
{
	std::string inFile("..\\..\\tiff_console\\input\\multipage.tif");
	std::string outPath("..\\..\\tiff_console\\output\\");
	std::string outFile("..\\..\\tiff_console\\output\\multipage_without_blank.tif");
	bool rv = RemoveBlankPage(inFile, outPath, outFile);
	printf("Removed blank page status[%d]. See %s\nPress enter to continue\n", rv, outFile);
	std::string s;
    std::getline(std::cin, s);
}