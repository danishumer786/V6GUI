#include "AutotunePowerPanel.h"
#include "Security/AccessByMACAddress.h"
#include "../CommonFunctions_GUI.h"
#include "../CommonGUIComponents/PowerMonitorReadout.h"
#include "../../CommonUtilities/ConfigurationManager.h"

using namespace std;



const wxString AUTOTUNE_POWER_STR = _("Calibrate Power");
const wxString AUTOTUNE_POWER_TOOLTIP = _(
	"Calibrate Power:\n"
	"  Automatically self-calibrate laser power.\n"
	"\n"
	"  Note: This process can take several minutes\n"
	"  and may need to be run more than once."
);

const wxString AUTOTUNE_RUNNING_MESSAGE_STR = _("Calibrating");
const wxString AUTOTUNE_ERROR_STR = _("Calibration Error");
const wxString CONFIRM_AUTOTUNE_STR = _("Confirm Calibrate Power");
const wxString CONFIRM_AUTOTUNE_MESSAGE = _(
	"The calibration process takes about 30 minutes.\n"
	"Please don't close the GUI while it is running.\n"
	"\n"
	"Are you sure you want to run calibration?"
);
const wxString AUTOTUNE_FAULT_MESSAGE = _(
	"Detected laser fault during calibration.\n"
	"Resolve the fault and reset the laser to recover original state.\n"
	"Then you may attempt calibrating again."
);

const wxString SAVE_LOG_STR = _("Save Log");
const wxString SETTINGS_STR = _("Settings");

const wxString SPEED_STR = _("Speed");
const wxString SPEED_TOOLTIP = _("Adjust wait time between steps.\n"
	" - Use low speed for lasers with\n   power monitors that are slow to respond\n"
	"   or when there are thermal effects that\n   need time to stabilize.");

const wxString PRECISION_MOTOR_STR = _("Precision\n(Motor)");

const wxString PRECISION_TEMPERATURE_STR = _("Precision\n(Temp.)");

const wxString PCT_DROP_THRESHOLD_STR = _("% Drop\nThreshold");
const wxString PCT_DROP_THRESHOLD_TOOLTIP = _("Set power drop threshold for\ndetermining tuning range.\n"
	"\nIf there are tuning components\nwith multiple peaks (e.g., SHG),\nmake this value greater than\nthe depth of local minima.");

const wxString MOTOR_RANGE_STR = _("Range\n(Motor)");
const wxString MOTOR_RANGE_TOOLTIP = _("Set motor travel range from starting value.");

const wxString TEMPERATURE_RANGE_STR = _("Range\n(Temp.)");
const wxString TEMPERATURE_RANGE_TOOLTIP = _("Set temperature range from starting value.");



// Handles continuously stepping the Autotune Power procedure
void StepAutotunePowerThread(shared_ptr<AutotunePowerManager> autotunePower, vector<AutotuneComponentPanel*> autotuneComponentPanels) {
	while (true) {
		if (autotunePower->IsRunning()) {
			autotunePower->Step();
			if (!IsInAccessMode(GuiAccessMode::END_USER))
				wxLogStatus(to_wx_string(autotunePower->GetStepSummary()));
			for (auto component : autotuneComponentPanels)
				component->RefreshAll();
		}
		else
			break;
	}
}


