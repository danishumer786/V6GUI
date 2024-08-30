#include "AutotuneDiagnosticsPanel.h"
#include "../CommonFunctions_GUI.h"

using namespace std;

const wxString DIAGNOSTICS_STR = _("Diagnostics");
const wxString DIAGNOSTICS_TOOLTIP = _(
	"Autotune Diagnostics:\n"
	"  Helps diagnose issues with Autotune components.\n"
	"\n"
	"  Runs each component forward, then backward, fits a quadratic\n"
	"  curve to each, and logs the following data:\n"
    "   -Peak position in the forward direction\n"
    "   -Peak power in the forward direction\n"
    "   -Peak position in the backward direction\n"
    "   -Peak power in the backward direction\n"
    "   -The distance between the two peaks"
);

const wxString DIAGNOSTICS_RUNNING_MESSAGE_STR = _("Running");
const wxString DIAGNOSTICS_ERROR_STR = _("Diagnostics Error");
const wxString CONFIRM_DIAGNOSTICS_STR = _("Confirm Run Diagnostics");
const wxString CONFIRM_DIAGNOSTICS_MESSAGE = _(
	"The diagnostics process takes 10 - 30 minutes.\n"
	"Please don't close the GUI while it is running.\n"
	"\n"
	"Are you sure you want to run diagnostics?"
);
const wxString DIAGNOSTICS_FAULT_MESSAGE = _(
	"Detected laser fault during diagnostics.\n"
	"Resolve the fault and reset the laser to recover original state.\n"
	"Then you may attempt diagnostics again."
);

const wxString SAVE_FULL_LOG_STR = _("Save Full Log");
const wxString SAVE_STATISTICS_LOG_STR = _("Save Statistics");


// Handles continuously stepping the Autotune Diagnostics procedure
void StepAutotuneDiagnosticsThread(shared_ptr<AutotuneDiagnostics> diagnostics, vector<AutotuneComponentPanel*> autotuneComponentPanels) {
	while (true) {
		if (diagnostics->IsRunning()) {
			diagnostics->Step();
			if (!IsInAccessMode(GuiAccessMode::END_USER))
				wxLogStatus(to_wx_string(diagnostics->GetStepSummary()));
			for (auto component : autotuneComponentPanels)
				component->RefreshAll();
		}
		else
			break;
	}
}


AutotuneDiagnosticsPanel::AutotuneDiagnosticsPanel(
	shared_ptr<MainLaserControllerInterface> _lc,
	shared_ptr<AutotuneDiagnostics> _diagnostics,
	wxWindow* parent
) :
	wxPanel(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL | wxBORDER_THEME),
	lc(_lc),
	diagnostics(_diagnostics)
{


	this->SetAutoLayout(false);
	this->SetBackgroundColour(FOREGROUND_PANEL_COLOR);

	sizer = new wxBoxSizer(wxVERTICAL);

	title = new FeatureTitle(this, DIAGNOSTICS_STR, DIAGNOSTICS_TOOLTIP);
	sizer->Add(title, 0, wxALIGN_CENTER_HORIZONTAL, 5);

	mainButton = new wxButton(this, wxID_ANY, START_TEXT, wxDefaultPosition, wxSize(160, 35), 0);
	mainButton->SetFont(FONT_MEDIUM_BOLD);
	mainButton->SetBackgroundColour(BUTTON_COLOR_INACTIVE);
	mainButton->Bind(wxEVT_BUTTON, &AutotuneDiagnosticsPanel::OnMainButtonClicked, this);
	sizer->Add(mainButton, 0, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 5);

	runningMessage = new DynamicStatusMessage(this, DIAGNOSTICS_RUNNING_MESSAGE_STR, 300, 4);
	runningMessage->Hide();
	sizer->Add(runningMessage, 0, wxALIGN_CENTER_HORIZONTAL | wxALL | wxRESERVE_SPACE_EVEN_IF_HIDDEN, 5);


	// Save log buttons

	saveFullLogButton = new wxButton(this, wxID_ANY, SAVE_FULL_LOG_STR, wxDefaultPosition, wxDefaultSize, 0);
	saveFullLogButton->SetBackgroundColour(RESET_BUTTON_COLOR);
	saveFullLogButton->Bind(wxEVT_BUTTON, &AutotuneDiagnosticsPanel::OnSaveFullLogButtonClicked, this);
	sizer->Add(saveFullLogButton, 0, wxALIGN_CENTER_HORIZONTAL | wxBOTTOM, 1);
	saveFullLogButton->Hide();

	saveStatisticsLogButton = new wxButton(this, wxID_ANY, SAVE_STATISTICS_LOG_STR, wxDefaultPosition, wxDefaultSize, 0);
	saveStatisticsLogButton->SetBackgroundColour(RESET_BUTTON_COLOR);
	saveStatisticsLogButton->Bind(wxEVT_BUTTON, &AutotuneDiagnosticsPanel::OnSaveStatisticsLogButtonClicked, this);
	sizer->Add(saveStatisticsLogButton, 0, wxALIGN_CENTER_HORIZONTAL | wxBOTTOM, 5);
	saveStatisticsLogButton->Hide();


	this->SetSizer(sizer);
	this->Layout();
	sizer->Fit(this);

	Init();
	RefreshStrings();
}



