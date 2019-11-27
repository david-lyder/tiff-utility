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

void TestRemoveByPage()
{
	TestRemoveByPage(1);
	TestRemoveByPage(2);
	TestRemoveByPage(3);
	TestRemoveByPage(4);
	TestRemoveByPageTooSmall();
	TestRemoveByPageTooLarge();
}

void TestRemoveByPage(int page)
{
	std::string inFile("..\\..\\tiff_console\\input\\four_page.tif");
	std::string outFile("..\\..\\tiff_console\\output\\three_page_sans_");
	outFile += std::to_string(page);
	outFile += ".tif";
	bool rv = RemoveByPage( inFile, outFile, page);
	printf("Removed page %d, status[%d]. See %s\nPress enter to continue\n", page, rv, outFile);
	std::string s;
    std::getline(std::cin, s);
}

void TestRemoveByPageTooSmall()
{
	std::string inFile("..\\..\\tiff_console\\input\\four_page.tif");
	std::string outFile("..\\..\\tiff_console\\output\\four_page_copy1.tif");
	bool rv = RemoveByPage( inFile, outFile, -1);
	printf("Removed page %d, status[%d]. See %s\nPress enter to continue\n", -1, rv, outFile);
	std::string s;
    std::getline(std::cin, s);
}

void TestRemoveByPageTooLarge()
{
	std::string inFile("..\\..\\tiff_console\\input\\four_page.tif");
	std::string outFile("..\\..\\tiff_console\\output\\four_page_copy2.tif");
	bool rv = RemoveByPage( inFile, outFile, 6);
	printf("Removed page %d, status[%d]. See %s\nPress enter to continue\n", 6, rv, outFile);
	std::string s;
    std::getline(std::cin, s);
}