AutotunePowerPanel::AutotunePowerPanel(
	shared_ptr<MainLaserControllerInterface> laser_controller,
	shared_ptr<AutotunePowerManager> autotune_power, 
	shared_ptr<AutotuneOscillatorManager> autotune_oscillator, 
	wxWindow* parent) :
		wxPanel(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL | wxBORDER_THEME),
		lc(laser_controller),
		autotunePower(autotune_power),
		autotuneOscillator(autotune_oscillator) {


	this->SetAutoLayout(false);
	this->SetBackgroundColour(FOREGROUND_PANEL_COLOR);

	sizer = new wxBoxSizer(wxVERTICAL);

	title = new FeatureTitle(this, AUTOTUNE_POWER_STR, AUTOTUNE_POWER_TOOLTIP);
	sizer->Add(title, 0, wxALIGN_CENTER_HORIZONTAL, 5);

	mainButton = new wxButton(this, wxID_ANY, START_TEXT, wxDefaultPosition, wxSize(160, 35), 0);
	mainButton->SetFont(FONT_MEDIUM_BOLD);
	mainButton->SetBackgroundColour(BUTTON_COLOR_INACTIVE);
	mainButton->Bind(wxEVT_BUTTON, &AutotunePowerPanel::OnMainButtonClicked, this);
	sizer->Add(mainButton, 0, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 5);

	runningMessage = new DynamicStatusMessage(this, AUTOTUNE_RUNNING_MESSAGE_STR, 300, 4);
	runningMessage->Hide();
	sizer->Add(runningMessage, 0, wxALIGN_CENTER_HORIZONTAL | wxALL | wxRESERVE_SPACE_EVEN_IF_HIDDEN, 5);


	// Save log button

	saveLogButton = new wxButton(this, wxID_ANY, SAVE_LOG_STR, wxDefaultPosition, wxDefaultSize, 0);
	saveLogButton->SetBackgroundColour(RESET_BUTTON_COLOR);
	saveLogButton->Bind(wxEVT_BUTTON, &AutotunePowerPanel::OnSaveLogButtonClicked, this);
	sizer->Add(saveLogButton, 0, wxALIGN_CENTER_HORIZONTAL | wxBOTTOM, 5);
	saveLogButton->Hide();

	powerMonitorsSizer = new wxBoxSizer(wxVERTICAL);
	sizer->Add(powerMonitorsSizer, 0, wxEXPAND, 5);


	//----------------------------------------------------------------------------------------
	// Collapsible Settings Panel for Autotune - Speed and Precision controls

	collapsibleSettingsPanel = new wxCollapsiblePane(this, wxID_ANY, SETTINGS_STR, wxDefaultPosition, wxDefaultSize, wxCP_DEFAULT_STYLE | wxCP_NO_TLW_RESIZE);
	collapsibleSettingsPanel->Collapse(true);
	collapsibleSettingsPanel->Bind(wxEVT_COLLAPSIBLEPANE_CHANGED, &AutotunePowerPanel::OnSettingsCollapse, this);

	wxBoxSizer* AutotuneCollapsibleSettingsSizer = new wxBoxSizer(wxVERTICAL);

	innerSettingsPanel = new wxPanel(collapsibleSettingsPanel->GetPane(), wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_THEME | wxTAB_TRAVERSAL);
	innerSettingsPanel->SetBackgroundColour(FOREGROUND_PANEL_COLOR);

	wxFlexGridSizer* AutotuneInnerSettingsSizer = new wxFlexGridSizer(0, 3, 0, 0);


	// Speed setting - Level 1 - 10; corresponds to wait time between tuning steps

	speedLabel = new wxStaticText(innerSettingsPanel, wxID_ANY, SPEED_STR, wxDefaultPosition, wxDefaultSize, 0);
	speedLabel->SetFont(FONT_SMALL_SEMIBOLD);
	AutotuneInnerSettingsSizer->Add(speedLabel, 0, wxALL | wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL, 5);

	speedInfoIcon = new wxStaticBitmap(innerSettingsPanel, wxID_ANY, wxBitmap(INFO_ICON_SMALL, wxBITMAP_TYPE_ANY), wxDefaultPosition, wxDefaultSize, 0);
	speedInfoIcon->SetToolTip(SPEED_TOOLTIP);
	AutotuneInnerSettingsSizer->Add(speedInfoIcon, 0, wxALL | wxALIGN_CENTER_VERTICAL, 0);

	speedSliderPanel = new wxPanel(innerSettingsPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_STATIC | wxTAB_TRAVERSAL);
	speedSliderPanel->SetFont(FONT_SMALL_SEMIBOLD);
	speedSliderPanel->SetBackgroundColour(EXTRA_LIGHT_BACKGROUND_COLOR);

	wxBoxSizer* AutotuneSpeedSliderSizer = new wxBoxSizer(wxVERTICAL);

	speedSlider = new wxSlider(speedSliderPanel, wxID_ANY, 3, 1, 10, wxDefaultPosition, wxSize(120, -1), wxSL_BOTH | wxSL_VALUE_LABEL);
	speedSlider->Bind(wxEVT_SLIDER, &AutotunePowerPanel::OnSpeedSliderMoved, this);
	AutotuneSpeedSliderSizer->Add(speedSlider, 0, wxTOP, 0);

	speedSliderPanel->SetSizer(AutotuneSpeedSliderSizer);
	speedSliderPanel->Layout();
	AutotuneSpeedSliderSizer->Fit(speedSliderPanel);
	AutotuneInnerSettingsSizer->Add(speedSliderPanel, 1, wxEXPAND | wxALL, 5);


	// Motor precision setting - Level 1 - 10; corresponds to motor step size 

	precisionLabel_Motor = new wxStaticText(innerSettingsPanel, wxID_ANY, PRECISION_MOTOR_STR, wxDefaultPosition, wxDefaultSize, wxALIGN_CENTER_HORIZONTAL);
	precisionLabel_Motor->SetFont(FONT_SMALL_SEMIBOLD);
	AutotuneInnerSettingsSizer->Add(precisionLabel_Motor, 0, wxALL | wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT, 5);

	precisionInfoIcon_Motor = new wxStaticBitmap(innerSettingsPanel, wxID_ANY, wxBitmap(INFO_ICON_SMALL, wxBITMAP_TYPE_ANY), wxDefaultPosition, wxDefaultSize, 0);
	SetMotorPrecisionTooltip();
	AutotuneInnerSettingsSizer->Add(precisionInfoIcon_Motor, 0, wxALL | wxALIGN_CENTER_VERTICAL, 0);

	precisionSliderPanel_Motor = new wxPanel(innerSettingsPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_STATIC | wxTAB_TRAVERSAL);
	precisionSliderPanel_Motor->SetFont(FONT_SMALL_SEMIBOLD);
	precisionSliderPanel_Motor->SetBackgroundColour(EXTRA_LIGHT_BACKGROUND_COLOR);

	wxBoxSizer* AutotunePrecisionSliderSizer_Motor = new wxBoxSizer(wxVERTICAL);

	precisionSlider_Motor = new wxSlider(precisionSliderPanel_Motor, wxID_ANY, 3, 1, 10, wxDefaultPosition, wxSize(120, -1), wxSL_BOTH | wxSL_VALUE_LABEL);
	precisionSlider_Motor->Bind(wxEVT_SLIDER, &AutotunePowerPanel::OnPrecisionSliderMoved_Motor, this);
	AutotunePrecisionSliderSizer_Motor->Add(precisionSlider_Motor, 0, wxTOP, 0);

	precisionSliderPanel_Motor->SetSizer(AutotunePrecisionSliderSizer_Motor);
	precisionSliderPanel_Motor->Layout();
	AutotunePrecisionSliderSizer_Motor->Fit(precisionSliderPanel_Motor);
	AutotuneInnerSettingsSizer->Add(precisionSliderPanel_Motor, 1, wxEXPAND | wxALL, 5);


	// Temperature precision setting - Level 1 - 10; corresponds to motor step size 

	precisionLabel_Temperature = new wxStaticText(innerSettingsPanel, wxID_ANY, PRECISION_TEMPERATURE_STR, wxDefaultPosition, wxDefaultSize, wxALIGN_CENTER_HORIZONTAL);
	precisionLabel_Temperature->SetFont(FONT_SMALL_SEMIBOLD);
	AutotuneInnerSettingsSizer->Add(precisionLabel_Temperature, 0, wxALL | wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT, 5);

	precisionInfoIcon_Temperature = new wxStaticBitmap(innerSettingsPanel, wxID_ANY, wxBitmap(INFO_ICON_SMALL, wxBITMAP_TYPE_ANY), wxDefaultPosition, wxDefaultSize, 0);
	SetTemperaturePrecisionTooltip();
	AutotuneInnerSettingsSizer->Add(precisionInfoIcon_Temperature, 0, wxALL | wxALIGN_CENTER_VERTICAL, 0);

	precisionSliderPanel_Temperature = new wxPanel(innerSettingsPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_STATIC | wxTAB_TRAVERSAL);
	precisionSliderPanel_Temperature->SetFont(FONT_SMALL_SEMIBOLD);
	precisionSliderPanel_Temperature->SetBackgroundColour(EXTRA_LIGHT_BACKGROUND_COLOR);

	wxBoxSizer* AutotunePrecisionSliderSizer_Temperature = new wxBoxSizer(wxVERTICAL);

	precisionSlider_Temperature = new wxSlider(precisionSliderPanel_Temperature, wxID_ANY, 3, 1, 10, wxDefaultPosition, wxSize(120, -1), wxSL_BOTH | wxSL_VALUE_LABEL);
	precisionSlider_Temperature->Bind(wxEVT_SLIDER, &AutotunePowerPanel::OnPrecisionSliderMoved_Temperature, this);
	AutotunePrecisionSliderSizer_Temperature->Add(precisionSlider_Temperature, 0, wxTOP, 0);

	precisionSliderPanel_Temperature->SetSizer(AutotunePrecisionSliderSizer_Temperature);
	precisionSliderPanel_Temperature->Layout();
	AutotunePrecisionSliderSizer_Temperature->Fit(precisionSliderPanel_Temperature);
	AutotuneInnerSettingsSizer->Add(precisionSliderPanel_Temperature, 1, wxEXPAND | wxALL, 5);


	// Power drop threshold setting
	dropThresholdLabel = new wxStaticText(innerSettingsPanel, wxID_ANY, PCT_DROP_THRESHOLD_STR, wxDefaultPosition, wxDefaultSize, wxALIGN_CENTER_HORIZONTAL);
	dropThresholdLabel->SetFont(FONT_SMALL_SEMIBOLD);
	AutotuneInnerSettingsSizer->Add(dropThresholdLabel, 0, wxALL | wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL, 5);

	dropThresholdInfoIcon = new wxStaticBitmap(innerSettingsPanel, wxID_ANY, wxBitmap(INFO_ICON_SMALL, wxBITMAP_TYPE_ANY), wxDefaultPosition, wxDefaultSize, 0);
	dropThresholdInfoIcon->SetToolTip(PCT_DROP_THRESHOLD_TOOLTIP);
	AutotuneInnerSettingsSizer->Add(dropThresholdInfoIcon, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);

	dropThresholdSpinCtrl = new wxSpinCtrl(innerSettingsPanel, wxID_ANY, wxEmptyString, wxDefaultPosition,
		wxSize(50, 28), wxSP_ARROW_KEYS | wxTE_PROCESS_ENTER, 0, 90, 20);
	dropThresholdSpinCtrl->SetFont(FONT_MED_SMALL_SEMIBOLD);
	dropThresholdSpinCtrl->Bind(wxEVT_SPINCTRL, &AutotunePowerPanel::SetDropThresholdWithSpin, this);
	dropThresholdSpinCtrl->Bind(wxEVT_TEXT_ENTER, &AutotunePowerPanel::SetDropThresholdWithText, this);
	AutotuneInnerSettingsSizer->Add(dropThresholdSpinCtrl, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);


	// Motor range setting
	motorRangeLabel = new wxStaticText(innerSettingsPanel, wxID_ANY, MOTOR_RANGE_STR, wxDefaultPosition, wxDefaultSize, wxALIGN_CENTER_HORIZONTAL);
	motorRangeLabel->SetFont(FONT_SMALL_SEMIBOLD);
	AutotuneInnerSettingsSizer->Add(motorRangeLabel, 0, wxALL | wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL, 5);

	motorRangeInfoIcon = new wxStaticBitmap(innerSettingsPanel, wxID_ANY, wxBitmap(INFO_ICON_SMALL, wxBITMAP_TYPE_ANY), wxDefaultPosition, wxDefaultSize, 0);
	motorRangeInfoIcon->SetToolTip(MOTOR_RANGE_TOOLTIP);
	AutotuneInnerSettingsSizer->Add(motorRangeInfoIcon, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);

	motorRangeSpinCtrl = new wxSpinCtrl(innerSettingsPanel, wxID_ANY, wxEmptyString, wxDefaultPosition,
		wxDefaultSize, wxSP_ARROW_KEYS | wxTE_PROCESS_ENTER, 0, 99999, 1000);
	motorRangeSpinCtrl->SetFont(FONT_MED_SMALL_SEMIBOLD);
	motorRangeSpinCtrl->Bind(wxEVT_SPINCTRL, &AutotunePowerPanel::SetMotorRangeWithSpin, this);
	motorRangeSpinCtrl->Bind(wxEVT_TEXT_ENTER, &AutotunePowerPanel::SetMotorRangeWithText, this);
	AutotuneInnerSettingsSizer->Add(motorRangeSpinCtrl, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);



	// Temperature range setting
	temperatureRangeLabel = new wxStaticText(innerSettingsPanel, wxID_ANY, TEMPERATURE_RANGE_STR, wxDefaultPosition, wxDefaultSize, wxALIGN_CENTER_HORIZONTAL);
	temperatureRangeLabel->SetFont(FONT_SMALL_SEMIBOLD);
	AutotuneInnerSettingsSizer->Add(temperatureRangeLabel, 0, wxALL | wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL, 5);

	temperatureRangeInfoIcon = new wxStaticBitmap(innerSettingsPanel, wxID_ANY, wxBitmap(INFO_ICON_SMALL, wxBITMAP_TYPE_ANY), wxDefaultPosition, wxDefaultSize, 0);
	temperatureRangeInfoIcon->SetToolTip(TEMPERATURE_RANGE_TOOLTIP);
	AutotuneInnerSettingsSizer->Add(temperatureRangeInfoIcon, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);

	temperatureRangeSpinCtrlDouble = new wxSpinCtrlDouble(innerSettingsPanel, wxID_ANY, wxEmptyString, wxDefaultPosition,
		wxDefaultSize, wxSP_ARROW_KEYS | wxTE_PROCESS_ENTER, 0.0, 99.9, 0.5, 0.1);
	temperatureRangeSpinCtrlDouble->SetFont(FONT_MED_SMALL_SEMIBOLD);
	temperatureRangeSpinCtrlDouble->Bind(wxEVT_SPINCTRL, &AutotunePowerPanel::SetTemperatureRangeWithSpin, this);
	temperatureRangeSpinCtrlDouble->Bind(wxEVT_TEXT_ENTER, &AutotunePowerPanel::SetTemperatureRangeWithText, this);
	AutotuneInnerSettingsSizer->Add(temperatureRangeSpinCtrlDouble, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);



	innerSettingsPanel->SetSizer(AutotuneInnerSettingsSizer);
	innerSettingsPanel->Layout();
	AutotuneInnerSettingsSizer->Fit(innerSettingsPanel);
	AutotuneCollapsibleSettingsSizer->Add(innerSettingsPanel, 0, wxALL, 0);

	collapsibleSettingsPanel->GetPane()->SetSizer(AutotuneCollapsibleSettingsSizer);
	collapsibleSettingsPanel->GetPane()->Layout();
	AutotuneCollapsibleSettingsSizer->Fit(collapsibleSettingsPanel->GetPane());
	sizer->Add(collapsibleSettingsPanel, 0, wxEXPAND | wxALL, 5);


	this->SetSizer(sizer);
	this->Layout();
	sizer->Fit(this);

	Init();
	RefreshStrings();
}



