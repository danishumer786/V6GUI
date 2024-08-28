@ECHO OFF
:: =============================================================================================================
:: Created: 1/26/23 - James Butcher (formerly called ReleaseNewAPIVersionToNetwork.bat)
:: Last edited: 5/17/23 - Added step to zip API and copy it and Release Notes to OneDrive folder,
::   where I can then create a link and send to anyone who may use a beta version.
::
:: This script is a one-click tool for carrying out all the steps needed for in-house deploying a new version 
:: of the API. It pushes the DLLs, header files, test client, and everything else needed to use the API to
:: the network where it can be used on another computer in the building.
::
:: - Organizes files into a specific directory structure different from its source structure
::
::	Include files (.h and .cpp files)  	--->   include/ directory
::
::	Library files (DLLs, LIBs, EXEs, and version files) 	--->   lib/ directory
::
::	Public documents (PublicDocs folder) 	--->   docs/ directory
::
:: =============================================================================================================


:: Read the version number found in the "APIversion.txt" file in the Output folder
SET /P VERSION=<LaserAPI\APIversion.txt

:PROMPT
SET /P CONFIRM=Do you want to deploy the latest API version (%VERSION%) to the network? [y/n]
IF /I "%CONFIRM%" NEQ "y" GOTO END



:: Add header lines to release notes documents (DISABLED - COMMENTED OUT THIS FUNCTIONALITY BELOW)

FOR /F "tokens=2" %%i IN ('date /t') DO SET TODAYS_DATE=%%i

SET DEV-RELEASE-NOTES=LaserAPI\DevDocs\APIReleaseNotes_DEV.txt
SET PUBLIC-RELEASE-NOTES=LaserAPI\PublicDocs\ReleaseNotes.txt



:: ------------------------------------------------------------------------------------------------
:: Copy to Latest Version folder


:: File structure:

SET ROOT=%NETWORK-SOFTWARE-FOLDER%\API

	SET MAIN-FOLDER=%ROOT%\LatestVersion

		SET DEV-DOC-FOLDER=%MAIN-FOLDER%\DevDocs
			:: Dev documents go in here

		SET MAIN-API-FOLDER=%MAIN-FOLDER%\LaserAPI

			SET PUBLIC-DOC-FOLDER=%MAIN-API-FOLDER%\docs
				:: Public documents go in here

			SET PUBLIC-INCLUDE-FOLDER=%MAIN-API-FOLDER%\include
				:: Include files go in here (.h and .cpp files)

			SET PUBLIC-LIB-FOLDER=%MAIN-API-FOLDER%\lib
				:: All builds (x64 and x86, Debug and Release) of library files go in here (DLLs, LIBs, EXEs, and version files)
				IF NOT EXIST %PUBLIC-LIB-FOLDER% MKDIR %PUBLIC-LIB-FOLDER%

				SET x64-FOLDER=%PUBLIC-LIB-FOLDER%\x64
					:: 64-bit (x64) library files go here
					IF NOT EXIST %x64-FOLDER% MKDIR %x64-FOLDER%

					SET x64-DEBUG-FOLDER=%x64-FOLDER%\Debug
						:: 64-bit Debug library files go in here
						IF NOT EXIST %x64-DEBUG-FOLDER% MKDIR %x64-DEBUG-FOLDER%

					SET x64-RELEASE-FOLDER=%x64-FOLDER%\Release
						:: 64-bit Release library files go in here
						IF NOT EXIST %x64-RELEASE-FOLDER% MKDIR %x64-RELEASE-FOLDER%

				SET x86-FOLDER=%PUBLIC-LIB-FOLDER%\Win32
					:: 32-bit (x86) library files go here
					IF NOT EXIST %x86-FOLDER% MKDIR %x86-FOLDER%

					SET x86-DEBUG-FOLDER=%x86-FOLDER%\Debug
						:: 32-bit Debug library files go in here
						IF NOT EXIST %x86-DEBUG-FOLDER% MKDIR %x86-DEBUG-FOLDER%

					SET x86-RELEASE-FOLDER=%x86-FOLDER%\Release
						:: 32-bit Release library files go in here
						IF NOT EXIST %x86-RELEASE-FOLDER% MKDIR %x86-RELEASE-FOLDER%



:: Copy include files
XCOPY /Y/I LaserAPI\Laser.h %PUBLIC-INCLUDE-FOLDER%
XCOPY /Y/I LaserAPI\Definitions.h %PUBLIC-INCLUDE-FOLDER%
XCOPY /Y/I LaserAPI\Commands.h %PUBLIC-INCLUDE-FOLDER%
XCOPY /Y/I LaserAPI\CommandRouter.h %PUBLIC-INCLUDE-FOLDER%
XCOPY /Y/I LaserAPI\APIUpdater.h %PUBLIC-INCLUDE-FOLDER%
XCOPY /Y/I LaserAPI_TestClient\LaserAPI_TestClient.cpp %PUBLIC-INCLUDE-FOLDER%
XCOPY /Y/I LaserAPI_Updater\LaserAPI_Updater.cpp %PUBLIC-INCLUDE-FOLDER%


