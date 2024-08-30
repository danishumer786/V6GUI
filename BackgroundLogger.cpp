#include <filesystem>

#include "BackgroundLogger.h"
#include "../CommonFunctions.h"
#include "../ErrorMessageStream.h"
#include "../LoginManager.h"
#include "../WindowsFunctions.h"

using namespace std;


string GetLogBaseDirectory() {
	return GetAppDataPath() + "Logs\\";
}


BackgroundLogger::BackgroundLogger(string laser_serial_number, string laser_model) {
	SetSerialNumberAndModel(laser_serial_number, laser_model);
}

BackgroundLogger::BackgroundLogger() {
}


void BackgroundLogger::SetSerialNumberAndModel(string laser_serial_number, string laser_model) {
	laserSerialNumber = laser_serial_number;
	laserModel = laser_model;

	// Avoid invalid characters that could cause program to crash (filesystem::directory_iterator(logDirectory) - 5/1/24)
	laserSerialNumber = replaceSubstr(laserSerialNumber, "/", "_");
	laserSerialNumber = replaceSubstr(laserSerialNumber, "\\", "_");
	laserModel = replaceSubstr(laserModel, "/", "_");
	laserModel = replaceSubstr(laserModel, "\\", "_");
}


void BackgroundLogger::Init() {
	// All background loggers will have their log data encrypted
	// Custom lines added by CommitLine will remain unencrypted
	SetEncryptOption(true);

	InitLogDirectory();
	InitLogFilePath();
	SetFilePath(logFilePath);

	// Write these non-encrypted lines to the top of the log file for traceability
	
	// If logged in, add username to top of log file
	if (LoginManager::GetInstance().IsLoggedIn())
		CommitLineMetadata("User: " + LoginManager::GetInstance().GetLoggedInUsername());
	CommitLineMetadata(laserSerialNumber + " - " + laserModel);
	CommitLineMetadata(GenerateDateString());
	CommitLineMetadata(GenerateTimeString());
	CommitLineMetadata(GetLogType());
}


void BackgroundLogger::InitLogDirectory() {
	// Build the path to the directory that will contain the log file.
	// e.g., 
	//			C:/Users/user1/AppData/Local/PhotonicsIndustries/
	//				Logs/
	//					23-001_RX-355-3/
	//						2022-2-8/
	//							LaserStateLogs/
	//
	string baseDirectoryName = GetLogBaseDirectory();
	string laserDirectoryName = laserSerialNumber + "_" + laserModel+ "\\";
	string dateDirectoryName = GenerateDateString() + "\\";
	string logTypeDirectoryName = GetLogType(); // <-- This is the virtual function that derived classes must override

	logDirectory = baseDirectoryName + laserDirectoryName + dateDirectoryName + logTypeDirectoryName;

	// Create the path to the directory if necessary
	filesystem::create_directories(logDirectory);

	// Check if directory creation succeeded
	if (!filesystem::exists(logDirectory)) {
		e << "Failed to create log directory: " << logDirectory << endl;
	}
}

string BackgroundLogger::GetLogDirectory() const {
	return logDirectory;
}

void BackgroundLogger::InitLogFilePath() {
	logFilePath = logDirectory + "\\" + GetLogFilename();
}

string BackgroundLogger::GetLogFilename() {
	// Create a log file name with the name being an integer one higher than
	//   the highest log file name in the folder.
	// E.g., ".._1.log" , ".._2.log" , ".._3.log" , ...
	//
	// Format: LogType_(SN#)_(Model)_(Date)_LogID.log
	return GetLogType() + "_(" + laserSerialNumber + ")_(" + laserModel + ")_(" + 
		GenerateDateString() + ")_" + GenerateLogIDString() + EXTENSION;
}

string BackgroundLogger::GenerateLogIDString() const {
	return to_string(FindHighestLogFileNumber() + 1);
}

int BackgroundLogger::FindHighestLogFileNumber() const {
	// Find the highest number appearing at the end of all filenames in the 
	// log files folder.
	// Example:
	//		".._1.log"
	//		".._2.log"
	//	--> ".._3.log"
	//   Returns "3"

	int maxID = 0;

	for (const auto& file : filesystem::directory_iterator(logDirectory)) {
		string filename = file.path().filename().string();

		string extractedGuiSessionNumber = "";
		bool reachedExtensionEnd = false;
		for (string::reverse_iterator it = filename.rbegin(); it != filename.rend(); ++it) {
			if (reachedExtensionEnd and (*it == '_' or !isdigit(*it)))
				break;
			if (reachedExtensionEnd and isdigit(*it))
				extractedGuiSessionNumber = *it + extractedGuiSessionNumber;
			if (*it == '.')
				reachedExtensionEnd = true;
		}

		if (extractedGuiSessionNumber.length() > 0) {
			if (ToIntSafely(extractedGuiSessionNumber) > maxID)
				maxID = ToIntSafely(extractedGuiSessionNumber);
		}
	}
	return maxID;
}
