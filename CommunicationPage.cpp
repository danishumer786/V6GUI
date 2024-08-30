#include <wx/gbsizer.h>

#include "../CommonFunctions_GUI.h"
#include "CommunicationPage.h"

using namespace std;


const wxString COMM_STR = _("Communication");
const wxString TITLE_STR = _("Manual RS232 Communication");
const wxString COMMAND_STR = _("Command:");
const wxString AUTOFILL_CHECKBOX_STR = _("Auto-fill checksum");
const wxString RESPONSE_STR = _("Response:");
const wxString CONTINUOUS_COMMAND_LOGGING_STR = _("Continuous Command Logging");
const wxString TIME_INTERVAL_STR = _("Time Interval (ms):");
const wxString RESPONSE_WAIT_TIME_STR = _("Response Wait Time (ms):");
const wxString TIME_INTERVAL_WARNING_STR = _("Can't start logging - time interval must be larger than response wait time");


CommunicationPage::CommunicationPage(shared_ptr<MainLaserControllerInterface> _lc, wxWindow* parent) :
	SettingsPage_Base(_lc, parent) {

	commandLoggingTimer.Bind(wxEVT_TIMER, &CommunicationPage::OnLogCommandTimerTick, this, commandLoggingTimer.GetId());

	wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);

	ManualRS232CommandsPanel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
	ManualRS232CommandsPanel->SetBackgroundColour(FOREGROUND_PANEL_COLOR);

	wxBoxSizer* ManualRS232CommandsSizer = new wxBoxSizer(wxVERTICAL);

	ManualRS232CommandsTitle = new wxStaticText(ManualRS232CommandsPanel, wxID_ANY, _(TITLE_STR), wxDefaultPosition, wxDefaultSize, 0);
	ManualRS232CommandsTitle->SetFont(FONT_MEDIUM_BOLD);
	ManualRS232CommandsSizer->Add(ManualRS232CommandsTitle, 0, wxALL | wxALIGN_CENTER_HORIZONTAL, 8);

	ManualRS232ChecksumCheckBox = new wxCheckBox(ManualRS232CommandsPanel, wxID_ANY, _(AUTOFILL_CHECKBOX_STR), wxDefaultPosition, wxDefaultSize, 0);
	ManualRS232ChecksumCheckBox->SetValue(true);
	ManualRS232ChecksumCheckBox->SetFont(FONT_VERY_SMALL_SEMIBOLD);
	ManualRS232CommandsSizer->Add(ManualRS232ChecksumCheckBox, 0, wxRIGHT | wxALIGN_RIGHT, 5);

	wxFlexGridSizer* ManualRS232CommandsInnerGridSizer = new wxFlexGridSizer(0, 2, 0, 0);

	ManualRS232CommandLabel = new wxStaticText(ManualRS232CommandsPanel, wxID_ANY, _(COMMAND_STR), wxDefaultPosition, wxDefaultSize, 0);
	ManualRS232CommandLabel->SetFont(FONT_VERY_SMALL_SEMIBOLD);
	ManualRS232CommandsInnerGridSizer->Add(ManualRS232CommandLabel, 0, wxALL | wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT, 5);

	ManualRS232CommandTextCtrl = new wxTextCtrl(ManualRS232CommandsPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_CENTER | wxTE_PROCESS_ENTER);
	ManualRS232CommandTextCtrl->Bind(wxEVT_TEXT_ENTER, &CommunicationPage::OnRS232CommandEntered, this, wxID_ANY);
	ManualRS232CommandsInnerGridSizer->Add(ManualRS232CommandTextCtrl, 0, wxEXPAND | wxALL | wxALIGN_CENTER_VERTICAL, 5);

	ManualRS232ResponseLabel = new wxStaticText(ManualRS232CommandsPanel, wxID_ANY, _(RESPONSE_STR), wxDefaultPosition, wxDefaultSize, 0);
	ManualRS232ResponseLabel->SetFont(FONT_VERY_SMALL_SEMIBOLD);
	ManualRS232CommandsInnerGridSizer->Add(ManualRS232ResponseLabel, 0, wxALL | wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT, 5);

	ManualRS232ResponseTextCtrl = new wxTextCtrl(ManualRS232CommandsPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_CENTER | wxTE_READONLY);
	ManualRS232CommandsInnerGridSizer->Add(ManualRS232ResponseTextCtrl, 0, wxEXPAND | wxALL | wxALIGN_CENTER_VERTICAL, 5);

	ManualRS232CommandsInnerGridSizer->AddGrowableCol(1);

	ManualRS232CommandsSizer->Add(ManualRS232CommandsInnerGridSizer, 1, wxEXPAND, 5);


	CommandLoggingPanel = new wxPanel(ManualRS232CommandsPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
	wxBoxSizer* CommandLoggingSizer = new wxBoxSizer(wxVERTICAL);

	CommandLoggingLabel = new wxStaticText(CommandLoggingPanel, wxID_ANY, _(CONTINUOUS_COMMAND_LOGGING_STR), wxDefaultPosition, wxDefaultSize, 0);
	CommandLoggingLabel->SetFont(FONT_MEDIUM_SEMIBOLD);

	CommandLoggingSizer->Add(CommandLoggingLabel, 0, wxALIGN_CENTER_HORIZONTAL | wxTOP, 15);

	wxBoxSizer* CommandLoggingControlsSizer = new wxBoxSizer(wxHORIZONTAL);

	CommandLoggingButton = new wxButton(CommandLoggingPanel, wxID_ANY, _(START_TEXT), wxDefaultPosition, wxDefaultSize, 0);
	CommandLoggingButton->Bind(wxEVT_BUTTON, &CommunicationPage::OnStartLoggingButtonClicked, this, wxID_ANY);
	CommandLoggingControlsSizer->Add(CommandLoggingButton, 0, wxALL, 5);

	CommandLoggingTimeIntervalLabel = new wxStaticText(CommandLoggingPanel, wxID_ANY, _(TIME_INTERVAL_STR), wxDefaultPosition, wxSize(115, -1), 0);
	CommandLoggingControlsSizer->Add(CommandLoggingTimeIntervalLabel, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);

	CommandLoggingTimeIntervalSpinCtrl = new wxSpinCtrl(CommandLoggingPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 10, 99999, 1000);
	CommandLoggingControlsSizer->Add(CommandLoggingTimeIntervalSpinCtrl, 0, wxRIGHT | wxALIGN_CENTER_VERTICAL, 15);

	CommandLoggingWaitTimeLabel = new wxStaticText(CommandLoggingPanel, wxID_ANY, _(RESPONSE_WAIT_TIME_STR), wxDefaultPosition, wxSize(155, -1), 0);
	CommandLoggingControlsSizer->Add(CommandLoggingWaitTimeLabel, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);

	CommandLoggingWaitTimeSpinCtrl = new wxSpinCtrl(CommandLoggingPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 1, 9999, 100);
	CommandLoggingControlsSizer->Add(CommandLoggingWaitTimeSpinCtrl, 0, wxRIGHT | wxALIGN_CENTER_VERTICAL, 5);

	CommandLoggingSizer->Add(CommandLoggingControlsSizer, 1, wxALIGN_CENTER_HORIZONTAL, 5);

	CommandLoggingPanel->SetSizer(CommandLoggingSizer);
	CommandLoggingPanel->Layout();
	CommandLoggingSizer->Fit(CommandLoggingPanel);
	ManualRS232CommandsSizer->Add(CommandLoggingPanel, 0, wxALIGN_CENTER_HORIZONTAL | wxALL, 5);

	LoggingStatusMessage = new TimedStatusMessage(CommandLoggingPanel, _("Logging"), _("Logging stopped"));
	CommandLoggingSizer->Add(LoggingStatusMessage, 0, wxALL | wxALIGN_CENTER_HORIZONTAL, 5);

	ManualRS232CommandsPanel->SetSizer(ManualRS232CommandsSizer);
	ManualRS232CommandsPanel->Layout();
	ManualRS232CommandsSizer->Fit(ManualRS232CommandsPanel);
	sizer->Add(ManualRS232CommandsPanel, 0, wxEXPAND | wxALL, 5);

	RefreshStrings();

	SetSizer(sizer);
	Layout();
	sizer->Fit(this);

    Init();
}


