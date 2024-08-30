#include "SensorPanel_Chiller.h"
#include "../CommonFunctions_GUI.h"

using namespace std;

const wxString CALIBRATION_STR = _("Calibration");
const wxString ALARM_LOW_LIMIT_STR = _("Alarm Low Limit");
const wxString ALARM_HIGH_LIMIT_STR = _("Alarm High Limit");


SensorPanel_Chiller::SensorPanel_Chiller(std::shared_ptr<MainLaserControllerInterface> _lc, int _id, wxWindow* parent
) :
	lc(_lc),
	id(_id),
	wxPanel(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_THEME) {

	SetBackgroundColour(FOREGROUND_PANEL_COLOR);

	wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);

	// Only need to use if more than 1 chiller (currently unused):
	/*title = new wxStaticText(this, wxID_ANY, wxEmptyString);
	sizer->Add(title, 0, wxTOP | wxALIGN_CENTER_HORIZONTAL, 5);*/

	flowReadout = new FloatReadoutSimple(
		lc, this, id, "", "L/m",
		[this]() { return lc->GetChillerFlowReading(id); },
		1
	);
	sizer->Add(flowReadout, 0, wxALL | wxALIGN_CENTER_HORIZONTAL, 3);

	calibrationSpin = new FloatSettingSpinSimple(
		lc, this, id, _(CALIBRATION_STR), "pulses/L",
		[this](float calibration) { lc->SetFlowCalibration(calibration, id); },
		[this]() { return lc->GetFlowCalibration(id); },
		1, 88, -1, 1.0
	);
	sizer->Add(calibrationSpin, 0, wxRIGHT | wxTOP, 3);

	alarmLowLimitSpin = new FloatSettingSpinSimple(
		lc, this, id, _(ALARM_LOW_LIMIT_STR), "L/m",
		[this](float limit) { lc->SetFlowLowLimit(limit, id); },
		[this]() { return lc->GetFlowLowLimit(id); },
		1, 88, -1, 0.1
	);
	sizer->Add(alarmLowLimitSpin, 0, wxRIGHT | wxTOP, 3);

	alarmHighLimitSpin = new FloatSettingSpinSimple(
		lc, this, id, _(ALARM_HIGH_LIMIT_STR), "L/m",
		[this](float limit) { lc->SetFlowHighLimit(limit, id); },
		[this]() { return lc->GetFlowHighLimit(id); },
		1, 88, -1, 0.1
	);
	sizer->Add(alarmHighLimitSpin, 0, wxRIGHT | wxTOP | wxBOTTOM, 3);


	SetSizer(sizer);
	Layout();
	sizer->Fit(this);

}


void SensorPanel_Chiller::RefreshAll() {
	flowReadout->RefreshAll();
	calibrationSpin->RefreshAll();
	alarmLowLimitSpin->RefreshAll();
	alarmHighLimitSpin->RefreshAll();
}

void SensorPanel_Chiller::RefreshStrings() {
	// Only need to use if more than 1 chiller (currently unused):
	//title->SetLabelText(_("Flow ") + to_wx_string(id));
	flowReadout->RefreshStrings();
	calibrationSpin->RefreshStrings();
	alarmLowLimitSpin->RefreshStrings();
	alarmHighLimitSpin->RefreshStrings();
}

void SensorPanel_Chiller::RefreshVisibility() {
	SetVisibilityBasedOnAccessMode(this, GuiAccessMode::SERVICE);
}