:: Copy library files

:: 32-bit Release
XCOPY /Y/I Output\CommonUtilities.dll %x86-RELEASE-FOLDER%
XCOPY /Y/I Output\LaserController.dll %x86-RELEASE-FOLDER%
XCOPY /Y/I Output\LaserAPI.dll %x86-RELEASE-FOLDER%
XCOPY /Y/I Output\zlibwapi.dll %x86-RELEASE-FOLDER%
XCOPY /Y/I Output\LaserAPI.lib %x86-RELEASE-FOLDER%
XCOPY /Y/I Output\LaserAPI_TestClient.exe %x86-RELEASE-FOLDER%
XCOPY /Y/I Output\LaserAPI_Updater.exe %x86-RELEASE-FOLDER%
XCOPY /Y/I ExternalPackages\dependencies_for_x86\Release\msvcp140.dll %x86-RELEASE-FOLDER%
XCOPY /Y/I ExternalPackages\dependencies_for_x86\Release\vcruntime140.dll %x86-RELEASE-FOLDER%
XCOPY /Y/I LaserAPI\APIversion.txt %x86-RELEASE-FOLDER%

::32-bit Debug
XCOPY /Y/I Output-DEBUG\CommonUtilities_d.dll %x86-DEBUG-FOLDER%
XCOPY /Y/I Output-DEBUG\LaserController_d.dll %x86-DEBUG-FOLDER%
XCOPY /Y/I Output-DEBUG\zlibwapi.dll %x86-DEBUG-FOLDER%
XCOPY /Y/I Output-DEBUG\LaserAPI_d.dll %x86-DEBUG-FOLDER%
XCOPY /Y/I Output-DEBUG\LaserAPI_d.lib %x86-DEBUG-FOLDER%
XCOPY /Y/I Output-DEBUG\LaserAPI_TestClient_d.exe %x86-DEBUG-FOLDER%
XCOPY /Y/I Output-DEBUG\LaserAPI_Updater_d.exe %x86-DEBUG-FOLDER%
XCOPY /Y/I ExternalPackages\dependencies_for_x86\Debug\ucrtbased.dll %x86-DEBUG-FOLDER%
XCOPY /Y/I ExternalPackages\dependencies_for_x86\Debug\msvcp140d.dll %x86-DEBUG-FOLDER%
XCOPY /Y/I ExternalPackages\dependencies_for_x86\Debug\vcruntime140d.dll %x86-DEBUG-FOLDER%
XCOPY /Y/I LaserAPI\APIversion.txt %x86-DEBUG-FOLDER%

:: 64-bit Release
XCOPY /Y/I Output_x64\CommonUtilities.dll %x64-RELEASE-FOLDER%
XCOPY /Y/I Output_x64\LaserController.dll %x64-RELEASE-FOLDER%
XCOPY /Y/I Output_x64\zlibwapi.dll %x64-RELEASE-FOLDER%
XCOPY /Y/I Output_x64\LaserAPI.dll %x64-RELEASE-FOLDER%
XCOPY /Y/I Output_x64\LaserAPI.lib %x64-RELEASE-FOLDER%
XCOPY /Y/I Output_x64\LaserAPI_TestClient.exe %x64-RELEASE-FOLDER%
XCOPY /Y/I Output_x64\LaserAPI_Updater.exe %x64-RELEASE-FOLDER%
XCOPY /Y/I ExternalPackages\dependencies_for_x64\Release\msvcp140.dll %x64-RELEASE-FOLDER%
XCOPY /Y/I ExternalPackages\dependencies_for_x64\Release\msvcp140_1.dll %x64-RELEASE-FOLDER%
XCOPY /Y/I ExternalPackages\dependencies_for_x64\Release\vcruntime140.dll %x64-RELEASE-FOLDER%
XCOPY /Y/I ExternalPackages\dependencies_for_x64\Release\vcruntime140_1.dll %x64-RELEASE-FOLDER%
XCOPY /Y/I LaserAPI\APIversion.txt %x64-RELEASE-FOLDER%

