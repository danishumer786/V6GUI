#pragma once

#include "wx/wx.h"
#include "MainLaserControllerInterface.h"
#include "../CommonGUIComponents/FloatReadoutSimple.h"
#include "../CommonGUIComponents/FloatSettingSpinSimple.h"


class SensorPanel_PowerMonitor : public wxPanel {

public:
	SensorPanel_PowerMonitor(std::shared_ptr<MainLaserControllerInterface> _lc, int _id, wxWindow* parent);

	void RefreshAll();
	void RefreshStrings();
	void RefreshVisibility();

private:
	std::shared_ptr<MainLaserControllerInterface> lc;
	int id;

	wxStaticText* title;
	FloatReadoutSimple* powerReadout;
	FloatSettingSpinSimple* zeroSpin;
	FloatSettingSpinSimple* scaleSpin;

};

