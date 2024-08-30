#include "TemperatureControlPanel.h"
#include "CommonFunctions.h"
#include "../CommonFunctions_GUI.h"
#include "../Resources.h"

using namespace std;

static wxString NEW_TEMP_STR = _("New temp:");
static wxString SETTINGS_STR = _("Settings");


float roundTemperature(float temperature) {
	// Round the given temperature to the nearest tenth of a degree, e.g., 48.67 -> 48.7
	return round(temperature * 10) / static_cast<float>(10);
}


TemperatureControlPanel::TemperatureControlPanel(
    shared_ptr<MainLaserControllerInterface> _lc,
	int temperature_id,
    wxWindow* parent,
    wxWindowID winid,
    const wxPoint& pos,
    const wxSize& size,
    long style,
    const wxString& name) :
    wxPanel(parent, winid, pos, size, style, name) {


    lc = _lc;
	temperatureId = temperature_id;

	// Main temperature control IDs
	ID_COLLAPSIBLE_SETTINGS_PANEL = wxNewId();
	ID_SLIDER = wxNewId();
	ID_COLDER_BUTTON = wxNewId();
	ID_HOTTER_BUTTON = wxNewId();
	ID_SET_TEMP_TEXT_CTRL = wxNewId();
	ID_SET_TEMP_BUTTON = wxNewId();

	// Temperature settings IDs
	ID_TEC_LABEL_TEXT_CTRL = wxNewId();
	ID_SET_HIGH_LIMIT_TEXT_CTRL = wxNewId();
	ID_SET_LOW_LIMIT_TEXT_CTRL = wxNewId();


	// Sizer for the whole temperature control panel
	TemperatureControlSizer = new wxGridBagSizer(0, 0);
	TemperatureControlSizer->SetFlexibleDirection(wxBOTH);
	TemperatureControlSizer->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);


	// Large temperature component label in top left corner
	TemperatureLabel = new wxStaticText(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(-1, -1), wxALIGN_CENTER_HORIZONTAL);
	TemperatureLabel->SetFont(FONT_MED_LARGE_BOLD);
	TemperatureControlSizer->Add(TemperatureLabel, wxGBPosition(0, 0), wxGBSpan(1, 2), wxALL | wxALIGN_CENTER_HORIZONTAL, 5);


	// Set temperature controls - a label "Set temp:", a text box to enter the new temperature, and a "SET/STOP" button
	SetTemperaturePanel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
	SetTemperaturePanel->SetBackgroundColour(wxColor(235, 235, 235));

	wxGridBagSizer* SetTemperatureSizer;
	SetTemperatureSizer = new wxGridBagSizer(0, 0);
	SetTemperatureSizer->SetFlexibleDirection(wxBOTH);
	SetTemperatureSizer->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);

	SetTemperatureLabel = new wxStaticText(SetTemperaturePanel, wxID_ANY, NEW_TEMP_STR);
	SetTemperatureLabel->SetFont(FONT_EXTRA_SMALL_SEMIBOLD);
	SetTemperatureSizer->Add(SetTemperatureLabel, wxGBPosition(0, 0), wxGBSpan(1, 1), wxALIGN_CENTER_HORIZONTAL, 0);

	SetTemperatureTextCtrl = new wxTextCtrl(SetTemperaturePanel, ID_SET_TEMP_TEXT_CTRL, wxEmptyString,
		wxDefaultPosition, wxSize(50, 23), wxTE_CENTER | wxTE_PROCESS_ENTER, wxTextValidator(wxFILTER_NUMERIC));
	SetTemperatureTextCtrl->Bind(wxEVT_TEXT_ENTER, &TemperatureControlPanel::OnEnterSetTemperature, this, ID_SET_TEMP_TEXT_CTRL);
	SetTemperatureTextCtrl->SetFont(FONT_SMALL_SEMIBOLD);
	SetTemperatureTextCtrl->SetMaxLength(5);
	SetTemperatureSizer->Add(SetTemperatureTextCtrl, wxGBPosition(1, 0), wxGBSpan(1, 1), wxALIGN_CENTER_HORIZONTAL | wxBOTTOM | wxLEFT | wxRIGHT, 5);

	SetTemperatureButton = new wxButton(SetTemperaturePanel, ID_SET_TEMP_BUTTON, _("SET"), wxDefaultPosition, wxSize(45, 30), wxTAB_TRAVERSAL);
	SetTemperatureButton->Bind(wxEVT_BUTTON, &TemperatureControlPanel::OnSetTempButtonClicked, this, ID_SET_TEMP_BUTTON);
	SetTemperatureSizer->Add(SetTemperatureButton, wxGBPosition(0, 1), wxGBSpan(2, 1), wxALL | wxALIGN_CENTER_VERTICAL, 3);

	SetTemperaturePanel->SetSizer(SetTemperatureSizer);
	SetTemperaturePanel->Layout();
	SetTemperatureSizer->Fit(SetTemperaturePanel);
	TemperatureControlSizer->Add(SetTemperaturePanel, wxGBPosition(1, 0), wxGBSpan(1, 1), wxALL | wxRESERVE_SPACE_EVEN_IF_HIDDEN, 4);


	// Main controls - a current index label, left and right arrow buttons, a slider, and a gauge
	wxFlexGridSizer* TemperatureSimpleControlsSizer;
	TemperatureSimpleControlsSizer = new wxFlexGridSizer(1, 2, 0, 0);
	TemperatureSimpleControlsSizer->SetFlexibleDirection(wxBOTH);
	TemperatureSimpleControlsSizer->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);

	TemperatureSliderPanel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_THEME | wxTAB_TRAVERSAL);
	TemperatureSliderPanel->SetBackgroundColour(EXTRA_LIGHT_BACKGROUND_COLOR);

	wxBoxSizer* TemperatureSliderSizer = new wxBoxSizer(wxHORIZONTAL);

	wxFlexGridSizer* TemperatrueSliderInnerSizer;
	TemperatrueSliderInnerSizer = new wxFlexGridSizer(3, 0, 0, 0);
	TemperatrueSliderInnerSizer->AddGrowableRow(1);
	TemperatrueSliderInnerSizer->SetFlexibleDirection(wxBOTH);
	TemperatrueSliderInnerSizer->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);

	TemperatureSliderMax = new wxStaticText(TemperatureSliderPanel, wxID_ANY, wxT("55.0"));
	TemperatureSliderMax->SetFont(FONT_EXTRA_SMALL_SEMIBOLD);
	TemperatrueSliderInnerSizer->Add(TemperatureSliderMax, 0, wxALIGN_CENTER_HORIZONTAL | wxTOP, 5);

	TemperatureSlider = new wxSlider(TemperatureSliderPanel, ID_SLIDER, 0, 0, 1, wxDefaultPosition, wxSize(-1, 150), wxSL_VERTICAL | wxSL_INVERSE);
	TemperatureSlider->Bind(wxEVT_SCROLL_THUMBRELEASE, &TemperatureControlPanel::OnSliderMoved, this, ID_SLIDER);
	TemperatrueSliderInnerSizer->Add(TemperatureSlider, 0, wxLEFT | wxRIGHT | wxEXPAND, 5);

	TemperatureSliderMin = new wxStaticText(TemperatureSliderPanel, wxID_ANY, wxT("45.0"));
	TemperatureSliderMin->SetFont(FONT_EXTRA_SMALL_SEMIBOLD);
	TemperatrueSliderInnerSizer->Add(TemperatureSliderMin, 0, wxALIGN_CENTER_HORIZONTAL | wxBOTTOM, 5);

	TemperatureSliderSizer->Add(TemperatrueSliderInnerSizer, 1, wxEXPAND, 5);

	TemperatureGauge = new wxGauge(TemperatureSliderPanel, wxID_ANY, 20, wxDefaultPosition, wxSize(-1, 125), wxGA_VERTICAL | wxGA_SMOOTH);
	TemperatureSliderSizer->Add(TemperatureGauge, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 3);

	TemperatureSliderPanel->SetSizer(TemperatureSliderSizer);
	TemperatureSliderPanel->Layout();
	TemperatureSliderSizer->Fit(TemperatureSliderPanel);
	TemperatureSimpleControlsSizer->Add(TemperatureSliderPanel, 1, wxEXPAND | wxALL, 0); //*****************

	TemperatureSimpleControls = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);

	wxFlexGridSizer* TemperatureControlsInnerSizer = new wxFlexGridSizer(3, 1, 0, 0);
	TemperatureControlsInnerSizer->AddGrowableRow(0);
	TemperatureControlsInnerSizer->AddGrowableRow(2);
	TemperatureControlsInnerSizer->SetFlexibleDirection(wxBOTH);
	TemperatureControlsInnerSizer->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);

	TemperatureHotterButton = new wxBitmapButton(TemperatureSimpleControls, ID_HOTTER_BUTTON, wxBitmap(UP_ARROW_IMAGE, wxBITMAP_TYPE_ANY),
		wxDefaultPosition, wxSize(30, 40), wxBU_AUTODRAW | 0);
	TemperatureHotterButton->Bind(wxEVT_LEFT_DOWN, &TemperatureControlPanel::OnHotterButtonClicked, this, ID_HOTTER_BUTTON);
	TemperatureHotterButton->Bind(wxEVT_LEFT_UP, &TemperatureControlPanel::OnHotterButtonClickStopped, this, ID_HOTTER_BUTTON);
	TemperatureHotterButton->Bind(wxEVT_LEAVE_WINDOW, &TemperatureControlPanel::OnHotterButtonClickStopped, this, ID_HOTTER_BUTTON);
	TemperatureControlsInnerSizer->Add(TemperatureHotterButton, 0, wxALIGN_BOTTOM | wxALIGN_CENTER_HORIZONTAL | wxBOTTOM | wxRESERVE_SPACE_EVEN_IF_HIDDEN, 5);

	wxFlexGridSizer* CurrentTemperatureSizer = new wxFlexGridSizer(1, 2, 0, 0);

	CurrentTemperatureLabel = new wxStaticText(TemperatureSimpleControls, wxID_ANY, wxT("0.0"), wxDefaultPosition, wxSize(40, -1), wxALIGN_CENTER_HORIZONTAL);
	CurrentTemperatureLabel->SetFont(FONT_MEDIUM_BOLD);
	CurrentTemperatureSizer->Add(CurrentTemperatureLabel, 0, wxALIGN_CENTER_VERTICAL, 0);

	CurrentTemperatureUnits = new wxStaticText(TemperatureSimpleControls, wxID_ANY, wxT("C"), wxDefaultPosition, wxDefaultSize, wxALIGN_CENTER_VERTICAL);
	CurrentTemperatureUnits->SetFont(FONT_MED_SMALL_SEMIBOLD);
	CurrentTemperatureSizer->Add(CurrentTemperatureUnits, 0, wxLEFT, 4);

	TemperatureControlsInnerSizer->Add(CurrentTemperatureSizer, 0, 0, 0);

	TemperatureColderButton = new wxBitmapButton(TemperatureSimpleControls, ID_COLDER_BUTTON, wxBitmap(DOWN_ARROW_IMAGE, wxBITMAP_TYPE_ANY),
		wxDefaultPosition, wxSize(30, 40), wxBU_AUTODRAW | 0);
	TemperatureColderButton->Bind(wxEVT_LEFT_DOWN, &TemperatureControlPanel::OnColderButtonClicked, this, ID_COLDER_BUTTON);
	TemperatureColderButton->Bind(wxEVT_LEFT_UP, &TemperatureControlPanel::OnColderButtonClickStopped, this, ID_COLDER_BUTTON);
	TemperatureColderButton->Bind(wxEVT_LEAVE_WINDOW, &TemperatureControlPanel::OnColderButtonClickStopped, this, ID_COLDER_BUTTON);
	TemperatureControlsInnerSizer->Add(TemperatureColderButton, 0, wxALIGN_TOP | wxALIGN_CENTER_HORIZONTAL | wxTOP | wxRESERVE_SPACE_EVEN_IF_HIDDEN, 5);

	TemperatureSimpleControls->SetSizer(TemperatureControlsInnerSizer);
	TemperatureSimpleControls->Layout();
	TemperatureControlsInnerSizer->Fit(TemperatureSimpleControls);
	TemperatureSimpleControlsSizer->Add(TemperatureSimpleControls, 1, wxEXPAND | wxALL, 5);

	TemperatureControlSizer->Add(TemperatureSimpleControlsSizer, wxGBPosition(2, 0), wxGBSpan(1, 2), wxEXPAND, 5);


	// Collapsible settings panel containing more advanced settings - change temperature component label, change high/low limits, etc.
	CollapsibleSettingsPanel = new wxCollapsiblePane(this, ID_COLLAPSIBLE_SETTINGS_PANEL, SETTINGS_STR, wxDefaultPosition, wxDefaultSize, wxCP_DEFAULT_STYLE | wxCP_NO_TLW_RESIZE);
	CollapsibleSettingsPanel->Collapse(true);
	Bind(wxEVT_COLLAPSIBLEPANE_CHANGED, &TemperatureControlPanel::OnSettingsCollapse, this, ID_COLLAPSIBLE_SETTINGS_PANEL);

	wxBoxSizer* CollapsibleSettingsSizer;
	CollapsibleSettingsSizer = new wxBoxSizer(wxVERTICAL);

	TemperatureInnerSettingsPanel = new wxPanel(CollapsibleSettingsPanel->GetPane(), wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_THEME | wxTAB_TRAVERSAL);
	TemperatureInnerSettingsPanel->SetFont(FONT_SMALL_SEMIBOLD);
	TemperatureInnerSettingsPanel->SetBackgroundColour(FOREGROUND_PANEL_COLOR);

	wxFlexGridSizer* TemperatureInnerSettingsSizer;
	TemperatureInnerSettingsSizer = new wxFlexGridSizer(0, 2, 0, 0);
	TemperatureInnerSettingsSizer->SetFlexibleDirection(wxBOTH);
	TemperatureInnerSettingsSizer->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);

	// Change Temperature Control Label Setting
	TemperatureLabelLabel = new wxStaticText(TemperatureInnerSettingsPanel, wxID_ANY, _("Label"));
	TemperatureInnerSettingsSizer->Add(TemperatureLabelLabel, 0, wxALL, 5);

	TemperatureLabelTextCtrl = new wxTextCtrl(TemperatureInnerSettingsPanel, ID_TEC_LABEL_TEXT_CTRL, wxEmptyString,
		wxDefaultPosition, wxSize(55, -1), wxTE_CENTER | wxTE_PROCESS_ENTER);
	TemperatureLabelTextCtrl->SetMaxLength(8);
	TemperatureLabelTextCtrl->Bind(wxEVT_TEXT_ENTER, &TemperatureControlPanel::OnTempLabelEntered, this, ID_TEC_LABEL_TEXT_CTRL);
	TemperatureInnerSettingsSizer->Add(TemperatureLabelTextCtrl, 0, wxALL, 5);

	// Change Temperature Low Limit Setting
	TemperatureLowLimitLabel = new wxStaticText(TemperatureInnerSettingsPanel, wxID_ANY, _("Low Limit"));
	TemperatureInnerSettingsSizer->Add(TemperatureLowLimitLabel, 0, wxALL, 5);

	TemperatureLowLimitTextCtrl = new wxTextCtrl(TemperatureInnerSettingsPanel, ID_SET_LOW_LIMIT_TEXT_CTRL, wxEmptyString,
		wxDefaultPosition, wxSize(55, -1), wxTE_CENTER | wxTE_PROCESS_ENTER, wxTextValidator(wxFILTER_NUMERIC));
	TemperatureLowLimitTextCtrl->SetMaxLength(5);
	TemperatureLowLimitTextCtrl->Bind(wxEVT_TEXT_ENTER, &TemperatureControlPanel::OnSetLowLimitEntered, this, ID_SET_LOW_LIMIT_TEXT_CTRL);
	TemperatureInnerSettingsSizer->Add(TemperatureLowLimitTextCtrl, 0, wxALL, 5);

	// Change Temperature High Limit Setting
	TemperatureHighLimitLabel = new wxStaticText(TemperatureInnerSettingsPanel, wxID_ANY, _("High Limit"));
	TemperatureInnerSettingsSizer->Add(TemperatureHighLimitLabel, 0, wxALL, 5);

	TemperatureHighLimitTextCtrl = new wxTextCtrl(TemperatureInnerSettingsPanel, ID_SET_HIGH_LIMIT_TEXT_CTRL, wxEmptyString,
		wxDefaultPosition, wxSize(55, -1), wxTE_CENTER | wxTE_PROCESS_ENTER, wxTextValidator(wxFILTER_NUMERIC));
	TemperatureHighLimitTextCtrl->SetMaxLength(5);
	TemperatureHighLimitTextCtrl->Bind(wxEVT_TEXT_ENTER, &TemperatureControlPanel::OnSetHighLimitEntered, this, ID_SET_HIGH_LIMIT_TEXT_CTRL);
	TemperatureInnerSettingsSizer->Add(TemperatureHighLimitTextCtrl, 0, wxALL, 5);

	// Alarm enabled checkbox
	AlarmEnabledLabel = new wxStaticText(TemperatureInnerSettingsPanel, wxID_ANY, _("Alarm Enabled"));
	TemperatureInnerSettingsSizer->Add(AlarmEnabledLabel, 0, wxALL, 5);

	AlarmEnabledCheckbox = new wxCheckBox(TemperatureInnerSettingsPanel, wxID_ANY, wxEmptyString);
	AlarmEnabledCheckbox->Bind(wxEVT_CHECKBOX, &TemperatureControlPanel::OnAlarmEnabledChecked, this);
	TemperatureInnerSettingsSizer->Add(AlarmEnabledCheckbox, 0, wxALL, 5);

	// Alarm high limit
	AlarmHighLimitLabel = new wxStaticText(TemperatureInnerSettingsPanel, wxID_ANY, _("Alarm High"));
	TemperatureInnerSettingsSizer->Add(AlarmHighLimitLabel, 0, wxALL, 5);

	AlarmHighLimitTextCtrl = new wxTextCtrl(TemperatureInnerSettingsPanel, wxID_ANY, wxEmptyString,
		wxDefaultPosition, wxSize(55, -1), wxTE_CENTER | wxTE_PROCESS_ENTER, wxTextValidator(wxFILTER_NUMERIC));
	AlarmHighLimitTextCtrl->SetMaxLength(5);
	AlarmHighLimitTextCtrl->Bind(wxEVT_TEXT_ENTER, &TemperatureControlPanel::OnAlarmHighLimitEntered, this);
	TemperatureInnerSettingsSizer->Add(AlarmHighLimitTextCtrl, 0, wxALL, 5);

	// Alarm low limit
	AlarmLowLimitLabel = new wxStaticText(TemperatureInnerSettingsPanel, wxID_ANY, _("Alarm Low"));
	TemperatureInnerSettingsSizer->Add(AlarmLowLimitLabel, 0, wxALL, 5);

	AlarmLowLimitTextCtrl = new wxTextCtrl(TemperatureInnerSettingsPanel, wxID_ANY, wxEmptyString,
		wxDefaultPosition, wxSize(55, -1), wxTE_CENTER | wxTE_PROCESS_ENTER, wxTextValidator(wxFILTER_NUMERIC));
	AlarmLowLimitTextCtrl->SetMaxLength(5);
	AlarmLowLimitTextCtrl->Bind(wxEVT_TEXT_ENTER, &TemperatureControlPanel::OnAlarmLowLimitEntered, this);
	TemperatureInnerSettingsSizer->Add(AlarmLowLimitTextCtrl, 0, wxALL, 5);


	TemperatureInnerSettingsPanel->SetSizer(TemperatureInnerSettingsSizer);
	TemperatureInnerSettingsPanel->Layout();
	TemperatureInnerSettingsSizer->Fit(TemperatureInnerSettingsPanel);
	CollapsibleSettingsSizer->Add(TemperatureInnerSettingsPanel, 0, wxALL, 5);

	CollapsibleSettingsPanel->GetPane()->SetSizer(CollapsibleSettingsSizer);
	CollapsibleSettingsPanel->GetPane()->Layout();
	CollapsibleSettingsSizer->Fit(CollapsibleSettingsPanel->GetPane());
	TemperatureControlSizer->Add(CollapsibleSettingsPanel, wxGBPosition(3, 0), wxGBSpan(1, 1), wxEXPAND | wxALL, 0);


	this->SetSizer(TemperatureControlSizer);
	this->Layout();
	this->SetBackgroundColour(FOREGROUND_PANEL_COLOR);


    Init();
}

