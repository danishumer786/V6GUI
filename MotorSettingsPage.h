#pragma once

#include "SettingsPage_Base.h"
#include "MotorControlPanel.h"
#include "MotorControlPanelLinked.h"
#include "MotorSequencerPanel.h"


class MotorSettingsPage : public SettingsPage_Base {

public:
	MotorSettingsPage(std::shared_ptr<MainLaserControllerInterface> _lc, wxWindow* parent);

	//bool ShouldBeVisibleToUser();

	void Init();
	void RefreshAll();
	void RefreshStrings();
	void RefreshVisibility();
	void RefreshControlEnabled();

	void RefreshSlidersPosition();

private:
	wxBoxSizer* sizer;

	wxBitmapButton* getMotorAccessKeyButton;

	wxTextCtrl* boxForKeyboardMotorControl;

	wxBoxSizer* motorPanelsSizer;
	wxBoxSizer* linkedMotorPanelsSizer;

	MotorSequencerPanel* motorSequencerPanel;

	wxVector<MotorControlPanel*> motorPanels;
	wxVector<MotorControlPanelLinked*> linkedMotorPanels;

	void OnGetMotorAccessKeyButtonClicked(wxCommandEvent& evt);
	void OnKeyboardChoiceSelected(wxCommandEvent& evt);
	void OnKeyDown(wxKeyEvent& evt);
	void OnKeyUp(wxKeyEvent& evt);

};