//-----------------------------------------------------------------------------
// Initialization

void AutotunePowerPanel::Init() {
	CorrectPowerDropThresholdIfZero();
	InitSettingsPanel();
	InitPowerMonitorsDisplay();
}

void AutotunePowerPanel::InitSettingsPanel() {
	speedSlider->SetValue(lc->GetAutotuneSpeedLevel());
	precisionSlider_Motor->SetValue(lc->GetAutotuneMotorPrecisionLevel());
	precisionSlider_Temperature->SetValue(lc->GetAutotuneTemperaturePrecisionLevel());
	dropThresholdSpinCtrl->SetValue(lc->GetAutotunePowerDropThreshold());
	motorRangeSpinCtrl->SetValue(lc->GetAutotuneMotorRange());
	temperatureRangeSpinCtrlDouble->SetValue(lc->GetAutotuneTemperatureRange());
}

void AutotunePowerPanel::InitPowerMonitorsDisplay() {
	for (int pmNumber : lc->GetAutotunePowerMonitorIds()) {
		PowerMonitorReadout* readout = new PowerMonitorReadout(lc, this, pmNumber, true);
		powerMonitorsSizer->Add(readout, 0, wxALL | wxALIGN_CENTER_HORIZONTAL, 5);
		powerMonitorReadouts.push_back(readout);
	}
	this->Layout();
	sizer->Fit(this);
}

