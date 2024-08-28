


SET PUBLIC-DOC-FOLDER=%NETWORK-SOFTWARE-FOLDER%\API\LatestVersion\LaserAPI\docs

XCOPY /Y/I/E LaserAPI\DevDocs\Doxygen\html %PUBLIC-DOC-FOLDER%\html
XCOPY /Y/I/E LaserAPI\DevDocs\Doxygen\Images %PUBLIC-DOC-FOLDER%\html

:: Copy index file from html folder into main top-level docs folder so it can act as the entry point to the html doxygen docs
XCOPY /Y/I LaserAPI\DevDocs\Doxygen\html\index.html %PUBLIC-DOC-FOLDER%

:: Rename index file
RENAME %PUBLIC-DOC-FOLDER%\index.html %PUBLIC-DOC-FOLDER%\"Photonics Laser API".html

PAUSE