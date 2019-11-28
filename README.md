
<h2>Usage</h2>
app.exe <br>
Manipulate TIFF image files. Remove page by number, remove blank page, concatenate two TIFF files.<br>       
Remove a blank page:  <br>
  app.exe  -remove-blank  [TIFF file name] [output TIFF file name]  <br>
Remove a page by page number:  <br>
    app.exe  -remove-page [page number] [TIFF file name]  [output TIFF file name]  <br>
Concatenate two TIFF files:  <br>
    app.exe  -cat  [TIFF file name 1] [TIFF file name 2] [output TIFF file name]  <br>
Get version info:  <br>
	 app.exe -version  <br>
Get help:  <br>
    app.exe -help  <br>

<h2>Examples</h2> 
The repo contains an input folder with suitable TIFF image files. <br>
blank.tif <br>
four_black.tif <br>
four_page.tif <br>
multipage.tif <br>
<b> NOTE: app.exe only supports uncompressed TIFF images. </b><br>
The utility at: https://online-converting.com/image/convert2tiff/
was used to create uncompressed TIFF images.

To remove a blank page: <br>
app.exe -remove-blank ..\input\multipage.tif blank_removed.tif <br>

To remove a page by mumber: <br>
app.exe -remove-page  2 ..\input\multipage.tif page_2_removed.tif <br>

To concatenate two files: <br>
app.exe -cat ..\input\four_page.tif ..\input\four_page.tif eight_page.tif <br>

<h2>Developer Notes</h2>
The tifflib distibution, tiff-4.0.10, is inluded in this repo. <br>
tifflib was built in release mode for Microsoft Visual Studio C++ using nmake: <br>
   nmake -f makefile.vc <br>
from the developer command prompt for VS2012 <br>

One change was required in tif_config.h. The following code was added: <br>
#if defined(_MSC_VER) <br>
#define strtoll _strtoi64 <br>
#endif <br>
app.exe includes a reference to the static library libtiff.lib <br>

<h3> Implementation decisions</h3>
Removing a page by number from a TIFF was relatively easy by cycling through the directories using TIFFReadDirectory, keeping track of the page count, and skipping TIFFReadScanline-TIFFWriteScanline for the requested page number. <br><br>

Merging TIFF files was accomplished using a similar technique, cycling through all of the input files and using IFFReadScanline-TIFFWriteScanline for every directory and writing all results to a new file. <br>

Identifying a blank TIFF proved to be difficult (perhaps there is a secret?). This is the method used: <br>
Cycle through the input TIFF and create a new TIFF file for each page <br>
Use the C code from the libtiff tools folder to create a "tiffcmp" function. Use the "tiffcmp" function to compare each page to a known blank TIFF image. <br>
If the blank image is found, return the page number and use the RemoveByPage function to remove the blank page. <br>

<b> Notes </b>
The TIFFStreamOpen function was useful in some cases, but in others errors were thrown regarding missing tags when the stream was used. <br>

<h2> Tests </h2>
The code includes tests for all functions: <br>
	TestRemoveByPage <br>
	TestRemoveBlankPage <br>
	TestMergeTiffs <br>







