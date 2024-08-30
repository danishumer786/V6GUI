#include <wx/slider.h>
#include <wx/scrolbar.h>
#include <wx/scrolwin.h>

#include "../CommonFunctions_GUI.h"
#include "MotorControlPanel.h"
#include "ConfigurationManager.h"

using namespace std;


const wxString GO_TO_INDEX_STR = _("Go to index:");
const wxString STEP_SIZE_STR = _("Step Size:");
//const wxString STEP_SIZE_TOOLTIP = _("Set this to 0 to make the arrows move\nmotor continuously while mouse is pressed.");
const wxString STEP_SIZE_TOOLTIP = _("Set this to 0 to move motor continuously as long as key is pressed.");
const wxString SETTINGS_STR = _("Settings");

const wxString SAVED_STEP_SIZE_KEY = "MotorStepSize";
const wxString SAVED_KEYBINDING_KEY = "MotorKeyBinding";
//const wxString SAVED_KEYBINDING_VALUE_LEFTRIGHT = "LeftRight";
//const wxString SAVED_KEYBINDING_VALUE_UPDOWN = "UpDown";
//const wxString SAVED_KEYBINDING_VALUE_AD = "AD";
//const wxString SAVED_KEYBINDING_VALUE_WS = "WS";

const wxString KEYBINDING_STR_LEFTRIGHT = "Left-Right";
const wxString KEYBINDING_STR_UPDOWN = "Up-Down";
const wxString KEYBINDING_STR_AD = "A-D";
const wxString KEYBINDING_STR_WS = "W-S";


int roundMotorIndex(int index) {
	// Round the given index to the nearest hundred, e.g., 16287 -> 16300
	int rem = index % 100;
	if (rem < 50)
		return index - rem;
	else
		return index - rem + 100;
}