void TemperatureControlPanel::Init() {

	moveCausedByButton = false;
	increaseTemperatureButtonPressed = false;
	decreaseTemperatureButtonPressed = false;

	TemperatureLabel->SetLabelText(lc->GetTemperatureControlLabel(temperatureId));
	currentTemp = lc->GetActualTemperature(temperatureId);
	float setTemp = lc->GetSetTemperature(temperatureId);
	minTemp = lc->GetMinSetTemperature(temperatureId);
	maxTemp = lc->GetMaxSetTemperature(temperatureId);
	TemperatureSlider->SetMin(minTemp * 100);
	TemperatureSliderMin->SetLabelText(to_wx_string(minTemp, 2));
	TemperatureSlider->SetMax(maxTemp * 100);
	TemperatureSliderMax->SetLabelText(to_wx_string(maxTemp, 2));
	TemperatureSlider->SetValue(setTemp * 100);
	TemperatureGauge->SetRange((maxTemp - minTemp) * 100);
	TemperatureGauge->SetValue((currentTemp - minTemp) * 100);
	CurrentTemperatureLabel->SetLabelText(to_wx_string(currentTemp, 2));
	SetTemperatureTextCtrl->SetValue(to_wx_string(setTemp, 2));
	TemperatureLabelTextCtrl->SetValue(lc->GetTemperatureControlLabel(temperatureId));
	TemperatureLowLimitTextCtrl->SetValue(to_wx_string(lc->GetMinSetTemperature(temperatureId), 2));
	TemperatureHighLimitTextCtrl->SetValue(to_wx_string(lc->GetMaxSetTemperature(temperatureId), 2));
	AlarmEnabledCheckbox->SetValue(lc->AlarmEnabled(GetTemperatureAlarmFromComponentID()));
	AlarmLowLimitTextCtrl->SetValue(to_wx_string(lc->GetTemperatureAlarmLowLimit(temperatureId), 2));
	AlarmHighLimitTextCtrl->SetValue(to_wx_string(lc->GetTemperatureAlarmHighLimit(temperatureId), 2));

	isDisplayOnly = lc->TemperatureControlIsDisplayOnly(temperatureId);
	if (isDisplayOnly) {
		SetTemperaturePanel->Hide();
		TemperatureColderButton->Hide();
		TemperatureHotterButton->Hide();
		TemperatureSlider->Disable();
	}
}


