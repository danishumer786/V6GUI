/**
* Communication Settings Panel - One of the pages in the settings notebook
* 
*  - Manual RS 232 commands tool
*  - RS 232 commands logging tool
* 
* @file CommunicationPage.h
* @author James Butcher
* @created 9/29/22
* @version 2.0  11/8/22 - Added commands logging functionality
*/

#pragma once

#include <wx/wx.h>
#include <wx/spinctrl.h>

#include "../CommonGUIComponents/TimedStatusMessage.h"
#include "Loggers/RS232CommandsLogger.h"
#include "SettingsPage_Base.h"



class CommunicationPage : public SettingsPage_Base {

public:
    CommunicationPage(std::shared_ptr<MainLaserControllerInterface> _lc, wxWindow* parent);

    void Init();
    void RefreshAll();
    void RefreshStrings();
    void RefreshVisibility();

protected:
    std::shared_ptr<RS232CommandsLogger> commandsLogger = nullptr;
    wxTimer commandLoggingTimer;
    bool loggingStarted = false;
    bool timeParametersCorrect;
    int responseWaitTimeInMs = 100;
    int logTimeIntervalInMs = 1000;

    wxPanel* ManualRS232CommandsPanel;
    wxStaticText* ManualRS232CommandsTitle;
    wxStaticText* ManualRS232CommandLabel;
    wxTextCtrl* ManualRS232CommandTextCtrl;
    wxCheckBox* ManualRS232ChecksumCheckBox;
    wxStaticText* ManualRS232ResponseLabel;
    wxTextCtrl* ManualRS232ResponseTextCtrl;
    wxPanel* CommandLoggingPanel;
    wxStaticText* CommandLoggingLabel;
    wxButton* CommandLoggingButton;
    wxStaticText* CommandLoggingTimeIntervalLabel;
    wxSpinCtrl* CommandLoggingTimeIntervalSpinCtrl;
    wxStaticText* CommandLoggingWaitTimeLabel;
    wxSpinCtrl* CommandLoggingWaitTimeSpinCtrl;
    TimedStatusMessage* LoggingStatusMessage;

    void RefreshCommandLoggingButton();
    void RefreshCommandLoggingParameters();

    void OnRS232CommandEntered(wxCommandEvent& evt);
    void OnStartLoggingButtonClicked(wxCommandEvent& evt);
    void OnLogCommandTimerTick(wxTimerEvent& evt);
    std::string GetResponseFromCommand();
    void StartLogging();
    void StopLogging();

};