::64-bit Debug
XCOPY /Y/I Output-DEBUG_x64\CommonUtilities_d.dll %x64-DEBUG-FOLDER%
XCOPY /Y/I Output-DEBUG_x64\LaserController_d.dll %x64-DEBUG-FOLDER%
XCOPY /Y/I Output-DEBUG_x64\zlibwapi.dll %x64-DEBUG-FOLDER%
XCOPY /Y/I Output-DEBUG_x64\LaserAPI_d.dll %x64-DEBUG-FOLDER%
XCOPY /Y/I Output-DEBUG_x64\LaserAPI_d.lib %x64-DEBUG-FOLDER%
XCOPY /Y/I Output-DEBUG_x64\LaserAPI_TestClient_d.exe %x64-DEBUG-FOLDER%
XCOPY /Y/I Output-DEBUG_x64\LaserAPI_Updater_d.exe %x64-DEBUG-FOLDER%
XCOPY /Y/I ExternalPackages\dependencies_for_x64\Debug\ucrtbased.dll %x64-DEBUG-FOLDER%
XCOPY /Y/I ExternalPackages\dependencies_for_x64\Debug\msvcp140d.dll %x64-DEBUG-FOLDER%
XCOPY /Y/I ExternalPackages\dependencies_for_x64\Debug\msvcp140_1d.dll %x64-DEBUG-FOLDER%
XCOPY /Y/I ExternalPackages\dependencies_for_x64\Debug\vcruntime140d.dll %x64-DEBUG-FOLDER%
XCOPY /Y/I ExternalPackages\dependencies_for_x64\Debug\vcruntime140_1d.dll %x64-DEBUG-FOLDER%
XCOPY /Y/I LaserAPI\APIversion.txt %x64-DEBUG-FOLDER%



:: Copy document files

XCOPY /Y/I LaserAPI\PublicDocs\ %PUBLIC-DOC-FOLDER%
XCOPY /Y/I LaserAPI\DevDocs\ %DEV-DOC-FOLDER%

:: Copy doxygen html output to public docs folder
XCOPY /Y/I/E LaserAPI\DevDocs\Doxygen\html %PUBLIC-DOC-FOLDER%\html
XCOPY /Y/I/E LaserAPI\DevDocs\Doxygen\Images %PUBLIC-DOC-FOLDER%\html



:: ------------------------------------------------------------------------------------------------
:: Copy all of this again to its own archive version folder 

:: Example: API_v0.0.8__5_17_2023
SET VERSIONED-API-FOLDER-NAME=API_v%VERSION%__%TODAYS_DATE:~0,2%_%TODAYS_DATE:~3,2%_%TODAYS_DATE:~6,4%

SET ARCHIVE-FOLDER=%ROOT%\AllVersions

SET ARCHIVE-VERSION-FOLDER=%ARCHIVE-FOLDER%\%VERSIONED-API-FOLDER-NAME%
IF NOT EXIST %ARCHIVE-VERSION-FOLDER% MKDIR %ARCHIVE-VERSION-FOLDER%

XCOPY /Y/I/E %DEV-DOC-FOLDER% %ARCHIVE-VERSION-FOLDER%\DevDocs
XCOPY /Y/I/E %MAIN-API-FOLDER% %ARCHIVE-VERSION-FOLDER%\LaserAPI


:: ------------------------------------------------------------------------------------------------
:: Zip the contents of the API folder and copy the zip file and release notes into a new OneDrive folder

SET ONEDRIVE-FOLDER=%ONEDRIVE-SOFTWARE-FOLDER%\API
SET ONEDRIVE-VERSION-FOLDER=%ONEDRIVE-FOLDER%\%VERSIONED-API-FOLDER-NAME%
IF NOT EXIST %ONEDRIVE-VERSION-FOLDER% MKDIR %ONEDRIVE-VERSION-FOLDER%

XCOPY /Y/I LaserAPI\PublicDocs\ReleaseNotes.txt %ONEDRIVE-VERSION-FOLDER%
XCOPY /Y/I LaserAPI\PublicDocs\README.txt %ONEDRIVE-VERSION-FOLDER%

XCOPY /Y/I/E %MAIN-API-FOLDER%\lib %VERSIONED-API-FOLDER-NAME%\lib
XCOPY /Y/I/E %MAIN-API-FOLDER%\docs %VERSIONED-API-FOLDER-NAME%\docs
XCOPY /Y/I/E %MAIN-API-FOLDER%\include %VERSIONED-API-FOLDER-NAME%\include
XCOPY /Y/I/E %MAIN-API-FOLDER%\vs %VERSIONED-API-FOLDER-NAME%\vs

tar.exe -a -c -f %ONEDRIVE-VERSION-FOLDER%\%VERSIONED-API-FOLDER-NAME%.zip %VERSIONED-API-FOLDER-NAME%
RMDIR /s %VERSIONED-API-FOLDER-NAME%

ECHO --- Done. ---

:END
PAUSE













