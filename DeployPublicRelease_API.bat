@ECHO OFF
:: =============================================================================================================
:: Created: 5/18/23 - James Butcher
:: Last updated: 11/10/23
::
:: This script is a one-click tool for carrying out all the steps needed for deploying a new version of the
:: API/SDK/DLL after building the solution in visual studio and running the DeployInternalRelease_API.bat
:: deployment script. It pushes the SDK .zip package, API version, and public release notes to SharePoint
:: where it can be downloaded by an existing older API via the APIUpdater module.
::
:: =============================================================================================================

SET /P VERSION=<LaserAPI\APIversion.txt

:PROMPT
SET /P CONFIRM=Do you want to deploy the latest API version (%VERSION%) to SharePoint? [y/n]
IF /I "%CONFIRM%" NEQ "y" GOTO END


::--------------------------------------------------------------------------------------------------------------
:: Create .zip file of API package in temporary folder



:: Zip and copy over API package
SET NETWORK-PATH=%NETWORK-SOFTWARE-FOLDER%\API\LatestVersion\LaserAPI
SET TEMP=temp\LaserAPI
MKDIR %TEMP%

XCOPY /Y/I/E %NETWORK-PATH%\lib %TEMP%\lib
XCOPY /Y/I/E %NETWORK-PATH%\docs %TEMP%\docs
XCOPY /Y/I/E %NETWORK-PATH%\include %TEMP%\include
XCOPY /Y/I/E %NETWORK-PATH%\vs %TEMP%\vs

cd temp
tar.exe -a -c -f LaserAPI.zip LaserAPI
cd ..


::--------------------------------------------------------------------------------------------------------------
:: Deploy to *UPDATE* repo - existing GUI installations in the field auto-update from this one.

SET SHAREPOINT-UPDATE-REPO="C:\Users\jbutcher\OneDrive - Photonics Industries\API"

:: Copy over files
XCOPY /Y/I LaserAPI\APIversion.txt %SHAREPOINT-UPDATE-REPO%
XCOPY /Y/I LaserAPI\PublicDocs\ReleaseNotes.txt %SHAREPOINT-UPDATE-REPO%
XCOPY /Y/I temp\LaserAPI.zip %SHAREPOINT-UPDATE-REPO%


::--------------------------------------------------------------------------------------------------------------
:: Deploy to *DOWNLOAD* repo - public repo where users can download manually for the first time.

SET SHAREPOINT-DOWNLOAD-REPO="C:\Users\jbutcher\OneDrive - Photonics Industries\Photonics Software\API\Photonics Laser Control API"

:: Copy over files
XCOPY /Y/I LaserAPI\PublicDocs\ReleaseNotes.txt %SHAREPOINT-DOWNLOAD-REPO%
XCOPY /Y/I LaserAPI\PublicDocs\README.txt %SHAREPOINT-DOWNLOAD-REPO%
XCOPY /Y/I %SHAREPOINT-UPDATE-REPO%\LaserAPI.zip %SHAREPOINT-DOWNLOAD-REPO%



:: Delete temporary folder for creating zip
RMDIR /s temp

ECHO --- Done. ---

:END
PAUSE