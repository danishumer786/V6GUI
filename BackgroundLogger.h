/**
* Background Logger: Writes structured data to a log file, with target log file
*   defaulting to a specified folder in computer's local app data folder,
*   with file name defaulting to an incrementing integer series.
*   Meant to be run continuously in the background during each GUI session.
* 
* All background loggers are set to log encrypted values.
* 
*  File structure:
* 
*	[ local app data folder ] /
* 
*		Logs /
* 
*			[ laser serial # ]_[ laser model ] /
* 
*				[ today's date ] /
* 
*					[ Log type name ] /
* 
*						[ log files indexed by GUI session # ]
* 
* 
*	Example: Three child loggers derived from BackgroundLogger:
*		ErrorLogs, LaserStateLogs, and UserActionLogs
*		
*		C:/Users/user1/AppData/Local/PhotonicsIndustries/
*			Logs/
*				23-001_RX-355-30/
*					2022-2-8/
*						ErrorLogs/
*							1.log
*							2.log
*							...
*						LaserStateLogs/
*							1.log
*							2.log
*							...
*						UserActionLogs/
*							1.log
*							2.log
*							...
* 
* 
*	NOT CURRENTLY IMPLEMENTED BUT POSSIBLY NEEDED IN THE FUTURE:
*   Unlike parent class LoggerBase, BackgroundLogger does not need a file path
*   supplied to its constructor. However, you may provide an optional prefix
*   string that will be appended to the default file names generated.
*   For example:
*		BackgroundLogger logger();  <-- Will generate filenames "1.log", "2.log", etc.
*		BackgroundLogger logger("autotune");  <-- Will generate filenames "1_autotune_1.log", "1_autotune_2.log", etc.
*
*
* @file BackgroundLogger.h
* @author James Butcher
* @created February 2023
* @version 1.0
*/

#pragma once

#include "LoggerBase.h"


// Get the top-level default directory for saving background logger log files.
LOG_API std::string GetLogBaseDirectory();


class BackgroundLogger : public LoggerBase {

protected:
	const std::string EXTENSION = ".log";
	std::string laserSerialNumber = "";
	std::string laserModel = "";
	std::string logDirectory = "";
	std::string logFilePath = "";

	// Derived classes must override this method with the desired folder name to 
	// contain this type of log files. e.g., "LaserStateLogs", "ErrorLogs", etc.
	virtual std::string GetLogType() = 0;

	void InitLogFilePath();
	void InitLogDirectory();
	std::string GenerateLogIDString() const;
	int FindHighestLogFileNumber() const;


public:
	LOG_API BackgroundLogger(std::string laser_serial_number,
							 std::string laser_model);


	LOG_API BackgroundLogger();
	LOG_API void SetSerialNumberAndModel(std::string laser_serial_number,
		std::string laser_model);

	// Initializes LoggerBase with the file path initialized and set
	//   to a pre-defined file name in a pre-defined directory.
	// You MUST call Init() before logging.
	LOG_API void Init();


	// For supplying default directory to file explorer window
	LOG_API std::string GetLogDirectory() const;
	// For supplying default filename to file explorer window
	LOG_API std::string GetLogFilename();
};

