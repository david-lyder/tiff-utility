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

void TestMergeTiffs()
{
	const char* in_files[] = {"..\\..\\tiff_console\\input\\four_page.tif", "..\\..\\tiff_console\\input\\four_black.tif"};
	std::vector<std::string> inFiles(in_files, in_files + 2);
	std::string outFile = "..\\..\\tiff_console\\output\\merged.tif";
	bool rv = MergeTiffs(inFiles, outFile);
	printf("Tiffs merged status:[%d]. See %s\nPress enter to continue\n", rv, outFile);
	std::string s;
    std::getline(std::cin, s);
}