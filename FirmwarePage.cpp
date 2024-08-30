#include "FirmwarePage.h"
#include "LaserControlProcedures/FirmwareManagement/FPGAManager.h"
#include "../CommonFunctions_GUI.h"
#include "../AccessCodeDialog.h"

using namespace std;

const wxString MAINBOARD_STR = _("MainBoard");
const wxString GENERATE_FIRMWARE_ACCESS_KEY_STR = _("Generate Firmware Updating Access Key");
const wxString FIRMWARE_ACCESS_KEY_MESSAGE_STR = _(
	"Send this code to a Photonics factory representative\n"
	"to request a temporary access code for updating firmware."
);


FirmwarePage::FirmwarePage(std::shared_ptr<MainLaserControllerInterface> _lc, wxWindow* parent) 
	: SettingsPage_Base(_lc, parent)
{

	wxBoxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);


	// Firmware Panel

	firmwarePanel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_THEME);
	firmwarePanel->SetBackgroundColour(FOREGROUND_PANEL_COLOR);
	firmwareSizer = new wxBoxSizer(wxVERTICAL);

	getFirmwareAccessKeyButton = new wxBitmapButton(firmwarePanel, wxID_ANY, wxBitmap(KEY_ICON, wxBITMAP_TYPE_ANY),
		wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW | 0);
	getFirmwareAccessKeyButton->Bind(wxEVT_BUTTON, &FirmwarePage::OnGetFirmwareAccessKeyButtonClicked, this);
	getFirmwareAccessKeyButton->SetToolTip(GENERATE_FIRMWARE_ACCESS_KEY_STR);
	firmwareSizer->Add(getFirmwareAccessKeyButton, 0, wxALL, 2);

	InitFirmwarePanel();

	firmwarePanel->SetSizer(firmwareSizer);
	sizer->Add(firmwarePanel, 1, wxALL, 10);


	// FPGA Panel

	fpgaPanel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_THEME);
	fpgaPanel->SetBackgroundColour(FOREGROUND_PANEL_COLOR);
	fpgaSizer = new wxBoxSizer(wxVERTICAL);

	InitFPGAPanel();

	fpgaPanel->SetSizer(fpgaSizer);
	sizer->Add(fpgaPanel, 1, wxALL, 10);


	this->SetSizer(sizer);
	this->Layout();
	sizer->Fit(this);

}

void FirmwarePage::Init() {
}

void FirmwarePage::RefreshStrings() {
	SetName(_(FIRMWARE_STR));

	firmwareTitle->SetLabelText(_(FIRMWARE_STR));
	mainBoardFirmwarePanel->RefreshStrings();
	fpgaTitle->SetLabelText(_(FPGA_STR));
	mainBoardFPGAPanel->RefreshStrings();
	getFirmwareAccessKeyButton->SetToolTip(_(GENERATE_FIRMWARE_ACCESS_KEY_STR));
}

void FirmwarePage::RefreshAll() {
	mainBoardFPGAPanel->RefreshAll();
	mainBoardFirmwarePanel->RefreshAll();
}

void FirmwarePage::RefreshVisibility() {
	mainBoardFPGAPanel->RefreshVisibilityBasedOnAccessMode();
	mainBoardFirmwarePanel->RefreshVisibilityBasedOnAccessMode();
	SetVisibilityBasedOnCondition(getFirmwareAccessKeyButton, GetGUIAccessMode() == GuiAccessMode::END_USER);
}


void FirmwarePage::InitFirmwarePanel() {
	firmwareSizer->Clear();

	firmwareTitle = new wxStaticText(firmwarePanel, wxID_ANY, _(FIRMWARE_STR), wxDefaultPosition, wxDefaultSize, 0);
	firmwareTitle->SetFont(FONT_MED_LARGE_SEMIBOLD);
	firmwareSizer->Add(firmwareTitle, 0, wxALL | wxALIGN_CENTER_HORIZONTAL, 5);


	// Add individual board firmware panels
	// ...
	mainBoardFirmwarePanel = new BoardFirmwarePanel_Firmware(firmwarePanel, lc);
	mainBoardFirmwarePanel->SetBoardTitle(std::string(_(MAINBOARD_STR)));
	firmwareSizer->Add(mainBoardFirmwarePanel, 0, wxALL | wxEXPAND, 8);

	firmwarePanel->Layout();
	firmwarePanel->Refresh();
}