void TemperatureControlPanel::RefreshStrings() {
	SetTemperatureLabel->SetLabelText(_(NEW_TEMP_STR));
	CollapsibleSettingsPanel->SetLabelText(_(SETTINGS_STR));
}


void TemperatureControlPanel::RefreshAll() {
	currentTemp = lc->GetActualTemperature(temperatureId);
	RefreshSetTempButton();
	RefreshCurrentTempDependentWidgets();

	if (increaseTemperatureButtonPressed) {
		lc->IncrementTemperature(temperatureId);
		SetTemperatureTextCtrl->SetValue(to_wx_string(lc->GetSetTemperature(temperatureId), 2));
		wxLogDebug("Incrementing set temperature of " + lc->GetTemperatureControlLabel(temperatureId) +
			": " + to_wx_string(lc->GetSetTemperature(temperatureId), 2));
	}
	if (decreaseTemperatureButtonPressed) {
		lc->DecrementTemperature(temperatureId);
		SetTemperatureTextCtrl->SetValue(to_wx_string(lc->GetSetTemperature(temperatureId), 2));
		wxLogDebug("Decrementing set temperature of " + lc->GetTemperatureControlLabel(temperatureId) +
			": " + to_wx_string(lc->GetSetTemperature(temperatureId), 2));
	}

	if (lc->TemperatureIsRampedNearSetPoint(temperatureId)) {
		if (!TemperatureHotterButton->IsEnabled())
			TemperatureHotterButton->Enable();
		if (!TemperatureColderButton->IsEnabled())
			TemperatureColderButton->Enable();
	}
	else {
		if (TemperatureHotterButton->IsEnabled())
			TemperatureHotterButton->Disable();
		if (TemperatureColderButton->IsEnabled())
			TemperatureColderButton->Disable();
	}
}


