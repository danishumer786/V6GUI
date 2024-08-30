#pragma once

#include "BoardFirmwarePanel_Base.h"
#include "LaserControlProcedures/FirmwareManagement/FPGAManager.h"
#include "MainLaserControllerInterface.h"


class BoardFirmwarePanel_FPGA : public BoardFirmwarePanel_Base, public Observer {

public:
	BoardFirmwarePanel_FPGA(wxPanel* parent, std::shared_ptr<MainLaserControllerInterface> _lc);

	void Init();
	void RefreshAll() override;
	void RefreshStrings() override;
	void RefreshVisibilityBasedOnAccessMode() override;

	void RefreshCurrentVersion() override;
	void RefreshLatestRelease() override;
	void RefreshLatestEngRev() override;

	void RefreshCurrentSlot();

protected:
	std::shared_ptr<FPGAManager> fm;

	wxArrayString slotChoices;
	int currentFPGASlot = 0;

	wxStaticText* currentFPGASlotLabel;
	wxStaticText* currentFPGASlotValue;
	wxButton* changeFPGASlotButton;
	wxChoice* changeFPGASlotChoice;

	bool slotButtonShown = true;
	bool slotChoiceShown = false;

	bool updateFinished = false;

	void RefreshStatusMessage() override;

	void OnChangeFPGASlotButtonClicked(wxCommandEvent& evt);
	void OnSlotChoiceSelected(wxCommandEvent& evt);

	void OnUpdateToLatestEngRevClicked(wxCommandEvent& evt);
	void OnUpdateToReleaseClicked(wxCommandEvent& evt);
	void OnUpdateToOtherClicked(wxCommandEvent& evt);

	void UpdateFPGA(std::string defaultDirectoryPath);

	void update() override;

};