void AutotunePowerPanel::CorrectPowerDropThresholdIfZero() {
	if (lc->GetAutotunePowerDropThreshold() == 0)
		lc->SetAutotunePowerDropThreshold(15);
}


//-----------------------------------------------------------------------------
// Refresh methods

void AutotunePowerPanel::RefreshAll() {
	if (!lc->AutotunePowerIsEnabledForUse())
		return;
	RefreshMainButtonState();
	RefreshAutotuneProcedure();
	RefreshAutotuneControlEnabled();
	RefreshWarnings();
	RefreshPowerMonitorReadouts();
}

void AutotunePowerPanel::RefreshMainButtonState() {
	SetTextBasedOnCondition(mainButton, autotunePower->IsRunning(), _(CANCEL_TEXT), _(START_TEXT));
	SetBGColorBasedOnCondition(mainButton, autotunePower->IsRunning(), TEXT_COLOR_RED, BUTTON_COLOR_INACTIVE);
}

void AutotunePowerPanel::RefreshPowerMonitorReadouts() {
	if (autotunePower->IsRunning()) {
		// Don't refresh anything while autotune is running - it takes care of refreshes on its own
	}
	else
		lc->RefreshPowerMonitorReadings(); // For the power monitor readout while autotune not running

	for (auto readout : powerMonitorReadouts)
		readout->RefreshAll();
}


