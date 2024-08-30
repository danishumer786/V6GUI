#pragma once

#include "SettingsPage_Base.h"
#include "TemperatureControlPanel.h"


class TemperatureSettingsPage : public SettingsPage_Base {

public:
	TemperatureSettingsPage(std::shared_ptr<MainLaserControllerInterface> _lc, wxWindow* parent);

	void Init();
	void RefreshAll();
	void RefreshStrings();
	void RefreshVisibility();
	void RefreshControlEnabled();

private:
	wxBoxSizer* sizer;
	wxBoxSizer* temperaturePanelsSizer;

	wxBitmapButton* getTemperatureAccessKeyButton;

	wxVector<TemperatureControlPanel*> temperaturePanels;

	void OnGetTemperatureAccessKeyButtonClicked(wxCommandEvent& evt);

};