void TemperatureControlPanel::RefreshVisibility() {
}


void TemperatureControlPanel::RefreshCurrentTempDependentWidgets() {
	CurrentTemperatureLabel->SetLabelText(to_wx_string(currentTemp, 2));
	TemperatureGauge->SetValue((currentTemp - minTemp) * 100);
}

void TemperatureControlPanel::RefreshSetTempButton() {
	if (!lc->TemperatureIsRampedNearSetPoint(temperatureId)) {
		if ((SetTemperatureButton->GetLabelText() != _("STOP"))) {
			SetTemperatureButton->SetLabelText(_("STOP"));
			SetTemperatureButton->SetBackgroundColour(TEXT_COLOR_RED);
		}
	}
	else if ((SetTemperatureButton->GetLabelText() != _("SET"))) {
		SetTemperatureButton->SetLabelText(_("SET"));
		SetTemperatureButton->SetBackgroundColour(BUTTON_COLOR_INACTIVE);
	}
}

void TemperatureControlPanel::OnEnterSetTemperature(wxCommandEvent& evt) {
	STAGE_ACTION("Set temperature entered")
	string newSetTempString = string(SetTemperatureTextCtrl->GetValue());
	STAGE_ACTION_ARGUMENTS("ID:" + to_string(temperatureId) + " - " + newSetTempString)
	if (containsSubstr(newSetTempString, "e"))
		return;
	float newSetTemp = ToFloatSafely(newSetTempString);
	lc->SetTemperature(temperatureId, newSetTemp);
	TemperatureSlider->SetValue(newSetTemp * 100);
	LOG_ACTION()
}

