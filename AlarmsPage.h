#pragma once

#include <map>

#include "SettingsPage_Base.h"


class AlarmsPage : public SettingsPage_Base {

public:
	AlarmsPage(std::shared_ptr<MainLaserControllerInterface> _lc, wxWindow* parent);

	void Init();
	void RefreshAll();
	void RefreshStrings();
	void RefreshVisibility();
	void RefreshControlEnabled();

private:
	wxBoxSizer* sizer;

	wxBoxSizer* generalAlarmsSizer;
	wxBoxSizer* softFaultsSizer;

	std::map<wxWindowID, Alarm> mapIDToAlarm;
	std::map<wxWindowID, wxCheckBox*> mapIDToAlarmCheckBox;

	wxStaticBitmap* hfSyncWarningIcon;

	wxStaticText* generalAlarmsTitle;
	wxStaticText* softFaultsTitle;

	void AddGeneralAlarmCheckBox(Alarm alarm);
	void AddSoftFaultCheckBox(Alarm alarm);

	void OnAlarmChecked(wxCommandEvent& evt);

};