MotorControlPanel::MotorControlPanel(
	shared_ptr<MainLaserControllerInterface> _lc,
	int motor_id,
	wxWindow* parent,
	wxWindowID winid,
	const wxPoint& pos,
	const wxSize& size,
	long style,
	const wxString& name) :
	wxPanel(parent, winid, pos, size, style, name) {

	lc = _lc;
	motorId = motor_id;

	motorRefreshCountdown = motorId;

	savedStepSizeKey = SAVED_STEP_SIZE_KEY + "_" + lc->GetSerialNumber() + "_" + to_string(motorId);
	savedKeyBindingKey = SAVED_KEYBINDING_KEY + "_" + lc->GetSerialNumber() + "_" + to_string(motorId);


	// Sizer for the whole motor control panel
	MotorControlsSizer = new wxGridBagSizer(0, 0);

	// Title
	MotorTitle = new FeatureTitle(this, "Motor ?");
	MotorControlsSizer->Add(MotorTitle, wxGBPosition(0, 0), wxGBSpan(1, 1), wxLEFT | wxRIGHT, 5);

	//----------------------------------------------------------------------------------------
	// Target index control - a label "Go to index:", a text box to enter the target index, and a "GO"/"STOP" button

	wxGridBagSizer* MotorTargetIndexSizer = new wxGridBagSizer(0, 0);

	MotorTargetIndexLabel = new wxStaticText(this, wxID_ANY, GO_TO_INDEX_STR, wxDefaultPosition, wxDefaultSize, 0);
	MotorTargetIndexLabel->SetFont(FONT_SMALL_SEMIBOLD);
	MotorTargetIndexSizer->Add(MotorTargetIndexLabel, wxGBPosition(0, 0), wxGBSpan(1, 1), wxLEFT, 5);

	MotorTargetIndexTextCtrl = new NumericTextCtrl(this, NumericTextCtrlType::DIGITS_ONLY, 5, wxSize(60, 23));
	MotorTargetIndexTextCtrl->SetFont(FONT_SMALL_SEMIBOLD);
	MotorTargetIndexTextCtrl->Bind(wxEVT_TEXT_ENTER, &MotorControlPanel::OnTargetIndexEntered, this);
	MotorTargetIndexSizer->Add(MotorTargetIndexTextCtrl, wxGBPosition(1, 0), wxGBSpan(1, 1), wxALIGN_CENTER_HORIZONTAL | wxLEFT | wxRIGHT, 5);

	MotorTargetIndexGoButton = new wxButton(this, wxID_ANY, _("STOP"), wxDefaultPosition, wxSize(45, 30), 0);
	MotorTargetIndexGoButton->Bind(wxEVT_BUTTON, &MotorControlPanel::OnGoToTargetIndexButtonClicked, this);
	MotorTargetIndexSizer->Add(MotorTargetIndexGoButton, wxGBPosition(0, 1), wxGBSpan(2, 1), wxALL | wxALIGN_CENTER_VERTICAL, 5);

	MotorPositionChoice = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, positionsChoices);
	MotorPositionChoice->SetFont(FONT_EXTRA_SMALL_SEMIBOLD);
	MotorPositionChoice->Bind(wxEVT_CHOICE, &MotorControlPanel::OnMotorPositionSelected, this);
	MotorTargetIndexSizer->Add(MotorPositionChoice, wxGBPosition(2, 0), wxGBSpan(1, 1), wxALL | wxALIGN_CENTER_VERTICAL, 0);


	keyboardSlots.Add("---None---");
	keyboardSlots.Add(KEYBINDING_STR_LEFTRIGHT);
	keyboardSlots.Add(KEYBINDING_STR_UPDOWN);
	keyboardSlots.Add(KEYBINDING_STR_AD);
	keyboardSlots.Add(KEYBINDING_STR_WS);
	AssignKeyboardChoice = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, keyboardSlots);
	AssignKeyboardChoice->Bind(wxEVT_CHOICE, &MotorControlPanel::OnKeyboardChoiceSelected, this);
	AssignKeyboardChoice->SetFont(FONT_EXTRA_SMALL_SEMIBOLD);
	AssignKeyboardChoice->SetToolTip(""
		"Choose a pair of keys to control this motor.\n"
		"Put the cursor in the square text box above to move the motors with the keyboard.");
	MotorTargetIndexSizer->Add(AssignKeyboardChoice, wxGBPosition(3, 0), wxGBSpan(1, 1), wxALL | wxALIGN_CENTER_VERTICAL, 0);

	if (ConfigurationManager::GetInstance().Exists(savedKeyBindingKey)) {
		wxString savedkeyBinding = ConfigurationManager::GetInstance().Get(savedKeyBindingKey);
		if (savedkeyBinding == KEYBINDING_STR_LEFTRIGHT)
			AssignKeyboardChoice->SetSelection(1);
		else if (savedkeyBinding == KEYBINDING_STR_UPDOWN)
			AssignKeyboardChoice->SetSelection(2);
		else if (savedkeyBinding == KEYBINDING_STR_AD)
			AssignKeyboardChoice->SetSelection(3);
		else if (savedkeyBinding == KEYBINDING_STR_WS)
			AssignKeyboardChoice->SetSelection(4);
	}
	else
		AssignKeyboardChoice->SetSelection(0);
	// Hidden by default - only show to factory users
	AssignKeyboardChoice->Hide();


	MotorControlsSizer->Add(MotorTargetIndexSizer, wxGBPosition(1, 0), wxGBSpan(1, 1), wxEXPAND, 5);

	// ------------------------------------------------------------------------------------------
	// Main controls - a current index label, left and right arrow buttons, a slider, and a gauge

	wxFlexGridSizer* MotorSimpleControlsSizer = new wxFlexGridSizer(2, 0, 0, 0);

	MotorSliderPanel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_THEME | wxTAB_TRAVERSAL);
	MotorSliderPanel->SetBackgroundColour(EXTRA_LIGHT_BACKGROUND_COLOR);
	wxBoxSizer* MotorSliderSizer = new wxBoxSizer(wxVERTICAL);

	MotorSlider = new wxSlider(MotorSliderPanel, wxID_ANY, 10000, 10000, 64000, wxDefaultPosition, wxSize(300, -1), wxSL_HORIZONTAL | wxSL_MIN_MAX_LABELS);
	MotorSlider->Bind(wxEVT_SCROLL_THUMBRELEASE, &MotorControlPanel::OnSliderMoved, this);
	MotorSliderSizer->Add(MotorSlider, 0, wxLEFT | wxRIGHT | wxALIGN_CENTER_HORIZONTAL, 5);

	MotorGauge = new wxGauge(MotorSliderPanel, wxID_ANY, 100, wxDefaultPosition, wxSize(198, 5), wxGA_HORIZONTAL | wxGA_SMOOTH);
	MotorSliderSizer->Add(MotorGauge, 0, wxALL | wxALIGN_CENTER_HORIZONTAL, 0);

	MotorSliderPanel->SetSizer(MotorSliderSizer);
	MotorSliderPanel->Layout();
	MotorSliderSizer->Fit(MotorSliderPanel);
	MotorSimpleControlsSizer->Add(MotorSliderPanel, 1, wxEXPAND | wxALL, 0);

	MotorSimpleControls = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);

	wxGridBagSizer* MotorControlsInnerSizer = new wxGridBagSizer(0, 0);

	MotorCCWButton = new wxBitmapButton(MotorSimpleControls, wxID_ANY, wxBitmap(LEFT_ARROW_IMAGE, wxBITMAP_TYPE_ANY),
		wxDefaultPosition, wxSize(40, 30), wxBU_AUTODRAW | 0);
	MotorCCWButton->Bind(wxEVT_LEFT_DOWN, &MotorControlPanel::OnCCWButtonClicked, this);
	MotorCCWButton->Bind(wxEVT_LEFT_UP, &MotorControlPanel::OnCCWButtonClickStopped, this);
	MotorCCWButton->Bind(wxEVT_LEAVE_WINDOW, &MotorControlPanel::OnCCWButtonClickStopped, this);
	MotorControlsInnerSizer->Add(MotorCCWButton, wxGBPosition(0, 0), wxGBSpan(1, 1), wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT, 0);

	MotorCurrentIndexLabel = new wxStaticText(MotorSimpleControls, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(60, -1), wxALIGN_CENTER_HORIZONTAL);
	MotorCurrentIndexLabel->SetFont(FONT_MED_SMALL_SEMIBOLD);
	MotorCurrentIndexLabel->SetBackgroundColour(EXTRA_LIGHT_BACKGROUND_COLOR);
	MotorControlsInnerSizer->Add(MotorCurrentIndexLabel, wxGBPosition(0, 1), wxGBSpan(1, 1), wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT, 5);

	MotorCWButton = new wxBitmapButton(MotorSimpleControls, wxID_ANY, wxBitmap(RIGHT_ARROW_IMAGE, wxBITMAP_TYPE_ANY), wxDefaultPosition, wxSize(40, 30), wxBU_AUTODRAW | 0);
	MotorCWButton->Bind(wxEVT_LEFT_DOWN, &MotorControlPanel::OnCWButtonClicked, this);
	MotorCWButton->Bind(wxEVT_LEFT_UP, &MotorControlPanel::OnCWButtonClickStopped, this);
	MotorCWButton->Bind(wxEVT_LEAVE_WINDOW, &MotorControlPanel::OnCWButtonClickStopped, this);
	MotorControlsInnerSizer->Add(MotorCWButton, wxGBPosition(0, 2), wxGBSpan(1, 1), wxALIGN_CENTER_VERTICAL, 0);

	MotorStepSizeLabel = new wxStaticText(MotorSimpleControls, wxID_ANY, STEP_SIZE_STR, wxDefaultPosition, wxDefaultSize, 0);
	MotorStepSizeLabel->SetFont(FONT_EXTRA_SMALL_SEMIBOLD);
	MotorControlsInnerSizer->Add(MotorStepSizeLabel, wxGBPosition(1, 0), wxGBSpan(1, 1), wxALL | wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL, 0);

	MotorStepSizeTextCtrl = new NumericTextCtrl(MotorSimpleControls, NumericTextCtrlType::DIGITS_ONLY, 5, wxSize(60, 23));
	MotorStepSizeTextCtrl->Bind(wxEVT_TEXT, &MotorControlPanel::OnMotorStepSizeChanged, this);
	MotorStepSizeTextCtrl->SetFont(FONT_VERY_SMALL);
	MotorControlsInnerSizer->Add(MotorStepSizeTextCtrl, wxGBPosition(1, 1), wxGBSpan(1, 1), wxALIGN_CENTER_HORIZONTAL | wxALL, 0);

	MotorStepSizeInfoIcon = new wxStaticBitmap(MotorSimpleControls, wxID_ANY, wxBitmap(INFO_ICON_SMALL, wxBITMAP_TYPE_ANY), wxDefaultPosition, wxDefaultSize, 0);
	MotorStepSizeInfoIcon->SetToolTip(STEP_SIZE_TOOLTIP);
	MotorControlsInnerSizer->Add(MotorStepSizeInfoIcon, wxGBPosition(1, 2), wxGBSpan(1, 1), wxALL | wxALIGN_CENTER_VERTICAL, 0);

	MotorControlsInnerSizer->AddGrowableCol(0);
	MotorControlsInnerSizer->AddGrowableCol(2);

	MotorSimpleControls->SetSizer(MotorControlsInnerSizer);
	MotorSimpleControls->Layout();
	MotorControlsInnerSizer->Fit(MotorSimpleControls);
	MotorSimpleControlsSizer->Add(MotorSimpleControls, 1, wxEXPAND | wxALL, 5);

	MotorControlsSizer->Add(MotorSimpleControlsSizer, wxGBPosition(0, 1), wxGBSpan(2, 1), wxEXPAND, 5);

	//----------------------------------------------------------------------------------------
	// Collapsible settings panel containing more advanced settings - change motor label, redefine current index, etc.

	MotorCollapsibleSettingsPanel = new wxCollapsiblePane(this, wxID_ANY, SETTINGS_STR, wxDefaultPosition, wxDefaultSize, wxCP_DEFAULT_STYLE | wxCP_NO_TLW_RESIZE);
	MotorCollapsibleSettingsPanel->Collapse(true);
	Bind(wxEVT_COLLAPSIBLEPANE_CHANGED, &MotorControlPanel::OnMotorSettingsCollapse, this);

	wxBoxSizer* MotorCollapsibleSettingsSizer = new wxBoxSizer(wxVERTICAL);

	MotorInnerSettingsPanel = new wxPanel(MotorCollapsibleSettingsPanel->GetPane(), wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_THEME | wxTAB_TRAVERSAL);
	MotorInnerSettingsPanel->SetFont(FONT_SMALL_SEMIBOLD);
	MotorInnerSettingsPanel->SetBackgroundColour(FOREGROUND_PANEL_COLOR);

	wxFlexGridSizer* MotorInnerSettingsSizer = new wxFlexGridSizer(0, 2, 0, 0);

	// Motor Label
	MotorLabelLabel = new wxStaticText(MotorInnerSettingsPanel, wxID_ANY, _("Label"), wxDefaultPosition, wxDefaultSize, 0);
	MotorInnerSettingsSizer->Add(MotorLabelLabel, 0, wxALL | wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL, 2);

	MotorLabelTextCtrl = new wxTextCtrl(MotorInnerSettingsPanel, wxID_ANY, wxEmptyString,
		wxDefaultPosition, wxDefaultSize, wxTE_CENTER | wxTE_PROCESS_ENTER);
	MotorLabelTextCtrl->SetMaxLength(8);
	MotorLabelTextCtrl->Bind(wxEVT_TEXT_ENTER, &MotorControlPanel::OnMotorLabelEntered, this);
	MotorInnerSettingsSizer->Add(MotorLabelTextCtrl, 0, wxALL, 2);

	// Redefine current index
	MotorRedefineCurrentIndexLabel = new wxStaticText(MotorInnerSettingsPanel, wxID_ANY, _("Redefine Current Index"), wxDefaultPosition, wxDefaultSize, 0);
	MotorInnerSettingsSizer->Add(MotorRedefineCurrentIndexLabel, 0, wxALL | wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL, 2);

	MotorRedefineCurrentIndexTextCtrl = new wxTextCtrl(MotorInnerSettingsPanel, wxID_ANY, wxEmptyString,
		wxDefaultPosition, wxDefaultSize, wxTE_CENTER | wxTE_PROCESS_ENTER, wxTextValidator(wxFILTER_DIGITS));
	MotorRedefineCurrentIndexTextCtrl->SetMaxLength(5);
	MotorRedefineCurrentIndexTextCtrl->Bind(wxEVT_TEXT_ENTER, &MotorControlPanel::OnRedefineCurrentIndexEntered, this);
	MotorInnerSettingsSizer->Add(MotorRedefineCurrentIndexTextCtrl, 0, wxALL, 2);

	// Min index
	MotorSetMinIndexLabel = new wxStaticText(MotorInnerSettingsPanel, wxID_ANY, _("Minimum Index"), wxDefaultPosition, wxDefaultSize, 0);
	MotorInnerSettingsSizer->Add(MotorSetMinIndexLabel, 0, wxALL | wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL, 2);

	MotorSetMinIndexTextCtrl = new wxTextCtrl(MotorInnerSettingsPanel, wxID_ANY, wxEmptyString,
		wxDefaultPosition, wxDefaultSize, wxTE_CENTER | wxTE_PROCESS_ENTER, wxTextValidator(wxFILTER_DIGITS));
	MotorSetMinIndexTextCtrl->SetMaxLength(5);
	MotorSetMinIndexTextCtrl->Bind(wxEVT_TEXT_ENTER, &MotorControlPanel::OnSetMinIndexEntered, this);
	MotorInnerSettingsSizer->Add(MotorSetMinIndexTextCtrl, 0, wxALL, 2);

	// Max index
	MotorSetMaxIndexLabel = new wxStaticText(MotorInnerSettingsPanel, wxID_ANY, _("Maximum Index"), wxDefaultPosition, wxDefaultSize, 0);
	MotorInnerSettingsSizer->Add(MotorSetMaxIndexLabel, 0, wxALL | wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL, 2);

	MotorSetMaxIndexTextCtrl = new wxTextCtrl(MotorInnerSettingsPanel, wxID_ANY, wxEmptyString,
		wxDefaultPosition, wxDefaultSize, wxTE_CENTER | wxTE_PROCESS_ENTER, wxTextValidator(wxFILTER_DIGITS));
	MotorSetMaxIndexTextCtrl->SetMaxLength(5);
	MotorSetMaxIndexTextCtrl->Bind(wxEVT_TEXT_ENTER, &MotorControlPanel::OnSetMaxIndexEntered, this);
	MotorInnerSettingsSizer->Add(MotorSetMaxIndexTextCtrl, 0, wxALL, 2);

	// Backlash
	MotorBacklashLabel = new wxStaticText(MotorInnerSettingsPanel, wxID_ANY, _("Backlash"), wxDefaultPosition, wxDefaultSize, 0);
	MotorBacklashLabel->SetToolTip(_("REQUIRES RESET"));
	MotorInnerSettingsSizer->Add(MotorBacklashLabel, 0, wxALL | wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL, 2);

	MotorBacklashTextCtrl = new wxTextCtrl(MotorInnerSettingsPanel, wxID_ANY, wxEmptyString,
		wxDefaultPosition, wxDefaultSize, wxTE_CENTER | wxTE_PROCESS_ENTER, wxTextValidator(wxFILTER_DIGITS));
	MotorBacklashTextCtrl->SetMaxLength(5);
	MotorBacklashTextCtrl->SetToolTip(_("REQUIRES RESET"));
	MotorBacklashTextCtrl->Bind(wxEVT_TEXT_ENTER, &MotorControlPanel::OnSetBacklashEntered, this);
	MotorInnerSettingsSizer->Add(MotorBacklashTextCtrl, 0, wxALL, 2);


	MotorInnerSettingsPanel->SetSizer(MotorInnerSettingsSizer);
	MotorInnerSettingsPanel->Layout();
	MotorInnerSettingsSizer->Fit(MotorInnerSettingsPanel);
	MotorCollapsibleSettingsSizer->Add(MotorInnerSettingsPanel, 0, wxALL, 5);

	MotorCollapsibleSettingsPanel->GetPane()->SetSizer(MotorCollapsibleSettingsSizer);
	MotorCollapsibleSettingsPanel->GetPane()->Layout();
	MotorCollapsibleSettingsSizer->Fit(MotorCollapsibleSettingsPanel->GetPane());
	MotorControlsSizer->Add(MotorCollapsibleSettingsPanel, wxGBPosition(0, 2), wxGBSpan(2, 1), wxEXPAND | wxALL, 0);

	this->SetBackgroundColour(FOREGROUND_PANEL_COLOR);
	this->SetSizer(MotorControlsSizer);
	this->Layout();


	Init();
}

