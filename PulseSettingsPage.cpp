#include <chrono>

#include "wx/gbsizer.h"

#include "PulseSettingsPage.h"
#include "../CommonFunctions_GUI.h"
#include "../AccessCodeDialog.h"

using namespace std;

const wxString PULSE_STR = _("Pulse");

const wxString OPEN_PULSE_CONTROLS_STR = _("Open Pulse Controls");

const wxString GENERATE_PULSE_ACCESS_KEY_STR = _("Generate Pulse Settings Access Key");
const wxString PULSE_ACCESS_KEY_MESSAGE_STR = _(
	"Send this code to a Photonics factory representative\n"
	"to request a temporary access code."
);

static wxString TERMINATION_SETTINGS_STR = _("External Termination Settings");
static wxString TERMINATION_SETTINGS_TOOLTIP = _(
	"Set the termination impedance for the following external sources.\n\n"
	" - 50 ohm termination no longer available for EXT PEC\n"
	" - REQUIRES RESET FOR CHANGES TO TAKE EFFECT"
);



PulseSettingsPage::PulseSettingsPage(shared_ptr<MainLaserControllerInterface> _lc, wxWindow* parent) :
	SettingsPage_Base(_lc, parent)
{

	sizer = new wxBoxSizer(wxVERTICAL);


	getPulseAccessKeyButton = new wxBitmapButton(this, wxID_ANY, wxBitmap(KEY_ICON, wxBITMAP_TYPE_ANY),
		wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW | 0);
	getPulseAccessKeyButton->Bind(wxEVT_BUTTON, &PulseSettingsPage::OngetPulseAccessKeyButtonClicked, this);
	getPulseAccessKeyButton->SetToolTip(GENERATE_PULSE_ACCESS_KEY_STR);
	sizer->Add(getPulseAccessKeyButton, 0, wxALL, 2);


	// Button to open pulse controls mini window
	OpenPulseControlMiniWindowButton = new wxButton(this, wxID_ANY, _(OPEN_PULSE_CONTROLS_STR));
	OpenPulseControlMiniWindowButton->SetFont(FONT_EXTRA_SMALL_SEMIBOLD);
	OpenPulseControlMiniWindowButton->SetBackgroundColour(PHOTONICS_TURQUOISE_COLOR);
	OpenPulseControlMiniWindowButton->Bind(wxEVT_BUTTON, &PulseSettingsPage::OnOpenPulseControlMiniWindow, this);
	sizer->Add(OpenPulseControlMiniWindowButton, 0, wxALL, 5);

	
	// PEC Settings Panel
	if (lc->PECIsEnabledForUse()) {
		PECSettings = new PECSettingsPanel(lc, this);
		sizer->Add(PECSettings, 0, wxALL, 5);
	}


	// Pulse Calibration Panel
	PulseCalibration = new PulseCalibrationPanel(lc, this);
	sizer->Add(PulseCalibration, 0, wxALL, 5);


	//if (compareVersions(lc->GetFirmwareVersion(), "3.0.1ER514") >= 0 and compareVersions(lc->GetFPGAVersion(), "7.0.0ER18") >= 0) {
	if (compareVersions(lc->GetFirmwareVersion(), "3.0.1ER514") >= 0 and lc->PSOIsEnabledForUse()) {
		// PSO RF Level Time Table Panel
		PSORFTimeTable = new PSORFTimeTablePanel(lc, this);
		sizer->Add(PSORFTimeTable, 0, wxALL, 5);
	}


	// Termination settings panel
	TerminationSettingsPanel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL | wxBORDER_THEME);
	TerminationSettingsPanel->SetBackgroundColour(FOREGROUND_PANEL_COLOR);

	wxBoxSizer* TerminationSettingsSizer = new wxBoxSizer(wxVERTICAL);

	TerminationSettingsTitle = new FeatureTitle(TerminationSettingsPanel, TERMINATION_SETTINGS_STR, TERMINATION_SETTINGS_TOOLTIP);
	TerminationSettingsSizer->Add(TerminationSettingsTitle, 0, wxALL | wxALIGN_CENTER_HORIZONTAL, 5);

	wxBoxSizer* TerminationSettingsRowsSizer = new wxBoxSizer(wxVERTICAL);

	PRFTerminationSetting = new TerminationSetting(lc, TerminationSettingsPanel, _("EXT PRF Impedance"),
		[this]() { return lc->GetPRFTerminationSetting(); },
		[this]() { lc->TogglePRFTerminationSetting(); }
	);
	TerminationSettingsRowsSizer->Add(PRFTerminationSetting, 0, wxALL | wxALIGN_CENTER_HORIZONTAL, 5);

	if (lc->PECIsEnabledForUse()) {
		PECTerminationSetting = new TerminationSetting(lc, TerminationSettingsPanel, _("EXT PEC Impedance"),
			[this]() { return lc->GetPECTerminationSetting(); },
			[this]() { lc->TogglePECTerminationSetting(); }
		);
		TerminationSettingsRowsSizer->Add(PECTerminationSetting, 0, wxALL | wxALIGN_CENTER_HORIZONTAL, 5);
	}

	if (!lc->IsType("SLM")) {
		GateTerminationSetting = new TerminationSetting(lc, TerminationSettingsPanel, _("EXT Gate Impedance"),
			[this]() { return lc->GetGateTerminationSetting(); },
			[this]() { lc->ToggleGateTerminationSetting(); }
		);
		TerminationSettingsRowsSizer->Add(GateTerminationSetting, 0, wxALL | wxALIGN_CENTER_HORIZONTAL, 5);
	}


	TerminationSettingsSizer->Add(TerminationSettingsRowsSizer, 0, wxEXPAND, 5);


	TerminationSettingsPanel->SetSizer(TerminationSettingsSizer);
	TerminationSettingsPanel->Layout();
	TerminationSettingsSizer->Fit(TerminationSettingsPanel);
	sizer->Add(TerminationSettingsPanel, 0, wxALL, 5);

	// Used a rare event type to avoid other common events causing window to close
	this->Bind(wxEVT_COMBOBOX_DROPDOWN, &PulseSettingsPage::OnPulseControlMiniWindowClosed, this);

	this->SetSizer(sizer);
	this->Layout();

    Init();
}

