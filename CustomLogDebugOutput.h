#pragma once

#include "Logging/LogObserver.h"


class CustomLogDebugOutput : public LogObserver {

private:
	void onDataPointLogged(std::map<std::string, std::string> data);

};

