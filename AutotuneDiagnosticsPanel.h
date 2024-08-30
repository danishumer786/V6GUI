/**
* Autotune Diagnostics Panel - For helping diagnose motor or TEC issues.
* 
* Runs components forward, then backward, fits a quadratic curve to each,
* and logs the following data:
* - Peak position in the forward direction
* - Peak power in the forward direction
* - Peak position in the backward direction
* - Peak power in the backward direction
* - The distance between the two peaks
*
* @file AutotuneDiagnosticsPanel.h
* @author James Butcher
* @version 1.0  2/28/24
*/

#pragma once

#include <thread>

#include <wx/collpane.h>
#include <wx/gbsizer.h>
#include <wx/spinctrl.h>
#include <wx/wx.h>

#include "AutotuneComponentPanel.h"
#include "LaserControlProcedures/AutotuneDiagnostics/AutotuneDiagnostics.h"
#include "LaserControlProcedures/AutotuneDiagnostics/ATD_Data.h"
#include "../CommonGUIComponents/DynamicStatusMessage.h"
#include "../CommonGUIComponents/FeatureTitle.h"
#include "../CommonGUIComponents/PowerMonitorReadout.h"


class AutotuneDiagnosticsPanel : public wxPanel {

public:
    AutotuneDiagnosticsPanel(
        std::shared_ptr<MainLaserControllerInterface> _lc,
        std::shared_ptr<AutotuneDiagnostics> _diagnostics,
        wxWindow* parent);
    ~AutotuneDiagnosticsPanel();

    std::vector<std::shared_ptr<ATD_Data>> diagnosticsData;
    std::vector<AutotuneComponentPanel*> autotuneComponentPanels;
    std::map<int, AutotuneComponentPanel*> mapIdToPlotPanel;

    void StartDiagnostics();
    void OnRetuneButtonClicked(wxCommandEvent& evt);

    void RefreshAll();
    void RefreshVisibilityBasedOnAccessMode();
    void RefreshStrings();


private:
    std::shared_ptr<MainLaserControllerInterface> lc;
    std::shared_ptr<AutotuneDiagnostics> diagnostics;

    std::shared_ptr<std::thread> diagnosticsThread = nullptr;
    bool faultDialogShown = false;

    wxBoxSizer* sizer;
    wxGridBagSizer* settingsSizer;
    FeatureTitle* title;
    wxButton* mainButton;
    DynamicStatusMessage* runningMessage;
    wxButton* saveFullLogButton;
    wxButton* saveStatisticsLogButton;

    bool startDiagnosticsTriggered = false;


    // Initialization
    void Init();

    // Refresh methods
    void RefreshMainButtonState();
    void RefreshControlEnabled();
    void RefreshDiagnosticsProcedure();
    void RefreshPanels();

    // Main Autotune-Diagnostics functionality
    void StartDiagnosticsStepThread();
    void RunFullDiagnostics();
    void CancelDiagnostics();

    // Callbacks
    void OnMainButtonClicked(wxCommandEvent& evt);
    void OnSaveFullLogButtonClicked(wxCommandEvent& evt);
    void OnSaveStatisticsLogButtonClicked(wxCommandEvent& evt);

};
