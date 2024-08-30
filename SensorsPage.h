#pragma once

#include "SettingsPage_Base.h"
#include "SensorPanel_PowerMonitor.h"
#include "SensorPanel_Chiller.h"
#include "SensorPanel_Humidity.h"


class SensorsPage : public SettingsPage_Base {

public:
	SensorsPage(std::shared_ptr<MainLaserControllerInterface> _lc, wxWindow* parent);

	void Init();
	void RefreshAll();
	void RefreshStrings();
	void RefreshVisibility();
	void RefreshControlEnabled();

private:
	wxBoxSizer* sizer;

	wxBitmapButton* getSensorsAccessKeyButton;

	wxPanel* powerMonitorsPanel;
	wxBoxSizer* powerMonitorsSizer;
	wxStaticText* powerMonitorsTitle;
	wxVector<SensorPanel_PowerMonitor*> powerMonitorPanels;

	wxPanel* chillersPanel;
	wxBoxSizer* chillersSizer;
	wxStaticText* chillerTitle;
	wxVector<SensorPanel_Chiller*> chillerPanels; // If ever need to use more than 1 chiller, change this to vector

	wxPanel* humidityPanel;
	wxBoxSizer* humiditySizer;
	wxStaticText* humidityTitle;
	wxVector<SensorPanel_Humidity*> humidityPanels;

	wxBoxSizer* wetSensorSizer;
	wxStaticText* wetSensorTitle;

	void AddPowerMonitorSettingsPanel(int pm_id);
	void AddChillerSettingsPanel(int chiller_id);
	void AddHumiditySettingsPanel(int humidity_id);

	void OnGetSensorsAccessKeyButtonClicked(wxCommandEvent& evt);

};

