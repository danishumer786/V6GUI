#pragma once
#include "../CommonUtilities/Logging/LogObserver.h"
#include "wx/wx.h"


class RealTimeObserver : public LogObserver {
public:
    RealTimeObserver(wxTextCtrl* textCtrl) : textCtrl_(textCtrl) {}

    void onDataPointLogged(std::map<std::string, std::string> data) override {
        textCtrl_->AppendText("Received Data:\n");
        for (const auto& entry : data) {
            std::string logEntry = entry.first + ": " + entry.second + "\n";
            textCtrl_->AppendText(logEntry);
        }
    }

private:
    wxTextCtrl* textCtrl_;  // Pointer to the wxTextCtrl in the LoggingPage
};
/*class RealTimeObserver :public LogObserver {
public:
	RealTimeObserver(wxTextCtrl* textCtrl) :textCtrl_(textCtrl) {}
	void  onDataPointLogged(std::map<std::string, std::string> data)override {
		//Assume temperature data is under the key "Temperature"
		std::string temperatureLog = "Temperature: " + data["Temperature"] + "\n";

		textCtrl_->AppendText(temperatureLog);
	}


private:
	wxTextCtrl* textCtrl_;//Pointer to the wxTextCtrl i the LoggingPage 



};
*/