void AutotunePowerPanel::RefreshAutotuneControlEnabled() {
	// Refresh main autotune power button enable state

	if (autotunePower->IsRunning()) {
		// Can always cancel Autotune at any time
		RefreshWidgetEnableBasedOnCondition(mainButton, true);
	}
	else {
		// To start Autotune, laser needs to be in running state and autotune oscillator cannot be running.
		bool canStartAutotune = lc->LaserIsRunning() and !lc->IsAutotuneOscillatorRunning();
		RefreshWidgetEnableBasedOnCondition(mainButton, canStartAutotune);
	}

	// Refresh Re-Tune buttons enabled
	bool canRetuneComponent = lc->LaserIsRunning() and !lc->IsAutotunePowerRunning() and !lc->IsAutotuneOscillatorRunning();
	for (auto componentPanel : autotuneComponentPanels)
		RefreshWidgetEnableBasedOnCondition(componentPanel->retuneButton, canRetuneComponent);
}


void AutotunePowerPanel::RefreshWarnings() {
	wxString warning = "";

	if (speedSlider->GetValue() > 5)
		warning += _("WARNING: Speed very high - May hurt Autotune performance\n");
	if (precisionSlider_Motor->GetValue() == 1)
		warning += _("WARNING: Motor precision very low - May hurt Autotune performance\n");
	if (precisionSlider_Temperature->GetValue() == 1)
		warning += _("WARNING: Temperature precision very low - May hurt Autotune performance\n");
	if (lc->GetAutotunePowerDropThreshold() < 5)
		warning += _("WARNING: Power drop threshold very low - Recommend at least 5%\n");

	if (warning != "" and GetGUIAccessMode() != GuiAccessMode::END_USER) // Only show these warnings in service and factory mode
		title->ShowWarning(warning);
	else
		title->HideWarning();
}