//-----------------------------------------------------------------------------
// Initialization

void AutotuneDiagnosticsPanel::Init() {
}


//-----------------------------------------------------------------------------
// Refresh methods

void AutotuneDiagnosticsPanel::RefreshAll() {
	if (!lc->AutotunePowerIsEnabledForUse())
		return;
	RefreshMainButtonState();
	RefreshDiagnosticsProcedure();
	RefreshControlEnabled();
}

void AutotuneDiagnosticsPanel::RefreshMainButtonState() {
	SetTextBasedOnCondition(mainButton, diagnostics->IsRunning(), _(CANCEL_TEXT), _(START_TEXT));
	SetBGColorBasedOnCondition(mainButton, diagnostics->IsRunning(), TEXT_COLOR_RED, BUTTON_COLOR_INACTIVE);
}


void AutotuneDiagnosticsPanel::RefreshControlEnabled() {
	// Refresh main autotune power button enable state

	bool running = lc->LaserIsRunning();
	bool diagnosticsRunning = diagnostics->IsRunning();
	bool otherAutotuneRunning = lc->IsAutotuneOscillatorRunning() or lc->IsAutotunePowerRunning();

	if (diagnosticsRunning) {
		// Can always cancel Autotune at any time
		RefreshWidgetEnableBasedOnCondition(mainButton, true);
	}
	else {
		// To start Autotune, laser needs to be in running state and autotune oscillator cannot be running.
		bool canStartDiagnostics = running and !otherAutotuneRunning;
		RefreshWidgetEnableBasedOnCondition(mainButton, canStartDiagnostics);
	}

	// Refresh Re-Tune buttons enabled
	bool canRetuneComponent = running and !diagnosticsRunning and !otherAutotuneRunning;
	for (auto componentPanel : autotuneComponentPanels)
		RefreshWidgetEnableBasedOnCondition(componentPanel->retuneButton, canRetuneComponent);
}


void AutotuneDiagnosticsPanel::RefreshDiagnosticsProcedure() {

	if (startDiagnosticsTriggered) {

		if (diagnostics->IsFinished()) {

			wxLogStatus(to_wx_string(diagnostics->GetSummary()));
			runningMessage->Set(_("Finished."));
			runningMessage->StopCycling();

			// Unlock regular laser controller refreshes caused by main timer in cMain
			wxCommandEvent unlockEvent(UNLOCK_LASER_REFRESHES_EVENT, GetId());
			unlockEvent.SetEventObject(this);
			ProcessWindowEvent(unlockEvent);

			saveFullLogButton->Show();
			saveStatisticsLogButton->Show();
			startDiagnosticsTriggered = false;

			RefreshPanels();
		}
		else if (diagnostics->IsError()) {

			if (lc->HasHardFault() or lc->HasSoftFault()) {
				// Only show the dialog once
				if (!faultDialogShown) {
					faultDialogShown = true; // Must go first
					wxMessageDialog faultDialog(nullptr, _(DIAGNOSTICS_FAULT_MESSAGE), _(DIAGNOSTICS_ERROR_STR));
					faultDialog.ShowModal();
				}
			}
			else
				runningMessage->Set(_("ERROR - ") + diagnostics->GetErrorMessage());

			runningMessage->StopCycling();

			// Unlock regular laser controller refreshes caused by main timer in cMain
			wxCommandEvent unlockEvent(UNLOCK_LASER_REFRESHES_EVENT, GetId());
			unlockEvent.SetEventObject(this);
			ProcessWindowEvent(unlockEvent);

			saveFullLogButton->Show();
			saveStatisticsLogButton->Show();
			startDiagnosticsTriggered = false;

			RefreshPanels();
		}
		else if (diagnostics->IsRunning()) {
			runningMessage->Set(_(DIAGNOSTICS_RUNNING_MESSAGE_STR) + " - " + to_wx_string(diagnostics->GetProgressPercentage()) + "%");
		}
	}
}


