#include "MotorSettingsPage.h"
#include "..\CommonFunctions_GUI.h"
#include "../AccessCodeDialog.h"

using namespace std;


const wxString GENERATE_MOTOR_ACCESS_KEY_STR = _("Generate Motor Access Key");
const wxString MOTOR_ACCESS_KEY_MESSAGE_STR = _(
	"Send this code to a Photonics representative\n"
	"to request a temporary access code."
);


MotorSettingsPage::MotorSettingsPage(
	shared_ptr<MainLaserControllerInterface> _lc,
	wxWindow* parent
) :
	SettingsPage_Base(_lc, parent) {

	sizer = new wxBoxSizer(wxVERTICAL);

	getMotorAccessKeyButton = new wxBitmapButton(this, wxID_ANY, wxBitmap(KEY_ICON, wxBITMAP_TYPE_ANY),
		wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW | 0);
	getMotorAccessKeyButton->Bind(wxEVT_BUTTON, &MotorSettingsPage::OnGetMotorAccessKeyButtonClicked, this);
	getMotorAccessKeyButton->SetToolTip(_(GENERATE_MOTOR_ACCESS_KEY_STR));
	sizer->Add(getMotorAccessKeyButton, 0, wxALL, 2);


	// 2/15/24 - JB - hack for allowing user to control multiple motors
	// with keyboard keys at the same time - need to put the focus somewhere
	// so it can receive keyboard events. 
	boxForKeyboardMotorControl = new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxSize(24, 24));
	boxForKeyboardMotorControl->Bind(wxEVT_KEY_DOWN, &MotorSettingsPage::OnKeyDown, this);
	boxForKeyboardMotorControl->Bind(wxEVT_KEY_UP, &MotorSettingsPage::OnKeyUp, this);
	sizer->Add(boxForKeyboardMotorControl, 0, wxALL, 5);

	Bind(wxEVT_CHOICE, &MotorSettingsPage::OnKeyboardChoiceSelected, this);


	Init();
}


//bool MotorSettingsPage::ShouldBeVisibleToUser() {
//	bool anyPanelShouldBeVisible = false;
//	for (auto& motorPanel : motorPanels) {
//		if (motorPanel->ShouldBeVisibleToUser())
//			anyPanelShouldBeVisible = true;
//	}
//	for (auto& linkedMotorPanel : linkedMotorPanels) {
//		if (linkedMotorPanel->ShouldBeVisibleToUser())
//			anyPanelShouldBeVisible = true;
//	}
//	return anyPanelShouldBeVisible;
//}


void MotorSettingsPage::Init() {

	motorSequencerPanel = new MotorSequencerPanel(lc, this);
	sizer->Add(motorSequencerPanel, 0, wxALL, 5);


	// Individual motor control panels

	motorPanelsSizer = new wxBoxSizer(wxVERTICAL);

	for (auto& id : lc->GetMotorIDs()) {
		MotorControlPanel* motorPanel = new MotorControlPanel(lc, id, this);
		motorPanelsSizer->Add(motorPanel, 0, wxALL, 5);
		motorPanels.push_back(motorPanel);
	}

	sizer->Add(motorPanelsSizer);


	// Linked motor control panels

	linkedMotorPanelsSizer = new wxBoxSizer(wxVERTICAL);

	for (auto& id : lc->GetLinkedMotorIDs()) {
		MotorControlPanelLinked* linkedMotorPanel = new MotorControlPanelLinked(lc, id, this);
		linkedMotorPanelsSizer->Add(linkedMotorPanel, 0, wxALL, 5);
		linkedMotorPanels.push_back(linkedMotorPanel);
	}

	sizer->Add(linkedMotorPanelsSizer);


	SetSizer(sizer);
	Layout();
	sizer->Fit(this);

}

void MotorSettingsPage::RefreshAll() {
	// TODO: swtich to this way of refreshing readings because it doesn't slow down GUI
	/*if (IsShownOnScreen()) {
		lc->PrioritizeMotorRefresh(true);
	}
	else {
		lc->PrioritizeMotorRefresh(false);
	}*/

	static int skipRefreshCounter = 0;
	skipRefreshCounter++;
	if (skipRefreshCounter % 2 == 0) {
		lc->RefreshMotorReadings();
	}

	motorSequencerPanel->RefreshAll();
	for (auto& motorPanel : motorPanels)
		motorPanel->RefreshAll();
	for (auto& linkedMotorPanel : linkedMotorPanels)
		linkedMotorPanel->RefreshAll();

}

void MotorSettingsPage::RefreshStrings() {
	SetName(_(MOTOR_STR));
	getMotorAccessKeyButton->SetToolTip(_(GENERATE_MOTOR_ACCESS_KEY_STR));

	for (auto& motorPanel : motorPanels)
		motorPanel->RefreshStrings();
	for (auto& linkedMotorPanel : linkedMotorPanels)
		linkedMotorPanel->RefreshStrings();
}

void MotorSettingsPage::RefreshVisibility() {
	for (auto& motorPanel : motorPanels)
		motorPanel->RefreshVisibility();
	for (auto& linkedMotorPanel : linkedMotorPanels)
		linkedMotorPanel->RefreshVisibility();

	SetVisibilityBasedOnAccessMode(motorSequencerPanel, GuiAccessMode::FACTORY);
	SetVisibilityBasedOnAccessMode(boxForKeyboardMotorControl, GuiAccessMode::FACTORY);
}

void MotorSettingsPage::RefreshControlEnabled() {
}

void MotorSettingsPage::RefreshSlidersPosition() {
	for (auto& motorPanel : motorPanels)
		motorPanel->ResetSliderPosition();
}


void MotorSettingsPage::OnGetMotorAccessKeyButtonClicked(wxCommandEvent& evt) {
	STAGE_ACTION("Generate motor access key")

	wxString code = GenerateOfflinePartialKey(AccessCodeType::MOTOR);
	AccessCodeDialog accessCodeDialog(this, code, _(MOTOR_ACCESS_KEY_MESSAGE_STR));
	accessCodeDialog.ShowModal();

	LOG_ACTION()
}


void MotorSettingsPage::OnKeyboardChoiceSelected(wxCommandEvent& evt) {
	boxForKeyboardMotorControl->SetFocus();
}


void MotorSettingsPage::OnKeyDown(wxKeyEvent& evt) {
	for (auto& m : motorPanels) {
		m->OnKeyDown(evt);
	}
}

void MotorSettingsPage::OnKeyUp(wxKeyEvent& evt) {
	for (auto& m : motorPanels) {
		m->OnKeyUp(evt);
	}
}

