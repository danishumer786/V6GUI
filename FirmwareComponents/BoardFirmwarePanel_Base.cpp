#include "BoardFirmwarePanel_Base.h"
#include "Security/AccessByIPAddress.h"
#include "../../CommonFunctions_GUI.h"


const wxString CURRENT_VERSION_STR = _("Current Version");
const wxString UPDATE_TO_STR = _("Update to:");
const wxString UPDATE_STR = _("Update");
const wxString LATEST_STR = _("Latest");
const wxString RELEASE_STR = _("Release");
const wxString OTHER_STR = _("Other");
const wxString OUT_OF_DATE_STR = _("Out Of Date");
const wxString UP_TO_DATE_STR = _("Up To Date");
const wxString ENGINEERING_STR = _("Engineering");



BoardFirmwarePanel_Base::BoardFirmwarePanel_Base(wxWindow* parent, std::shared_ptr<MainLaserControllerInterface> _lc)
	: wxPanel(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_RAISED), lc(_lc) {


	sizer = new wxBoxSizer(wxVERTICAL);


	// Title

	boardTitleText = new wxStaticText(this, wxID_ANY, wxEmptyString);
	boardTitleText->SetFont(FONT_MED_SMALL_SEMIBOLD);


	// Current Version

	currentVersionSizer = new wxBoxSizer(wxHORIZONTAL);

	currentVersionLabel = new wxStaticText(this, wxID_ANY, _(CURRENT_VERSION_STR));
	currentVersionLabel->SetFont(FONT_EXTRA_SMALL_SEMIBOLD);
	currentVersionSizer->Add(currentVersionLabel, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 3);

	currentVersionValue = new wxStaticText(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxBORDER_THEME);
	currentVersionValue->SetFont(FONT_SMALL_SEMIBOLD);
	currentVersionSizer->Add(currentVersionValue, 0, wxALIGN_CENTER_VERTICAL | wxLEFT, 3);


	// Version status - Indicates whether current firmware version is:
	//	- out of date ( < release version - Red )
	//	- up to date ( == release version - Green )
	//	- engineering ( > release version - Orange )

	versionStatusValue = new wxStaticText(this, wxID_ANY, "?");
	versionStatusValue->SetFont(FONT_MED_SMALL_BOLD);


	// Update controls

	updateLabel = new wxStaticText(this, wxID_ANY, _(UPDATE_TO_STR));

	versionControlsSizer = new wxFlexGridSizer(0, 2, 3, 10);

	// Latest Eng. Rev. version - Orange button that says "Latest"
	updateToLatestEngRevButton = new wxButton(this, wxID_ANY, _(LATEST_STR));
	updateToLatestEngRevButton->SetFont(FONT_SMALL_SEMIBOLD);
	updateToLatestEngRevButton->SetBackgroundColour(BUTTON_COLOR_ORANGE);
	updateToLatestEngRevButton->Bind(wxEVT_BUTTON, &BoardFirmwarePanel_Base::OnUpdateToLatestEngRevClicked, this);
	versionControlsSizer->Add(updateToLatestEngRevButton, 0, wxALIGN_CENTER_VERTICAL, 0);

	latestEngRevValue = new wxStaticText(this, wxID_ANY, wxEmptyString);
	latestEngRevValue->SetFont(FONT_SMALL_SEMIBOLD);
	versionControlsSizer->Add(latestEngRevValue, 0, wxALIGN_CENTER_VERTICAL, 0);

	// Release version - Green button that says "Release"
	updateToReleaseButton = new wxButton(this, wxID_ANY, _(RELEASE_STR));
	updateToReleaseButton->SetFont(FONT_SMALL_SEMIBOLD);
	updateToReleaseButton->SetBackgroundColour(BUTTON_COLOR_ACTIVE);
	updateToReleaseButton->Bind(wxEVT_BUTTON, &BoardFirmwarePanel_Base::OnUpdateToReleaseClicked, this);
	versionControlsSizer->Add(updateToReleaseButton, 0, wxALIGN_CENTER_VERTICAL, 0);

	latestReleaseValue = new wxStaticText(this, wxID_ANY, wxEmptyString);
	latestReleaseValue->SetFont(FONT_SMALL_SEMIBOLD);
	versionControlsSizer->Add(latestReleaseValue, 0, wxALIGN_CENTER_VERTICAL, 0);

	// Other version - Gray button that says "Other"
	updateToOtherButton = new wxButton(this, wxID_ANY, _(OTHER_STR));
	updateToOtherButton->SetFont(FONT_SMALL_SEMIBOLD);
	updateToOtherButton->SetBackgroundColour(RESET_BUTTON_COLOR);
	updateToOtherButton->Bind(wxEVT_BUTTON, &BoardFirmwarePanel_Base::OnUpdateToOtherClicked, this);
	versionControlsSizer->Add(updateToOtherButton, 0, wxALIGN_CENTER_VERTICAL, 0);
	

	// End user update button - for use outside of factory - blue button that just says "Update"

	endUserUpdateButton = new wxButton(this, wxID_ANY, _(UPDATE_STR));
	endUserUpdateButton->SetFont(FONT_SMALL_SEMIBOLD);
	endUserUpdateButton->SetBackgroundColour(PHOTONICS_TURQUOISE_COLOR);
	endUserUpdateButton->Bind(wxEVT_BUTTON, &BoardFirmwarePanel_Base::OnUpdateToLatestEngRevClicked, this);


	updateStatusMessage = new DynamicStatusMessage(this, wxEmptyString);

}