void CommunicationPage::Init() {
	RefreshCommandLoggingButton();
	RefreshCommandLoggingParameters();
}


void CommunicationPage::RefreshAll() {
}


void CommunicationPage::RefreshStrings() {
	SetName(_(COMM_STR));

	ManualRS232CommandsTitle->SetLabelText(_(TITLE_STR));
	ManualRS232CommandLabel->SetLabelText(_(COMMAND_STR));
	ManualRS232ChecksumCheckBox->SetLabelText(_(AUTOFILL_CHECKBOX_STR));
	ManualRS232ResponseLabel->SetLabelText(_(RESPONSE_STR));
	CommandLoggingLabel->SetLabelText(_(CONTINUOUS_COMMAND_LOGGING_STR));
	CommandLoggingTimeIntervalLabel->SetLabelText(_(TIME_INTERVAL_STR));
	CommandLoggingWaitTimeLabel->SetLabelText(_(RESPONSE_WAIT_TIME_STR));
	RefreshCommandLoggingButton();
}

void CommunicationPage::RefreshVisibility() {
	SetVisibilityBasedOnAccessMode(CommandLoggingPanel, 1);
}


void CommunicationPage::RefreshCommandLoggingButton() {
	SetBGColorBasedOnCondition(CommandLoggingButton, loggingStarted, TEXT_COLOR_RED, BUTTON_COLOR_INACTIVE);
	SetTextBasedOnCondition(CommandLoggingButton, loggingStarted, _(STOP_TEXT), _(START_TEXT));
}