void AutotuneDiagnosticsPanel::RefreshPanels() {
	this->GetParent()->Layout();
	this->GetParent()->Update();
	this->Refresh();
}



//-----------------------------------------------------------------------------
// Main Autotune-Diagnostics functionality

void AutotuneDiagnosticsPanel::RunFullDiagnostics() {
	diagnostics->Reset();

	for (auto panel : autotuneComponentPanels)
		panel->ClearAll();

	for (auto data : diagnosticsData) {
		if (data->type == "Motor")
			diagnostics->AddMotorComponent(data);
		else
			diagnostics->AddTemperatureComponent(data);
	}
	StartDiagnostics();
}


void AutotuneDiagnosticsPanel::StartDiagnostics() {

	diagnostics->Start();
	if (!diagnostics->CanStartDiagnostics()) {
		wxMessageBox(_("Error") + " - " + _(diagnostics->GetCantStartReason()), _(DIAGNOSTICS_ERROR_STR), wxICON_ERROR);
		runningMessage->Set(_("ERROR"));
		runningMessage->StopCycling();
		return;
	}
	runningMessage->Show();
	runningMessage->Set(_(DIAGNOSTICS_RUNNING_MESSAGE_STR));
	runningMessage->StartCycling();

	// Lock regular laser controller refreshes caused by main timer in cMain -> this could interfere with refreshes from within
	//	the Autotune procedure because it includes periodic wxGetApp().Yield() commands.
	wxCommandEvent lockEvent(LOCK_LASER_REFRESHES_EVENT, GetId());
	lockEvent.SetEventObject(this);
	ProcessWindowEvent(lockEvent);

	StartDiagnosticsStepThread();
	startDiagnosticsTriggered = true;
	faultDialogShown = false;

	RefreshPanels();
}


void AutotuneDiagnosticsPanel::StartDiagnosticsStepThread() {
	if (diagnosticsThread != nullptr) {
		diagnosticsThread->join();
	}
	diagnosticsThread = make_shared<std::thread>(StepAutotuneDiagnosticsThread, diagnostics, autotuneComponentPanels);
}


void AutotuneDiagnosticsPanel::CancelDiagnostics() {
	diagnostics->Cancel();
	runningMessage->Set(_("Canceled"));
	runningMessage->StopCycling();
	saveFullLogButton->Show();
	saveStatisticsLogButton->Show();
	RefreshPanels();
}



//-----------------------------------------------------------------------------
// Callbacks

void AutotuneDiagnosticsPanel::OnMainButtonClicked(wxCommandEvent& evt) {
	STAGE_ACTION("Main Autotune-Diagnostics button clicked")
		if (diagnostics->IsRunning())
			CancelDiagnostics();
		else {
			wxString msg = _(CONFIRM_DIAGNOSTICS_MESSAGE);
			wxMessageDialog confirmAutotuneDialog(nullptr, msg, _(CONFIRM_DIAGNOSTICS_STR), wxOK | wxCANCEL);
			confirmAutotuneDialog.SetOKLabel(_("Yes"));
			if (confirmAutotuneDialog.ShowModal() == wxID_OK) {
				RunFullDiagnostics();
				STAGE_ACTION_ARGUMENTS("Confirmed");
			}
			else {
				STAGE_ACTION_ARGUMENTS("Cancelled");
			}
		}
	LOG_ACTION()
}