//bool MotorControlPanel::ShouldBeVisibleToUser() {
//	bool hasAnyIndexedPositions = lc->GetMotorIndexedPositions(motorId).size() > 0;
//	bool notLinked = !lc->IsLinkedMotor(motorId); // Don't show twice if linked motor, will already show up as X-Y with other motor.
//	bool motorAccessGranted = AccessManager::GetInstance().GetCurrentAccessCodeType() == AccessCodeType::MOTOR;
//	return (hasAnyIndexedPositions and notLinked) or motorAccessGranted;
//}

void MotorControlPanel::Init() {
	//moveCausedByButton = false;

	//MotorLabel->SetLabelText(lc->GetMotorLabel(motorId));
	MotorTitle->SetTitle(lc->GetMotorLabel(motorId));

	int currentIndex = lc->GetMotorIndex(motorId);
	minIndex = lc->GetMotorMinIndex(motorId);
	maxIndex = lc->GetMotorMaxIndex(motorId);
	MotorSlider->SetValue(currentIndex);
	MotorSlider->SetMin(minIndex);
	MotorSlider->SetMax(maxIndex);
	MotorGauge->SetRange(maxIndex - minIndex);
	MotorGauge->SetValue(currentIndex - minIndex);
	MotorCurrentIndexLabel->SetLabelText(to_wx_string(currentIndex));
	MotorTargetIndexTextCtrl->SetValue(to_wx_string(currentIndex));

	if (ConfigurationManager::GetInstance().Exists(savedStepSizeKey))
		MotorStepSizeTextCtrl->SetValue(to_wx_string(ConfigurationManager::GetInstance().Get(savedStepSizeKey)));
	else
		MotorStepSizeTextCtrl->SetValue(to_wx_string(lc->GetAutotuneMotorStepSize()));


	MotorLabelTextCtrl->SetValue(lc->GetMotorLabel(motorId));
	MotorRedefineCurrentIndexTextCtrl->SetValue(to_wx_string(currentIndex));
	MotorSetMinIndexTextCtrl->SetValue(to_wx_string(minIndex));
	MotorSetMaxIndexTextCtrl->SetValue(to_wx_string(maxIndex));
	MotorBacklashTextCtrl->SetValue(to_wx_string(lc->GetMotorBacklash(motorId)));
	RefreshPositionsChoices();
	RefreshCurrentPosition();
	RefreshVisibility();
}


