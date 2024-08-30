#include <wx/slider.h>
#include <wx/scrolbar.h>
#include <wx/scrolwin.h>

#include "../CommonFunctions_GUI.h"
#include "MotorControlPanelLinked.h"
#include "ConfigurationManager.h"

using namespace std;




MotorControlPanelLinked::MotorControlPanelLinked(
	shared_ptr<MainLaserControllerInterface> _lc,
	int linked_motor_id,
	wxWindow* parent,
	wxWindowID winid,
	const wxPoint& pos,
	const wxSize& size,
	long style,
	const wxString& name) :
	wxPanel(parent, winid, pos, size, style, name) {

	lc = _lc;
	linkedMotorID = linked_motor_id;

	// Sizer for the whole motor control panel
	wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);

	// Large motor label in top left corner
	LinkedMotorLabel = new wxStaticText(this, wxID_ANY, wxT("Motor ?"));
	LinkedMotorLabel->SetFont(FONT_MED_LARGE_BOLD);
	sizer->Add(LinkedMotorLabel, 0, wxLEFT | wxRIGHT, 5);


	LinkedMotorPositionChoice = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, positionsChoices);
	LinkedMotorPositionChoice->SetFont(FONT_EXTRA_SMALL_SEMIBOLD);
	LinkedMotorPositionChoice->Bind(wxEVT_CHOICE, &MotorControlPanelLinked::OnLinkedMotorPositionSelected, this);
	sizer->Add(LinkedMotorPositionChoice, 0, wxALL, 0);


	this->SetBackgroundColour(FOREGROUND_PANEL_COLOR);
	this->SetSizer(sizer);
	this->Layout();

	Init();
}

bool MotorControlPanelLinked::ShouldBeVisibleToUser() {
	return (lc->GetMotorIndexedPositions(linkedMotorID).size() > 0);
}

void MotorControlPanelLinked::Init() {

	motorID_X = lc->GetLinkedMotorId_X(linkedMotorID);
	motorID_Y = lc->GetLinkedMotorId_Y(linkedMotorID);

	wxString label = lc->GetMotorLabel(motorID_X) + "-" + lc->GetMotorLabel(motorID_Y);

	LinkedMotorLabel->SetLabelText(label);

	RefreshPositionsChoices();
	RefreshCurrentPosition();
	RefreshVisibility();
}


void MotorControlPanelLinked::RefreshStrings() {
}


void MotorControlPanelLinked::RefreshAll() {
	RefreshControlEnabled();
	RefreshCurrentPosition();
}

void MotorControlPanelLinked::RefreshVisibility() {
	if (IsInAccessMode(GuiAccessMode::END_USER)) {
		if (ShouldBeVisibleToUser())
			this->Show();
		else
			this->Hide();
	}

	EmitSizeEvent(this);
}

void MotorControlPanelLinked::RefreshControlEnabled() {
	bool autotuning = lc->IsAutotuneRunning();
	bool running = lc->LaserIsRunning();

	bool allowControl = running and !autotuning;

	RefreshWidgetEnableBasedOnCondition(LinkedMotorPositionChoice, allowControl);
}


void MotorControlPanelLinked::RefreshCurrentPosition() {
	vector<int> indexedPositions = lc->GetLinkedMotorIndexedPositions(linkedMotorID);
	int currentPosition = lc->GetLinkedMotorPosition(linkedMotorID);
	int positionIndex = 0;
	for (int i = 0; i < int(indexedPositions.size()); i++) {
		if (indexedPositions[i] == currentPosition) {
			LinkedMotorPositionChoice->SetSelection(i);
			break;
		}
	}
}

void MotorControlPanelLinked::RefreshPositionsChoices() {
	positionsChoices.Clear();
	for (int position : lc->GetLinkedMotorIndexedPositions(linkedMotorID)) {
		int hours = lc->GetLinkedMotorPositionHours(linkedMotorID, position);
		wxString positionLabel = wxString::Format("P%d - %d hours", position, hours);
		positionsChoices.Add(positionLabel);
	}
	LinkedMotorPositionChoice->Set(positionsChoices);
}


void MotorControlPanelLinked::OnLinkedMotorPositionSelected(wxCommandEvent& evt) {
	int positionIndex = evt.GetInt();
	int position = lc->GetLinkedMotorIndexedPositions(linkedMotorID)[positionIndex];
	lc->SetLinkedMotorPosition(linkedMotorID, position);
	RefreshCurrentPosition();
}