void CommunicationPage::RefreshCommandLoggingParameters() {
	logTimeIntervalInMs = CommandLoggingTimeIntervalSpinCtrl->GetValue();
	responseWaitTimeInMs = CommandLoggingWaitTimeSpinCtrl->GetValue();
	if (logTimeIntervalInMs > responseWaitTimeInMs)
		timeParametersCorrect = true;
	else 
		timeParametersCorrect = false;
}


string CommunicationPage::GetResponseFromCommand() {
	// 9/29/22 - This contains some repeated code from the laser interactor, command controller, and command operator
	// - In the future, when I add more service and factory functionality, I should re-design the communication
	//   architecture to eliminate repeated code.
	string command = string(ManualRS232CommandTextCtrl->GetValue());
	STAGE_ACTION_ARGUMENTS(command)
	bool autoFillChecksum = ManualRS232ChecksumCheckBox->GetValue();
	responseWaitTimeInMs = CommandLoggingWaitTimeSpinCtrl->GetValue();
	string response = lc->SendManualRS232Command(command, autoFillChecksum, responseWaitTimeInMs);
	string formattedResponse;

	// Change response text color if error
	bool isError = (response[1] != '3' && response[1] != '2'); // Valid responses always start with "03..." (read response) or "02..." (write response)
	if (isError) {
		ManualRS232ResponseTextCtrl->SetForegroundColour(TEXT_COLOR_RED);
		formattedResponse = response;
	}
	else {
		ManualRS232ResponseTextCtrl->SetForegroundColour(TEXT_COLOR_BLACK);
		// Separate the extracted response payload
		string prefix = safe_substr(response, 0, 8);
		int payloadLength = HexStringToDecimal(safe_substr(response, 2, 2)) * 2;
		string responsePayload = safe_substr(response, 8, payloadLength);
		string suffix = safe_substr(response, 8 + payloadLength, response.length());
		formattedResponse = prefix + " " + responsePayload + " " + suffix;
	}

	ManualRS232ResponseTextCtrl->SetLabelText(to_wx_string(formattedResponse));
	this->Update();

	return formattedResponse;
}


void CommunicationPage::OnRS232CommandEntered(wxCommandEvent& evt) {
	STAGE_ACTION("Manual RS232 Command Entered")
	GetResponseFromCommand();
	LOG_ACTION()
}


void CommunicationPage::OnStartLoggingButtonClicked(wxCommandEvent& evt) {
	STAGE_ACTION("Start Logging Manual RS232 Commands button clicked")
	if (loggingStarted)
		StopLogging();
	else
		StartLogging();
	LOG_ACTION()
}


void CommunicationPage::OnLogCommandTimerTick(wxTimerEvent& evt) {
	RefreshCommandLoggingParameters();
	RefreshCommandLoggingButton();

	if (loggingStarted) {

		if (!timeParametersCorrect) {
			StopLogging();
		}

		string response = GetResponseFromCommand();

		vector<string> values;
		values.push_back(string(ManualRS232CommandTextCtrl->GetValue()));
		values.push_back(response);
		commandsLogger->LogDataPoint(values);
	}
}


void CommunicationPage::StartLogging() {
	RefreshCommandLoggingButton();
	RefreshCommandLoggingParameters();
	if (!timeParametersCorrect) {
		wxMessageBox(_(TIME_INTERVAL_WARNING_STR));
		return;
	}

	commandsLogger = make_shared<RS232CommandsLogger>(lc->GetSerialNumber(), lc->GetLaserModel());
	commandsLogger->CommitLineMetadata("Interval: " + to_string(logTimeIntervalInMs) + " ms, Response wait time : " + to_string(responseWaitTimeInMs) + "ms");
	commandsLogger->AddColumn("Command");
	commandsLogger->AddColumn("Response");
	commandsLogger->WriteHeaderLine();

	commandLoggingTimer.Start(logTimeIntervalInMs);
	loggingStarted = true;
	LoggingStatusMessage->ShowProcessingMessage();
	wxLogStatus(wxString::Format("Start logging command \"%s\" every %i ms with %i ms wait time",
		ManualRS232CommandTextCtrl->GetValue(),
		logTimeIntervalInMs,
		responseWaitTimeInMs
	));
	RefreshCommandLoggingButton();
}


void CommunicationPage::StopLogging() {
	LoggingStatusMessage->ShowTimedCompletionMessage();
	commandLoggingTimer.Stop();
	loggingStarted = false;
	RefreshCommandLoggingButton();
}
