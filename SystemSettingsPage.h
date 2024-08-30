#pragma once

#include "TamperManager.h"
#include "SettingsPage_Base.h"


class SystemSettingsPage : public SettingsPage_Base {

public:
	SystemSettingsPage(std::shared_ptr<MainLaserControllerInterface> _lc, std::shared_ptr<TamperManager> _tm, wxWindow* parent);

	void Init();
	void RefreshAll();
	void RefreshStrings();
	void RefreshVisibility();

protected:
	std::shared_ptr<TamperManager> tm;

	wxCheckBox* enableTamperCheckbox;
	wxCheckBox* qswControlCheckbox;

	void OnEnableTamperChecked(wxCommandEvent& evt);
	void OnQSWControlChecked(wxCommandEvent& evt);
};

