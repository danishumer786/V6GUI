#include "AutotuneOscillatorPanel.h"
#include "../CommonFunctions_GUI.h"

using namespace std;


static wxString AUTOTUNE_OSCILLATOR_STR = _("Autotune-Oscillator");
static wxString AUTOTUNE_OSCILLATOR_TOOLTIP = _("Autotune Oscillator:\n  Maximize oscillator stability (HF sync signal) by tuning\n  SESAM motors to center of X and Y mode-locking range.");

static wxString START_SEED_ONLY_STR = _("Start - Seed Only");
static wxString AUTOTUNE_RUNNING_MESSAGE_STR = _("Running Autotune");
static wxString SAVE_LOG_STR = _("Save Log");
static wxString START_INDEX_STR = _("Start Index");
static wxString FINAL_INDEX_STR = _("Final Index");


// Handles continuously stepping the Autotune Oscillator procedure
void StepAutotuneOscillatorThread(shared_ptr<AutotuneOscillatorManager> autotuneOscillator) {
	while (true) {
		if (autotuneOscillator->IsRunning()) {
			autotuneOscillator->Step();
			if (!IsInAccessMode(GuiAccessMode::END_USER))
				wxLogStatus(to_wx_string(autotuneOscillator->GetStepSummary()));
		}
		else
			break;
	}
}