void AutotunePowerPanel::RefreshAutotuneProcedure() {

	if (startAutotuneTriggered) {

		if (autotunePower->IsFinished()) {

			wxLogStatus(to_wx_string(autotunePower->GetSummary()));
			runningMessage->Set(_("Finished."));
			runningMessage->StopCycling();
			
			// Unlock regular laser controller refreshes caused by main timer in cMain
			wxCommandEvent unlockEvent(UNLOCK_LASER_REFRESHES_EVENT, GetId());
			unlockEvent.SetEventObject(this);
			ProcessWindowEvent(unlockEvent);

			saveLogButton->Show();
			startAutotuneTriggered = false;

			RefreshPanels();
		}
		else if (autotunePower->IsError()) {

			if (lc->HasHardFault() or lc->HasSoftFault()) {
				// Only show the dialog once
				if (!faultDuringAutotuneDialogShown) {
					faultDuringAutotuneDialogShown = true; // Must go first
					wxMessageDialog faultDialog(nullptr, _(AUTOTUNE_FAULT_MESSAGE), _(AUTOTUNE_ERROR_STR));
					faultDialog.ShowModal();
				}
			}
			else
				runningMessage->Set(_("ERROR - ") + autotunePower->GetErrorMessage());

			runningMessage->StopCycling();

			// Unlock regular laser controller refreshes caused by main timer in cMain
			wxCommandEvent unlockEvent(UNLOCK_LASER_REFRESHES_EVENT, GetId());
			unlockEvent.SetEventObject(this);
			ProcessWindowEvent(unlockEvent);

			saveLogButton->Show();
			startAutotuneTriggered = false;

			RefreshPanels();
		}
		else if (autotunePower->IsRunning()) {
			runningMessage->Set(_(AUTOTUNE_RUNNING_MESSAGE_STR) + " - " + to_wx_string(autotunePower->GetProgressPercentage()) + "%");
		}
	}
}


void AutotunePowerPanel::RefreshPanels() {
	this->GetParent()->Layout();
	this->GetParent()->Update();
	this->Refresh();
}



//-----------------------------------------------------------------------------
// Main Autotune-power functionality

void AutotunePowerPanel::RunFullAutotune() {
	autotunePower->Reset();

	for (auto panel : autotuneComponentPanels)
		panel->ClearAll();

	for (auto data : autotunePowerTuneData) {
		if (data->type == "Motor")
			autotunePower->AddMotorComponent(data);
		else
			autotunePower->AddTemperatureComponent(data);
	}
	StartAutotune();
}


void AutotunePowerPanel::StartAutotune() {

	autotunePower->Start();

	// Check whether Autotune can be run (requires all LDDs to be fully powered
	// up, all shutters open, and output enabled).
	if (!autotunePower->CanStartAutotune()) {

		// If in factory mode, allow choice to start Autotune anyway
		if (IsInAccessMode(GuiAccessMode::FACTORY)) {

			wxMessageDialog cantStartDialog(nullptr,
				_("Error") + " - " + _(autotunePower->GetCantStartReason() + "\n" +
					"Do you want to run Autotune anyway?"),
				_(AUTOTUNE_ERROR_STR), wxYES_NO);
			if (cantStartDialog.ShowModal() == wxID_YES) {
				autotunePower->EnableStartOverride();
				autotunePower->Start();
			}
			else {
				runningMessage->Set(_("ERROR"));
				runningMessage->StopCycling();
				return;
			}

		}
		else {
			wxMessageBox(_("Error") + " - " + _(autotunePower->GetCantStartReason()), _(AUTOTUNE_ERROR_STR), wxICON_ERROR);
			runningMessage->Set(_("ERROR"));
			runningMessage->StopCycling();
			return;
		}

	}
	runningMessage->Show();
	runningMessage->Set(_(AUTOTUNE_RUNNING_MESSAGE_STR));
	runningMessage->StartCycling();


	// Lock regular laser controller refreshes caused by main timer in cMain -> this could interfere with refreshes from within
	//	the Autotune procedure because it includes periodic wxGetApp().Yield() commands.
	wxCommandEvent lockEvent(LOCK_LASER_REFRESHES_EVENT, GetId());
	lockEvent.SetEventObject(this);
	ProcessWindowEvent(lockEvent);

	StartAutotuneStepThread();
	startAutotuneTriggered = true;
	faultDuringAutotuneDialogShown = false;

	RefreshPanels();
}


void AutotunePowerPanel::StartAutotuneStepThread() {
	if (autotunePowerThread != nullptr) {
		autotunePowerThread->join();
	}
	autotunePowerThread = make_shared<std::thread>(StepAutotunePowerThread, autotunePower, autotuneComponentPanels);
}


void AutotunePowerPanel::CancelAutotune() {
	autotunePower->Cancel();
	runningMessage->Set(_("Canceled"));
	runningMessage->StopCycling();
	saveLogButton->Show();
	RefreshPanels();
}



//-----------------------------------------------------------------------------
// Helper functions

void AutotunePowerPanel::SetMotorPrecisionTooltip() {
	wxString precisionTooltip_motor = _("Adjust motor step size.\n - Use high precision for lasers sensitive\n   to motorized mirror alignment.\n\n");
	precisionTooltip_motor += _(" Level  Step Size\n");
	precisionTooltip_motor += lc->GetAutotuneMotorPrecisionTableString();
	precisionInfoIcon_Motor->SetToolTip(precisionTooltip_motor);
}

