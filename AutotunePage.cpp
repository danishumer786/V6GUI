#include "AutotunePage.h"
#include "AutotuneComponentPanel.h"
#include "../CommonFunctions_GUI.h"
#include "../AccessCodeDialog.h"

using namespace std;

const wxString AUTOTUNE_STR = _("Calibrate"); // Switch to using this term at Hans' suggestion - 2-8-24
const wxString GENERATE_AUTOTUNE_SETTINGS_ACCESS_KEY_STR = _("Generate Access Key");
const wxString AUTOTUNE_SETTINGS_ACCESS_KEY_MESSAGE_STR = _(
	"Send this code to a Photonics factory representative\n"
	"to request a temporary access code."
);


AutotunePage::AutotunePage(shared_ptr<MainLaserControllerInterface> _lc, wxWindow* parent) :
	SettingsPage_Base(_lc, parent) {

	sizer = new wxBoxSizer(wxHORIZONTAL);

	getAutotuneSettingsAccessKeyButton = new wxBitmapButton(this, wxID_ANY, wxBitmap(KEY_ICON, wxBITMAP_TYPE_ANY),
		wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW | 0);
	getAutotuneSettingsAccessKeyButton->Bind(wxEVT_BUTTON, &AutotunePage::OnGetAutotuneSettingsAccessKeyButtonClicked, this);
	getAutotuneSettingsAccessKeyButton->SetToolTip(GENERATE_AUTOTUNE_SETTINGS_ACCESS_KEY_STR);
	sizer->Add(getAutotuneSettingsAccessKeyButton, 0, wxALL, 2);

	controlsSizer = new wxBoxSizer(wxVERTICAL);

	sizer->Add(controlsSizer, 0, wxALL, 5);

	powerPlotsPanel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_SUNKEN);
	powerPlotsPanel->SetBackgroundColour(HIDEABLE_PANEL_COLOR);

	powerPlotsSizer = new wxBoxSizer(wxVERTICAL);


	powerPlotsPanel->SetSizer(powerPlotsSizer);
	powerPlotsPanel->Layout();
	powerPlotsSizer->Fit(powerPlotsPanel);

	sizer->Add(powerPlotsPanel, 0, wxALL, 5);


	this->SetSizer(sizer);

	Init();
}


void AutotunePage::Init() {
	InitAutotunePower();
	InitAutotuneOscillator();
	InitAutotuneDiagnostics();
	InitAutotunePowerPlotsPanel();
	windowsToHideFromEndUser = {
		powerPlotsPanel
	};

	RefreshVisibility();

	this->Layout();
	sizer->Fit(this);
}


void AutotunePage::InitAutotunePower() {
	if (lc->AutotunePowerIsEnabledForUse()) {
		autotunePower = make_shared<AutotunePowerManager>(lc);
		autotunePowerPanel = new AutotunePowerPanel(lc, autotunePower, autotuneOscillator, this);
		controlsSizer->Add(autotunePowerPanel, 0, wxALL, 5);
	}
}


void AutotunePage::InitAutotuneOscillator() {
	if (lc->AutotuneOscillatorIsEnabledForUse()) {
		autotuneOscillator = make_shared<AutotuneOscillatorManager>(lc);
		autotuneOscillatorPanel = new AutotuneOscillatorPanel(lc, autotunePower, autotuneOscillator, this);
		controlsSizer->Add(autotuneOscillatorPanel, 0, wxALL, 5);
	}
}


void AutotunePage::InitAutotuneDiagnostics() {
	if (lc->AutotunePowerIsEnabledForUse()) {
		autotuneDiagnostics = make_shared<AutotuneDiagnostics>(lc);
		autotuneDiagnosticsPanel = new AutotuneDiagnosticsPanel(lc, autotuneDiagnostics, this);
		controlsSizer->Add(autotuneDiagnosticsPanel, 0, wxALL, 5);
	}
}


void AutotunePage::OnGetAutotuneSettingsAccessKeyButtonClicked(wxCommandEvent& evt) {
	STAGE_ACTION("Generate Autotune Settings access key")
	wxString code = GenerateOfflinePartialKey(AccessCodeType::AUTOTUNE_SETTINGS);
	AccessCodeDialog accessCodeDialog(this, code, _(AUTOTUNE_SETTINGS_ACCESS_KEY_MESSAGE_STR));
	accessCodeDialog.ShowModal();
	LOG_ACTION()
}


