@ECHO OFF
:: =============================================================================================================
:: Created: 5/19/22 - James Butcher
:: Last edited: 5/18/23 - Added step to make zip file of msi and setup files and put this and release notes
::   into designated public GUI release folder on OneDrive (step 3)
::
:: This script is a one-click tool for carrying out all the steps needed for deploying a new version of the GUI
:: after building the solution in visual studio. It pushes the DLL, EXE, Image, and version files to GitHub 
:: where it can be downloaded by an existing older GUI in order to update itself.
::
::   	Step 1 - Update filenames index file using python script "generate_filenames_index_file.py"
::	Step 2 - Copy version and release notes files into output folder
:: 	Step 3 - Copy release notes and zip file into designated public GUI release folder on OneDrive
::   	Step 4 - Execute Git commands: git add, git commit, git push
:: =============================================================================================================

:: Read the version number found in the "version.txt" file in the Output folder
SET /P VERSION=<Output\version.txt

:PROMPT
SET /P CONFIRM=Do you want to deploy the latest GUI version (%VERSION%) to GitHub? [y/n]
IF /I "%CONFIRM%" NEQ "y" GOTO END

:: Step 1 -----------------------------

python access_code_encryptor.py

:: TO BE REMOVED AFTER NEXT PUBLIC RELEASE - GITHUB BEING REPLACED WITH SHAREPOINT
python generate_filenames_index_file.py


:: Step 2 -----------------------------

XCOPY /Y LaserGUI\PublicDocs\ReleaseNotes.txt Output
XCOPY /Y LaserGUI\version.txt Output


:: Step 3 -----------------------------

::SET PUBLIC-RELEASE-FOLDER=%ONEDRIVE-SOFTWARE-FOLDER%\"GUI v6"\"Photonics Laser Control v6.0"
SET PUBLIC-RELEASE-FOLDER="C:\Users\jbutcher\OneDrive - Photonics Industries\Photonics Software\GUI v6\Photonics Laser Control v6.0"

:: Remove any existing files in this folder
DEL /S/Q %PUBLIC-RELEASE-FOLDER%\*

:: Copy GUI public release notes file into designated public GUI release folder on OneDrive
XCOPY /Y/I LaserGUI\PublicDocs\ReleaseNotes.txt %PUBLIC-RELEASE-FOLDER%

:: Create GUI zip file in designated public GUI release folder on OneDrive
FOR /F "tokens=2" %%i IN ('date /t') DO SET TODAYS_DATE=%%i
SET VERSIONED-GUI-FOLDER-NAME=GUI_v%VERSION%__%TODAYS_DATE:~0,2%_%TODAYS_DATE:~3,2%_%TODAYS_DATE:~6,4%
XCOPY /Y/I %NETWORK-SOFTWARE-FOLDER%\"Laser Controller.msi" %VERSIONED-GUI-FOLDER-NAME%
XCOPY /Y/I %NETWORK-SOFTWARE-FOLDER%\"setup.exe" %VERSIONED-GUI-FOLDER-NAME%
tar.exe -a -c -f %PUBLIC-RELEASE-FOLDER%\PhotonicsLaserControlV6_Setup.zip %VERSIONED-API-FOLDER-NAME%

RMDIR /s %VERSIONED-GUI-FOLDER-NAME%


:: Step 4 -----------------------------

cd Output
git add "fc.txt" "sc.txt" "version.txt" "setup.exe" "Laser Controller.msi" "filenames_index.txt" "ReleaseNotes.txt"
git commit -m "%VERSION%"
git push

ECHO --- Done. ---

:END
PAUSE