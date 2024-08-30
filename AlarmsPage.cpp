#include "AlarmsPage.h"
#include "MainDefinitions.h"
#include "..\CommonFunctions_GUI.h"
#include "Security/AccessByMACAddress.h"

using namespace std;

const wxString GENERAL_ALARMS_STR = _("General Alarms");
const wxString ACTUAL_CURRENT_STR = _("Actual Current");
const wxString WET_SENSOR_STR = _("Wet Sensor");
const wxString HUMIDITY_STR = _("Humidity");
const wxString HF_SYNC_STR = _("HF Sync");
const wxString FLOW_STR = _("Flow");
const wxString POWER_MONITOR_STR = _("Power Monitor");
const wxString LDD1_STR = _("LDD1");
const wxString LDD2_STR = _("LDD2");
const wxString QSW_STR = _("QSW");

const wxString SOFT_FAULTS_STR = _("Soft Faults");
const wxString LDD_INTERLOCK_STR = _("LDD Interlock");
const wxString SHUTTER_INTERLOCK_STR = _("Shutter Interlock");
const wxString LOW_EXT_STR = _("Low EXT PRF");
const wxString HIGH_EXT_STR = _("High EXT PRF");
const wxString PP_SYNC_STR = _("PP Sync");


extern std::string ALARM_ACTUAL_CURRENT;


AlarmsPage::AlarmsPage(
	shared_ptr<MainLaserControllerInterface> _lc,
	wxWindow* parent
) :
	SettingsPage_Base(_lc, parent) 
{

	sizer = new wxBoxSizer(wxHORIZONTAL);

	Init();

}

void AlarmsPage::Init() {

	// General Alarms

	generalAlarmsSizer = new wxBoxSizer(wxVERTICAL);

	generalAlarmsTitle = new wxStaticText(this, wxID_ANY, _(GENERAL_ALARMS_STR));
	generalAlarmsTitle->SetFont(FONT_MEDIUM_SEMIBOLD);
	generalAlarmsSizer->Add(generalAlarmsTitle, 0, wxALIGN_CENTER_HORIZONTAL | wxALL, 5);

	for (Alarm alarm : GeneralAlarms.getAll()) {
		AddGeneralAlarmCheckBox(alarm);
	}

	sizer->Add(generalAlarmsSizer, 0, wxALL, 10);


	// Soft Faults

	softFaultsSizer = new wxBoxSizer(wxVERTICAL);

	softFaultsTitle = new wxStaticText(this, wxID_ANY, _(GENERAL_ALARMS_STR));
	softFaultsTitle->SetFont(FONT_MEDIUM_SEMIBOLD);
	softFaultsSizer->Add(softFaultsTitle, 0, wxALIGN_CENTER_HORIZONTAL | wxALL, 5);

	for (Alarm alarm : SoftFaults.getAll()) {
		AddSoftFaultCheckBox(alarm);
	}

	sizer->Add(softFaultsSizer, 0, wxALL, 10);


	RefreshStrings();
	RefreshAll();

	SetSizer(sizer);
	Layout();
	sizer->Fit(this);

}

void AlarmsPage::RefreshAll() {
	for (auto& [alarmID, checkbox] : mapIDToAlarmCheckBox)
		checkbox->SetValue(lc->AlarmEnabled(mapIDToAlarm[alarmID]));

	RefreshVisibility();
}

void AlarmsPage::RefreshStrings() {

	SetName(_(ALARMS_STR));

	SetText(generalAlarmsTitle, _(GENERAL_ALARMS_STR));
	SetText(softFaultsTitle, _(SOFT_FAULTS_STR));

	for (auto& [alarmID, checkbox] : mapIDToAlarmCheckBox) {
		Alarm alarm = mapIDToAlarm[alarmID];
		if (contains(GeneralAlarms.getAll(), alarm))
			SetText(checkbox, _(GeneralAlarms[alarm]));
		else if (contains(SoftFaults.getAll(), alarm))
			SetText(checkbox, _(SoftFaults[alarm]));
	}
}

