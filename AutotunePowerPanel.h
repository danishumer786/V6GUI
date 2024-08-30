/**
* Autotune Power Panel - Sub-panel within Autotune Panel that contains main
* controls for Autotune-Power procedure.
* 
*   - Contains main Autotune-Power start button, running message, save log
*     button, power monitor readouts, and settings
* 
*   - Does NOT contain the Autotune-Power component plot panels. Those are
*     handled by the parent panel "AutotunePanel". This panel does however
*     have references to those panels and their associated PowerTuneData data
*     structures to enable individual component re-tuning.
*   
*
* @file AutotunePowerPanel.h
* @author James Butcher
* @created February, 2023
* @version 1.0 - February, 2022 - Extracted from parent panel for better cohesion
*/

#pragma once

#include <thread>

#include <wx/collpane.h>
#include <wx/gbsizer.h>
#include <wx/spinctrl.h>
#include <wx/wx.h>

#include "AutotuneComponentPanel.h"
#include "../CommonGUIComponents/DynamicStatusMessage.h"
#include "../CommonGUIComponents/FeatureTitle.h"
#include "../CommonGUIComponents/PowerMonitorReadout.h"
#include "LaserControlProcedures/AutotunePower/AutotunePowerManager.h"
#include "LaserControlProcedures/AutotuneOscillator/AutotuneOscillatorManager.h"


class AutotunePowerPanel : public wxPanel {

public:
	AutotunePowerPanel(
        std::shared_ptr<MainLaserControllerInterface> _lc,
        std::shared_ptr<AutotunePowerManager> autotune_power,
        std::shared_ptr<AutotuneOscillatorManager> autotune_oscillator,
        wxWindow* parent);
    ~AutotunePowerPanel();

    // Main Autotune Settings Panel needs to access these data structures and methods:

    std::vector<std::shared_ptr<PowerTuneData>> autotunePowerTuneData;
    std::vector<AutotuneComponentPanel*> autotuneComponentPanels;
    std::map<int, AutotuneComponentPanel*> mapIdToPlotPanel;

    void StartAutotune();
    void OnRetuneButtonClicked(wxCommandEvent& evt);

	void RefreshAll();
	void RefreshVisibilityBasedOnAccessMode();
	void RefreshStrings();


private:
    std::shared_ptr<MainLaserControllerInterface> lc;
    std::shared_ptr<AutotunePowerManager> autotunePower;
    std::shared_ptr<AutotuneOscillatorManager> autotuneOscillator;

    std::shared_ptr<std::thread> autotunePowerThread = nullptr;
    bool faultDuringAutotuneDialogShown = false;

    wxBoxSizer* sizer;
    wxGridBagSizer* settingsSizer;
    FeatureTitle* title;
    wxButton* mainButton;
    DynamicStatusMessage* runningMessage;
    wxButton* saveLogButton;
    wxBoxSizer* powerMonitorsSizer;
    std::vector<PowerMonitorReadout*> powerMonitorReadouts;

    // Main Autotune settings collapsible panel
    wxCollapsiblePane* collapsibleSettingsPanel;
    wxPanel* innerSettingsPanel;

    // Speed setting
    wxStaticText* speedLabel;
    wxStaticBitmap* speedInfoIcon;
    wxPanel* speedSliderPanel;
    wxSlider* speedSlider;

    // Motor precision setting
    wxStaticText* precisionLabel_Motor;
    wxStaticBitmap* precisionInfoIcon_Motor;
    wxPanel* precisionSliderPanel_Motor;
    wxSlider* precisionSlider_Motor;

    // Temperature precision setting
    wxStaticText* precisionLabel_Temperature;
    wxStaticBitmap* precisionInfoIcon_Temperature;
    wxPanel* precisionSliderPanel_Temperature;
    wxSlider* precisionSlider_Temperature;

    // % Drop Threshold setting
    wxStaticText* dropThresholdLabel;
    wxStaticBitmap* dropThresholdInfoIcon;
    wxSpinCtrl* dropThresholdSpinCtrl;

    // Motor range setting
    wxStaticText* motorRangeLabel;
    wxStaticBitmap* motorRangeInfoIcon;
    wxSpinCtrl* motorRangeSpinCtrl;

    // Temperature range setting
    wxStaticText* temperatureRangeLabel;
    wxStaticBitmap* temperatureRangeInfoIcon;
    wxSpinCtrlDouble* temperatureRangeSpinCtrlDouble;


    bool startAutotuneTriggered = false;


    // Initialization
    void Init();
    void InitSettingsPanel();
    void InitPowerMonitorsDisplay();
    void CorrectPowerDropThresholdIfZero();

    // Refresh methods
    void RefreshMainButtonState();
    void RefreshPowerMonitorReadouts();
    void RefreshAutotuneControlEnabled();
    void RefreshWarnings();
    void RefreshAutotuneProcedure();
    void RefreshPanels();

    // Main Autotune-power functionality
    void StartAutotuneStepThread();
    void RunFullAutotune();
    void CancelAutotune();

    // Helper functions
    void SetMotorPrecisionTooltip();
    void SetTemperaturePrecisionTooltip();
    void SetDropThreshold();
    void SetMotorRange();
    void SetTemperatureRange(float newRange = -1.0f);

    // Callbacks
    void OnMainButtonClicked(wxCommandEvent& evt);
    void OnSaveLogButtonClicked(wxCommandEvent& evt);
    void OnSettingsCollapse(wxCollapsiblePaneEvent& evt);
    void OnSpeedSliderMoved(wxCommandEvent& evt);
    void OnPrecisionSliderMoved_Motor(wxCommandEvent& evt);
    void OnPrecisionSliderMoved_Temperature(wxCommandEvent& evt);
    void SetDropThresholdWithText(wxCommandEvent& evt);
    void SetDropThresholdWithSpin(wxSpinEvent& evt);
    void SetMotorRangeWithText(wxCommandEvent& evt);
    void SetMotorRangeWithSpin(wxSpinEvent& evt);
    void SetTemperatureRangeWithText(wxCommandEvent& evt);
    void SetTemperatureRangeWithSpin(wxSpinEvent& evt);

};