void AutotunePage::InitAutotunePowerPlotsPanel() {

	vector<int> pmIds = lc->GetAutotunePowerMonitorIds();

	for (vector<int>::reverse_iterator pmId = pmIds.rbegin(); pmId != pmIds.rend(); ++pmId) {

		// 1st TEC
		int firstTecId = lc->GetAutotuneFirstTecId(*pmId);
		if (firstTecId != -1) {

			shared_ptr<PowerTuneData> data = make_shared<PowerTuneData>(*pmId, firstTecId);
			data->type = TEMPERATURE_STR;

			shared_ptr<ATD_Data> diagnosticData = make_shared<ATD_Data>(*pmId, firstTecId);
			diagnosticData->type = TEMPERATURE_STR;

			InitTuningPlotPanel(data, diagnosticData);
		}

		// 2nd TEC
		int secondTecId = lc->GetAutotuneSecondTecId(*pmId);
		if (secondTecId != -1) {

			shared_ptr<PowerTuneData> data = make_shared<PowerTuneData>(*pmId, secondTecId);
			data->type = TEMPERATURE_STR;

			shared_ptr<ATD_Data> diagnosticData = make_shared<ATD_Data>(*pmId, secondTecId);
			diagnosticData->type = TEMPERATURE_STR;

			InitTuningPlotPanel(data, diagnosticData);
		}

		// 1st motor
		int firstMotorId = lc->GetAutotuneFirstMotorId(*pmId);
		if (firstMotorId != -1) {

			shared_ptr<PowerTuneData> data = make_shared<PowerTuneData>(*pmId, firstMotorId);
			data->type = MOTOR_STR;

			shared_ptr<ATD_Data> diagnosticData = make_shared<ATD_Data>(*pmId, firstMotorId);
			diagnosticData->type = MOTOR_STR;

			InitTuningPlotPanel(data, diagnosticData);
		}

		// 2nd motor
		int secondMotorId = lc->GetAutotuneSecondMotorId(*pmId);
		if (secondMotorId != -1) {

			shared_ptr<PowerTuneData> data = make_shared<PowerTuneData>(*pmId, secondMotorId);
			data->type = MOTOR_STR;

			shared_ptr<ATD_Data> diagnosticData = make_shared<ATD_Data>(*pmId, secondMotorId);
			diagnosticData->type = MOTOR_STR;

			InitTuningPlotPanel(data, diagnosticData);
		}

	}

}


void AutotunePage::InitTuningPlotPanel(shared_ptr<PowerTuneData> data, std::shared_ptr<ATD_Data> diagnostic_data) {

	int plotPanelID = wxNewId();
	AutotuneComponentPanel* autotuneComponentPanel = new AutotuneComponentPanel(lc, autotunePower, data, plotPanelID, powerPlotsPanel);

	autotuneComponentPanel->AddAutotuneDiagnostics(autotuneDiagnostics);
	autotuneComponentPanel->AddDiagnosticData(diagnostic_data);

	autotunePowerPanel->mapIdToPlotPanel[plotPanelID] = autotuneComponentPanel;
	autotuneDiagnosticsPanel->mapIdToPlotPanel[plotPanelID] = autotuneComponentPanel;

	autotuneComponentPanel->retuneButton->Bind(wxEVT_BUTTON, &AutotunePowerPanel::OnRetuneButtonClicked, autotunePowerPanel);
	autotuneComponentPanel->runDiagnosticsButton->Bind(wxEVT_BUTTON, &AutotuneDiagnosticsPanel::OnRetuneButtonClicked, autotuneDiagnosticsPanel);

	powerPlotsSizer->Add(autotuneComponentPanel, 0, wxALL, 5);

	autotunePowerPanel->autotuneComponentPanels.push_back(autotuneComponentPanel);
	autotunePowerPanel->autotunePowerTuneData.push_back(data);

	autotuneDiagnosticsPanel->autotuneComponentPanels.push_back(autotuneComponentPanel);
	autotuneDiagnosticsPanel->diagnosticsData.push_back(diagnostic_data);
}


void AutotunePage::RefreshAll() {
	if (autotunePowerPanel)
		autotunePowerPanel->RefreshAll();
	if (autotuneDiagnosticsPanel)
		autotuneDiagnosticsPanel->RefreshAll();
	if (autotuneOscillatorPanel)
		autotuneOscillatorPanel->RefreshAll();
}


void AutotunePage::RefreshStrings() {
	SetName(_(AUTOTUNE_STR));

	if (autotunePowerPanel) {
		autotunePowerPanel->RefreshStrings();
		for (auto component : autotunePowerPanel->autotuneComponentPanels)
			component->RefreshStrings();
	}
	if (autotuneOscillatorPanel)
		autotuneOscillatorPanel->RefreshStrings();
	if (autotuneDiagnosticsPanel)
		autotuneDiagnosticsPanel->RefreshStrings();
}


void AutotunePage::RefreshVisibility() {
	for (wxWindow* window : windowsToHideFromEndUser)
		SetVisibilityBasedOnAccessMode(window, 1);

	if (autotunePowerPanel)
		autotunePowerPanel->RefreshVisibilityBasedOnAccessMode();

	if (autotuneOscillatorPanel)
		autotuneOscillatorPanel->RefreshVisibilityBasedOnAccessMode();

	if (autotuneDiagnosticsPanel)
		autotuneDiagnosticsPanel->RefreshVisibilityBasedOnAccessMode();

	this->Layout();
	this->Update();
}

void AutotunePage::RefreshControlEnabled() {
}
