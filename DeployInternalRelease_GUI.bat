@ECHO OFF
:: =============================================================================================================
:: Created: 10/5/22 - James Butcher  (formerly called ReleaseNewGUIVersionToNetwork.bat)
:: Last edited: 5/18/23 - Added step to copy setup.exe, msi file, and public Release Notes to OneDrive folder,
::   where I can then create a link and send to anyone who may use a beta version.
::
:: This script is a one-click tool for carrying out all the steps needed for in-house deploying a new version 
:: of the GUI after building the solution in visual studio. It copies the DLL, EXE, Image, release notes and 
:: version files to the network where it can be installed on another computer in the building, and also to
:: the Photonics Software OneDrive folder where I can send a link to anyone outside the NY factory who may
:: use or test a beta version.
::
:: =============================================================================================================



:: --------------------------------------------------------------------------------------------------------------------
:: 1. Read the version number found in the "version.txt" file in the Output folder

SET /P VERSION=<Output\version.txt



:: --------------------------------------------------------------------------------------------------------------------
:: 2. Ask for confirmation before deploying

:PROMPT
SET /P CONFIRM=Do you want to deploy the latest GUI version (%VERSION%) to the network?  (Will not deploy to SharePoint)  [y/n]
IF /I "%CONFIRM%" NEQ "y" GOTO END



:: --------------------------------------------------------------------------------------------------------------------
:: 3. Add standard formatting to top of release notes files

SET DEV-RELEASE-NOTES=LaserGUI\DevDocs\GUIReleaseNotes_DEV.txt
SET PUBLIC-RELEASE-NOTES=LaserGUI\PublicDocs\ReleaseNotes.txt

SET TEMP=LaserAPI\DevDocs\tempnotes.txt

FOR /F "tokens=2" %%i IN ('date /t') DO SET TODAYS_DATE=%%i


python remove_tabs.py %DEV-RELEASE-NOTES% %PUBLIC-RELEASE-NOTES%


::copy %DEV-RELEASE-NOTES% %TEMP%