AutotuneOscillatorPanel::AutotuneOscillatorPanel(
	shared_ptr<MainLaserControllerInterface> _lc,
	shared_ptr<AutotunePowerManager> autotune_power, 
	shared_ptr<AutotuneOscillatorManager> autotune_oscillator, 
	wxWindow* parent) :
	wxPanel(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL | wxBORDER_THEME),
	lc(_lc),
	autotunePower(autotune_power),
	autotuneOscillator(autotune_oscillator) {


	this->SetBackgroundColour(FOREGROUND_PANEL_COLOR);

	wxBoxSizer* AutotuneOscillatorSizer = new wxBoxSizer(wxVERTICAL);

	// Title
	title = new FeatureTitle(this, AUTOTUNE_OSCILLATOR_STR, AUTOTUNE_OSCILLATOR_TOOLTIP);
	AutotuneOscillatorSizer->Add(title, 0, wxALL | wxALIGN_CENTER_HORIZONTAL, 5);

	// Start - Seed Only button
	startSeedOnlyButton = new wxButton(this, wxID_ANY, START_SEED_ONLY_STR, wxDefaultPosition, wxSize(160, 35), 0);
	startSeedOnlyButton->SetFont(FONT_MEDIUM_BOLD);
	startSeedOnlyButton->SetBackgroundColour(BUTTON_COLOR_INACTIVE);
	startSeedOnlyButton->Bind(wxEVT_BUTTON, &AutotuneOscillatorPanel::OnStartSeedOnlyClicked, this);
	AutotuneOscillatorSizer->Add(startSeedOnlyButton, 0, wxALL | wxALIGN_CENTER_HORIZONTAL, 5);

	// Start - Full Run button
	startFullRunButton = new wxButton(this, wxID_ANY, START_TEXT, wxDefaultPosition, wxSize(160, 35), 0);
	startFullRunButton->SetFont(FONT_MEDIUM_BOLD);
	startFullRunButton->SetBackgroundColour(BUTTON_COLOR_INACTIVE);
	startFullRunButton->Bind(wxEVT_BUTTON, &AutotuneOscillatorPanel::OnStartFullRunClicked, this);
	AutotuneOscillatorSizer->Add(startFullRunButton, 0, wxALL | wxALIGN_CENTER_HORIZONTAL, 5);

	// Blue status message
	runningMessage = new DynamicStatusMessage(this, AUTOTUNE_RUNNING_MESSAGE_STR, 300, 4);
	AutotuneOscillatorSizer->Add(runningMessage, 0, wxALL | wxALIGN_CENTER_HORIZONTAL | wxRESERVE_SPACE_EVEN_IF_HIDDEN, 5);

	// Save log button
	saveLogButton = new wxButton(this, wxID_ANY, SAVE_LOG_STR, wxDefaultPosition, wxDefaultSize, 0);
	saveLogButton->SetBackgroundColour(RESET_BUTTON_COLOR);
	saveLogButton->Bind(wxEVT_BUTTON, &AutotuneOscillatorPanel::OnSaveLogButtonClicked, this);
	AutotuneOscillatorSizer->Add(saveLogButton, 0, wxALIGN_CENTER_HORIZONTAL | wxBOTTOM, 5);

	// Results display
	wxBoxSizer* AutotuneOscillatorMotorResultsRowsSizer = new wxBoxSizer(wxVERTICAL);

	motorResultsPanel_X = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_THEME | wxTAB_TRAVERSAL);
	motorResultsPanel_X->SetBackgroundColour(READING_BACKGROUND_COLOR);

	wxBoxSizer* AutotuneOscillatorMotorResultsSizer_X = new wxBoxSizer(wxHORIZONTAL);

	motorLabel_X = new wxStaticText(motorResultsPanel_X, wxID_ANY, wxT("SESAMX:"), wxDefaultPosition, wxDefaultSize, 0);
	motorLabel_X->SetFont(FONT_MED_SMALL_SEMIBOLD);
	AutotuneOscillatorMotorResultsSizer_X->Add(motorLabel_X, 0, wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT, 10);

	wxFlexGridSizer* AutotuneOscillatorStartFinishSizer_X = new wxFlexGridSizer(0, 2, 0, 8);

	startIndexLabel_X = new wxStaticText(motorResultsPanel_X, wxID_ANY, START_INDEX_STR, wxDefaultPosition, wxDefaultSize, 0);
	startIndexLabel_X->SetFont(FONT_VERY_SMALL);
	AutotuneOscillatorStartFinishSizer_X->Add(startIndexLabel_X, 0, wxALL | wxALIGN_CENTER_HORIZONTAL, 0);

	finalIndexLabel_X = new wxStaticText(motorResultsPanel_X, wxID_ANY, FINAL_INDEX_STR, wxDefaultPosition, wxDefaultSize, 0);
	finalIndexLabel_X->SetFont(FONT_VERY_SMALL);
	AutotuneOscillatorStartFinishSizer_X->Add(finalIndexLabel_X, 0, wxALIGN_CENTER_HORIZONTAL | wxBOTTOM, 0);

	startIndexValue_X = new wxStaticText(motorResultsPanel_X, wxID_ANY, wxT("00000"), wxDefaultPosition, wxDefaultSize, 0);
	startIndexValue_X->SetFont(FONT_VERY_SMALL);
	startIndexValue_X->SetForegroundColour(TEXT_COLOR_GRAY);
	AutotuneOscillatorStartFinishSizer_X->Add(startIndexValue_X, 0, wxALL | wxALIGN_CENTER_HORIZONTAL, 0);

	finalIndexValue_X = new wxStaticText(motorResultsPanel_X, wxID_ANY, wxT("00000"), wxDefaultPosition, wxDefaultSize, 0);
	finalIndexValue_X->SetFont(FONT_VERY_SMALL);
	finalIndexValue_X->SetForegroundColour(TEXT_COLOR_DARK_GREEN);
	AutotuneOscillatorStartFinishSizer_X->Add(finalIndexValue_X, 0, wxALL | wxALIGN_CENTER_HORIZONTAL, 0);

	AutotuneOscillatorMotorResultsSizer_X->Add(AutotuneOscillatorStartFinishSizer_X, 0, wxALL, 5);

	motorResultsPanel_X->SetSizer(AutotuneOscillatorMotorResultsSizer_X);
	motorResultsPanel_X->Layout();
	AutotuneOscillatorMotorResultsSizer_X->Fit(motorResultsPanel_X);
	AutotuneOscillatorSizer->Add(motorResultsPanel_X, 0, wxALL | wxALIGN_CENTER_HORIZONTAL, 5);


	motorResultsPanel_Y = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_THEME | wxTAB_TRAVERSAL);
	motorResultsPanel_Y->SetBackgroundColour(READING_BACKGROUND_COLOR);

	wxBoxSizer* AutotuneOscillatorMotorResultsSizer_Y = new wxBoxSizer(wxHORIZONTAL);

	motorLabel_Y = new wxStaticText(motorResultsPanel_Y, wxID_ANY, wxT("SESAMY"), wxDefaultPosition, wxDefaultSize, 0);
	motorLabel_Y->SetFont(FONT_MED_SMALL_SEMIBOLD);
	AutotuneOscillatorMotorResultsSizer_Y->Add(motorLabel_Y, 0, wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT, 10);

	wxFlexGridSizer* AutotuneOscillatorStartFinishSizer_Y = new wxFlexGridSizer(0, 2, 0, 8);

	startIndexLabel_Y = new wxStaticText(motorResultsPanel_Y, wxID_ANY, START_INDEX_STR, wxDefaultPosition, wxDefaultSize, 0);
	startIndexLabel_Y->SetFont(FONT_VERY_SMALL);
	AutotuneOscillatorStartFinishSizer_Y->Add(startIndexLabel_Y, 0, wxALL | wxALIGN_CENTER_HORIZONTAL, 0);

	finalIndexLabel_Y = new wxStaticText(motorResultsPanel_Y, wxID_ANY, FINAL_INDEX_STR, wxDefaultPosition, wxDefaultSize, 0);
	finalIndexLabel_Y->SetFont(FONT_VERY_SMALL);
	AutotuneOscillatorStartFinishSizer_Y->Add(finalIndexLabel_Y, 0, wxALIGN_CENTER_HORIZONTAL | wxBOTTOM, 0);

	startIndexValue_Y = new wxStaticText(motorResultsPanel_Y, wxID_ANY, wxT("00000"), wxDefaultPosition, wxDefaultSize, 0);
	startIndexValue_Y->SetFont(FONT_VERY_SMALL);
	startIndexValue_Y->SetForegroundColour(TEXT_COLOR_GRAY);
	AutotuneOscillatorStartFinishSizer_Y->Add(startIndexValue_Y, 0, wxALL | wxALIGN_CENTER_HORIZONTAL, 0);

	finalIndexValue_Y = new wxStaticText(motorResultsPanel_Y, wxID_ANY, wxT("00000"), wxDefaultPosition, wxDefaultSize, 0);
	finalIndexValue_Y->SetFont(FONT_VERY_SMALL);
	finalIndexValue_Y->SetForegroundColour(TEXT_COLOR_DARK_GREEN);
	AutotuneOscillatorStartFinishSizer_Y->Add(finalIndexValue_Y, 0, wxALL | wxALIGN_CENTER_HORIZONTAL, 0);

	AutotuneOscillatorMotorResultsSizer_Y->Add(AutotuneOscillatorStartFinishSizer_Y, 0, wxALL, 5);

	motorResultsPanel_Y->SetSizer(AutotuneOscillatorMotorResultsSizer_Y);
	motorResultsPanel_Y->Layout();
	AutotuneOscillatorMotorResultsSizer_Y->Fit(motorResultsPanel_Y);
	AutotuneOscillatorSizer->Add(motorResultsPanel_Y, 0, wxALL | wxALIGN_CENTER_HORIZONTAL, 5);

	this->SetSizer(AutotuneOscillatorSizer);
	this->Layout();
	AutotuneOscillatorSizer->Fit(this);

	runningMessage->Hide();
	saveLogButton->Hide();
	motorResultsPanel_X->Hide();
	motorResultsPanel_Y->Hide();

}