// Assumes the individual component panel already reset the 
// Autotune Diagnostics Manager and added only itself for the next run
void AutotuneDiagnosticsPanel::OnRetuneButtonClicked(wxCommandEvent& evt) {
	STAGE_ACTION_("Autotune-Diagnostics Retune button clicked", to_string(evt.GetId() - 1))
	AutotuneComponentPanel* panel = mapIdToPlotPanel.at(evt.GetId() - 1);
	panel->ClearAll();
	diagnostics->Reset();
	if (panel->diagnosticData->type == MOTOR_STR)
		diagnostics->AddMotorComponent(panel->diagnosticData);
	else
		diagnostics->AddTemperatureComponent(panel->diagnosticData);

	StartDiagnostics();

	LOG_ACTION()
}


AutotuneDiagnosticsPanel::~AutotuneDiagnosticsPanel() {
	if (diagnosticsThread) {
		diagnosticsThread->join();
	}
}


void AutotuneDiagnosticsPanel::OnSaveFullLogButtonClicked(wxCommandEvent& evt) {
	STAGE_ACTION("Save Autotune-Diagnostics Full Log button clicked")
		wxString defaultPath = diagnostics->GetDefaultLogPath_Full();
	wxString defaultFilename = diagnostics->GetDefaultLogFilename_Full();

	wxFileDialog saveMemoryFileDialog(nullptr, (_("Save Log File")), defaultPath, defaultFilename, "LOG files (*.log)|*.log", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

	if (saveMemoryFileDialog.ShowModal() == wxID_OK) {
		string path = string(saveMemoryFileDialog.GetPath());
		STAGE_ACTION_ARGUMENTS(path)

			// If in higher access mode, save log unencrypted
			if (GetGUIAccessMode() == GuiAccessMode::SERVICE or GetGUIAccessMode() == GuiAccessMode::FACTORY)
				diagnostics->SaveLog_Full(path);
			else
				diagnostics->SaveLogEncrypted_Full(path);

		if (diagnostics->SaveLogSuccessful_Full())
			wxLogStatus(_("Save successful."));
		else
			wxLogStatus(_("Save failed."));
	}
	else {
		wxLogStatus(_("Save log file cancelled."));
	}
	LOG_ACTION()
}


void AutotuneDiagnosticsPanel::OnSaveStatisticsLogButtonClicked(wxCommandEvent& evt) {
	STAGE_ACTION("Save Autotune-Diagnostics Statistics Log button clicked")
		wxString defaultPath = diagnostics->GetDefaultLogPath_Statistics();
	wxString defaultFilename = diagnostics->GetDefaultLogFilename_Statistics();

	wxFileDialog saveMemoryFileDialog(nullptr, (_("Save Log File")), defaultPath, defaultFilename, "LOG files (*.log)|*.log", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

	if (saveMemoryFileDialog.ShowModal() == wxID_OK) {
		string path = string(saveMemoryFileDialog.GetPath());
		STAGE_ACTION_ARGUMENTS(path)

			// If in higher access mode, save log unencrypted
			if (GetGUIAccessMode() == GuiAccessMode::SERVICE or GetGUIAccessMode() == GuiAccessMode::FACTORY)
				diagnostics->SaveLog_Statistics(path);
			else
				diagnostics->SaveLogEncrypted_Statistics(path);

		if (diagnostics->SaveLogSuccessful_Statistics())
			wxLogStatus(_("Save successful."));
		else
			wxLogStatus(_("Save failed."));
	}
	else {
		wxLogStatus(_("Save log file cancelled."));
	}
	LOG_ACTION()
}



//-----------------------------------------------------------------------------
// Public methods

void AutotuneDiagnosticsPanel::RefreshVisibilityBasedOnAccessMode() {
	SetVisibilityBasedOnAccessMode(this, GuiAccessMode::SERVICE);
	RefreshPanels();
}


void AutotuneDiagnosticsPanel::RefreshStrings() {
	if (lc->AutotunePowerIsEnabledForUse()) {
		title->RefreshStrings();
		saveFullLogButton->SetLabelText(SAVE_FULL_LOG_STR);
		saveStatisticsLogButton->SetLabelText(SAVE_STATISTICS_LOG_STR);
	}
}