void TemperatureControlPanel::OnSetTempButtonClicked(wxCommandEvent& evt) {
	STAGE_ACTION("Set temperature button clicked")
	// If clicked on while in STOP state (while temperature is ramping):
	if (!lc->TemperatureIsRampedNearSetPoint(temperatureId)) {
		STAGE_ACTION_ARGUMENTS("ID:" + to_string(temperatureId) + " - " + "Cancel")
		CancelChangeTemperature();
		increaseTemperatureButtonPressed = false;
		decreaseTemperatureButtonPressed = false;
	}
	// If clicked on while in SET state (temperature is stable):
	else {
		float targetTemp = wxAtof(SetTemperatureTextCtrl->GetValue());
		STAGE_ACTION_ARGUMENTS("ID:" + to_string(temperatureId) + " - " + to_string_with_precision(targetTemp, 2))
		float constrainedtargetTemp = min(max(targetTemp, minTemp), maxTemp);
		lc->SetTemperature(temperatureId, constrainedtargetTemp);
		TemperatureSlider->SetValue(constrainedtargetTemp * 100);
		SetTemperatureTextCtrl->SetLabelText(to_wx_string(constrainedtargetTemp, 2));
	}
	RefreshSetTempButton();
	LOG_ACTION()
}

void TemperatureControlPanel::OnSliderMoved(wxScrollEvent& evt) {
	STAGE_ACTION("Set temperature slider moved")
	float targetTemp = TemperatureSlider->GetValue() / static_cast<float>(100);
	STAGE_ACTION_ARGUMENTS("ID:" + to_string(temperatureId) + " - " + to_string_with_precision(targetTemp, 2))
	float targetTempRounded = roundTemperature(targetTemp);
	TemperatureSlider->SetValue(targetTempRounded * 100);
	lc->SetTemperature(temperatureId, targetTempRounded);
	SetTemperatureTextCtrl->SetLabelText(to_wx_string(targetTempRounded, 2));
	RefreshSetTempButton();
	LOG_ACTION()
}