AutotuneOscillatorPanel::~AutotuneOscillatorPanel() {
	if (autotuneOscillatorThread) {
		autotuneOscillatorThread->join();
	}
}




//-----------------------------------------------------------------------------
// Refresh methods

void AutotuneOscillatorPanel::RefreshAll() {
	if (lc->AutotuneOscillatorIsEnabledForUse()) {
		RefreshControlsEnabled();
		RefreshAutotuneProcedure();
	}
}


void AutotuneOscillatorPanel::RefreshAutotuneProcedure() {
	if (!autotuneStarted)
		return;

	if (autotuneOscillator->IsError()) {
		runningMessage->Set(_("ERROR"));
		wxString errorMessage = _("ERROR") + " - " + autotuneOscillator->GetErrorMessage();
		//runningMessage->Set(_(errorMessage));
		if (!IsInAccessMode(GuiAccessMode::END_USER))
			wxLogStatus(errorMessage);
		ResetWidgetsWhenAutotuneStops();
	}
	else if (autotuneOscillator->IsFinished()) {
		DisplayResults();
		ResetWidgetsWhenAutotuneStops();
	}
	// No step block necessary here - the autotuneOscillatorThread takes care of stepping the procedure while running
}


void AutotuneOscillatorPanel::RefreshControlsEnabled() {
	//bool canStartAutotuneOscillator = lc->LaserIsRunning() and !lc->IsAutotunePowerRunning();

	//// Disable "Start - Seed Only" button when Full Procedure running and vice versa
	//bool isRunningSeedOnly = autotuneOscillator->IsRunning() && !autotuneOscillator->IsSetToFullProcedure();
	//bool isRunningFullProcedure = autotuneOscillator->IsRunning() && autotuneOscillator->IsSetToFullProcedure();
	//RefreshWidgetEnableBasedOnCondition(startSeedOnlyButton, canStartAutotuneOscillator && !isRunningFullProcedure);
	//RefreshWidgetEnableBasedOnCondition(startFullRunButton, canStartAutotuneOscillator && !isRunningSeedOnly);


	bool canStartAutotuneOscillator = lc->LaserIsRunning() and !lc->IsAutotunePowerRunning();

	// Disable "Start - Seed Only" button when Full Procedure running and vice versa
	bool isRunningSeedOnly = autotuneOscillator->IsRunning() && !autotuneOscillator->IsSetToFullProcedure();
	bool isRunningFullProcedure = autotuneOscillator->IsRunning() && autotuneOscillator->IsSetToFullProcedure();
	if (autotuneOscillator->IsRunning()) {
		RefreshWidgetEnableBasedOnCondition(startSeedOnlyButton, !isRunningFullProcedure);
		RefreshWidgetEnableBasedOnCondition(startFullRunButton, !isRunningSeedOnly);
	}
	else {
		RefreshWidgetEnableBasedOnCondition(startSeedOnlyButton, canStartAutotuneOscillator and !isRunningFullProcedure);
		RefreshWidgetEnableBasedOnCondition(startFullRunButton, canStartAutotuneOscillator and !isRunningSeedOnly);
	}
}




