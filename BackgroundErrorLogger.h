/**
*  Background Error Logger - Easy error logging using "<<" operator.
*	Works together with ErrorMessageStream.
*
* - Allows error logging while GUI is using this LaserController project as a dll
* - Writes to a .log file automatically created for each GUI session,
*	located in "ErrorLogFiles" folder in local app data Logs folder for this laser.
*
* Usage:
* - Use "e" just like "cout".
* - Outputs message to both log file and [cout] (only in Debug configuration), so
*   if you are using the console, the message will display to the console
*   AND get logged to the error log file.
* - Inserts a header with data and time and " |*|*| Error - " label in front of error message that gets logged.
* - Must terminate error message with [endl]
*
* Example:
*	int errorCode = 3;
*   e << "This is an error! Error code:" << errorCode << "."  <<  endl;
*	e << "This is another error!"  <<  endl;
*
* In error log file:
*   2022-5-4,18:47:46,1,This is an error! Error code:3.
*   2022-5-4,18:47:46,2,This is another error!
*
* @file BackgroundErrorLogger.h - formerly "ErrorLogger.h" created 5/4/22
* @author James Butcher
* @created February 2023
* @version 1.1 - 2/16/23 - added initialization delay
*/
#pragma once

#include "BackgroundLogger.h"


class BackgroundErrorLogger : public BackgroundLogger {

private:
	////////////////////////////
	// Singleton methods
	BackgroundErrorLogger();
	~BackgroundErrorLogger();
	BackgroundErrorLogger(const BackgroundErrorLogger&);
	const BackgroundErrorLogger& operator=(const BackgroundErrorLogger&);
	////////////////////////////

	// Need these flags to delay initializing (adding header metadata) until 
	// after MainLaserController loads and adds serial number and laser model
	// information and there is at least 1 error to add.
	// This prevents cluttering error log files with header information
	// even when there are no errors.
	bool isInitialized = false; 
	bool laserLoaded = false;

	std::string tempErrorFilePath;

protected:
	virtual std::string GetLogType() override { return "ErrorLogs"; }

public:
	LOG_API static BackgroundErrorLogger& GetInstance();

	LOG_API void Init();
	LOG_API bool IsInitialized();
	LOG_API bool DetectedErrorBeforeMainLaserControllerLoaded();
	LOG_API bool LaserLoaded();
	LOG_API bool SetLaserLoaded(bool loaded);
	LOG_API void TransferTemporaryErrorLogs();

	// Call this when disconnecting from a laser
	LOG_API void ResetToTemporaryOutputFile();

};
