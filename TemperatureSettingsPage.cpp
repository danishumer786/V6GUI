#include "TemperatureSettingsPage.h"
#include "../CommonFunctions_GUI.h"
#include "../AccessCodeDialog.h"

using namespace std;


const wxString GENERATE_TEMPERATURE_ACCESS_KEY_STR = _("Generate Temperature Access Key");
const wxString TEMPERATURE_ACCESS_KEY_MESSAGE_STR = _(
	"Send this code to a Photonics representative\n"
	"to request a temporary access code."
);


TemperatureSettingsPage::TemperatureSettingsPage(
	shared_ptr<MainLaserControllerInterface> _lc,
	wxWindow* parent
) :
	SettingsPage_Base(_lc, parent) {

	sizer = new wxBoxSizer(wxHORIZONTAL);

	Init();
}

void TemperatureSettingsPage::Init() {

	getTemperatureAccessKeyButton = new wxBitmapButton(this, wxID_ANY, wxBitmap(KEY_ICON, wxBITMAP_TYPE_ANY),
		wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW | 0);
	getTemperatureAccessKeyButton->Bind(wxEVT_BUTTON, &TemperatureSettingsPage::OnGetTemperatureAccessKeyButtonClicked, this);
	getTemperatureAccessKeyButton->SetToolTip(_(GENERATE_TEMPERATURE_ACCESS_KEY_STR));
	sizer->Add(getTemperatureAccessKeyButton, 0, wxALL, 2);


	temperaturePanelsSizer = new wxBoxSizer(wxHORIZONTAL);

	for (auto& id : lc->GetTemperatureControlIDs()) {
		TemperatureControlPanel* tempPanel = new TemperatureControlPanel(lc, id, this);
		temperaturePanelsSizer->Add(tempPanel, 0, wxALL, 5);
		temperaturePanels.push_back(tempPanel);
	}

	sizer->Add(temperaturePanelsSizer);

	SetSizer(sizer);
	Layout();
	sizer->Fit(this);

}

void TemperatureSettingsPage::RefreshAll() {
	// TODO: swtich to this way of refreshing readings because it doesn't slow down GUI
	/*if (IsShownOnScreen()) {
		lc->PrioritizeTemperatureRefresh(true);
	}
	else {
		lc->PrioritizeTemperatureRefresh(false);
	}*/

	lc->RefreshTemperatureReadings();

	for (auto& tempPanel : temperaturePanels)
		tempPanel->RefreshAll();

}

void TemperatureSettingsPage::RefreshStrings() {
	SetName(_(TEMPERATURE_STR));
	getTemperatureAccessKeyButton->SetToolTip(_(GENERATE_TEMPERATURE_ACCESS_KEY_STR));

	for (auto& tempPanel : temperaturePanels)
		tempPanel->RefreshStrings();

}

void TemperatureSettingsPage::RefreshVisibility() {
	for (auto& tempPanel : temperaturePanels)
		tempPanel->RefreshVisibility();
}

void TemperatureSettingsPage::RefreshControlEnabled() {
}

void TemperatureSettingsPage::OnGetTemperatureAccessKeyButtonClicked(wxCommandEvent& evt) {
	STAGE_ACTION("Generate temperature access key")

	wxString code = GenerateOfflinePartialKey(AccessCodeType::TEMPERATURE);
	AccessCodeDialog accessCodeDialog(this, code, _(TEMPERATURE_ACCESS_KEY_MESSAGE_STR));
	accessCodeDialog.ShowModal();

	LOG_ACTION()
}