//-----------------------------------------------------------------------------
// Main Autotune-power functionality

void AutotuneOscillatorPanel::Start(bool fullRun) {
	// Try starting
	if (fullRun)
		autotuneOscillator->StartFull();
	else
		autotuneOscillator->StartSeedOnly();

	// Check if it ran into an error
	if (!autotuneOscillator->CanStartAutotune()) {
		wxMessageBox(_("Cannot start Autotune Oscillator") + ":\n" + to_wx_string(autotuneOscillator->GetCantStartReason()));
		//autotuneOscillatorRunning = false;
	}
	else {
		autotuneStarted = true;
		StartAutotuneStepThread();
		SetButtonToCancelState();
		saveLogButton->Hide();

		runningMessage->Set(AUTOTUNE_RUNNING_MESSAGE_STR);
		runningMessage->Show();
		runningMessage->StartCycling();

		motorResultsPanel_X->Show();
		startIndexValue_X->SetLabelText(to_wx_string(autotuneOscillator->GetMotorCurrentIndex_X()));
		finalIndexValue_X->SetLabelText("");

		motorResultsPanel_Y->Show();
		startIndexValue_Y->SetLabelText(to_wx_string(autotuneOscillator->GetMotorCurrentIndex_Y()));
		finalIndexValue_Y->SetLabelText("");

		this->GetParent()->Layout();
		this->GetParent()->Update();
	}
}

void AutotuneOscillatorPanel::StartAutotuneStepThread() {
	if (autotuneOscillatorThread != nullptr) {
		autotuneOscillatorThread->join();
	}
	autotuneOscillatorThread = make_shared<std::thread>(StepAutotuneOscillatorThread, autotuneOscillator);
}


void AutotuneOscillatorPanel::Cancel() {
	autotuneOscillator->Cancel();
	runningMessage->Set(_("Canceled"));
	ResetWidgetsWhenAutotuneStops();
}



//-----------------------------------------------------------------------------
// Callbacks

void AutotuneOscillatorPanel::OnStartSeedOnlyClicked(wxCommandEvent& evt) {
	STAGE_ACTION_("Start Autotune-Oscillator button clicked", "Seed Only")
	Start(false);
	LOG_ACTION()
}

void AutotuneOscillatorPanel::OnStartFullRunClicked(wxCommandEvent& evt) {
	STAGE_ACTION_("Start Autotune-Oscillator button clicked", "Full Run")
	Start(true);
	LOG_ACTION()
}

void AutotuneOscillatorPanel::OnCancelFullRunClicked(wxCommandEvent& evt) {
	STAGE_ACTION_("Cancel Autotune-Oscillator button clicked", "Full Run")
	Cancel();
	LOG_ACTION()
}

void AutotuneOscillatorPanel::OnCancelSeedOnlyClicked(wxCommandEvent& evt) {
	STAGE_ACTION_("Cancel Autotune-Oscillator button clicked", "Seed Only")
	Cancel();
	LOG_ACTION()
}

