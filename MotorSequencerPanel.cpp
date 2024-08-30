#include "../CommonFunctions_GUI.h"
#include "MotorSequencerPanel.h"

using namespace std;

static const wxString TITLE = _("Motor Sequencer");
static const wxString TOOLTIP = _(
	"Motor Sequencer - Factory tool for testing motors\n"
	"\n"
	"1. Select the motors you wish to include in the test.\n"
	"2. Choose the motor indexing distance.\n"
	"3. Click the Start button.\n"
	"\n"
	"All the motors will simultaneously move CW by the specified\n"
	"distance, then CCW by twice that distance, then CW by the same\n"
	"distance, and should then end up where it started.\n"
	"Press Cancel at any time to stop all motors."
);

static const wxString SELECT_MOTORS_TO_INCLUDE_STR = _("Select motors to include:");
static const wxString DISTANCE_STR = _("Distance");


MotorSequencerPanel::MotorSequencerPanel(std::shared_ptr<MainLaserControllerInterface> laser_controller, wxWindow* parent) :
	wxPanel(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL | wxBORDER_THEME) {

	lc = laser_controller;
	sequencer = make_shared<MotorMovementSequencerManager>(laser_controller);

	SetFont(FONT_VERY_SMALL_SEMIBOLD);
	SetBackgroundColour(FOREGROUND_PANEL_COLOR);

	wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);

	title = new FeatureTitle(this, _(TITLE), _(TOOLTIP));
	sizer->Add(title, 0, wxALL | wxALIGN_CENTER_HORIZONTAL, 5);

	// Horizontal sizer containing left and right groups of controls
	wxBoxSizer* mainControlsSizer = new wxBoxSizer(wxHORIZONTAL);

	// Left static box sizer containing checkboxes with title label
	includeSizerLabelled = new wxStaticBoxSizer(new wxStaticBox(this, wxID_ANY, _(SELECT_MOTORS_TO_INCLUDE_STR)), wxVERTICAL);

	// Flex Grid Sizer inside left sizer for listing motors to include in a 2-column grid
	includeSizer = new wxFlexGridSizer(0, 2, 0, 0);

	InitMotorIncludeCheckboxes();

	includeSizerLabelled->Add(includeSizer, 0, wxALIGN_CENTER_HORIZONTAL, 5);

	mainControlsSizer->Add(includeSizerLabelled, 1, wxEXPAND, 5);


	wxBoxSizer* runningControlsSizer = new wxBoxSizer(wxVERTICAL);

	distanceLabel = new wxStaticText(this, wxID_ANY, DISTANCE_STR, wxDefaultPosition, wxDefaultSize, 0);
	runningControlsSizer->Add(distanceLabel, 0, wxALIGN_CENTER_HORIZONTAL | wxTOP, 5);

	distanceTextCtrl = new NumericTextCtrl(this, NumericTextCtrlType::DIGITS_ONLY, 5, wxSize(60, -1));
	distanceTextCtrl->SetLabelText(to_wx_string(sequencer->GetDistance()));
	runningControlsSizer->Add(distanceTextCtrl, 0, wxALIGN_CENTER_HORIZONTAL | wxBOTTOM, 5);

	startButton = new wxButton(this, wxID_ANY, START_TEXT, wxDefaultPosition, wxDefaultSize, 0);
	startButton->SetBackgroundColour(PHOTONICS_TURQUOISE_COLOR);
	startButton->Bind(wxEVT_BUTTON, &MotorSequencerPanel::OnStartButtonClicked, this);

	runningControlsSizer->Add(startButton, 0, wxALL, 5);


	mainControlsSizer->Add(runningControlsSizer, 0, 0, 5);


	sizer->Add(mainControlsSizer, 1, wxEXPAND, 5);

	message = new DynamicStatusMessage(this, wxEmptyString, -1, 2, 3);

	sizer->Add(message, 0, wxALL | wxALIGN_CENTER_HORIZONTAL, 5);


	this->SetSizer(sizer);
	this->Layout();
	sizer->Fit(this);

}


void MotorSequencerPanel::RefreshAll() {
	RefreshStartButtonState();
	RefreshMessage();
}

void MotorSequencerPanel::RefreshVisibilityBasedOnAccessMode() {
}

void MotorSequencerPanel::RefreshStrings() {
	title->RefreshStrings();
	SetText(distanceLabel, _(DISTANCE_STR));
	SetText(startButton, _(START_TEXT));
	SetText(includeSizerLabelled->GetStaticBox(), _(SELECT_MOTORS_TO_INCLUDE_STR));
}


void MotorSequencerPanel::InitMotorIncludeCheckboxes() {
	includeSizer->Clear(true);
	mapIDToCheckbox.clear();

	for (int motorID : lc->GetMotorIDs()) {

		wxCheckBox* motorIncludeCheckbox = new wxCheckBox(this, wxID_ANY,
			to_wx_string(lc->GetMotorLabel(motorID)),
			wxDefaultPosition, wxDefaultSize, 0);

		includeSizer->Add(motorIncludeCheckbox, 0, wxALL, 5);

		mapIDToCheckbox[motorID] = motorIncludeCheckbox;
	}
}


void MotorSequencerPanel::RefreshStartButtonState() {
	SetTextBasedOnCondition(startButton, sequencer->IsRunning(), _(CANCEL_TEXT), _(START_TEXT));
	SetBGColorBasedOnCondition(startButton, sequencer->IsRunning(), TEXT_COLOR_RED, BUTTON_COLOR_INACTIVE);
}


void MotorSequencerPanel::RefreshMessage() {
	wxString newMessage;
	if (sequencer->IsError())
		newMessage = _(sequencer->GetErrorMessage());
	else if (sequencer->FinishedSuccessfully())
		newMessage = _("Done");

	if (!message->GetLabelText().StartsWith(newMessage)) { // Use StartsWith(_) instead of == to allow for CycleMessageDots to work properly
		SetText(message, newMessage);
		EmitSizeEvent(this);
		Layout();
		Refresh();
	}

	CycleMessageOnCondition(message, sequencer->IsRunning());
}


void MotorSequencerPanel::OnStartButtonClicked(wxCommandEvent& evt) {
	if (sequencer->IsRunning()) {
		sequencer->Stop();
		message->Set(_("Canceled"));
	}
	else {
		sequencer->SetDistance(wxAtoi(distanceTextCtrl->GetValue()));
		for (auto& [id, checkbox] : mapIDToCheckbox) {
			if (checkbox->IsChecked())
				sequencer->AddMotorID(id);
			else
				sequencer->RemoveMotorID(id);
		}
		sequencer->Start();
		message->Set(_("Running"));
	}
}
