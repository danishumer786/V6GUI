#include "SensorPanel_PowerMonitor.h"
#include "../CommonFunctions_GUI.h"

using namespace std;

const wxString ZERO_STR = _("Zero");
const wxString SCALE_STR = _("Scale");


SensorPanel_PowerMonitor::SensorPanel_PowerMonitor(std::shared_ptr<MainLaserControllerInterface> _lc, int _id, wxWindow* parent
) :
	lc(_lc),
	id(_id),
	wxPanel(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_THEME)
{

	SetBackgroundColour(FOREGROUND_PANEL_COLOR);

	wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);


	title = new wxStaticText(this, wxID_ANY, wxEmptyString);
	sizer->Add(title, 0, wxTOP | wxALIGN_CENTER_HORIZONTAL, 5);

	powerReadout = new FloatReadoutSimple(
		lc, this, id, "", "W",
		[this]() { return lc->GetPowerMonitorReadingInWatts(id); },
		1
	);
	sizer->Add(powerReadout, 0, wxALL | wxALIGN_CENTER_HORIZONTAL, 3);

	zeroSpin = new FloatSettingSpinSimple(
		lc, this, id, _(ZERO_STR), "",
		[this](float zero) {lc->SetPowerMonitorZeroLevel(id, zero); },
		[this]() {return lc->GetPowerMonitorZeroLevel(id); },
		1, 30, -1, 1.0
	);
	sizer->Add(zeroSpin, 0, wxRIGHT | wxTOP, 3);

	scaleSpin = new FloatSettingSpinSimple(
		lc, this, id, _(SCALE_STR), "",
		[this](float scale) {lc->SetPowerMonitorScale(id, scale); },
		[this]() {return lc->GetPowerMonitorScale(id); },
		1, 30
	);
	sizer->Add(scaleSpin, 0, wxRIGHT | wxTOP | wxBOTTOM, 3);


	SetSizer(sizer);
	Layout();
	sizer->Fit(this);

}


void SensorPanel_PowerMonitor::RefreshAll() {
	powerReadout->RefreshAll();
	zeroSpin->RefreshAll();
	scaleSpin->RefreshAll();
}

void SensorPanel_PowerMonitor::RefreshStrings() {
	title->SetLabelText(lc->GetPowerMonitorLabel(id));
	powerReadout->RefreshStrings();
	zeroSpin->RefreshStrings();
	scaleSpin->RefreshStrings();
}

void SensorPanel_PowerMonitor::RefreshVisibility() {
	// 1-24-24 - Basic power monitor calibration should always be visible to user.
	//SetVisibilityBasedOnAccessMode(this, GuiAccessMode::SERVICE);
}