void MotorControlPanel::RefreshStrings() {
	MotorTargetIndexLabel->SetLabelText(_(GO_TO_INDEX_STR));
	MotorStepSizeLabel->SetLabelText(_(STEP_SIZE_STR));
	MotorStepSizeInfoIcon->SetToolTip(_(STEP_SIZE_TOOLTIP));
	MotorCollapsibleSettingsPanel->SetToolTip(_(SETTINGS_STR));
}


void MotorControlPanel::RefreshAll() {
	RefreshControlEnabled();

	RefreshGoToTargetButton();
	RefreshCurrentIndexDependentWidgets();
	RefreshCurrentPosition();

	if (lc->MotorIsMoving(motorId))
		motorRefreshCountdown = 5;

	if (motorRefreshCountdown > 0) {
		motorRefreshCountdown--;
		lc->RefreshMotorIndexReading(motorId);
		YieldToApp();
	}

	CheckIfCantMoveMotorDueToLDDCurrentLimit();

}

//void MotorControlPanel::RefreshVisibility() {
//	// When to show entire motor control panel
//	// - Service or factory mode or has indexing positions or user entered motor access code
//
//	bool showThisPanel = 
//		IsInAccessMode(GuiAccessMode::SERVICE) or 
//		IsInAccessMode(GuiAccessMode::FACTORY) or
//		ShouldBeVisibleToUser();
//
//	SetVisibilityBasedOnCondition(this, showThisPanel);
//
//
//	// When to show basic motor controls
//	// - Service or factory mode or user entered motor access code
//
//	bool showBasicMotorControls = 
//		IsInAccessMode(GuiAccessMode::SERVICE) or 
//		IsInAccessMode(GuiAccessMode::FACTORY) or 
//		AccessManager::GetInstance().GetCurrentAccessCodeType() == AccessCodeType::MOTOR;
//
//	SetVisibilityBasedOnCondition(MotorTargetIndexLabel, showBasicMotorControls);
//	SetVisibilityBasedOnCondition(MotorTargetIndexTextCtrl, showBasicMotorControls);
//	SetVisibilityBasedOnCondition(MotorTargetIndexGoButton, showBasicMotorControls);
//	SetVisibilityBasedOnCondition(MotorSliderPanel, showBasicMotorControls);
//	SetVisibilityBasedOnCondition(MotorSimpleControls, showBasicMotorControls);
//
//
//	// When to show motor settings
//	// - Service or factory mode
//
//	SetVisibilityBasedOnAccessMode(MotorCollapsibleSettingsPanel, GuiAccessMode::SERVICE);
//	SetVisibilityBasedOnAccessMode(AssignKeyboardChoice, GuiAccessMode::FACTORY);
//
//	EmitSizeEvent(this);
//}


