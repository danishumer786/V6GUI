
#include <filesystem>

#include "BoardFirmwarePanel_Firmware.h"
#include "LaserControlProcedures/FirmwareManagement/FirmwarePathFunctions.h"
#include "LaserControlProcedures/LaserResetter.h"
#include "../CommonUtilities/Security/AccessByIPAddress.h"
#include "../../CommonFunctions_GUI.h"
#include "../../Resources.h"

#include "wx/statline.h"

using namespace std;

const wxString SELECT_FIRMWARE_FILE_STR = _("Select Firmware File");
const wxString UNDO_STR = _("Undo");
const wxString UNDO_TOOLTIP = _("Revert to previous firmware version");
const wxString SWITCH_FLASH_BANK_STR = _("Switch flash bank");
const wxString SWITCHING_FLASH_BANK_STR = _("Switching flash bank");
const wxString SWITCH_FLASH_BANK_TOOLTIP = _(
	"Acts as an \"undo\" function after updating firmware.\n\n"
	"There are 2 sets of firmware on each chip.\n"
	"Updating firmware replaces the version on the\n"
	"other flash bank and then switches to it.\n"
	"Click this to switch to using the firmware\n"
	"on the other flash bank.\n"
);


BoardFirmwarePanel_Firmware::BoardFirmwarePanel_Firmware(wxPanel* parent,
	std::shared_ptr<MainLaserControllerInterface> _lc
) :
	BoardFirmwarePanel_Base(parent, _lc) {

	fm = make_shared<FirmwareManager>(lc);
	fm->addObserver(this);

	sizer->Add(boardTitleText, 0, wxALL | wxALIGN_CENTER_HORIZONTAL, 5);

	SwitchFlashBankButton = new wxButton(this, wxID_ANY, _(UNDO_STR));
	SwitchFlashBankButton->SetToolTip(_(UNDO_TOOLTIP));
	SwitchFlashBankButton->SetFont(FONT_EXTRA_SMALL_SEMIBOLD);
	SwitchFlashBankButton->Bind(wxEVT_BUTTON, &BoardFirmwarePanel_Firmware::OnSwitchFlashBankButtonClicked, this);
	sizer->Add(SwitchFlashBankButton, 0, wxALL | wxALIGN_CENTER_HORIZONTAL, 5);

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


void BoardFirmwarePanel_Firmware::Init() {
	RefreshCurrentVersion();
	RefreshLatestEngRev();
	RefreshLatestRelease();
	RefreshVersionStatus();

	updateStatusMessage->Hide();
	SwitchFlashBankButton->Hide();

	RefreshVisibilityBasedOnAccessMode();
}

void BoardFirmwarePanel_Firmware::RefreshAll() {
	RefreshStatusMessage();

	if (updateFinished) {
		RefreshCurrentVersion();
		RefreshVersionStatus();
		RefreshLatestEngRev();
		RefreshControlsEnabled();
		RefreshVisibilityBasedOnAccessMode();
		updateFinished = false;
		if (fm->IsError())
			updateStatusMessage->SetLabelText(fm->GetErrorMessage());
		else
			updateStatusMessage->SetLabelText("Finished");
		updateStatusMessage->StopCycling();
	}

	BoardFirmwarePanel_Base::RefreshAll();
}


void BoardFirmwarePanel_Firmware::RefreshCurrentVersion() {
	currentVersionValue->SetLabelText(to_wx_string(lc->GetFirmwareVersion()));
}

void BoardFirmwarePanel_Firmware::RefreshLatestRelease() {
	string latestRelease = GetLatestFirmwareVersionFilename(lc->GetFactoryFirmwareReleasesFolderPath(), lc->GetFirmwareFilePattern());
	string latestReleaseVersionStr = ExtractVersionFromFirmwareFilename(latestRelease);
	latestReleaseValue->SetLabelText(to_wx_string(latestReleaseVersionStr));
}

void BoardFirmwarePanel_Firmware::RefreshLatestEngRev() {
	string latestEngRev = GetLatestFirmwareVersionFilename(lc->GetFactoryFirmwareLatestEngRevFolderPath(), lc->GetFirmwareFilePattern());
	string latestEngRevVersionStr = ExtractVersionFromFirmwareFilename(latestEngRev);
	latestEngRevValue->SetLabelText(to_wx_string(latestEngRevVersionStr));
}

void BoardFirmwarePanel_Firmware::RefreshVisibilityBasedOnAccessMode() {

	bool firmwareUpdatingAccessCodeEntered = AccessManager::GetInstance().GetCurrentAccessCodeType() == AccessCodeType::FIRMWARE_UPDATE;
	bool showSwitchFlashBankButton = (IsInAccessMode(GuiAccessMode::SERVICE) and updateFinished) or IsInAccessMode(GuiAccessMode::FACTORY) or firmwareUpdatingAccessCodeEntered;

	SetVisibilityBasedOnCondition(SwitchFlashBankButton, showSwitchFlashBankButton);
	RefreshUndoButton();
	BoardFirmwarePanel_Base::RefreshVisibilityBasedOnAccessMode();
}

void BoardFirmwarePanel_Firmware::RefreshControlsEnabled() {
	bool updating = lc->IsUpdating();
	bool autotuning = lc->IsAutotuneRunning();
	RefreshWidgetEnableBasedOnCondition(SwitchFlashBankButton, !updating and !autotuning);
	BoardFirmwarePanel_Base::RefreshControlsEnabled();
}


void BoardFirmwarePanel_Firmware::RefreshStatusMessage() {
	if (fm->IsUpdating()) {
		if (!updateStatusMessage->IsShown())
			updateStatusMessage->Show();
		updateStatusMessage->Set(to_wx_string(fm->GetStatusMessage()) + " - " + to_wx_string(fm->GetUpdatingProgress()) + "%");
	}
}


void BoardFirmwarePanel_Firmware::RefreshUndoButton() {
	if (IsInAccessMode(GuiAccessMode::FACTORY)) {
		SetText(SwitchFlashBankButton, _(SWITCH_FLASH_BANK_STR));
		SwitchFlashBankButton->SetToolTip(_(SWITCH_FLASH_BANK_TOOLTIP));
	}
	else {
		SetText(SwitchFlashBankButton, _(UNDO_STR));
		SwitchFlashBankButton->SetToolTip(_(UNDO_TOOLTIP));
	}
}


void BoardFirmwarePanel_Firmware::RefreshStrings() {
	RefreshUndoButton();
	BoardFirmwarePanel_Base::RefreshStrings();
}


void BoardFirmwarePanel_Firmware::OnUpdateToLatestEngRevClicked(wxCommandEvent& evt) {
	lc->StageUserAction("Update firmware to latest engineering rev. clicked");
	UpdateFirmware(lc->GetFactoryFirmwareLatestEngRevFolderPath());
}

void BoardFirmwarePanel_Firmware::OnUpdateToReleaseClicked(wxCommandEvent& evt) {
	lc->StageUserAction("Update firmware to release clicked");
	UpdateFirmware(lc->GetFactoryFirmwareReleasesFolderPath());
}

void BoardFirmwarePanel_Firmware::OnUpdateToOtherClicked(wxCommandEvent& evt) {
	lc->StageUserAction("Update firmware to other clicked");
	UpdateFirmware(lc->GetFactoryFirmwareOldFolderPath());
}

void BoardFirmwarePanel_Firmware::OnSwitchFlashBankButtonClicked(wxCommandEvent& evt) {

	lc->StageUserAction("Switch firmware flash bank button clicked");

	lc->ActivateFirmwareFlashBank();

	updateStatusMessage->Show();
	updateStatusMessage->SetLabelText(_(SWITCHING_FLASH_BANK_STR));
	updateStatusMessage->StartCycling();

	// 2-2-24 - Apparently this takes much longer on the V4 chip boards
	if (lc->IsSTM32H725()) {
		for (int i = 0; i < 10; i++) {
			YieldToApp();
			this_thread::sleep_for(chrono::seconds(3)); 
		}
	}

	Layout();
	Refresh();
	LaserResetter r(lc);
	r.ResetLaser();
	while (r.IsResetting()) {
		YieldToApp();
	}

	lc->UpdateFirmwareVersion();
	RefreshCurrentVersion();
	RefreshVersionStatus();
	RefreshLatestEngRev();
	RefreshControlsEnabled();
	BoardFirmwarePanel_Base::RefreshAll();

	updateStatusMessage->Hide();
	updateStatusMessage->SetLabelText("");
	updateStatusMessage->StopCycling();

	lc->StageUserActionArguments(lc->GetFirmwareVersion());
	lc->LogUserAction(GetResultingError());

}


void BoardFirmwarePanel_Firmware::UpdateFirmware(std::string defaultDirectoryPath) {

	wxString wildcard;
	if (lc->IsSTM32())
		wildcard = "BIN files (*.bin)|*.bin";
	else
		wildcard = "HEX files (*.hex)|*.hex";

	wxFileDialog loadFirmwareFileDialog(this,
		_(SELECT_FIRMWARE_FILE_STR),
		to_wx_string(defaultDirectoryPath),
		to_wx_string(lc->GetFirmwareFilePattern()),
		wildcard,
		wxFD_OPEN | wxFD_FILE_MUST_EXIST);

	if (loadFirmwareFileDialog.ShowModal() == wxID_CANCEL) {
		lc->StageUserActionArguments("Canceled");
		lc->LogUserAction(GetResultingError());
		return;
	}

	string path = string(loadFirmwareFileDialog.GetPath());

	fm->Update(path);

	if (fm->IsError())
		wxMessageBox(to_wx_string(fm->GetErrorMessage()));
	else {
		updateStatusMessage->StartCycling();
	}

	RefreshCurrentVersion();
	RefreshVersionStatus();

	lc->StageUserActionArguments(lc->GetFirmwareVersion());
	lc->LogUserAction(GetResultingError());
}


void BoardFirmwarePanel_Firmware::update() {
	updateFinished = true;
}

