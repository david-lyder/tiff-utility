
<h2>Usage</h2>
Manipulate TIFF image files. Remove page by number, remove blank page, concatenate two TIFF files.<br>       
Remove a blank page:  <br>
  app.exe  -remove-blank  <TIFF file name> <output TIFF file name>  <br>
Remove a page by page number:  <br>
    app.exe  -remove-page <page number> <TIFF file name>  <output TIFF file name>  <br>
Concatenate two TIFF files:  <br>
    app.exe  -cat  <TIFF file name 1> <TIFF file name 2> <output TIFF file name>  <br>
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
from developer command prompt for VS2012 <br>

One change was required in tif_config.h. The following code was added: <br>
#if defined(_MSC_VER) <br>
#define strtoll _strtoi64 <br>
#endif <br>
app.exe includes a reference to the static library libbtiff.lib <br>