void AutotunePowerPanel::SetTemperaturePrecisionTooltip() {
	wxString precisionTooltip_temperature = _("Adjust temperature step size.\n - Use high precision for lasers sensitive\n   to harmonics temperature.\n\n");
	precisionTooltip_temperature += _(" Level  Step Size (deg. C)\n");
	precisionTooltip_temperature += lc->GetAutotuneTemperaturePrecisionTableString();
	precisionInfoIcon_Temperature->SetToolTip(precisionTooltip_temperature);
}

void AutotunePowerPanel::SetDropThreshold() {
	STAGE_ACTION("Set Autotune Power Drop Threshold");
	int newThreshold = dropThresholdSpinCtrl->GetValue();
	STAGE_ACTION_ARGUMENTS(to_string(newThreshold));
	lc->SetAutotunePowerDropThreshold(newThreshold);
	newThreshold = lc->GetAutotunePowerDropThreshold();
	dropThresholdSpinCtrl->SetValue(newThreshold);
	wxLogStatus("---New Power Drop Threshold Set: " + to_wx_string(newThreshold) + _(" percent"));
	LOG_ACTION();
}

void AutotunePowerPanel::SetMotorRange() {
	STAGE_ACTION("Set Autotune Motor Range");
	int newRange = motorRangeSpinCtrl->GetValue();
	STAGE_ACTION_ARGUMENTS(to_string(newRange));
	lc->SetAutotuneMotorRange(newRange);
	newRange = lc->GetAutotuneMotorRange();
	motorRangeSpinCtrl->SetValue(newRange);
	wxLogStatus("---New Motor Range Set: " + to_wx_string(newRange));
	LOG_ACTION();
}

void AutotunePowerPanel::SetTemperatureRange(float newRange) {
	STAGE_ACTION("Set Autotune Temperature Range");
	if (newRange < 0.0f)
		newRange = temperatureRangeSpinCtrlDouble->GetValue();
	STAGE_ACTION_ARGUMENTS(to_string_with_precision(newRange, 1));
	lc->SetAutotuneTemperatureRange(newRange);
	newRange = lc->GetAutotuneTemperatureRange();
	temperatureRangeSpinCtrlDouble->SetValue(newRange);
	wxLogStatus("---New Temperature Range Set: " + to_wx_string(newRange, 1));
	LOG_ACTION();
}



//-----------------------------------------------------------------------------
// Callbacks

void AutotunePowerPanel::OnMainButtonClicked(wxCommandEvent& evt) {
	STAGE_ACTION("Main Autotune-Power button clicked")
	if (autotunePower->IsRunning())
		CancelAutotune();
	else {
		wxString msg = _(CONFIRM_AUTOTUNE_MESSAGE);
		wxMessageDialog confirmAutotuneDialog(nullptr, msg, _(CONFIRM_AUTOTUNE_STR), wxOK | wxCANCEL);
		confirmAutotuneDialog.SetOKLabel(_("Yes"));
		if (confirmAutotuneDialog.ShowModal() == wxID_OK) {
			RunFullAutotune();
			STAGE_ACTION_ARGUMENTS("Confirmed");
		}
		else {
			STAGE_ACTION_ARGUMENTS("Cancelled");
		}
	}
	LOG_ACTION()
}


// Assumes the individual component panel already reset the 
// Autotune Power Manager and added only itself for the next run
void AutotunePowerPanel::OnRetuneButtonClicked(wxCommandEvent& evt) {
	STAGE_ACTION_("Autotune-Power Retune button clicked", to_string(evt.GetId()))
	AutotuneComponentPanel* panel = mapIdToPlotPanel.at(evt.GetId());
	panel->ClearAll();
	autotunePower->Reset();
	if (panel->data->type == MOTOR_STR)
		autotunePower->AddMotorComponent(panel->data);
	else
		autotunePower->AddTemperatureComponent(panel->data);

	StartAutotune();

	LOG_ACTION()
}


AutotunePowerPanel::~AutotunePowerPanel() {
	if (autotunePowerThread) {
		autotunePowerThread->join();
	}
}


