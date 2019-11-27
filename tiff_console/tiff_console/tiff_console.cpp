// tiff_console.cpp : Defines the entry point for the console application.
//

// Remove blank pages
// Remove pages by page number
// Merge two TIFFs by placing the pages of the second TIFF at the end of the first TIFF

#include "stdafx.h"
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include "tiffio.h"
#include <string>
#include <vector>
#include <sstream> 
#include <fstream>
#include <iostream>
#include "tiffio.hxx"
#include "tiff_console.h"

bool verbose = false;
std::string version = "1.0.0";

void Usage()
{
	const char *usage =
"Manipulate TIFF image files. Remove page by number, remove blank page, concatenate two TIFF files.\n"
"Remove a blank page:\n"
"    app.exe  -remove-blank  <TIFF file name> <output TIFF file name>\n"
"Remove a page by page number:\n"
"    app.exe  -remove-page <page number> <TIFF file name>  <output TIFF file name>\n" 
"Concatenate two TIFF files:\n"
"    app.exe  -cat  <TIFF file name 1> <TIFF file name 2> <output TIFF file name>\n"
"Get version info:\n"
"	 app.exe -version\n"
"Get help:\n"
"    app.exe -help\n";
	std::cout << usage << std::endl;
}

void Version()
{
	std::cout << "app.exe version " << version << " author David Lyder" << std::endl;
}

int _tmain(int argc,  char* argv[])
{
	bool rv = true;

	if( argc == 1 )
	{
		Usage();
		return(0);
	}
	std::vector<std::string> arguments = std::vector<std::string>(argv, argv + argc);

	if( arguments[1] == "-version")
	{
		Version();
	}
	else if ( arguments[1] == "-remove-blank" && argc == 4)
	{
		std::cout << arguments[0] << " " << arguments[1] << " " << arguments[2] << " " <<arguments[3] << std::endl;
		rv = RemoveBlankPage(arguments[2],  GetDirectory(arguments[2]), arguments[3]);
	}
	else if ( arguments[1] == "-remove-page" && argc == 5)
	{
		std::cout << arguments[0] << " " << arguments[1] << " " << arguments[2] << " " <<arguments[3] << " " <<arguments[4] << std::endl;
		rv = RemoveByPage(arguments[3], arguments[4], stoi(arguments[2]));
	}
	else if ( arguments[1] == "-cat" && argc == 5)
	{
		std::cout << arguments[0] << " " << arguments[1] << " " << arguments[2] << " " <<arguments[3] << " " <<arguments[4] << std::endl;
		std::vector<std::string> inFiles;
		inFiles.push_back(arguments[2]);
		inFiles.push_back(arguments[3]);
		rv = MergeTiffs( inFiles, arguments[4]);
	}
	else
	{
		Usage();
	}

	if( rv == true )
		return(0);	
	else
		return(-1);
}



