#pragma once

#include "SettingsPage_Base.h"
#include "DiodeSettingsPanel.h"


class DiodeSettingsPage : public SettingsPage_Base {

public:
	DiodeSettingsPage(std::shared_ptr<MainLaserControllerInterface> _lc, wxWindow* parent);

	void Init();
	void RefreshAll();
	void RefreshStrings();
	void RefreshVisibility();
	void RefreshControlEnabled();

private:
	wxBoxSizer* sizer;
	wxBoxSizer* lddPanelsSizer;

	wxVector<DiodeSettingsPanel*> diodePanels;

};

