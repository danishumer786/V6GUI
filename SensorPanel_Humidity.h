#pragma once
#include "wx/wx.h"
#include "MainLaserControllerInterface.h"
#include "../CommonGUIComponents/FloatReadoutSimple.h"
#include "../CommonGUIComponents/FloatSettingSpinSimple.h"


class SensorPanel_Humidity : public wxPanel {

public:
	SensorPanel_Humidity(std::shared_ptr<MainLaserControllerInterface> _lc, int _id, wxWindow* parent);

	void RefreshAll();
	void RefreshStrings();
	void RefreshVisibility();

private:
	std::shared_ptr<MainLaserControllerInterface> lc;
	int id;

	wxStaticText* title;
	FloatReadoutSimple* humidityReadout;
	FloatSettingSpinSimple* calibrationSpin;
	FloatSettingSpinSimple* alarmLimitSpin;

};

