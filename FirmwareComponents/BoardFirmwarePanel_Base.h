// 10/16/23
// James Butcher
//
// Panel displaying current firmware version and controls for updating
// for a given board. Currently MainBoard only.

#pragma once

#include <string>

#include "wx/wx.h"
#include "../../CommonGUIComponents/DynamicStatusMessage.h"
#include "MainLaserControllerInterface.h"


class BoardFirmwarePanel_Base : public wxPanel {

public:
	BoardFirmwarePanel_Base(wxWindow* parent, std::shared_ptr<MainLaserControllerInterface> _lc);

	virtual void RefreshAll();
	virtual void RefreshStrings();
	virtual void RefreshVisibilityBasedOnAccessMode();
	virtual void RefreshControlsEnabled();

	virtual void RefreshCurrentVersion() = 0;
	virtual void RefreshLatestRelease() = 0;
	virtual void RefreshLatestEngRev() = 0;

	void SetBoardTitle(std::string title);


protected:
	std::shared_ptr<MainLaserControllerInterface> lc;

	std::string boardTitle;

	wxBoxSizer* sizer;

	wxStaticText* boardTitleText;

	wxBoxSizer* currentVersionSizer;

	wxFlexGridSizer* versionControlsSizer;

	wxStaticText* currentVersionLabel;
	wxStaticText* currentVersionValue;

	wxStaticText* versionStatusValue;

	wxStaticText* updateLabel;

	wxButton* updateToLatestEngRevButton;
	wxStaticText* latestEngRevValue;

	wxButton* updateToReleaseButton;
	wxStaticText* latestReleaseValue;

	wxButton* updateToOtherButton;


	wxButton* endUserUpdateButton;


	DynamicStatusMessage* updateStatusMessage;

	void RefreshVersionStatus();

	virtual void RefreshStatusMessage() {}

	virtual void OnUpdateToLatestEngRevClicked(wxCommandEvent& evt) = 0;
	virtual void OnUpdateToReleaseClicked(wxCommandEvent& evt) = 0;
	virtual void OnUpdateToOtherClicked(wxCommandEvent& evt) = 0;
	

};