void PulseSettingsPage::Init() {

	windowsToHideFromEndUser = {
		//AutoCalibratePECPanel
	};

	RefreshVisibility();

	RefreshTerminationSettings();
	if (PECTerminationSetting)
		PECTerminationSetting->Disable();
}


void PulseSettingsPage::RefreshStrings() {
	SetName(_(PULSE_STR));
	OpenPulseControlMiniWindowButton->SetLabelText(_(OPEN_PULSE_CONTROLS_STR));

	if (PECSettings)
		PECSettings->RefreshStrings();
	PulseCalibration->RefreshStrings();
	if (PSORFTimeTable)
		PSORFTimeTable->RefreshStrings();

	TerminationSettingsTitle->RefreshStrings();
	PRFTerminationSetting->RefreshStrings();
	if (PECTerminationSetting)
		PECTerminationSetting->RefreshStrings();
	if (GateTerminationSetting)
		GateTerminationSetting->RefreshStrings();
}


void PulseSettingsPage::RefreshAll() {
	if (PECSettings)
		PECSettings->RefreshAll();

	PulseCalibration->RefreshAll();

	if (PSORFTimeTable)
		PSORFTimeTable->RefreshAll();

	if (pulseControlMiniWindowOpen)
		pulseControlMiniWindow->RefreshAll();
}


void PulseSettingsPage::RefreshVisibility() {

	for (wxWindow* window : windowsToHideFromEndUser)
		SetVisibilityBasedOnAccessMode(window, 1);

	bool showPulseSettings =
		IsInAccessMode(GuiAccessMode::SERVICE) or
		IsInAccessMode(GuiAccessMode::FACTORY) or
		AccessManager::GetInstance().GetCurrentAccessCodeType() == AccessCodeType::PULSE;

	SetVisibilityBasedOnCondition(OpenPulseControlMiniWindowButton, showPulseSettings);

	bool ShowPECSettings = showPulseSettings and lc->PECIsEnabledForUse();

	if (PECSettings) {
		SetVisibilityBasedOnCondition(PECSettings, ShowPECSettings);
		PECSettings->RefreshVisibility();
	}

	SetVisibilityBasedOnCondition(PulseCalibration, showPulseSettings);
	PulseCalibration->RefreshVisibility();

	if (PSORFTimeTable)
		PSORFTimeTable->RefreshVisibility();

	this->Layout();
	this->Update();
}


void PulseSettingsPage::RefreshTerminationSettings() {
	PRFTerminationSetting->RefreshAll();
	if (PECTerminationSetting)
		PECTerminationSetting->RefreshAll();
	if (GateTerminationSetting)
		GateTerminationSetting->RefreshAll();
}


void PulseSettingsPage::InitPulseControlMiniWindow() {
	if (!pulseControlMiniWindowOpen) {
		pulseControlMiniWindow = new PulseControlMiniWindow(lc, this, wxID_ANY);
		pulseControlMiniWindowOpen = true;
	}
}

void PulseSettingsPage::OngetPulseAccessKeyButtonClicked(wxCommandEvent& evt) {
	STAGE_ACTION("Generate pulse access key")
	wxString code = GenerateOfflinePartialKey(AccessCodeType::PULSE);
	AccessCodeDialog accessCodeDialog(this, code, _(PULSE_ACCESS_KEY_MESSAGE_STR));
	accessCodeDialog.ShowModal();
	LOG_ACTION()
}

void PulseSettingsPage::OnOpenPulseControlMiniWindow(wxCommandEvent& evt) {
	STAGE_ACTION("Open pulse control mini window button clicked")
	InitPulseControlMiniWindow();
	LOG_ACTION()
}

void PulseSettingsPage::OnPulseControlMiniWindowClosed(wxCommandEvent& evt) {
	pulseControlMiniWindowOpen = false;
	delete pulseControlMiniWindow;
}

