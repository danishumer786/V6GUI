#include "DiodeSettingsPanel.h"
#include "../Resources.h"
#include "../CommonFunctions_GUI.h"

using namespace std;

const wxString SAVE_STR = _("Save");
const wxString SET_STR = _("Set");
const wxString CURRENT_STR = _("Current");
const wxString VOLTAGE_STR = _("Voltage");
const wxString CALIBRATION_STR = _("Calibration");
const wxString SHUTTER_LIMITS_STR = _("Shutter Limits");
const wxString OTHER_SETTINGS_STR = _("Other Settings");
const wxString PRF_CURRENT_LIMITS_STR = _("PRF Current Limits");


const int SETTING_LABEL_WIDTH = 110;
const int SETTING_PADDING = 2;
const long SETTING_STYLE = wxALL | wxALIGN_CENTER_HORIZONTAL;


DiodeSettingsPanel::DiodeSettingsPanel(
	std::shared_ptr<MainLaserControllerInterface> _lc,
	int _id,
	wxWindow* parent
) :
	id(_id),
	lc(_lc),
	wxPanel(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_THEME)
{
	SetBackgroundColour(FOREGROUND_PANEL_COLOR);
	sizer = new wxBoxSizer(wxVERTICAL);
	Init();
}


void DiodeSettingsPanel::Init() {

	// Title
	title = new wxStaticText(this, wxID_ANY, wxEmptyString);
	title->SetFont(FONT_SMALL_BOLD);
	sizer->Add(title, 0, wxALL | wxALIGN_CENTER_HORIZONTAL, 3);

	// Enable/Disable diode button
	enableButton = new wxButton(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
	enableButton->Bind(wxEVT_BUTTON, &DiodeSettingsPanel::OnEnableButtonClicked, this);
	enableButton->SetFont(FONT_VERY_SMALL_BOLD);
	sizer->Add(enableButton, 0, wxALIGN_CENTER_HORIZONTAL, 3);

	// TODO: Add saveSetCurrentAsDefaultButton somewhere?

	// Set current slider-gauge-spin
	sliderGaugeSpin = new SliderGaugeSpin(
		lc,
		this,
		"LDD" + to_string(id),
		_(SET_STR),
		"A",
		0.0,
		0.1,
		[this]() { return lc->GetLDDMaxCurrent(id); },
		[this](float newCurrent) { lc->SetLDDSetCurrent(id, newCurrent); },
		[this]() { return lc->GetLDDSetCurrent(id); },
		[this]() { return lc->GetLDDActualCurrent(id); }
	);
	sizer->Add(sliderGaugeSpin, 0, wxALL | wxALIGN_CENTER_HORIZONTAL, 2);


	wxStaticText* maxString = new wxStaticText(this, wxID_ANY, _("Max"),
		wxDefaultPosition, wxSize(120, -1), wxALIGN_RIGHT);
	maxString->SetFont(FONT_VERY_SMALL_SEMIBOLD);
	sizer->Add(maxString, 0,  wxALIGN_CENTER_HORIZONTAL, 2);

	// Actual current readout with max spin ctrl
	actualCurrentReadoutWithMaxSpinCtrl = new FloatReadoutWithSettable(
		lc,
		this,
		id,
		"LDD" + to_string(id) + " max current",
		_(CURRENT_STR),
		"A",
		0.0,
		99.9,
		0.01,
		[this](float newCurrent) { lc->SetLDDMaxCurrent(id, newCurrent); },
		[this]() { return lc->GetLDDMaxCurrent(id); },
		[this]() { return lc->GetLDDActualCurrent(id); },
		1
	);
	sizer->Add(actualCurrentReadoutWithMaxSpinCtrl, 0, SETTING_STYLE, 0);

	voltageReadout = new FloatReadoutSimple(
		lc,
		this,
		id,
		_(VOLTAGE_STR),
		wxT("V"),
		[this]() { return lc->GetLDDVoltage(id); },
		1,
		-1,
		68
	);
	sizer->Add(voltageReadout, 0, SETTING_STYLE, SETTING_PADDING);



	//-------------------------------------------------------------------------
	// Calibration Settings

	calibrationSectionTitle = new wxStaticText(this, wxID_ANY, _(CALIBRATION_STR));
	sizer->Add(calibrationSectionTitle, 0, wxTOP | wxALIGN_CENTER_HORIZONTAL, 10);


	maxSetCurrentSettingSpin = new FloatSettingSpinSimple(
		lc,
		this,
		id,
		_("Max Set Current"),
		"A",
		[this](float value) { lc->SetCalibrationScale(id, value); },
		[this]() { return lc->GetCalibrationScale(id); },
		1,
		SETTING_LABEL_WIDTH,
		SETTING_PADDING
	);
	sizer->Add(maxSetCurrentSettingSpin, 0, SETTING_STYLE, SETTING_PADDING);

	maxMeasuredCurrentSettingSpin = new FloatSettingSpinSimple(
		lc,
		this,
		id,
		_("Max Meas. Current"),
		"A",
		[this](float value) { lc->SetMeasurementScale(id, value); },
		[this]() { return lc->GetMeasurementScale(id); },
		1,
		SETTING_LABEL_WIDTH,
		SETTING_PADDING
	);
	sizer->Add(maxMeasuredCurrentSettingSpin, 0, SETTING_STYLE, SETTING_PADDING);

	maxMeasuredVoltageSettingSpin = new FloatSettingSpinSimple(
		lc,
		this,
		id,
		_("Max Meas. Voltage"),
		"V",
		[this](float value) { lc->SetVoltageScale(id, value); },
		[this]() { return lc->GetVoltageScale(id); },
		1,
		SETTING_LABEL_WIDTH,
		SETTING_PADDING
	);
	sizer->Add(maxMeasuredVoltageSettingSpin, 0, SETTING_STYLE, SETTING_PADDING);

	minMeasuredCurrentSettingSpin = new FloatSettingSpinSimple(
		lc,
		this,
		id,
		_("Min Meas. Current"),
		"A",
		[this](float value) { lc->SetActualCurrentCompensation(id, value); },
		[this]() { return lc->GetActualCurrentCompensation(id); },
		1,
		SETTING_LABEL_WIDTH,
		SETTING_PADDING
	);
	sizer->Add(minMeasuredCurrentSettingSpin, 0, SETTING_STYLE, SETTING_PADDING);

	minSetCurrentSettingSpin = new FloatSettingSpinSimple(
		lc,
		this,
		id,
		_("Min Set Current"),
		"A",
		[this](float value) { lc->SetMinimumVoltageCalibration(id, value); },
		[this]() { return lc->GetMinimumVoltageCalibration(id); },
		1,
		SETTING_LABEL_WIDTH,
		SETTING_PADDING
	);
	sizer->Add(minSetCurrentSettingSpin, 0, SETTING_STYLE, SETTING_PADDING);

	minDiodeVoltageSettingSpin = new FloatSettingSpinSimple(
		lc,
		this,
		id,
		_("Max Meas. Voltage"),
		"V",
		[this](float value) { lc->SetVoltageBaseline(id, value); },
		[this]() { return lc->GetVoltageBaseline(id); },
		1,
		SETTING_LABEL_WIDTH,
		SETTING_PADDING
	);
	sizer->Add(minDiodeVoltageSettingSpin, 0, SETTING_STYLE, SETTING_PADDING);

	lddVoltageCalibrationSettingSpin = new FloatSettingSpinSimple(
		lc,
		this,
		id,
		_("Voltage Calibration"),
		"V",
		[this](float value) { lc->SetVoltageCalibration(id, value); },
		[this]() { return lc->GetVoltageCalibration(id); },
		1,
		SETTING_LABEL_WIDTH,
		SETTING_PADDING
	);
	sizer->Add(lddVoltageCalibrationSettingSpin, 0, SETTING_STYLE, SETTING_PADDING);


	if (lc->ShutterIsEnabledForUse(id)) {

		shutterLimitsSectionTitle = new wxStaticText(this, wxID_ANY, _(SHUTTER_LIMITS_STR));
		sizer->Add(shutterLimitsSectionTitle, 0, wxTOP | wxALIGN_CENTER_HORIZONTAL, 10);

		shutterOpenHighLimitSettingSpin = new FloatSettingSpinSimple(
			lc,
			this,
			id,
			_("High Limit"),
			"A",
			[this](float value) { lc->SetOpenShutterCurrentHighLimit(id, value); },
			[this]() { return lc->GetOpenShutterCurrentHighLimit(id); },
			1,
			SETTING_LABEL_WIDTH,
			SETTING_PADDING
		);
		sizer->Add(shutterOpenHighLimitSettingSpin, 0, SETTING_STYLE, SETTING_PADDING);

		shutterOpenLowLimitSettingSpin = new FloatSettingSpinSimple(
			lc,
			this,
			id,
			_("Low Limit"),
			"A",
			[this](float value) { lc->SetOpenShutterCurrentLowLimit(id, value); },
			[this]() { return lc->GetOpenShutterCurrentLowLimit(id); },
			1,
			SETTING_LABEL_WIDTH,
			SETTING_PADDING
		);
		sizer->Add(shutterOpenLowLimitSettingSpin, 0, SETTING_STYLE, SETTING_PADDING);
	}



	//-------------------------------------------------------------------------
	// Other Settings


	if (lc->IsType({ "RX", "FX" })) {

		otherSettingsSectionTitle = new wxStaticText(this, wxID_ANY, _(OTHER_SETTINGS_STR));
		sizer->Add(otherSettingsSectionTitle, 0, wxTOP | wxALIGN_CENTER_HORIZONTAL, 10);


		hfCurrentLimitSettingSpin = new FloatSettingSpinSimple(
			lc,
			this,
			id,
			_("HF Current Limit"),
			"A",
			[this](float value) { lc->SetLDDHFCurrentLimit(id, value); },
			[this]() { return lc->GetLDDHFCurrentLimit(id); },
			1,
			SETTING_LABEL_WIDTH,
			SETTING_PADDING,
			0.1
		);
		if (compareVersions(lc->GetFirmwareVersion(), "3.0.1ER453") < 0)
			hfCurrentLimitSettingSpin->SetToolTip(_("Requires firmware 3.0.1ER453"));
		sizer->Add(hfCurrentLimitSettingSpin, 0, SETTING_STYLE, SETTING_PADDING);

	}



	//-------------------------------------------------------------------------
	// PRF Current Limits Table

	if (lc->IsLDDPRFCurrentLimitEnabled(id)) {

		prfCurrentLimitsTitle = new wxStaticText(this, wxID_ANY, _(PRF_CURRENT_LIMITS_STR));
		sizer->Add(prfCurrentLimitsTitle, 0, wxTOP | wxALIGN_CENTER_HORIZONTAL, 10);

		//prfCurrentLimitsTable = new wxGrid(this, wxID_ANY);
		prfCurrentLimitsTable = new wxGrid(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxWANTS_CHARS | wxBORDER_THEME);
		prfCurrentLimitsTable->CreateGrid(5, 2);
		prfCurrentLimitsTable->SetScrollRate(0, 0);
		prfCurrentLimitsTable->Bind(wxEVT_GRID_CELL_CHANGED, &DiodeSettingsPanel::OnPRFCurrentLimitsTableEntry, this);
		prfCurrentLimitsTable->SetCellHighlightColour(PHOTONICS_TURQUOISE_COLOR);
		prfCurrentLimitsTable->SetCellHighlightPenWidth(2);

		// Make cell values integers
		prfCurrentLimitsTable->SetColFormatNumber(0);
		prfCurrentLimitsTable->SetColFormatFloat(1, -1, 2);

		// Alignment
		prfCurrentLimitsTable->SetColLabelAlignment(wxALIGN_CENTER, wxALIGN_CENTER);
		prfCurrentLimitsTable->SetDefaultCellAlignment(wxALIGN_LEFT, wxALIGN_TOP);

		// Font
		prfCurrentLimitsTable->SetDefaultCellFont(FONT_EXTRA_SMALL_SEMIBOLD);
		prfCurrentLimitsTable->SetLabelFont(FONT_EXTRA_SMALL_BOLD);

		// Sizes
		prfCurrentLimitsTable->HideRowLabels();
		prfCurrentLimitsTable->SetColSize(0, 70);
		prfCurrentLimitsTable->SetColSize(1, 70);
		prfCurrentLimitsTable->DisableColResize(0);
		prfCurrentLimitsTable->DisableColResize(1);
		prfCurrentLimitsTable->DisableDragGridSize();
		prfCurrentLimitsTable->DisableDragColSize();
		prfCurrentLimitsTable->DisableDragRowSize();

		for (int row = 0; row < prfCurrentLimitsTable->GetNumberRows(); row++) {
			prfCurrentLimitsTable->SetCellAlignment(row, 0, wxALIGN_LEFT, wxALIGN_TOP);
			prfCurrentLimitsTable->SetCellAlignment(row, 1, wxALIGN_LEFT, wxALIGN_TOP);
			prfCurrentLimitsTable->DisableRowResize(row);

			prfCurrentLimitsTable->SetCellValue(row, 0, to_wx_string(lc->GetLDD_PRFCurrentLimitTable_PRF(id, row)));
			prfCurrentLimitsTable->SetCellValue(row, 1, to_wx_string(lc->GetLDD_PRFCurrentLimitTable_Current(id, row), 2));
		}

		sizer->Add(prfCurrentLimitsTable, 0, SETTING_STYLE, SETTING_PADDING);
	}



	RefreshStrings();
	RefreshAll();

	SetSizer(sizer);
	Layout();
	sizer->Fit(this);
}


void DiodeSettingsPanel::RefreshAll() {

	RefreshEnableButtonState();

	sliderGaugeSpin->RefreshAll();
	actualCurrentReadoutWithMaxSpinCtrl->RefreshAll(); // Don't need to call directly anymore
	voltageReadout->RefreshAll();
	maxSetCurrentSettingSpin->RefreshAll();
	maxMeasuredCurrentSettingSpin->RefreshAll();
	maxMeasuredVoltageSettingSpin->RefreshAll();
	minMeasuredCurrentSettingSpin->RefreshAll();
	minSetCurrentSettingSpin->RefreshAll();
	minDiodeVoltageSettingSpin->RefreshAll();
	lddVoltageCalibrationSettingSpin->RefreshAll();

	if (shutterOpenHighLimitSettingSpin)
		shutterOpenHighLimitSettingSpin->RefreshAll();
	if (shutterOpenLowLimitSettingSpin)
		shutterOpenLowLimitSettingSpin->RefreshAll();

	if (hfCurrentLimitSettingSpin) {
		hfCurrentLimitSettingSpin->RefreshAll();
		if (compareVersions(lc->GetFirmwareVersion(), "3.0.1ER453") < 0)
			SetTooltip_(hfCurrentLimitSettingSpin, _("Requires firmware 3.0.1ER453"));
		else
			hfCurrentLimitSettingSpin->UnsetToolTip();
	}
}

void DiodeSettingsPanel::RefreshStrings() {

	SetText(title, _(lc->GetLDDLabel(id)));
	RefreshEnableButtonState();

	sliderGaugeSpin->RefreshStrings();
	actualCurrentReadoutWithMaxSpinCtrl->RefreshStrings(); // Don't need to call directly anymore
	voltageReadout->RefreshStrings();

	SetText(calibrationSectionTitle, _(CALIBRATION_STR));

	maxSetCurrentSettingSpin->RefreshStrings();
	maxMeasuredCurrentSettingSpin->RefreshStrings();
	maxMeasuredVoltageSettingSpin->RefreshStrings();
	minMeasuredCurrentSettingSpin->RefreshStrings();
	minSetCurrentSettingSpin->RefreshStrings();
	minDiodeVoltageSettingSpin->RefreshStrings();
	lddVoltageCalibrationSettingSpin->RefreshStrings();

	if (shutterLimitsSectionTitle)
		SetText(shutterLimitsSectionTitle, _(SHUTTER_LIMITS_STR));

	if (shutterOpenHighLimitSettingSpin)
		shutterOpenHighLimitSettingSpin->RefreshStrings();
	if (shutterOpenLowLimitSettingSpin)
		shutterOpenLowLimitSettingSpin->RefreshStrings();

	if (otherSettingsSectionTitle)
		SetText(otherSettingsSectionTitle, _(OTHER_SETTINGS_STR));

	if (hfCurrentLimitSettingSpin)
		hfCurrentLimitSettingSpin->RefreshStrings();

	if (prfCurrentLimitsTable) {
		SetText(prfCurrentLimitsTitle, _(PRF_CURRENT_LIMITS_STR));
		prfCurrentLimitsTable->SetColLabelValue(0, "PRF (Hz)");
		prfCurrentLimitsTable->SetColLabelValue(1, _("Max\nCurrent (A)"));
	}

}

void DiodeSettingsPanel::RefreshVisibility() {

	bool inPrivilegedMode = IsInAccessMode(GuiAccessMode::SERVICE) or IsInAccessMode(GuiAccessMode::FACTORY);
	bool diodeAccessCodeEntered = AccessManager::GetInstance().GetCurrentAccessCodeType() == AccessCodeType::DIODE;
	SetVisibilityBasedOnCondition(this, inPrivilegedMode or diodeAccessCodeEntered);

}

void DiodeSettingsPanel::RefreshControlEnabled() {
}


void DiodeSettingsPanel::RefreshEnableButtonState() {
	bool isOn = lc->LddIsOn(id);

	if (isOn) {
		SetBGColor(enableButton, BUTTON_COLOR_ACTIVE);
		SetText(enableButton, _(POWER_DOWN_TEXT));
	}
	else {
		SetBGColor(enableButton, BUTTON_COLOR_INACTIVE);
		SetText(enableButton, _(POWER_UP_TEXT));
	}

}

void DiodeSettingsPanel::OnEnableButtonClicked(wxCommandEvent& evt) {
	lc->ToggleEnableLDD(id);
	RefreshEnableButtonState();
	EmitSizeEvent(enableButton);
}


void DiodeSettingsPanel::OnPRFCurrentLimitsTableEntry(wxGridEvent& evt) {
	int row = evt.GetRow();
	int col = evt.GetCol();

	if (col == 0) {
		int prf = ToIntSafely(std::string(prfCurrentLimitsTable->GetCellValue(row, col)));
		lc->SetLDD_PRFCurrentLimitTable_PRF(id, row, prf);
		prfCurrentLimitsTable->SetCellValue(row, 0, to_wx_string(lc->GetLDD_PRFCurrentLimitTable_PRF(id, row)));
	}
	else {
		float currentLimit = ToFloatSafely(std::string(prfCurrentLimitsTable->GetCellValue(row, col))) + 0.0001f;
		lc->SetLDD_PRFCurrentLimitTable_Current(id, row, currentLimit);
		prfCurrentLimitsTable->SetCellValue(row, 1, to_wx_string(lc->GetLDD_PRFCurrentLimitTable_Current(id, row)));
	}
}