void TemperatureControlPanel::OnColderButtonClicked(wxMouseEvent& evt) {
	STAGE_ACTION_("Set temperature colder button clicked", "ID:" + to_string(temperatureId))
	TemperatureSlider->SetValue(TemperatureSlider->GetMin());
	RefreshSetTempButton();
	moveCausedByButton = true;
	decreaseTemperatureButtonPressed = true;
	LOG_ACTION()
}


// Do not log action 
void TemperatureControlPanel::OnColderButtonClickStopped(wxMouseEvent& evt) {
	if (moveCausedByButton) {
		TemperatureSlider->SetValue(lc->GetSetTemperature(temperatureId) * 100);
		RefreshSetTempButton();
		moveCausedByButton = false;
		decreaseTemperatureButtonPressed = false;
	}
}


void TemperatureControlPanel::OnHotterButtonClicked(wxMouseEvent& evt) {
	STAGE_ACTION_("Set temperature hotter button clicked", "ID:" + to_string(temperatureId))
	TemperatureSlider->SetValue(TemperatureSlider->GetMax());
	RefreshSetTempButton();
	moveCausedByButton = true;
	increaseTemperatureButtonPressed = true;
	LOG_ACTION()
}


// Do not log action
void TemperatureControlPanel::OnHotterButtonClickStopped(wxMouseEvent& evt) {
	if (moveCausedByButton) {
		TemperatureSlider->SetValue(lc->GetSetTemperature(temperatureId) * 100);
		RefreshSetTempButton();
		moveCausedByButton = false;
		increaseTemperatureButtonPressed = false;
	}
}