void BoardFirmwarePanel_Base::RefreshAll() {
	RefreshControlsEnabled();
}


void BoardFirmwarePanel_Base::RefreshStrings() {
	SetText(boardTitleText, _(to_wx_string(boardTitle)));
	SetText(updateLabel, _(UPDATE_TO_STR));
	SetText(currentVersionLabel, _(CURRENT_VERSION_STR));
	SetText(endUserUpdateButton, _(UPDATE_STR));
	SetText(updateToLatestEngRevButton, _(LATEST_STR));
	SetText(updateToReleaseButton, _(RELEASE_STR));
	SetText(updateToOtherButton, _(OTHER_STR));
	RefreshVersionStatus();
	EmitSizeEvent(this);
}


void BoardFirmwarePanel_Base::RefreshVisibilityBasedOnAccessMode() {

	// Can only use the advanced update controls if connected to factory
	// network, because it requires access to photonix04
	bool showAdvancedUpdateControls = IsConnectedToFactoryNetwork() and IsInAccessMode(GuiAccessMode::FACTORY);

	SetVisibilityBasedOnCondition(versionStatusValue, showAdvancedUpdateControls);
	SetVisibilityBasedOnCondition(updateLabel, showAdvancedUpdateControls);
	SetVisibilityBasedOnCondition(updateToLatestEngRevButton, showAdvancedUpdateControls);
	SetVisibilityBasedOnCondition(latestEngRevValue, showAdvancedUpdateControls);
	SetVisibilityBasedOnCondition(updateToReleaseButton, showAdvancedUpdateControls);
	SetVisibilityBasedOnCondition(latestReleaseValue, showAdvancedUpdateControls);
	SetVisibilityBasedOnCondition(updateToOtherButton, showAdvancedUpdateControls);

	bool firmwareUpdateCodeEntered = AccessManager::GetInstance().GetCurrentAccessCodeType() == AccessCodeType::FIRMWARE_UPDATE;
	bool showSimpleUpdateButton = !showAdvancedUpdateControls and (!IsInAccessMode(GuiAccessMode::END_USER) or firmwareUpdateCodeEntered);

	SetVisibilityBasedOnCondition(endUserUpdateButton, showSimpleUpdateButton);
}


void BoardFirmwarePanel_Base::RefreshControlsEnabled() {
	bool updating = lc->IsUpdating();
	bool autotuning = lc->IsAutotuneRunning();

	bool updatingAllowed = !updating and !autotuning;

	bool alreadyHasLatestEngRevVersion = currentVersionValue->GetLabelText() == latestEngRevValue->GetLabelText();
	RefreshWidgetEnableBasedOnCondition(updateToLatestEngRevButton, updatingAllowed and !alreadyHasLatestEngRevVersion);

	bool alreadyHasReleaseVersion = currentVersionValue->GetLabelText() == latestReleaseValue->GetLabelText();
	RefreshWidgetEnableBasedOnCondition(updateToReleaseButton, updatingAllowed and !alreadyHasReleaseVersion);

	RefreshWidgetEnableBasedOnCondition(updateToOtherButton, updatingAllowed);
	RefreshWidgetEnableBasedOnCondition(endUserUpdateButton, updatingAllowed);
}


void BoardFirmwarePanel_Base::SetBoardTitle(std::string title) {
	boardTitle = title;
	SetText(boardTitleText, _(to_wx_string(boardTitle)));
}


void BoardFirmwarePanel_Base::RefreshVersionStatus() {
	std::string currentVersion = std::string(currentVersionValue->GetLabelText());
	std::string latestRelease = std::string(latestReleaseValue->GetLabelText());

	if (currentVersion == "" or latestRelease == "")
		return;

	int comparison = compareVersions(currentVersion, latestRelease);
	if (comparison < 0) {
		SetText(versionStatusValue, _(OUT_OF_DATE_STR));
		SetFGColor(versionStatusValue, TEXT_COLOR_RED);
	}
	else if (comparison > 0) {
		SetText(versionStatusValue, _(ENGINEERING_STR));
		SetFGColor(versionStatusValue, TEXT_COLOR_ORANGE);
	}
	else {
		SetText(versionStatusValue, _(UP_TO_DATE_STR));
		SetFGColor(versionStatusValue, TEXT_COLOR_GREEN);
	}

}