void MotorControlPanel::RefreshVisibility() {

	//-------------------------------------------------------------------------
	// When to show entire motor control panel

	bool endUserMode = IsInAccessMode(GuiAccessMode::END_USER);
	bool serviceMode = IsInAccessMode(GuiAccessMode::SERVICE);
	bool factoryMode = IsInAccessMode(GuiAccessMode::FACTORY);
	bool motorAccessGranted = AccessManager::GetInstance().GetCurrentAccessCodeType() == AccessCodeType::MOTOR;
	bool hasIndexPositions = lc->GetMotorIndexedPositions(motorId).size() > 0;
	bool notLinked = !lc->IsLinkedMotor(motorId); 
	bool isDMDH = lc->IsDMDualHead();
	bool isAttenuator = lc->MotorIsAttenuator(motorId);

	// Always show panel if there are index positions.
	// User should be able to change index positions themselves.
	// However, if this is one of two linked motors, don't show the index
	// positions of this individual motor. It will already show up as part of
	// a separate MotorControlPanelLinked panel.
	if (hasIndexPositions and notLinked) {
		SetVisibilityBasedOnCondition(this, true);
	}
	// Show panel if this is one of the output alignment motors on DM Dual Head.
	// (X_near, X_far, Y_near, Y_far)
	// User should be able to manually adjust motors for beam pointing.
	else if (isDMDH and !isAttenuator) {
		SetVisibilityBasedOnCondition(this, true);
	}
	// Show panel if this is motorized end-mirror (i.e., not a position indexable
	// motor) AND the user enters a motor access code.
	// For manual motor adjustment by user or FSE to correct laser misalignment.
	else if (!hasIndexPositions and motorAccessGranted) {
		SetVisibilityBasedOnCondition(this, true);
	}
	else if (factoryMode) {
		SetVisibilityBasedOnCondition(this, true);
	}
	else {
		SetVisibilityBasedOnCondition(this, false);
	}



	/*bool showThisPanel =
		IsInAccessMode(GuiAccessMode::SERVICE) or
		IsInAccessMode(GuiAccessMode::FACTORY) or
		ShouldBeVisibleToUser();

	SetVisibilityBasedOnCondition(this, showThisPanel);

	return (hasAnyIndexedPositions and notLinked) or motorAccessGranted;*/


	//-------------------------------------------------------------------------
	// When to show individual motor controls
	// - Service or factory mode or user entered motor access code

	/*bool showBasicMotorControls =
		IsInAccessMode(GuiAccessMode::SERVICE) or
		IsInAccessMode(GuiAccessMode::FACTORY) or
		AccessManager::GetInstance().GetCurrentAccessCodeType() == AccessCodeType::MOTOR;*/

	// Only show basic motor indexing controls for end mirrors, and only for
	// DM Dual Head or if motor access code has been entered.
	bool showBasicMotorControls = (isDMDH and !isAttenuator) or (!hasIndexPositions and motorAccessGranted) or factoryMode;

	SetVisibilityBasedOnCondition(MotorTargetIndexLabel, showBasicMotorControls);
	SetVisibilityBasedOnCondition(MotorTargetIndexTextCtrl, showBasicMotorControls);
	SetVisibilityBasedOnCondition(MotorTargetIndexGoButton, showBasicMotorControls);
	SetVisibilityBasedOnCondition(MotorSliderPanel, showBasicMotorControls);
	SetVisibilityBasedOnCondition(MotorSimpleControls, showBasicMotorControls);


	// Only show position choice if this motor has non-zero index positions set.
	bool showPositionChoice = (hasIndexPositions and notLinked) or factoryMode;
	SetVisibilityBasedOnCondition(MotorPositionChoice, showPositionChoice);


	// Only show these things in factory mode
	SetVisibilityBasedOnAccessMode(MotorCollapsibleSettingsPanel, GuiAccessMode::FACTORY);
	SetVisibilityBasedOnAccessMode(AssignKeyboardChoice, GuiAccessMode::FACTORY);

	EmitSizeEvent(this);
}