void TemperatureControlPanel::OnTempLabelEntered(wxCommandEvent& evt) {
	STAGE_ACTION("Temperature label entered")
	string newLabel = string(TemperatureLabelTextCtrl->GetValue());
	STAGE_ACTION_ARGUMENTS("ID:" + to_string(temperatureId) + " - " + newLabel)
	lc->SetTemperatureControlLabel(temperatureId, newLabel);
	string newLabelConfirmed = lc->GetTemperatureControlLabel(temperatureId);
	TemperatureLabelTextCtrl->SetValue(newLabelConfirmed);
	TemperatureLabel->SetLabelText(newLabelConfirmed);
	LOG_ACTION()
}

void TemperatureControlPanel::OnSetHighLimitEntered(wxCommandEvent& evt) {
	STAGE_ACTION("Temperature high limit entered")
	float newHighLimit = wxAtof(TemperatureHighLimitTextCtrl->GetValue());
	STAGE_ACTION_ARGUMENTS("ID:" + to_string(temperatureId) + " - " + to_string_with_precision(newHighLimit, 2))
	newHighLimit = min(max(newHighLimit, minTemp), 99.9f);
	lc->SetMaxSetTemperature(temperatureId, newHighLimit);
	float newHighLimitConfirmed = lc->GetMaxSetTemperature(temperatureId);
	maxTemp = newHighLimitConfirmed;
	TemperatureHighLimitTextCtrl->SetValue(to_wx_string(maxTemp, 2));
	TemperatureSlider->SetMax(maxTemp * 100);
	TemperatureSliderMax->SetLabelText(to_wx_string(maxTemp, 2));
	TemperatureGauge->SetRange((maxTemp - minTemp) * 100);
	LOG_ACTION()
}

void TemperatureControlPanel::OnSetLowLimitEntered(wxCommandEvent& evt) {
	STAGE_ACTION("Temperature low limit entered")
	float newLowLimit = wxAtof(TemperatureLowLimitTextCtrl->GetValue());
	STAGE_ACTION_ARGUMENTS("ID:" + to_string(temperatureId) + " - " + to_string_with_precision(newLowLimit, 2))
	newLowLimit = min(max(newLowLimit, 0.0f), maxTemp);
	lc->SetMinSetTemperature(temperatureId, newLowLimit);
	float newLowLimitConfirmed = lc->GetMinSetTemperature(temperatureId);
	minTemp = newLowLimitConfirmed;
	TemperatureLowLimitTextCtrl->SetValue(to_wx_string(minTemp, 2));
	TemperatureSlider->SetMin(minTemp * 100);
	TemperatureSliderMin->SetLabelText(to_wx_string(minTemp, 2));
	TemperatureGauge->SetRange((maxTemp - minTemp) * 100);
	LOG_ACTION()
}


