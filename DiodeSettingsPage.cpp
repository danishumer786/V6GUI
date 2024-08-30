#include "DiodeSettingsPage.h"
#include "..\CommonFunctions_GUI.h"

using namespace std;

const wxString DIODE_STR = _("Diode");


DiodeSettingsPage::DiodeSettingsPage(
	shared_ptr<MainLaserControllerInterface> _lc,
	wxWindow* parent
) :
	SettingsPage_Base(_lc, parent) 
{
	sizer = new wxBoxSizer(wxVERTICAL);

	Init();
}


void DiodeSettingsPage::Init() {

	lddPanelsSizer = new wxBoxSizer(wxHORIZONTAL);

	for (auto& id : lc->GetLddIds()) {
		DiodeSettingsPanel* lddPanel = new DiodeSettingsPanel(lc, id, this);
		lddPanelsSizer->Add(lddPanel, 0, wxALL, 5);
		diodePanels.push_back(lddPanel);
	}

	sizer->Add(lddPanelsSizer);

	SetSizer(sizer);
	Layout();
	sizer->Fit(this);
}

void DiodeSettingsPage::RefreshAll() {
	if (IsShownOnScreen()) {
		lc->PrioritizeLDDRefresh(true);
	}
	else {
		lc->PrioritizeLDDRefresh(false);
	}

	for (auto& diodePanel : diodePanels)
		diodePanel->RefreshAll();

}

void DiodeSettingsPage::RefreshStrings() {
	SetName(_(DIODE_STR));

	for (auto& diodePanel : diodePanels)
		diodePanel->RefreshStrings();

}

void DiodeSettingsPage::RefreshVisibility() {
	for (auto& diodePanel : diodePanels)
		diodePanel->RefreshVisibility();
}

void DiodeSettingsPage::RefreshControlEnabled() {
}
