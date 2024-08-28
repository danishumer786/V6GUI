@ECHO OFF
:: =============================================================================================================
:: Created: 5/19/22 - James Butcher
:: Last edited: 11/10/23 - Switched to using SharePoint instead of GitHub
::
:: This script is a one-click tool for carrying out all the steps needed for deploying a new version of the GUI
:: after building the solution in visual studio. It pushes the DLL, EXE, Image, and version files to SharePoint
:: where it can be downloaded by an existing older GUI in order to update itself.
::
::   	Step 1 - Update filenames index file using python script "generate_filenames_index_file.py"
::	Step 2 - Copy version and release notes files into output folder
:: 	Step 3 - Copy release notes and zip file into designated public GUI release folder on OneDrive
::   	Step 4 - Execute Git commands: git add, git commit, git push
:: =============================================================================================================

SET /P VERSION=<LaserGUI\version.txt

:PROMPT
SET /P CONFIRM=Do you want to deploy the latest GUI version (%VERSION%) to SharePoint ? [y/n]
IF /I "%CONFIRM%" NEQ "y" GOTO END


::--------------------------------------------------------------------------------------------------------------
:: Deploy to *UPDATE* repo - existing GUI installations in the field auto-update from this one.

SET SHAREPOINT-UPDATE-REPO="C:\Users\jbutcher\OneDrive - Photonics Industries\v6"

:: Copy over version and release notes files
XCOPY /Y/I LaserGUI\version.txt %SHAREPOINT-UPDATE-REPO%
XCOPY /Y/I LaserGUI\PublicDocs\ReleaseNotes.txt %SHAREPOINT-UPDATE-REPO%

:: Copy over installation files
XCOPY /Y/I "LaserControllerSetup\Laser Controller.msi" %SHAREPOINT-UPDATE-REPO%
XCOPY /Y/I "LaserControllerSetup\setup.exe" %SHAREPOINT-UPDATE-REPO%


::--------------------------------------------------------------------------------------------------------------
:: Deploy to *DOWNLOAD* repo - public repo where users can download manually for the first time.

SET SHAREPOINT-DOWNLOAD-REPO="C:\Users\jbutcher\OneDrive - Photonics Industries\Photonics Software\GUI v6\Photonics Laser Control v6.0"

:: Copy over version and release notes files [ REMOVED - DON'T NEED TO SHOW THIS TO THE GENERAL PUBLIC ]
::XCOPY /Y/I LaserGUI\version.txt %SHAREPOINT-DOWNLOAD-REPO%
::XCOPY /Y/I LaserGUI\PublicDocs\ReleaseNotes.txt %SHAREPOINT-DOWNLOAD-REPO%

:: Copy over installation files
XCOPY /Y/I "LaserControllerSetup\Laser Controller.msi" %SHAREPOINT-DOWNLOAD-REPO%
XCOPY /Y/I "LaserControllerSetup\setup.exe" %SHAREPOINT-DOWNLOAD-REPO%



ECHO --- Done. ---

:END
PAUSE