void TemperatureControlPanel::OnAlarmEnabledChecked(wxCommandEvent& evt) {
	if (AlarmEnabledCheckbox->IsChecked()) {
		STAGE_ACTION_("Temperature alarm enabled", "ID:" + to_string(temperatureId))
		lc->SetAlarm(GetTemperatureAlarmFromComponentID(), true);
	}
	else {
		STAGE_ACTION_("Temperature alarm disabled", "ID:" + to_string(temperatureId))
		lc->SetAlarm(GetTemperatureAlarmFromComponentID(), false);
	}
	AlarmEnabledCheckbox->SetValue(lc->AlarmEnabled(GetTemperatureAlarmFromComponentID()));
	LOG_ACTION()
}

void TemperatureControlPanel::OnAlarmHighLimitEntered(wxCommandEvent& evt) {
	STAGE_ACTION("Temperature alarm high limit entered")
	float newHighLimit = wxAtof(AlarmHighLimitTextCtrl->GetValue());
	STAGE_ACTION_ARGUMENTS("ID:" + to_string(temperatureId) + " - " + to_string_with_precision(newHighLimit, 2))
	newHighLimit = min(max(newHighLimit, 0.0f), 99.9f);
	lc->SetTemperatureAlarmHighLimit(temperatureId, newHighLimit);
	float newHighLimitConfirmed = lc->GetTemperatureAlarmHighLimit(temperatureId);
	AlarmHighLimitTextCtrl->SetValue(to_wx_string(newHighLimitConfirmed, 2));
	LOG_ACTION()
}

void TemperatureControlPanel::OnAlarmLowLimitEntered(wxCommandEvent& evt) {
	STAGE_ACTION("Temperature alarm low limit entered")
	float newLowLimit = wxAtof(AlarmLowLimitTextCtrl->GetValue());
	STAGE_ACTION_ARGUMENTS("ID:" + to_string(temperatureId) + " - " + to_string_with_precision(newLowLimit, 2))
	newLowLimit = max(newLowLimit, 0.0f);
	lc->SetTemperatureAlarmLowLimit(temperatureId, newLowLimit);
	float newLowLimitConfirmed = lc->GetTemperatureAlarmLowLimit(temperatureId);
	AlarmLowLimitTextCtrl->SetValue(to_wx_string(newLowLimitConfirmed, 2));
	LOG_ACTION()
}


void TemperatureControlPanel::OnSettingsCollapse(wxCollapsiblePaneEvent& event) {
	STAGE_ACTION("Temperature settings collapbe button clicked")
	this->Layout();
	TemperatureControlSizer->Fit(this);
	this->GetParent()->Layout();
	LOG_ACTION()
}

void TemperatureControlPanel::CancelChangeTemperature() {
	// Change the set temperature to whatever the current temperature is
	// - effectively stops the temperature from ramping to a new set temperature
	float currentTempRounded = roundTemperature(currentTemp);
	lc->SetTemperature(temperatureId, currentTemp);
	SetTemperatureTextCtrl->SetLabelText(to_wx_string(currentTemp, 2));
	TemperatureSlider->SetValue(currentTemp * 100);
}

Alarm TemperatureControlPanel::GetTemperatureAlarmFromComponentID() {
	if (temperatureId == 0) return Alarm::TEMPERATURE_0;
	if (temperatureId == 1) return Alarm::TEMPERATURE_1;
	if (temperatureId == 2) return Alarm::TEMPERATURE_2;
	if (temperatureId == 3) return Alarm::TEMPERATURE_3;
	if (temperatureId == 4) return Alarm::TEMPERATURE_4;
	if (temperatureId == 5) return Alarm::TEMPERATURE_5;
	if (temperatureId == 6) return Alarm::TEMPERATURE_6;
	if (temperatureId == 7) return Alarm::TEMPERATURE_7;
	if (temperatureId == 8) return Alarm::TEMPERATURE_8;
	if (temperatureId == 9) return Alarm::TEMPERATURE_9;
	if (temperatureId == 10) return Alarm::TEMPERATURE_10;
	if (temperatureId == 11) return Alarm::TEMPERATURE_11;
	if (temperatureId == 12) return Alarm::TEMPERATURE_12;
	if (temperatureId == 13) return Alarm::TEMPERATURE_13;
	if (temperatureId == 14) return Alarm::TEMPERATURE_14;
	if (temperatureId == 15) return Alarm::TEMPERATURE_15;

	return Alarm::TEMPERATURE_0;
}