void AutotuneOscillatorPanel::OnSaveLogButtonClicked(wxCommandEvent& evt) {
	STAGE_ACTION("Save Autotune-Oscillator Log button clicked")
	wxString defaultPath = autotuneOscillator->GetDefaultLogPath();
	wxString defaultFilename = autotuneOscillator->GetDefaultLogFilename();

	wxFileDialog saveMemoryFileDialog(nullptr, (_("Save Log File")), defaultPath, defaultFilename, "LOG files (*.log)|*.log", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

	if (saveMemoryFileDialog.ShowModal() == wxID_OK) {
		string path = string(saveMemoryFileDialog.GetPath());
		STAGE_ACTION_ARGUMENTS(path)

		// If in higher access mode, save log unencrypted
		if (GetGUIAccessMode() == GuiAccessMode::SERVICE or GetGUIAccessMode() == GuiAccessMode::FACTORY)
			autotuneOscillator->SaveLog(path);
		else
			autotuneOscillator->SaveLogEncrypted(path);

		if (autotuneOscillator->SaveLogSuccessful())
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
// Helper methods

void AutotuneOscillatorPanel::SetButtonToCancelState() {
	if (autotuneOscillator->IsSetToFullProcedure()) {
		startFullRunButton->SetLabelText(CANCEL_TEXT);
		startFullRunButton->SetBackgroundColour(TEXT_COLOR_RED);
		startFullRunButton->Unbind(wxEVT_BUTTON, &AutotuneOscillatorPanel::OnStartFullRunClicked, this, wxID_ANY);
		startFullRunButton->Bind(wxEVT_BUTTON, &AutotuneOscillatorPanel::OnCancelFullRunClicked, this, wxID_ANY);
	}
	else {
		startSeedOnlyButton->SetLabelText(CANCEL_TEXT);
		startSeedOnlyButton->SetBackgroundColour(TEXT_COLOR_RED);
		startSeedOnlyButton->Unbind(wxEVT_BUTTON, &AutotuneOscillatorPanel::OnStartSeedOnlyClicked, this, wxID_ANY);
		startSeedOnlyButton->Bind(wxEVT_BUTTON, &AutotuneOscillatorPanel::OnCancelSeedOnlyClicked, this, wxID_ANY);
	}
}


void AutotuneOscillatorPanel::SetButtonToStartState() {
	if (autotuneOscillator->IsSetToFullProcedure()) {
		startFullRunButton->SetLabelText(START_TEXT);
		startFullRunButton->SetBackgroundColour(BUTTON_COLOR_INACTIVE);
		startFullRunButton->Unbind(wxEVT_BUTTON, &AutotuneOscillatorPanel::OnCancelFullRunClicked, this, wxID_ANY);
		startFullRunButton->Bind(wxEVT_BUTTON, &AutotuneOscillatorPanel::OnStartFullRunClicked, this, wxID_ANY);
	}
	else {
		startSeedOnlyButton->SetLabelText(START_SEED_ONLY_STR);
		startSeedOnlyButton->SetBackgroundColour(BUTTON_COLOR_INACTIVE);
		startSeedOnlyButton->Unbind(wxEVT_BUTTON, &AutotuneOscillatorPanel::OnCancelSeedOnlyClicked, this, wxID_ANY);
		startSeedOnlyButton->Bind(wxEVT_BUTTON, &AutotuneOscillatorPanel::OnStartSeedOnlyClicked, this, wxID_ANY);
	}
}


void AutotuneOscillatorPanel::DisplayResults() {
	runningMessage->Set(_("Finished."));
	wxLogStatus(_(autotuneOscillator->GetSummary()));

	finalIndexValue_X->SetLabelText(to_wx_string(autotuneOscillator->GetMotorCurrentIndex_X()));
	finalIndexValue_Y->SetLabelText(to_wx_string(autotuneOscillator->GetMotorCurrentIndex_Y()));
}


void AutotuneOscillatorPanel::ResetWidgetsWhenAutotuneStops() {
	saveLogButton->Show();
	autotuneStarted = false;
	runningMessage->StopCycling();
	SetButtonToStartState();
	this->GetParent()->Layout();
	this->GetParent()->Update();
	this->Refresh();
}



//-----------------------------------------------------------------------------
// Public updating methods

void AutotuneOscillatorPanel::RefreshVisibilityBasedOnAccessMode() {
	if (lc->AutotuneOscillatorIsEnabledForUse())
		SetVisibilityBasedOnAccessMode(startSeedOnlyButton, 1);
}


void AutotuneOscillatorPanel::RefreshStrings() {
	if (lc->AutotuneOscillatorIsEnabledForUse()) {
		title->RefreshStrings();
		runningMessage->Set(_(AUTOTUNE_RUNNING_MESSAGE_STR));
		startSeedOnlyButton->SetLabelText(_(START_SEED_ONLY_STR));
		startFullRunButton->SetLabelText(_(START_TEXT));
		saveLogButton->SetLabelText(_(SAVE_LOG_STR));
		startIndexLabel_X->SetLabelText(_(START_INDEX_STR));
		finalIndexLabel_X->SetLabelText(_(FINAL_INDEX_STR));
		startIndexLabel_Y->SetLabelText(_(START_INDEX_STR));
		finalIndexLabel_Y->SetLabelText(_(FINAL_INDEX_STR));
	}
}
