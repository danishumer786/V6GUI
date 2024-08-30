#pragma once

#include "wx/wx.h"
#include "MainLaserControllerInterface.h"
#include "../CommonGUIComponents/FloatReadoutSimple.h"
#include "../CommonGUIComponents/FloatSettingSpinSimple.h"


class SensorPanel_Chiller : public wxPanel {

public:
	SensorPanel_Chiller(std::shared_ptr<MainLaserControllerInterface> _lc, int _id, wxWindow* parent);

	void RefreshAll();
	void RefreshStrings();
	void RefreshVisibility();

private:
	std::shared_ptr<MainLaserControllerInterface> lc;
	int id;

	//wxStaticText* title; // <-- Only need to use if more than 1 chiller (currently unused)
	FloatReadoutSimple* flowReadout;
	FloatSettingSpinSimple* calibrationSpin;
	FloatSettingSpinSimple* alarmLowLimitSpin;
	FloatSettingSpinSimple* alarmHighLimitSpin;

};