void FirmwarePage::InitFPGAPanel() {
	fpgaSizer->Clear();

	fpgaTitle = new wxStaticText(fpgaPanel, wxID_ANY, _(FPGA_STR), wxDefaultPosition, wxDefaultSize, 0);
	fpgaTitle->SetFont(FONT_MED_LARGE_SEMIBOLD);
	fpgaSizer->Add(fpgaTitle, 0, wxALL | wxALIGN_CENTER_HORIZONTAL, 5);


	// Add individual board fpga panels
	// ...
	mainBoardFPGAPanel = new BoardFirmwarePanel_FPGA(fpgaPanel, lc);
	mainBoardFPGAPanel->SetBoardTitle(std::string(_(MAINBOARD_STR)));
	fpgaSizer->Add(mainBoardFPGAPanel, 0, wxALL | wxEXPAND, 8);

	fpgaPanel->Layout();
	fpgaPanel->Refresh();
}


void FirmwarePage::OnGetFirmwareAccessKeyButtonClicked(wxCommandEvent& evt) {
	STAGE_ACTION("Generate firmware updating access key")

	wxString code = GenerateOfflinePartialKey(AccessCodeType::FIRMWARE_UPDATE);
	AccessCodeDialog accessCodeDialog(this, code, _(FIRMWARE_ACCESS_KEY_MESSAGE_STR));
	accessCodeDialog.ShowModal();

	//	// If in End User mode, generates an access key that can be activated to unlock service mode
	//	if (AccessManager::GetInstance().IsCurrentAccessLevel(AccessLevel::END_USER)) {
	//		STAGE_ACTION_ARGUMENTS("Current Mode: End User")
	//			wxString code = GenerateOfflinePartialKey(AccessCodeType::SERVICE_MODE);
	//		AccessCodeDialog accessCodeDialog(this, code, _(ACCESS_KEY_MESSAGE_STR));
	//		accessCodeDialog.ShowModal();
	//	}
	//// If in service mode already, generates a fully-acitvated service mode access code valid for 48 hours
	//	else if (AccessManager::GetInstance().IsCurrentAccessLevel(AccessLevel::SERVICE_MODE)) {
	//		STAGE_ACTION_ARGUMENTS("Current Mode: Service")
	//			wxString message = _(wxString::Format(ACCESS_CODE_MESSAGE_STR, DEFAULT_OFFLINE_ACCESS_CODE_EXPIRATION_IN_H));
	//		wxString code = GenerateOfflineAccessCode(DEFAULT_OFFLINE_ACCESS_CODE_EXPIRATION_IN_H, AccessCodeType::SERVICE_MODE);
	//		AccessCodeDialog accessCodeDialog(this, code, message);
	//		accessCodeDialog.ShowModal();
	//	}
	//// If in service mode already, generates a fully-acitvated factory mode access code valid for 48 hours
	//	else if (AccessManager::GetInstance().IsCurrentAccessLevel(AccessLevel::FACTORY_MODE)) {
	//		STAGE_ACTION_ARGUMENTS("Current Mode: Factory")
	//			wxString message = _(wxString::Format(ACCESS_CODE_MESSAGE_STR, DEFAULT_OFFLINE_ACCESS_CODE_EXPIRATION_IN_H));
	//		wxString code = GenerateOfflineAccessCode(DEFAULT_OFFLINE_ACCESS_CODE_EXPIRATION_IN_H, AccessCodeType::FACTORY_MODE);
	//		AccessCodeDialog accessCodeDialog(this, code, message);
	//		accessCodeDialog.ShowModal();
	//	}

	LOG_ACTION()
}
