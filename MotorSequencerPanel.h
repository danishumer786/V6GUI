#pragma once

#include <map>

#include "wx/wx.h"

#include "../CommonGUIComponents/FeatureTitle.h"
#include "../CommonGUIComponents/DynamicStatusMessage.h"
#include "../CommonGUIComponents/NumericTextCtrl.h"
#include "MainLaserControllerInterface.h"
#include "LaserControlProcedures/MotorMovementSequencer/MotorMovementSequencerManager.h"


class MotorSequencerPanel : public wxPanel {

public:
	MotorSequencerPanel(std::shared_ptr<MainLaserControllerInterface> laser_controller, wxWindow* parent);

	void RefreshAll();
	void RefreshVisibilityBasedOnAccessMode();
	void RefreshStrings();


private:
	std::shared_ptr<MainLaserControllerInterface> lc;
	std::shared_ptr<MotorMovementSequencerManager> sequencer;

	std::map<int, wxCheckBox*> mapIDToCheckbox;

	FeatureTitle* title;
	wxStaticBoxSizer* includeSizerLabelled;
	wxFlexGridSizer* includeSizer;
	wxStaticText* distanceLabel;
	NumericTextCtrl* distanceTextCtrl;
	wxButton* startButton;
	DynamicStatusMessage* message;

	void InitMotorIncludeCheckboxes();
	void RefreshStartButtonState();
	void RefreshMessage();

	void OnStartButtonClicked(wxCommandEvent& evt);

};