::echo( > %DEV-RELEASE-NOTES%
::echo( >> %DEV-RELEASE-NOTES%
::echo ------------------------------------------------------------------------------- >> %DEV-RELEASE-NOTES%
::echo Laser Control GUI version %VERSION% >> %DEV-RELEASE-NOTES%
::echo (C) Photonics Industries International, Inc. >> %DEV-RELEASE-NOTES%
::echo %TODAYS_DATE% >> %DEV-RELEASE-NOTES%

::echo( >> %DEV-RELEASE-NOTES%
::type %TEMP% >> %DEV-RELEASE-NOTES%

::del %TEMP%


::copy %PUBLIC-RELEASE-NOTES% %TEMP%

::echo( > %PUBLIC-RELEASE-NOTES%
::echo( >> %PUBLIC-RELEASE-NOTES%
::echo ------------------------------------------------------------------------------- >> %PUBLIC-RELEASE-NOTES%
::echo Laser Control GUI version %VERSION% >> %PUBLIC-RELEASE-NOTES%
::echo (C) Photonics Industries International, Inc. >>  %PUBLIC-RELEASE-NOTES%
::echo %TODAYS_DATE% >>  %PUBLIC-RELEASE-NOTES%

::echo( >> %PUBLIC-RELEASE-NOTES%
::type %TEMP% >>  %PUBLIC-RELEASE-NOTES%

::del %TEMP%



:: --------------------------------------------------------------------------------------------------------------------
:: 4. Create new folder on the network for this current version - e.g., "\GUI_v1.0.14__05_19_2022\[copied files]"

:: Example: GUI_v6.0.7__5_18_2023
SET VERSIONED-GUI-FOLDER-NAME=GUI_v%VERSION%__%TODAYS_DATE:~0,2%_%TODAYS_DATE:~3,2%_%TODAYS_DATE:~6,4%

SET NETWORK-GUI-FOLDER=%NETWORK-SOFTWARE-FOLDER%\GUI

::SET DEST-FOLDER="\\photonix04\Third Party Manuals and Software\Photonics Software\GUI\PILaserController2022\GUI"\GUI_v%VERSION%__%TODAYS_DATE:~0,2%_%TODAYS_DATE:~3,2%_%TODAYS_DATE:~6,4%
SET DEST-FOLDER=%NETWORK-GUI-FOLDER%\%VERSIONED-GUI-FOLDER-NAME%
::SET SETUP-FOLDER="\\photonix04\Third Party Manuals and Software\Photonics Software\GUI\PILaserController2022\"
IF NOT EXIST %DEST-FOLDER% MKDIR %DEST-FOLDER%



:: --------------------------------------------------------------------------------------------------------------------
:: 5. Copy application files to network

XCOPY /Y/I Output\Images %DEST-FOLDER%\Images
XCOPY /Y/I Output\CommonUtilities.dll %DEST-FOLDER%
XCOPY /Y/I Output\LaserController.dll %DEST-FOLDER%
XCOPY /Y/I Output\SimulatedLaserEditor.dll %DEST-FOLDER%
XCOPY /Y/I Output\zlibwapi.dll %DEST-FOLDER%
XCOPY /Y/I Output\LaserGUI.exe %DEST-FOLDER%
XCOPY /Y/I Output\version.txt %DEST-FOLDER%
XCOPY /Y/I/S Output\Locale\ %DEST-FOLDER%\Locale
XCOPY /Y/I "LaserControllerSetup\Laser Controller.msi" %DEST-FOLDER%
::XCOPY /Y/I "LaserControllerSetup\Laser Controller.msi" %NETWORK-SOFTWARE-FOLDER%
XCOPY /Y/I "LaserControllerSetup\setup.exe" %DEST-FOLDER%
::XCOPY /Y/I "LaserControllerSetup\setup.exe" %NETWORK-SOFTWARE-FOLDER%



:: --------------------------------------------------------------------------------------------------------------------
:: 6. Copy release notes to network

XCOPY /Y/I %DEV-RELEASE-NOTES% %DEST-FOLDER%
XCOPY /Y/I %DEV-RELEASE-NOTES% %NETWORK-SOFTWARE-FOLDER%
XCOPY /Y/I %PUBLIC-RELEASE-NOTES% %DEST-FOLDER%
XCOPY /Y/I %PUBLIC-RELEASE-NOTES% %NETWORK-SOFTWARE-FOLDER%



:: --------------------------------------------------------------------------------------------------------------------
:: 7. Copy application files and public release notes to OneDrive folder

SET ONEDRIVE-GUI-FOLDER=%ONEDRIVE-SOFTWARE-FOLDER%\"GUI v6"
SET ONEDRIVE-VERSION-FOLDER=%ONEDRIVE-GUI-FOLDER%\%VERSIONED-GUI-FOLDER-NAME%
IF NOT EXIST %ONEDRIVE-VERSION-FOLDER% MKDIR %ONEDRIVE-VERSION-FOLDER%

XCOPY /Y/I %PUBLIC-RELEASE-NOTES% %ONEDRIVE-VERSION-FOLDER%
XCOPY /Y/I "LaserControllerSetup\Laser Controller.msi" %ONEDRIVE-VERSION-FOLDER%
XCOPY /Y/I "LaserControllerSetup\setup.exe" %ONEDRIVE-VERSION-FOLDER%


:: --------------------------------------------------------------------------------------------------------------------
:: 8. Copy everything again to "Latest" folder on network for easy download with InstallGUIv6 tool


RMDIR %NETWORK-GUI-FOLDER%\Latest\ /S /Q 

XCOPY /Y/I/E %DEST-FOLDER% %NETWORK-GUI-FOLDER%\Latest\%VERSIONED-GUI-FOLDER-NAME%




ECHO --- Done. ---

:END
PAUSE