void MotorControlPanel::RefreshControlEnabled() {
	RefreshMotorControlsEnabled();
}

void MotorControlPanel::RefreshCurrentIndexDependentWidgets() {
	int currentIndex = lc->GetMotorIndex(motorId);
	MotorCurrentIndexLabel->SetLabelText(to_wx_string(currentIndex));
	MotorGauge->SetValue(currentIndex - minIndex);
}

void MotorControlPanel::RefreshGoToTargetButton() {
	bool isMoving = lc->MotorIsMoving(motorId);
	SetBGColorBasedOnCondition(MotorTargetIndexGoButton, isMoving, TEXT_COLOR_RED, BUTTON_COLOR_INACTIVE);
	SetTextBasedOnCondition(MotorTargetIndexGoButton, isMoving, _("STOP"), _("GO"));
}

void MotorControlPanel::RefreshMotorControlsEnabled() {
	bool autotuning = lc->IsAutotuneRunning();
	bool running = lc->LaserIsRunning();

	bool allowMotorControl = running and !autotuning;

	RefreshWidgetEnableBasedOnCondition(MotorTargetIndexTextCtrl, allowMotorControl);
	RefreshWidgetEnableBasedOnCondition(MotorTargetIndexGoButton, allowMotorControl);
	RefreshWidgetEnableBasedOnCondition(MotorSlider, allowMotorControl);
	RefreshWidgetEnableBasedOnCondition(MotorCCWButton, allowMotorControl);
	RefreshWidgetEnableBasedOnCondition(MotorCWButton, allowMotorControl);
	RefreshWidgetEnableBasedOnCondition(MotorStepSizeTextCtrl, allowMotorControl);
	RefreshWidgetEnableBasedOnCondition(MotorPositionChoice, allowMotorControl);
}

void MotorControlPanel::ResetSliderPosition() {
	// Use this to re-position the slider at the current motor index. Useful when another function changes motor position, such as Autotune.
	MotorSlider->SetValue(lc->GetMotorIndex(motorId));
}

void MotorControlPanel::RefreshCurrentPosition() {
	vector<int> indexedPositions = lc->GetMotorIndexedPositions(motorId);
	int currentPosition = lc->GetMotorPosition(motorId);
	int positionIndex = 0;
	for (int i = 0; i < int(indexedPositions.size()); i++) {
		if (indexedPositions[i] == currentPosition) {
			MotorPositionChoice->SetSelection(i);
			break;
		}
	}
}

void MotorControlPanel::RefreshPositionsChoices() {
	positionsChoices.Clear();
	for (int position : lc->GetMotorIndexedPositions(motorId)) {
		int hours = lc->GetMotorPositionHours(motorId, position);
		wxString positionLabel = wxString::Format("P%d - %d hours", position, hours);
		positionsChoices.Add(positionLabel);
	}
	MotorPositionChoice->Set(positionsChoices);
}


void MotorControlPanel::OnTargetIndexEntered(wxCommandEvent& evt) {
	if (CheckIfCantMoveMotorDueToLDDCurrentLimit(true))
		return;

	STAGE_ACTION_("Motor target index entered", string(MotorTargetIndexTextCtrl->GetValue()))
	GoToTargetIndex();
	LOG_ACTION()
}


void MotorControlPanel::OnGoToTargetIndexButtonClicked(wxCommandEvent& evt) {
	if (CheckIfCantMoveMotorDueToLDDCurrentLimit(true))
		return;

	STAGE_ACTION_("Motor go to target index button clicked", string(MotorTargetIndexTextCtrl->GetValue()))
	// If clicked on while in STOP state (while motor is moving), just stop motor and set slider to current position
	if (lc->MotorIsMoving(motorId)) {
		lc->StopMotor(motorId);
		lc->RefreshMotorIndexReading(motorId);
		MotorSlider->SetValue(lc->GetMotorIndex(motorId));
	}
	// If clicked on while in GO state (motor is not moving):
	else {
		GoToTargetIndex();
	}
	RefreshGoToTargetButton();
	LOG_ACTION()
}


