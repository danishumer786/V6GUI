#pragma once

#include <wx/wx.h>
#include <wx/spinbutt.h>
#include <wx/gbsizer.h>

#include "../CommonGUIComponents/FeatureTitle.h"
#include "../CommonGUIComponents/TerminationSetting.h"

#include "SettingsPage_Base.h"
#include "../PulseControlMiniWindow.h"
#include "../PulseControls/PECSettingsPanel.h"
#include "../PulseControls/PulseCalibrationPanel.h"
#include "../PulseControls/PSORFTimeTablePanel.h"


class PulseSettingsPage : public SettingsPage_Base {

public:
    PulseSettingsPage(std::shared_ptr<MainLaserControllerInterface> _lc, wxWindow* parent);

    void Init();
    void RefreshAll();
    void RefreshStrings();
    void RefreshVisibility();


protected:
    std::vector<wxWindow*> windowsToHideFromEndUser;

    wxBoxSizer* sizer = nullptr;

    wxBitmapButton* getPulseAccessKeyButton;
    wxButton* OpenPulseControlMiniWindowButton;
    PulseControlMiniWindow* pulseControlMiniWindow;
    bool pulseControlMiniWindowOpen;

    PECSettingsPanel* PECSettings;

    PulseCalibrationPanel* PulseCalibration;

    PSORFTimeTablePanel* PSORFTimeTable;

    wxPanel* TerminationSettingsPanel = nullptr;
    FeatureTitle* TerminationSettingsTitle = nullptr;
    TerminationSetting* PRFTerminationSetting = nullptr;
    TerminationSetting* PECTerminationSetting = nullptr;
    TerminationSetting* GateTerminationSetting = nullptr;


    void RefreshTerminationSettings();
    void InitPulseControlMiniWindow();
    void OngetPulseAccessKeyButtonClicked(wxCommandEvent& evt);
    void OnOpenPulseControlMiniWindow(wxCommandEvent& evt);
    void OnPulseControlMiniWindowClosed(wxCommandEvent& evt);

};


