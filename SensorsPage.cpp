#include "SensorsPage.h"
#include "MainDefinitions.h"
#include "../CommonFunctions_GUI.h"
#include "../AccessCodeDialog.h"

using namespace std;

const wxString POWER_MONITORS_STR = _("Power Monitors");
const wxString CHILLER_STR = _("Chiller");
const wxString HUMIDITY_STR = _("Humidity");
const wxString GENERATE_SENSORS_ACCESS_KEY_STR = _("Generate Sensors Access Key");
const wxString SENSORS_ACCESS_KEY_MESSAGE_STR = _(
	"Send this code to a Photonics representative\n"
	"to request a temporary access code."
);


SensorsPage::SensorsPage(
	shared_ptr<MainLaserControllerInterface> _lc,
	wxWindow* parent
) :
	SettingsPage_Base(_lc, parent) {

	sizer = new wxBoxSizer(wxVERTICAL);

	getSensorsAccessKeyButton = new wxBitmapButton(this, wxID_ANY, wxBitmap(KEY_ICON, wxBITMAP_TYPE_ANY),
		wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW | 0);
	getSensorsAccessKeyButton->Bind(wxEVT_BUTTON, &SensorsPage::OnGetSensorsAccessKeyButtonClicked, this);
	getSensorsAccessKeyButton->SetToolTip(_(GENERATE_SENSORS_ACCESS_KEY_STR));
	sizer->Add(getSensorsAccessKeyButton, 0, wxALL, 2);

	Init();
}


void SensorsPage::Init() {

	// Power Monitors

	if (lc->GetPowerMonitorIDs().size() > 0) {

		powerMonitorsPanel = new wxPanel(this, wxID_ANY, wxDefaultPosition,
			wxDefaultSize, wxBORDER_THEME | wxTAB_TRAVERSAL);
		powerMonitorsPanel->SetBackgroundColour(READOUT_PANEL_COLOR);

		wxBoxSizer* powerMonitorsPanelSizer = new wxBoxSizer(wxVERTICAL);

		powerMonitorsTitle = new wxStaticText(powerMonitorsPanel, wxID_ANY, _(POWER_MONITORS_STR));
		powerMonitorsTitle->SetFont(FONT_SMALL_SEMIBOLD);
		powerMonitorsPanelSizer->Add(powerMonitorsTitle, 0, wxTOP | wxLEFT, 5);

		powerMonitorsSizer = new wxBoxSizer(wxHORIZONTAL);

		for (auto pmId : lc->GetPowerMonitorIDs()) {
			AddPowerMonitorSettingsPanel(pmId);
		}

		powerMonitorsPanelSizer->Add(powerMonitorsSizer, 0, wxALL, 3);

		powerMonitorsPanel->SetSizer(powerMonitorsPanelSizer);
		powerMonitorsPanel->Layout();
		powerMonitorsPanelSizer->Fit(powerMonitorsPanel);

		sizer->Add(powerMonitorsPanel, 0, wxTOP | wxLEFT, 10);

	}

	// Chiller 

	if (lc->GetChillerFlowIds().size() > 0) {

		chillersPanel = new wxPanel(this, wxID_ANY, wxDefaultPosition,
			wxDefaultSize, wxBORDER_THEME | wxTAB_TRAVERSAL);
		chillersPanel->SetBackgroundColour(READOUT_PANEL_COLOR);

		wxBoxSizer* chillersPanelSizer = new wxBoxSizer(wxVERTICAL);

		chillerTitle = new wxStaticText(chillersPanel, wxID_ANY, _(CHILLER_STR));
		chillerTitle->SetFont(FONT_SMALL_SEMIBOLD);
		chillersPanelSizer->Add(chillerTitle, 0, wxTOP | wxLEFT, 5);

		chillersSizer = new wxBoxSizer(wxHORIZONTAL);

		for (auto chillerId : lc->GetChillerFlowIds()) {
			AddChillerSettingsPanel(chillerId);
		}

		chillersPanelSizer->Add(chillersSizer, 0, wxALL, 3);

		chillersPanel->SetSizer(chillersPanelSizer);
		chillersPanel->Layout();
		chillersPanelSizer->Fit(chillersPanel);

		sizer->Add(chillersPanel, 0, wxTOP | wxLEFT, 10);

	}

	// Humidity

	if (lc->GetHumidityIds().size() > 0) {

		humidityPanel = new wxPanel(this, wxID_ANY, wxDefaultPosition,
			wxDefaultSize, wxBORDER_THEME | wxTAB_TRAVERSAL);
		humidityPanel->SetBackgroundColour(READOUT_PANEL_COLOR);

		wxBoxSizer* humidityPanelSizer = new wxBoxSizer(wxVERTICAL);

		humidityTitle = new wxStaticText(humidityPanel, wxID_ANY, _(HUMIDITY_STR));
		humidityTitle->SetFont(FONT_SMALL_SEMIBOLD);
		humidityPanelSizer->Add(humidityTitle, 0, wxTOP | wxLEFT, 5);

		humiditySizer = new wxBoxSizer(wxHORIZONTAL);

		for (auto humidityId : lc->GetHumidityIds()) {
			AddHumiditySettingsPanel(humidityId);
		}

		humidityPanelSizer->Add(humiditySizer, 0, wxALL, 3);

		humidityPanel->SetSizer(humidityPanelSizer);
		humidityPanel->Layout();
		humidityPanelSizer->Fit(humidityPanel);

		sizer->Add(humidityPanel, 0, wxTOP | wxLEFT, 10);
	}

	RefreshStrings();
	RefreshAll();

	SetSizer(sizer);
	Layout();
	sizer->Fit(this);

}

