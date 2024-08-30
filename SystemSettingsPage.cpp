#include "../CommonFunctions_GUI.h"
#include "Security/DataDecryptor.h"
#include "SystemSettingsPage.h"
#include <Security/AccessByMACAddress.h>


const wxString SYSTEM_STR = _("System");

const wxString ENABLE_TAMPER_CHECKBOX_STR = _(decryptofy("'8T$djyw!Zl}'i{/Wg{qt,&")); // Enable Tamper Detect
const wxString ENABLE_TAMPER_CHECKBOX_TOOLTIP = _(
	decryptofy("e5Knie#5t (-~w'ipwv~u/!kuvit7xw&so~qur07") + "\n" +
	decryptofy(">7bi}k1w%tq7k!t3zqq~g~7}-q(ixi##m*o5l#}~%pgr'|yu") + "\n" +
	decryptofy("i2f|ftu {k=)&!kuy-tut'|~!'n~u7#vzo|qu1y}rj%'t#b)3>")
);
// "Check box to enable tamper detection.\n"
// "Make sure all tamper switches are down before\n"
// "enabling, otherwise it will trigger right away."

const wxString QSW_CONTROL_STR = _("QSW Control");
const wxString QSW_CONTROL_TOOLTIP = _(
	"Enable/Disable manual QSW ON/OFF button on Home Screen QSW panel"
);



SystemSettingsPage::SystemSettingsPage(
	std::shared_ptr<MainLaserControllerInterface> _lc,
	std::shared_ptr<TamperManager> _tm,
	wxWindow* parent
) :
	SettingsPage_Base(_lc, parent),
	tm(_tm)
{

	wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);

	enableTamperCheckbox = new wxCheckBox(this, wxID_ANY, _(ENABLE_TAMPER_CHECKBOX_STR), wxDefaultPosition, wxDefaultSize, 0);
	enableTamperCheckbox->SetToolTip(_(ENABLE_TAMPER_CHECKBOX_TOOLTIP));
	enableTamperCheckbox->SetFont(FONT_VERY_SMALL_SEMIBOLD);
	enableTamperCheckbox->Bind(wxEVT_CHECKBOX, &SystemSettingsPage::OnEnableTamperChecked, this);
	sizer->Add(enableTamperCheckbox, 0, wxALL, 5);

	qswControlCheckbox = new wxCheckBox(this, wxID_ANY, _(QSW_CONTROL_STR), wxDefaultPosition, wxDefaultSize, 0);
	qswControlCheckbox->SetToolTip(_(QSW_CONTROL_TOOLTIP));
	qswControlCheckbox->SetFont(FONT_VERY_SMALL_SEMIBOLD);
	qswControlCheckbox->Bind(wxEVT_CHECKBOX, &SystemSettingsPage::OnQSWControlChecked, this);
	sizer->Add(qswControlCheckbox, 0, wxALL, 5);

	this->SetSizer(sizer);
	this->Layout();
	sizer->Fit(this);
	
	Init();
}


void SystemSettingsPage::Init() {
	tm->UpdateTamperEnabled();
	enableTamperCheckbox->SetValue(tm->IsTamperEnabled());
}


void SystemSettingsPage::RefreshStrings() {
	SetName(_(SYSTEM_STR));

	enableTamperCheckbox->SetLabelText(_(ENABLE_TAMPER_CHECKBOX_STR));
	enableTamperCheckbox->SetToolTip(_(ENABLE_TAMPER_CHECKBOX_TOOLTIP));

	qswControlCheckbox->SetLabelText(_(QSW_CONTROL_STR));
	qswControlCheckbox->SetToolTip(_(QSW_CONTROL_TOOLTIP));
}


void SystemSettingsPage::RefreshAll() {

	// Only update tamper triggered every 5 refreshes to avoid hanging GUI
	static int updateStep = 0;
	updateStep++;
	if (updateStep % 10 == 0)
		tm->UpdateTamperEnabled();

	enableTamperCheckbox->SetValue(tm->IsTamperEnabled());
	qswControlCheckbox->SetValue(lc->ManualQSWControlIsEnabledForUse());
}


void SystemSettingsPage::OnEnableTamperChecked(wxCommandEvent& evt) {
	STAGE_ACTION("Enable tamper checkbox");
	if (enableTamperCheckbox->IsChecked()) {
		tm->EnableTamper();
		STAGE_ACTION_ARGUMENTS("Checked");
	}
	else {
		tm->DisableTamper();
		STAGE_ACTION_ARGUMENTS("Unchecked");
	}
	RefreshAll();
	LOG_ACTION();
}


void SystemSettingsPage::OnQSWControlChecked(wxCommandEvent& evt) {
	STAGE_ACTION("Enable QSW control checkbox");
	if (qswControlCheckbox->IsChecked()) {
		lc->EnableManualQSWControl();
		STAGE_ACTION_ARGUMENTS("Checked");
	}
	else {
		lc->DisableManualQSWControl();
		STAGE_ACTION_ARGUMENTS("Unchecked");
	}
	RefreshAll();
	LOG_ACTION();
}


void SystemSettingsPage::RefreshVisibility() {

	SetVisibilityBasedOnCondition(enableTamperCheckbox, IsInAccessMode(GuiAccessMode::FACTORY) or ThisMACHasTamperAccess());

	SetVisibilityBasedOnAccessMode(qswControlCheckbox, GuiAccessMode::SERVICE);
}
