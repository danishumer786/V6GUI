#include "SettingsPage_Base.h"
#include "../Resources.h"
#include "../CommonFunctions_GUI.h"

using namespace std;


SettingsPage_Base::SettingsPage_Base(
	std::shared_ptr<MainLaserControllerInterface> _lc,
	wxWindow* parent
) :
	wxScrolledWindow(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL),
	lc(_lc) 
{
	SetBackgroundColour(MIDGROUND_PANEL_COLOR);
	SetScrollRate(5, 5);
}
