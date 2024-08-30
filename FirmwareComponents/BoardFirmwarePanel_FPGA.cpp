#include <filesystem>

#include "BoardFirmwarePanel_FPGA.h"
#include "LaserControlProcedures/FirmwareManagement/FirmwarePathFunctions.h"
#include "../CommonUtilities/Security/AccessByIPAddress.h"
#include "../../CommonFunctions_GUI.h"

#include "wx/statline.h"

using namespace std;

const wxString CURRENT_SLOT_STR = _("Current Slot");
const wxString CHANGE_STR = _("Change");
const wxString SELECT_FPGA_FILE_STR = _("Select FPGA File");


BoardFirmwarePanel_FPGA::BoardFirmwarePanel_FPGA(wxPanel* parent,
	std::shared_ptr<MainLaserControllerInterface> _lc
) : 
	BoardFirmwarePanel_Base(parent, _lc)
{
	
	/*if (lc->HasPulsingBoardRX())
		fm = make_shared<FPGAManager>(lc, Board::PULSE);
	else
		fm = make_shared<FPGAManager>(lc, Board::MAIN);*/
		
	fm = make_shared<FPGAManager>(lc, Board::MAIN);

	fm->addObserver(this);

	sizer->Add(boardTitleText, 0, wxALL | wxALIGN_CENTER_HORIZONTAL, 5);


	wxBoxSizer* fpgaSlotSizer = new wxBoxSizer(wxHORIZONTAL);

	currentFPGASlotLabel = new wxStaticText(this, wxID_ANY, _(CURRENT_SLOT_STR));
	currentFPGASlotLabel->SetFont(FONT_EXTRA_SMALL_SEMIBOLD);
	fpgaSlotSizer->Add(currentFPGASlotLabel, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);

	currentFPGASlotValue = new wxStaticText(this, wxID_ANY, _(CURRENT_SLOT_STR));
	currentFPGASlotValue->SetFont(FONT_SMALL_SEMIBOLD);
	fpgaSlotSizer->Add(currentFPGASlotValue, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);

	changeFPGASlotButton = new wxButton(this, wxID_ANY, _(CHANGE_STR),
		wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
	changeFPGASlotButton->SetFont(FONT_EXTRA_SMALL_SEMIBOLD);
	changeFPGASlotButton->Bind(wxEVT_BUTTON, &BoardFirmwarePanel_FPGA::OnChangeFPGASlotButtonClicked, this);
	fpgaSlotSizer->Add(changeFPGASlotButton, 0, wxLEFT | wxALIGN_CENTER_VERTICAL, 5);

	slotChoices.Add("0 (Default)");
	slotChoices.Add("1");
	slotChoices.Add("2");
	slotChoices.Add("3");
	slotChoices.Add("4");
	slotChoices.Add("5");
	changeFPGASlotChoice = new wxChoice(this, wxID_ANY,
		wxDefaultPosition, wxSize(50, 20), slotChoices);
	changeFPGASlotChoice->SetFont(FONT_EXTRA_SMALL_SEMIBOLD);
	changeFPGASlotChoice->Bind(wxEVT_CHOICE, &BoardFirmwarePanel_FPGA::OnSlotChoiceSelected, this);
	fpgaSlotSizer->Add(changeFPGASlotChoice, 0, wxLEFT | wxALIGN_CENTER_VERTICAL, 5);
	changeFPGASlotChoice->Hide();

	sizer->Add(fpgaSlotSizer, 0, wxALIGN_CENTER_HORIZONTAL, 0);

	sizer->Add(currentVersionSizer, 0, wxALIGN_CENTER_HORIZONTAL, 0);
	sizer->Add(versionStatusValue, 0, wxALL | wxALIGN_CENTER_HORIZONTAL, 5);
	sizer->Add(updateLabel, 0, wxALIGN_CENTER_HORIZONTAL, 0);
	sizer->Add(versionControlsSizer, 0, wxBOTTOM | wxALIGN_CENTER_HORIZONTAL, 8);
	sizer->Add(endUserUpdateButton, 0, wxALL | wxALIGN_CENTER_HORIZONTAL, 7);
	sizer->Add(updateStatusMessage, 0, wxBOTTOM | wxALIGN_CENTER_HORIZONTAL | wxRESERVE_SPACE_EVEN_IF_HIDDEN, 7);

	Init();

	this->SetSizer(sizer);
	this->Layout();
	sizer->Fit(this);

	Layout();
	Refresh();
	EmitSizeEvent(this);
}


void BoardFirmwarePanel_FPGA::Init() {
	RefreshCurrentSlot();
	RefreshCurrentVersion();
	RefreshLatestEngRev();
	RefreshLatestRelease();
	RefreshVersionStatus();
	updateStatusMessage->Hide();
	RefreshVisibilityBasedOnAccessMode();
}

void BoardFirmwarePanel_FPGA::RefreshAll() {
	RefreshStatusMessage();

	if (updateFinished) {
		RefreshCurrentVersion();
		RefreshVersionStatus();
		RefreshLatestEngRev();
		RefreshControlsEnabled();
		updateFinished = false;
	}

	BoardFirmwarePanel_Base::RefreshAll();
}

void BoardFirmwarePanel_FPGA::RefreshStrings() {
	SetText(currentFPGASlotLabel, _(CURRENT_SLOT_STR));
	SetText(changeFPGASlotButton, _(CHANGE_STR));
	BoardFirmwarePanel_Base::RefreshStrings();
}

void BoardFirmwarePanel_FPGA::RefreshVisibilityBasedOnAccessMode() {

	bool firmwareUpdateAccessCodeEntered = AccessManager::GetInstance().GetCurrentAccessCodeType() == AccessCodeType::FIRMWARE_UPDATE;
	bool showControls = IsInAccessMode(GuiAccessMode::SERVICE) or IsInAccessMode(GuiAccessMode::FACTORY) or firmwareUpdateAccessCodeEntered;

	SetVisibilityBasedOnCondition(currentFPGASlotLabel, showControls);
	SetVisibilityBasedOnCondition(currentFPGASlotValue, showControls);
	SetVisibilityBasedOnCondition(changeFPGASlotButton, showControls and slotButtonShown);
	SetVisibilityBasedOnCondition(changeFPGASlotChoice, showControls and slotChoiceShown);
	BoardFirmwarePanel_Base::RefreshVisibilityBasedOnAccessMode();
}


void BoardFirmwarePanel_FPGA::RefreshCurrentVersion() {
	SetText(currentVersionValue, to_wx_string(fm->GetVersion()));
}

void BoardFirmwarePanel_FPGA::RefreshLatestRelease() {
	string latestRelease = GetLatestFPGAVersionFilename(lc->GetFactoryFirmwareReleasesFolderPath(), lc->GetFPGAFilePattern());
	string latestReleaseVersionStr = ExtractVersionFromFPGAFilename(latestRelease);
	SetText(latestReleaseValue, to_wx_string(latestReleaseVersionStr));
}

void BoardFirmwarePanel_FPGA::RefreshLatestEngRev() {
	string latestEngRev = GetLatestFPGAVersionFilename(lc->GetFactoryFirmwareLatestEngRevFolderPath(), lc->GetFPGAFilePattern());
	string latestEngRevVersionStr = ExtractVersionFromFPGAFilename(latestEngRev);
	SetText(latestEngRevValue, to_wx_string(latestEngRevVersionStr));
}

void BoardFirmwarePanel_FPGA::RefreshCurrentSlot() {
	currentFPGASlotValue->SetLabelText(slotChoices[fm->GetCurrentSlot()].substr(0, 1));
}


void BoardFirmwarePanel_FPGA::OnChangeFPGASlotButtonClicked(wxCommandEvent& evt) {
	changeFPGASlotButton->Hide();
	slotButtonShown = false;
	changeFPGASlotChoice->Show();
	slotChoiceShown = true;
	changeFPGASlotChoice->SetStringSelection(slotChoices[fm->GetCurrentSlot()]);
	Layout();
	Refresh();
}


void BoardFirmwarePanel_FPGA::OnSlotChoiceSelected(wxCommandEvent& evt) {
	lc->StageUserAction("FPGA slot choice selected");
	
	int selectedSlot = evt.GetInt();
	wxString msg = wxString::Format("Chose slot %d", selectedSlot);

	YieldToApp();
	fm->ChangeSlot(selectedSlot);

	RefreshCurrentSlot();
	RefreshCurrentVersion();
	RefreshLatestRelease();
	RefreshVersionStatus();
	RefreshControlsEnabled();

	changeFPGASlotButton->Show();
	slotButtonShown = true;
	changeFPGASlotChoice->Hide();
	slotChoiceShown = false;

	Layout();
	Refresh();

	lc->StageUserActionArguments(to_string(selectedSlot) + " - " + lc->GetFPGAVersion());
	lc->LogUserAction(GetResultingError());
}


void BoardFirmwarePanel_FPGA::RefreshStatusMessage() {
	if (!updateStatusMessage->IsShown())
		updateStatusMessage->Show();
	if (fm->IsError()) {
		updateStatusMessage->Set(fm->GetErrorMessage());
		updateStatusMessage->StopCycling();
	}
	else if (fm->IsUpdateFinished()) {
		updateStatusMessage->Set(fm->GetStatusMessage());
		updateStatusMessage->StopCycling();
	}
	else if (fm->IsUpdating()) {
		updateStatusMessage->Set(to_wx_string(fm->GetStatusMessage()) + " - " + to_wx_string(fm->GetUpdatingProgress()) + "%");
	}
	else {
		updateStatusMessage->Set("");
		updateStatusMessage->StopCycling();
	}
}


void BoardFirmwarePanel_FPGA::OnUpdateToLatestEngRevClicked(wxCommandEvent& evt) {
	lc->StageUserAction("Update FPGA to latest engineering rev. clicked");
	UpdateFPGA(lc->GetFactoryFirmwareLatestEngRevFolderPath());
}

void BoardFirmwarePanel_FPGA::OnUpdateToReleaseClicked(wxCommandEvent& evt) {
	lc->StageUserAction("Update FPGA to release clicked");
	UpdateFPGA(lc->GetFactoryFirmwareReleasesFolderPath());
}

void BoardFirmwarePanel_FPGA::OnUpdateToOtherClicked(wxCommandEvent& evt) {
	lc->StageUserAction("Update FPGA to other clicked");
	UpdateFPGA(lc->GetFactoryFirmwareOldFolderPath());
}


void BoardFirmwarePanel_FPGA::UpdateFPGA(std::string defaultDirectoryPath) {

	wxFileDialog loadFPGAFileDialog(this,
		_(SELECT_FPGA_FILE_STR), 
		to_wx_string(defaultDirectoryPath), 
		to_wx_string(lc->GetFPGAFilePattern()),
		"BIN files (*.bin)|*.bin", 
		wxFD_OPEN | wxFD_FILE_MUST_EXIST);

	if (loadFPGAFileDialog.ShowModal() == wxID_CANCEL) {
		return;
	}

	// Otherwise, use the file path for the next steps
	string path = string(loadFPGAFileDialog.GetPath());

	fm->Update(path);

	if (fm->IsError())
		wxMessageBox(to_wx_string(fm->GetErrorMessage()));
	else {
		updateStatusMessage->StartCycling();
	}

	RefreshCurrentVersion();
	RefreshVersionStatus();

	lc->StageUserActionArguments(lc->GetFPGAVersion());
	lc->LogUserAction(GetResultingError());
}


void BoardFirmwarePanel_FPGA::update() {
	updateFinished = true;
}