void SensorsPage::RefreshAll() {
	for (auto& pmPanel : powerMonitorPanels)
		pmPanel->RefreshAll();
	for (auto& humidityPanel : humidityPanels)
		humidityPanel->RefreshAll();
	for (auto& cPanel : chillerPanels)
		cPanel->RefreshAll();
}


void SensorsPage::RefreshStrings() {
	SetName(_(SENSORS_STR));

	if (powerMonitorsTitle)
		powerMonitorsTitle->SetLabelText(_(POWER_MONITORS_STR));
	if (chillerTitle)
		chillerTitle->SetLabelText(_(CHILLER_STR));
	if (humidityTitle)
		humidityTitle->SetLabelText(_(HUMIDITY_STR));

	for (auto& pmPanel : powerMonitorPanels)
		pmPanel->RefreshStrings();
	for (auto& humidityPanel : humidityPanels)
		humidityPanel->RefreshStrings();
	for (auto& cPanel : chillerPanels)
		cPanel->RefreshStrings();
}


void SensorsPage::RefreshVisibility() {

	bool inPrivilegedMode = IsInAccessMode(GuiAccessMode::SERVICE) or IsInAccessMode(GuiAccessMode::FACTORY);
	bool sensorAccessCodeEntered = AccessManager::GetInstance().GetCurrentAccessCodeType() == AccessCodeType::SENSOR;

	// Basic power monitor calibration should be available to user by default.
	// But only show humidity and chiller calibration to user if access code entered.
	SetVisibilityBasedOnCondition(humidityPanel, inPrivilegedMode or sensorAccessCodeEntered);
	SetVisibilityBasedOnCondition(chillersPanel, inPrivilegedMode or sensorAccessCodeEntered);


	for (auto& pmPanel : powerMonitorPanels)
		pmPanel->RefreshVisibility();

	for (auto& humidityPanel : humidityPanels)
		humidityPanel->RefreshVisibility();

	for (auto& cPanel : chillerPanels)
		cPanel->RefreshVisibility();
}


void SensorsPage::RefreshControlEnabled() {
}


void SensorsPage::AddPowerMonitorSettingsPanel(int pm_id) {
	SensorPanel_PowerMonitor* powerMonitorPanel = new SensorPanel_PowerMonitor(lc, pm_id, powerMonitorsPanel);
	powerMonitorsSizer->Add(powerMonitorPanel, 0, wxALL, 4);
	powerMonitorPanels.push_back(powerMonitorPanel);
}

void SensorsPage::AddChillerSettingsPanel(int chiller_id) {
	SensorPanel_Chiller* cPanel = new SensorPanel_Chiller(lc, chiller_id, chillersPanel);
	chillersSizer->Add(cPanel, 0, wxALL, 4);
	chillerPanels.push_back(cPanel);
}

void SensorsPage::AddHumiditySettingsPanel(int humidity_id) {
	SensorPanel_Humidity* hPanel = new SensorPanel_Humidity(lc, humidity_id, humidityPanel);
	humiditySizer->Add(hPanel, 0, wxALL, 4);
	humidityPanels.push_back(hPanel);
}

void SensorsPage::OnGetSensorsAccessKeyButtonClicked(wxCommandEvent& evt) {
	STAGE_ACTION("Generate sensors access key")

	wxString code = GenerateOfflinePartialKey(AccessCodeType::SENSOR);
	AccessCodeDialog accessCodeDialog(this, code, _(SENSORS_ACCESS_KEY_MESSAGE_STR));
	accessCodeDialog.ShowModal();

	LOG_ACTION()
}