void MotorControlPanel::OnSliderMoved(wxScrollEvent& evt) {
	if (CheckIfCantMoveMotorDueToLDDCurrentLimit(true))
		return;

	STAGE_ACTION("Motor index slider moved")
	int targetIndex = MotorSlider->GetValue();
	STAGE_ACTION_ARGUMENTS(to_string(targetIndex))
	int targetIndexRounded = roundMotorIndex(targetIndex);
	MotorSlider->SetValue(targetIndexRounded);
	lc->MoveMotorToIndex(motorId, targetIndexRounded);
	MotorTargetIndexTextCtrl->SetLabelText(to_wx_string(targetIndexRounded));
	RefreshGoToTargetButton();
	LOG_ACTION()
}


void MotorControlPanel::OnCCWButtonClicked(wxMouseEvent& evt) {
	if (CheckIfCantMoveMotorDueToLDDCurrentLimit(true))
		return;

	STAGE_ACTION_("Motor CCW button clicked", to_string(GetStepSize()))
	MoveCCW();
	LOG_ACTION()
}


void MotorControlPanel::OnCCWButtonClickStopped(wxMouseEvent& evt) {
	STAGE_ACTION_("Motor CCW button click stopped", to_string(GetStepSize()))
	StopMoving();
	movingMotor = false;
	LOG_ACTION()
}


void MotorControlPanel::OnCWButtonClicked(wxMouseEvent& evt) {
	if (CheckIfCantMoveMotorDueToLDDCurrentLimit(true))
		return;

	STAGE_ACTION_("Motor CW button clicked", to_string(GetStepSize()))
	MoveCW();
	LOG_ACTION()
}


void MotorControlPanel::OnCWButtonClickStopped(wxMouseEvent& evt) {
	STAGE_ACTION_("Motor CW button click stopped", to_string(GetStepSize()))
	StopMoving();
	movingMotor = false;
	LOG_ACTION()
}


void MotorControlPanel::OnMotorStepSizeChanged(wxCommandEvent& evt) {
	ConfigurationManager::GetInstance().Set(savedStepSizeKey, to_string(GetStepSize()));
}


int MotorControlPanel::GetKeyboardSlotSelection() {
	return AssignKeyboardChoice->GetSelection();
}

void MotorControlPanel::MoveCCW() {
	if (CheckIfCantMoveMotorDueToLDDCurrentLimit(true))
		return;

	if (lc->GetMotorIndex(motorId) != minIndex and !movingMotor) {
		if (GetStepSize() == 0) {
			lc->MoveMotorToIndex(motorId, minIndex + lc->GetMotorBacklash(motorId) + 1);
			MotorSlider->SetValue(MotorSlider->GetMin() + 1);
		}
		else {
			lc->TurnMotorCounterclockwise(motorId, GetStepSize());
			MotorSlider->SetValue(MotorSlider->GetValue() - GetStepSize());
		}
		RefreshGoToTargetButton();
		movingMotor = true;
	}
}

void MotorControlPanel::MoveCW() {
	if (CheckIfCantMoveMotorDueToLDDCurrentLimit(true))
		return;

	if (lc->GetMotorIndex(motorId) != maxIndex and !movingMotor) {
		if (GetStepSize() == 0) {
			lc->MoveMotorToIndex(motorId, maxIndex);
			MotorSlider->SetValue(MotorSlider->GetMax());
		}
		else {
			lc->TurnMotorClockwise(motorId, GetStepSize());
			MotorSlider->SetValue(MotorSlider->GetValue() + GetStepSize());
		}
		RefreshGoToTargetButton();
		movingMotor = true;
	}
}


void MotorControlPanel::StopMoving() {
	if (GetStepSize() == 0) {
		lc->StopMotor(motorId);
		MotorSlider->SetValue(lc->GetMotorIndex(motorId));
	}
	RefreshGoToTargetButton();

}


// Controlled by MotorSettingsPage
void MotorControlPanel::OnKeyDown(wxKeyEvent& evt) {

	STAGE_ACTION_("Moving motor by key", to_string(motorId));

	if (AssignKeyboardChoice->GetSelection() == 1) { // Left-Right
		if (evt.GetKeyCode() == WXK_LEFT)
			MoveCCW();
		else if (evt.GetKeyCode() == WXK_RIGHT)
			MoveCW();
	}
	else if (AssignKeyboardChoice->GetSelection() == 2) { // Up-Down
		if (evt.GetKeyCode() == WXK_DOWN)
			MoveCCW();
		else if (evt.GetKeyCode() == WXK_UP)
			MoveCW();
	}
	else if (AssignKeyboardChoice->GetSelection() == 3) { // A-D
		if (evt.GetKeyCode() == (int)'a' or evt.GetKeyCode() == (int)'A')
			MoveCCW();
		else if (evt.GetKeyCode() == (int)'d' or evt.GetKeyCode() == (int)'D')
			MoveCW();
	}
	else if (AssignKeyboardChoice->GetSelection() == 4) { // W-S
		if (evt.GetKeyCode() == (int)'s' or evt.GetKeyCode() == (int)'S')
			MoveCCW();
		else if (evt.GetKeyCode() == (int)'w' or evt.GetKeyCode() == (int)'W')
			MoveCW();
	}

	LOG_ACTION();
}

// Controlled by MotorSettingsPage
void MotorControlPanel::OnKeyUp(wxKeyEvent& evt) {
	STAGE_ACTION_("Moving motor by key stopped", to_string(motorId));
	StopMoving();
	movingMotor = false;
	LOG_ACTION();
}


