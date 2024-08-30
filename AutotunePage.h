/**
* Autotune Panel - Contains all Autotune functionality.
*   - This is the panel on the Settings page designated by the "Autotune" tab
*   - Contains Autotune-Power and Autotune-Oscillator controls
* 
* Left panel: Always visible, contains button for starting/canceling Autotune procedure,
*   settings for optimizing speed and precision.
* 
* Left bottom panel: Always visible, contains button for starting/canceling Autotune Oscillator procedure
* 
* Right panel: Not visible to end-user by default. Containins Autotune-Power component plot panels
*
* @file AutotunePanel.h
* @author James Butcher
* @created June - July, 2022
* @version 2.1 - 2/29/24 - Added Autotune Diagnostics feature.
*/

#pragma once

#include <wx/wx.h>
#include <wx/gbsizer.h>

#include "AutotunePowerPanel.h"
#include "AutotuneOscillatorPanel.h"
#include "AutotuneDiagnosticsPanel.h"

#include "LaserControlProcedures/AutotunePower/AutotunePowerManager.h"
#include "LaserControlProcedures/AutotuneOscillator/AutotuneOscillatorManager.h"
#include "LaserControlProcedures/AutotuneDiagnostics/AutotuneDiagnostics.h"

#include "SettingsPage_Base.h"


class AutotunePage : public SettingsPage_Base {


public:
    AutotunePage(std::shared_ptr<MainLaserControllerInterface> _lc, wxWindow* parent);

    void Init();
    void RefreshAll();
    void RefreshStrings();
    void RefreshVisibility();
    void RefreshControlEnabled();


private:
    std::shared_ptr<AutotunePowerManager> autotunePower;
    std::shared_ptr<AutotuneOscillatorManager> autotuneOscillator;
    std::shared_ptr<AutotuneDiagnostics> autotuneDiagnostics;

    wxBoxSizer* sizer;
    wxBitmapButton* getAutotuneSettingsAccessKeyButton;
    wxBoxSizer* controlsSizer;
    AutotunePowerPanel* autotunePowerPanel;
    AutotuneOscillatorPanel* autotuneOscillatorPanel;
    AutotuneDiagnosticsPanel* autotuneDiagnosticsPanel;
    wxPanel* powerPlotsPanel;
    wxBoxSizer* powerPlotsSizer;

    std::vector<wxWindow*> windowsToHideFromEndUser;

    void InitAutotunePower();
    void InitTuningPlotPanel(std::shared_ptr<PowerTuneData> data, std::shared_ptr<ATD_Data> diagnostic_data);
    void InitAutotunePowerPlotsPanel();
    void InitAutotuneOscillator();
    void InitAutotuneDiagnostics();

    void OnGetAutotuneSettingsAccessKeyButtonClicked(wxCommandEvent& evt);

};