void AlarmsPage::RefreshVisibility() {
	bool showHFSyncDisabledWarning = lc->IsType({ "RX", "FX" }) and !lc->AlarmEnabled(Alarm::HF_SYNC);
	SetVisibilityBasedOnCondition(hfSyncWarningIcon, showHFSyncDisabledWarning);
}

void AlarmsPage::RefreshControlEnabled() {
	for (auto& [alarmID, checkbox] : mapIDToAlarmCheckBox) {
		bool IsWetOrHumidityAlarm = contains({ Alarm::WET_SENSOR, Alarm::HUMIDITY }, mapIDToAlarm[alarmID]);
		if (IsWetOrHumidityAlarm and !(IsInAccessMode(GuiAccessMode::FACTORY) or ThisMACHasProductionAccess()))
			checkbox->Disable();
		else
			checkbox->Enable();
	}
}

void AlarmsPage::AddGeneralAlarmCheckBox(Alarm alarm) {

	wxWindowID alarmID = wxNewId();
	mapIDToAlarm[alarmID] = alarm;

	wxCheckBox* generalAlarmCheckBox = new wxCheckBox(this, alarmID, _(ACTUAL_CURRENT_STR));
	generalAlarmCheckBox->SetFont(FONT_EXTRA_SMALL_SEMIBOLD);
	generalAlarmCheckBox->Bind(wxEVT_CHECKBOX, &AlarmsPage::OnAlarmChecked, this, alarmID);

	if (alarm == Alarm::HF_SYNC) {
		wxBoxSizer* hfSyncSizer = new wxBoxSizer(wxHORIZONTAL);
		hfSyncSizer->Add(generalAlarmCheckBox, 0, wxALIGN_CENTER_VERTICAL, 0);
		hfSyncWarningIcon = new wxStaticBitmap(this, wxID_ANY, wxBitmap(WARNING_ICON_EXTRA_SMALL, wxBITMAP_TYPE_ANY));
		hfSyncWarningIcon->SetToolTip(_("WARNING: HF Sync alarm is disabled."));
		hfSyncSizer->Add(hfSyncWarningIcon, 0, wxALIGN_CENTER_VERTICAL, 0);
		hfSyncWarningIcon->Hide();
		generalAlarmsSizer->Add(hfSyncSizer, 0, wxLEFT | wxTOP, 5);
	}
	else {
		generalAlarmsSizer->Add(generalAlarmCheckBox, 0, wxLEFT | wxTOP, 5);
	}

	mapIDToAlarmCheckBox[alarmID] = generalAlarmCheckBox;

}

void AlarmsPage::AddSoftFaultCheckBox(Alarm alarm) {

	wxWindowID alarmID = wxNewId();
	mapIDToAlarm[alarmID] = alarm;

	wxCheckBox* softFaultCheckBox = new wxCheckBox(this, alarmID, _(ACTUAL_CURRENT_STR));
	softFaultCheckBox->SetFont(FONT_EXTRA_SMALL_SEMIBOLD);
	softFaultCheckBox->Bind(wxEVT_CHECKBOX, &AlarmsPage::OnAlarmChecked, this, alarmID);
	softFaultsSizer->Add(softFaultCheckBox, 0, wxLEFT | wxTOP, 5);
	mapIDToAlarmCheckBox[alarmID] = softFaultCheckBox;

}

void AlarmsPage::OnAlarmChecked(wxCommandEvent& evt) {
	wxWindowID alarmID = evt.GetId();
	Alarm alarm = mapIDToAlarm[alarmID];

	if (lc->AlarmEnabled(alarm)) {
		if (alarm == Alarm::HF_SYNC) {
			wxMessageDialog dialog(nullptr,
				"WARNING\n"
				"\n"
				"Are you sure you want to disable HF Sync alarm?\n"
				"This can lead to issues if not properly handled.",
				"Disable HF Sync Alarm",
				wxYES_NO | wxNO_DEFAULT
			);
			if (dialog.ShowModal() == wxID_NO)
				return;
		}
		lc->SetAlarm(alarm, false);
	}
	else
		lc->SetAlarm(alarm, true);
}