void MotorControlPanel::OnMotorLabelEntered(wxCommandEvent& evt) {
	STAGE_ACTION("Motor label entered")
		string newLabel = string(MotorLabelTextCtrl->GetValue());
	STAGE_ACTION_ARGUMENTS(newLabel)
		lc->SetMotorLabel(motorId, newLabel);
	string newLabelConfirmed = lc->GetMotorLabel(motorId);
	MotorLabelTextCtrl->SetValue(newLabelConfirmed);

	//MotorLabel->SetLabelText(newLabelConfirmed);
	MotorTitle->SetTitle(newLabelConfirmed);

	LOG_ACTION()
}

void MotorControlPanel::OnRedefineCurrentIndexEntered(wxCommandEvent& evt) {
	STAGE_ACTION("Motor redefine current index extered")
		int newCurrentIndex = wxAtoi(MotorRedefineCurrentIndexTextCtrl->GetValue());
	STAGE_ACTION_ARGUMENTS(to_string(newCurrentIndex))
		newCurrentIndex = min(max(newCurrentIndex, minIndex), maxIndex);
	MotorRedefineCurrentIndexTextCtrl->SetValue(to_wx_string(newCurrentIndex));
	lc->RedefineCurrentMotorIndex(motorId, newCurrentIndex);
	RefreshCurrentIndexDependentWidgets();
	MotorSlider->SetValue(newCurrentIndex);
	LOG_ACTION()
}

void MotorControlPanel::OnSetMinIndexEntered(wxCommandEvent& evt) {
	STAGE_ACTION("Motor set min index extered")
		int newMinIndex = wxAtoi(MotorSetMinIndexTextCtrl->GetValue());
	STAGE_ACTION_ARGUMENTS(to_string(newMinIndex))
		lc->SetMotorMinIndex(motorId, newMinIndex);
	minIndex = lc->GetMotorMinIndex(motorId);
	MotorSetMinIndexTextCtrl->SetValue(to_wx_string(minIndex));
	MotorSlider->SetMin(minIndex);
	MotorGauge->SetRange(maxIndex - minIndex);
	LOG_ACTION()
}

void MotorControlPanel::OnSetMaxIndexEntered(wxCommandEvent& evt) {
	STAGE_ACTION("Motor set max index extered")
		int newMaxIndex = wxAtoi(MotorSetMaxIndexTextCtrl->GetValue());
	STAGE_ACTION_ARGUMENTS(to_string(newMaxIndex))
		lc->SetMotorMaxIndex(motorId, newMaxIndex);
	maxIndex = lc->GetMotorMaxIndex(motorId);
	MotorSetMaxIndexTextCtrl->SetValue(to_wx_string(maxIndex));
	MotorSlider->SetMax(maxIndex);
	MotorGauge->SetRange(maxIndex - minIndex);
	LOG_ACTION()
}

void MotorControlPanel::OnSetBacklashEntered(wxCommandEvent& evt) {
	STAGE_ACTION("Motor set backlash extered")
		int newBacklash = wxAtoi(MotorBacklashTextCtrl->GetValue());
	STAGE_ACTION_ARGUMENTS(to_string(newBacklash))
		lc->SetMotorBacklash(motorId, newBacklash);
	LOG_ACTION()
}


void MotorControlPanel::OnMotorPositionSelected(wxCommandEvent& evt) {
	int positionIndex = evt.GetInt();
	int position = lc->GetMotorIndexedPositions(motorId)[positionIndex];
	lc->SetMotorPosition(motorId, position);
	RefreshCurrentPosition();
}


void MotorControlPanel::OnKeyboardChoiceSelected(wxCommandEvent& evt) {
	std::string choice = std::string(AssignKeyboardChoice->GetString(AssignKeyboardChoice->GetSelection()));
	ConfigurationManager::GetInstance().Set(savedKeyBindingKey, choice);
	evt.Skip(); // Pass up to Motor Settings Page so it can put the cursor in the text box
}


void MotorControlPanel::OnMotorSettingsCollapse(wxCollapsiblePaneEvent& event) {
	STAGE_ACTION("Motor settings collapse button clicked")
		this->Layout();
	MotorControlsSizer->Fit(this);
	this->GetParent()->Layout();
	LOG_ACTION()
}

void MotorControlPanel::GoToTargetIndex() {
	// Stop motor if currently moving
	if (lc->MotorIsMoving(motorId))
		lc->StopMotor(motorId);

	int targetIndex = wxAtoi(MotorTargetIndexTextCtrl->GetValue());
	int constrainedtargetIndex = min(max(targetIndex, minIndex), maxIndex);
	lc->MoveMotorToIndex(motorId, constrainedtargetIndex);
	MotorSlider->SetValue(constrainedtargetIndex);
	MotorTargetIndexTextCtrl->SetValue(to_wx_string(constrainedtargetIndex));

	RefreshGoToTargetButton();
}


int MotorControlPanel::GetStepSize() {
	return MotorStepSizeTextCtrl->GetIntValue();
}


bool MotorControlPanel::CheckIfCantMoveMotorDueToLDDCurrentLimit(bool show_message_box) {
	if (!lc->CanMoveMotorWithLDDCurrentLimit(motorId)) {

		int linkedLddID = lc->GetMotorLDDLinkID(motorId);
		wxString linkedLddLabel = lc->GetLDDLabel(linkedLddID);
		float currentLimit = lc->GetMotorLDDCurrentLimit(motorId);

		wxString warning = _("Can't move motor due to LDD current limit");
		warning += wxString::Format(" - %s (%.2fA). ", linkedLddLabel, currentLimit);
		warning += wxString::Format(_("Lower %s set current below %.2fA before moving motor."), linkedLddLabel, currentLimit);

		MotorTitle->ShowWarning(warning);
		if (show_message_box)
			wxMessageBox(warning, _("Warning"));

		return true;
	}
	else {
		MotorTitle->HideWarning();
		return false;
	}
}
