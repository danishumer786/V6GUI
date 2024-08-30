#include "SensorPanel_Humidity.h"
#include "../CommonFunctions_GUI.h"

using namespace std;

const wxString CALIBRATION_STR = _("Calibration");
const wxString ALARM_LIMIT_STR = _("Alarm Limit");


SensorPanel_Humidity::SensorPanel_Humidity(std::shared_ptr<MainLaserControllerInterface> _lc, int _id, wxWindow* parent
) :
	lc(_lc),
	id(_id),
	wxPanel(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_THEME) {

	SetBackgroundColour(FOREGROUND_PANEL_COLOR);

	wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);


	title = new wxStaticText(this, wxID_ANY, wxEmptyString);
	sizer->Add(title, 0, wxTOP | wxALIGN_CENTER_HORIZONTAL, 5);

	humidityReadout = new FloatReadoutSimple(
		lc, this, id, "", "%",
		[this]() { return lc->GetHumidityReading(id); },
		1, -1, -1, 0
	);
	sizer->Add(humidityReadout, 0, wxALL | wxALIGN_CENTER_HORIZONTAL, 3);

	calibrationSpin = new FloatSettingSpinSimple(
		lc, this, id, _(CALIBRATION_STR), "",
		[this](int calibration) {lc->SetHumidityCalibration(id, calibration); },
		[this]() {return lc->GetHumidityCalibration(id); },
		1, 64, -1, 1.0
	);
	sizer->Add(calibrationSpin, 0, wxRIGHT | wxTOP, 3);

	alarmLimitSpin = new FloatSettingSpinSimple(
		lc, this, id, _(ALARM_LIMIT_STR), "",
		[this](float scale) {lc->SetHumidityAlarmLimit(id, scale); },
		[this]() {return lc->GetHumidityAlarmLimit(id); },
		1, 64, -1, 1.0
	);
	sizer->Add(alarmLimitSpin, 0, wxRIGHT | wxTOP | wxBOTTOM, 3);


	SetSizer(sizer);
	Layout();
	sizer->Fit(this);

}


void SensorPanel_Humidity::RefreshAll() {
	humidityReadout->RefreshAll();
	calibrationSpin->RefreshAll();
	alarmLimitSpin->RefreshAll();
}

void SensorPanel_Humidity::RefreshStrings() {
	title->SetLabelText(lc->GetHumidityLabel(id));
	humidityReadout->RefreshStrings();
	calibrationSpin->RefreshStrings();
	alarmLimitSpin->RefreshStrings();
}

void SensorPanel_Humidity::RefreshVisibility() {
	SetVisibilityBasedOnAccessMode(this, GuiAccessMode::SERVICE);
}
