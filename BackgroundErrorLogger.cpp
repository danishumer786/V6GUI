#include <filesystem>

#include "BackgroundErrorLogger.h"
#include "../WindowsFunctions.h"

using namespace std;


const string TEMP_ERROR_FILE_NAME = "errorLogTemp.log";


// ------ For Singleton implementation ------
BackgroundErrorLogger::BackgroundErrorLogger() {
	// When first created, set target file to a temporary error log output file.
	// This will capture all error messages before mainLaserController loads.
	// Once MainLaserController loads, can finally call Init() on this
	// logger, which will redirect output to the correct error log file in the
	// correct directory for this laser, date, and category.
	tempErrorFilePath = GetAppDataPath() + TEMP_ERROR_FILE_NAME;
	filesystem::create_directory(GetAppDataPath());
	SetFilePath(tempErrorFilePath);
}

BackgroundErrorLogger::~BackgroundErrorLogger() {
}
BackgroundErrorLogger::BackgroundErrorLogger(const BackgroundErrorLogger&) { }
const BackgroundErrorLogger& BackgroundErrorLogger::operator=(const BackgroundErrorLogger&) {
	return BackgroundErrorLogger::GetInstance();
}
// Get the single ErrorLogger instance. Create it if none exists.
BackgroundErrorLogger& BackgroundErrorLogger::GetInstance() {
	static BackgroundErrorLogger errorLogger;
	return errorLogger;
}
// -----------------------------------------

void BackgroundErrorLogger::Init() {
	BackgroundLogger::Init();
	isInitialized = true;
	AddColumn("Error #");
	AddColumn("Error");
	WriteHeaderLine();
}

bool BackgroundErrorLogger::IsInitialized() {
	return isInitialized;
}

bool BackgroundErrorLogger::DetectedErrorBeforeMainLaserControllerLoaded() {
	if (filesystem::exists(tempErrorFilePath)) {
		if (!filesystem::is_empty(tempErrorFilePath))
			return true;
	}
	return false;
}

bool BackgroundErrorLogger::LaserLoaded() {
	return laserLoaded;
}

bool BackgroundErrorLogger::SetLaserLoaded(bool loaded) {
	return laserLoaded = loaded;
}

void BackgroundErrorLogger::TransferTemporaryErrorLogs() {

	// Open temporary error log file
	ifstream tempErrorLogFile(tempErrorFilePath);

	// Open main error log file
	logFile.open(filePath, ios::app);

	// Transfer contents of temporary log file to main log file
	if (tempErrorLogFile.is_open() and logFile.is_open()) {
		string line;
		while (getline(tempErrorLogFile, line)) {
			logFile << line << "\n";
		}
	}
	tempErrorLogFile.close();
	logFile.close();

	// Remove temporary log file
	filesystem::remove(tempErrorFilePath);

}

void BackgroundErrorLogger::ResetToTemporaryOutputFile() {
	Reset();
	isInitialized = false;
	laserLoaded = false;
	SetFilePath(tempErrorFilePath);
}
