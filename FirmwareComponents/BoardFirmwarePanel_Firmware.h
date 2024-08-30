#pragma once

#include "BoardFirmwarePanel_Base.h"
#include "LaserControlProcedures/FirmwareManagement/FirmwareManager.h"
#include "MainLaserControllerInterface.h"


class BoardFirmwarePanel_Firmware : public BoardFirmwarePanel_Base, public Observer {

public:
	BoardFirmwarePanel_Firmware(wxPanel* parent, std::shared_ptr<MainLaserControllerInterface> _lc);

	void Init();
	void RefreshAll() override;
	void RefreshStrings() override;
	void RefreshVisibilityBasedOnAccessMode() override;


protected:
	std::shared_ptr<FirmwareManager> fm;

	wxButton* SwitchFlashBankButton; // "Undo" button

	bool updateFinished = false;

	void RefreshCurrentVersion() override;
	void RefreshLatestRelease() override;
	void RefreshLatestEngRev() override;
	void RefreshStatusMessage() override;
	void RefreshControlsEnabled() override;
	void RefreshUndoButton();

	void OnUpdateToLatestEngRevClicked(wxCommandEvent& evt);
	void OnUpdateToReleaseClicked(wxCommandEvent& evt);
	void OnUpdateToOtherClicked(wxCommandEvent& evt);
	void OnSwitchFlashBankButtonClicked(wxCommandEvent& evt);

	void UpdateFirmware(std::string defaultDirectoryPath);

	void update() override;

};