void AutotunePowerPanel::OnSaveLogButtonClicked(wxCommandEvent& evt) {
	STAGE_ACTION("Save Autotune-Power Log button clicked")
	wxString defaultPath = autotunePower->GetDefaultLogPath();
	wxString defaultFilename = autotunePower->GetDefaultLogFilename();

	wxFileDialog saveMemoryFileDialog(nullptr, (_("Save Log File")), defaultPath, defaultFilename, "LOG files (*.log)|*.log", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

	if (saveMemoryFileDialog.ShowModal() == wxID_OK) {
		string path = string(saveMemoryFileDialog.GetPath());
		STAGE_ACTION_ARGUMENTS(path)

		// If in higher access mode, save log unencrypted
		if (GetGUIAccessMode() == GuiAccessMode::SERVICE or GetGUIAccessMode() == GuiAccessMode::FACTORY)
			autotunePower->SaveLog(path);
		else
			autotunePower->SaveLogEncrypted(path);

		if (autotunePower->SaveLogSuccessful())
			wxLogStatus(_("Save successful."));
		else
			wxLogStatus(_("Save failed."));
	}
	else {
		wxLogStatus(_("Save log file cancelled."));
	}
	LOG_ACTION()
}

void AutotunePowerPanel::OnSettingsCollapse(wxCollapsiblePaneEvent& evt) {
	STAGE_ACTION("Autotune-Power settings collapse button clicked")
	RefreshPanels();
	LOG_ACTION()
}

void AutotunePowerPanel::OnSpeedSliderMoved(wxCommandEvent& evt) {
	STAGE_ACTION("Autotune-Power speed slider moved")
	int newSpeedLevel = speedSlider->GetValue();
	STAGE_ACTION_ARGUMENTS(to_string(newSpeedLevel))
	lc->SetAutotuneSpeedLevel(newSpeedLevel);
	LOG_ACTION()
}

void AutotunePowerPanel::OnPrecisionSliderMoved_Motor(wxCommandEvent& evt) {
	STAGE_ACTION("Autotune-Power motor precision slider moved")
	int newPrecisionLevel = precisionSlider_Motor->GetValue();
	STAGE_ACTION_ARGUMENTS(to_string(newPrecisionLevel))
	lc->SetAutotuneMotorPrecisionLevel(newPrecisionLevel);
	LOG_ACTION()
}

void AutotunePowerPanel::OnPrecisionSliderMoved_Temperature(wxCommandEvent& evt) {
	STAGE_ACTION("Autotune-Power temperature precision slider moved")
	int newPrecisionLevel = precisionSlider_Temperature->GetValue();
	STAGE_ACTION_ARGUMENTS(to_string(newPrecisionLevel))
	lc->SetAutotuneTemperaturePrecisionLevel(newPrecisionLevel);
	LOG_ACTION()
}

void AutotunePowerPanel::SetDropThresholdWithText(wxCommandEvent& evt) {
	SetDropThreshold();
}

void AutotunePowerPanel::SetDropThresholdWithSpin(wxSpinEvent& evt) {
	SetDropThreshold();
}


void AutotunePowerPanel::SetMotorRangeWithSpin(wxSpinEvent& evt) {
	SetMotorRange();
}

void AutotunePowerPanel::SetMotorRangeWithText(wxCommandEvent& evt) {
	SetMotorRange();
}

void AutotunePowerPanel::SetTemperatureRangeWithSpin(wxSpinEvent& evt) {
	SetTemperatureRange();
}

void AutotunePowerPanel::SetTemperatureRangeWithText(wxCommandEvent& evt) {
	float newRangef = ToFloatSafely(string(evt.GetString())) + 0.0001f; // Add small amount to avoid truncation error
	SetTemperatureRange(newRangef);
}



//-----------------------------------------------------------------------------
// Public methods

void AutotunePowerPanel::RefreshVisibilityBasedOnAccessMode() {
	// 5/23/24 - JB - Autotune settings can now only be accessed in factory mode
	// or with Autotune access code or if MAC Address has production access (PLT)
	bool factoryMode = IsInAccessMode(GuiAccessMode::FACTORY);
	bool autotuneSettingsAccessCodeEntered = AccessManager::GetInstance().GetCurrentAccessCodeType() == AccessCodeType::AUTOTUNE_SETTINGS;
	bool hasProductionAccess = ThisMACHasProductionAccess();

	bool showSettings = factoryMode or autotuneSettingsAccessCodeEntered or hasProductionAccess;

	SetVisibilityBasedOnCondition(collapsibleSettingsPanel, showSettings);

	RefreshPanels();
}


void AutotunePowerPanel::RefreshStrings() {
	if (lc->AutotunePowerIsEnabledForUse()) {
		title->RefreshStrings();
		saveLogButton->SetLabelText(SAVE_LOG_STR);
		collapsibleSettingsPanel->SetLabelText(_(SETTINGS_STR));
		speedLabel->SetLabelText(_(SPEED_STR));
		speedInfoIcon->SetToolTip(_(SPEED_TOOLTIP));
		precisionLabel_Motor->SetLabelText(_(PRECISION_MOTOR_STR));
		precisionLabel_Temperature->SetLabelText(_(PRECISION_TEMPERATURE_STR));
		SetMotorPrecisionTooltip();
		SetTemperaturePrecisionTooltip();
		dropThresholdLabel->SetLabelText(_(PCT_DROP_THRESHOLD_STR));
		dropThresholdInfoIcon->SetToolTip(_(PCT_DROP_THRESHOLD_TOOLTIP));
		motorRangeLabel->SetLabelText(_(MOTOR_RANGE_STR));
		motorRangeInfoIcon->SetLabelText(_(MOTOR_RANGE_TOOLTIP));
		temperatureRangeLabel->SetLabelText(_(TEMPERATURE_RANGE_STR));
		temperatureRangeInfoIcon->SetLabelText(_(TEMPERATURE_RANGE_TOOLTIP));
	}
}
