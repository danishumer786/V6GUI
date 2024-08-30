#pragma once

#include "wx/wx.h"
#include "wx/grid.h"

#include "MainLaserControllerInterface.h"
#include "../CommonGUIComponents/SliderGaugeSpin.h"
#include "../CommonGUIComponents/FloatReadoutWithSettable.h"
#include "../CommonGUIComponents/FloatReadoutSimple.h"
#include "../CommonGUIComponents/FloatSettingSpinSimple.h"


class DiodeSettingsPanel : public wxPanel {

public:
	DiodeSettingsPanel(std::shared_ptr<MainLaserControllerInterface> _lc, int id, wxWindow* parent);

	void Init();
	void RefreshAll();
	void RefreshStrings();
	void RefreshVisibility();
	void RefreshControlEnabled();


protected:
	std::shared_ptr<MainLaserControllerInterface> lc;
	int id; // Diode ID

	wxBoxSizer* sizer;

	wxStaticText* title;

	wxButton* enableButton;
	//wxButton* saveSetCurrentAsDefaultButton;
	SliderGaugeSpin* sliderGaugeSpin;
	FloatReadoutWithSettable* actualCurrentReadoutWithMaxSpinCtrl;
	FloatReadoutSimple* voltageReadout;

	wxStaticText* calibrationSectionTitle;
	FloatSettingSpinSimple* maxSetCurrentSettingSpin;
	FloatSettingSpinSimple* maxMeasuredCurrentSettingSpin;
	FloatSettingSpinSimple* maxMeasuredVoltageSettingSpin;
	FloatSettingSpinSimple* minMeasuredCurrentSettingSpin;
	FloatSettingSpinSimple* minSetCurrentSettingSpin;
	FloatSettingSpinSimple* minDiodeVoltageSettingSpin;
	FloatSettingSpinSimple* lddVoltageCalibrationSettingSpin;

	wxStaticText* shutterLimitsSectionTitle;
	FloatSettingSpinSimple* shutterOpenHighLimitSettingSpin;
	FloatSettingSpinSimple* shutterOpenLowLimitSettingSpin;

	wxStaticText* otherSettingsSectionTitle;
	FloatSettingSpinSimple* hfCurrentLimitSettingSpin;

	wxStaticText* prfCurrentLimitsTitle;
	wxGrid* prfCurrentLimitsTable;


	void RefreshEnableButtonState();

	void OnEnableButtonClicked(wxCommandEvent& evt);

	void OnPRFCurrentLimitsTableEntry(wxGridEvent& evt);

};

