// James Butcher
// 9/26/23

#pragma once

#include "wx/wx.h"

#include "FirmwareComponents/BoardFirmwarePanel_FPGA.h"
#include "FirmwareComponents/BoardFirmwarePanel_Firmware.h"
#include "SettingsPage_Base.h"



class FirmwarePage : public SettingsPage_Base {

public:
	FirmwarePage(std::shared_ptr<MainLaserControllerInterface> _lc, wxWindow* parent);

	void Init();
	void RefreshAll();
	void RefreshStrings();
	void RefreshVisibility();


private:
	wxBitmapButton* getFirmwareAccessKeyButton;

	wxStaticText* firmwareTitle;
	wxPanel* firmwarePanel;
	wxBoxSizer* firmwareSizer;
	BoardFirmwarePanel_Firmware* mainBoardFirmwarePanel;

	wxStaticText* fpgaTitle;
	wxPanel* fpgaPanel;
	wxBoxSizer* fpgaSizer;
	BoardFirmwarePanel_FPGA* mainBoardFPGAPanel;

	void InitFirmwarePanel();
	void InitFPGAPanel();

	void OnGetFirmwareAccessKeyButtonClicked(wxCommandEvent& evt);

};

