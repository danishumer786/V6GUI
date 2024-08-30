/**
* Autotune Component Panel - Runs Autotune on an individual component and displays results.
* - User can run Autotune on this component only by clicking the "Retune" button
*
* @file AutotuneComponentPanel.h
* @author James Butcher
* @created  June - July, 2022
* @version 1.1  2/29/24 - Added Autotune Diagnostics Feature.
*/

#pragma once

#include <deque>
#include <wx/wx.h>
#include <wx/gbsizer.h>

#include "AutotunePlotCanvas.h"
#include "LaserControlProcedures/AutotunePower/AutotunePowerManager.h"
#include "LaserControlProcedures/AutotunePower/PowerTuneData.h"
#include "LaserControlProcedures/AutotuneDiagnostics/ATD_Data.h"
#include "LaserControlProcedures/AutotuneDiagnostics/AutotuneDiagnostics.h"


class AutotuneComponentPanel : public wxPanel {

public:
    AutotuneComponentPanel(
        std::shared_ptr<MainLaserControllerInterface> laser_controller,
        std::shared_ptr<AutotunePowerManager> autotunePower,
        std::shared_ptr<PowerTuneData> data,
        int panel_id,
        wxWindow* parent);

    // Need to make this button publicly accessible so it can be bound to main autotune start/cancel button
    wxButton* retuneButton;
    wxButton* runDiagnosticsButton;
    wxButton* clearButton;
    std::shared_ptr<PowerTuneData> data;
    std::shared_ptr<ATD_Data> diagnosticData;

    void AddAutotuneDiagnostics(std::shared_ptr<AutotuneDiagnostics> _diagnostics);
    void AddDiagnosticData(std::shared_ptr<ATD_Data> diagnostic_data);

    void RefreshStrings();
    void RefreshAll();
    void ClearAll();


private:
    std::shared_ptr<MainLaserControllerInterface> lc;
    std::shared_ptr<AutotunePowerManager> autotunePower;
    std::shared_ptr<AutotuneDiagnostics> diagnostics;

    wxStaticText* label;
    wxStaticText* startLabel;
    wxStaticText* finalLabel;
    wxStaticText* startValue;
    wxStaticText* finalValue;
    wxStaticText* startPowerLabel;
    wxStaticText* finalPowerLabel;
    wxStaticText* startPowerValue;
    wxStaticText* finalPowerValue;
    wxGauge* gauge;
    AutotunePlotCanvas* canvas;
    wxStaticText* message;

    wxPanel* DiagnosticsResultsPanel;
    wxStaticText* diagosticsForwardLabel;
    wxStaticText* diagosticsBackwardLabel;
    wxStaticText* diagosticsDifferenceLabel;
    wxStaticText* diagosticsMaxPowerLabel;
    wxStaticText* diagosticsMaxLocLabel;
    wxStaticText* diagosticsForwardMaxPower;
    wxStaticText* diagosticsBackwardMaxPower;
    wxStaticText* diagosticsDifferenceMaxPower;
    wxStaticText* diagosticsForwardMaxLoc;
    wxStaticText* diagosticsBackwardMaxLoc;
    wxStaticText* diagosticsDifferenceMaxLoc;

    // Flags to signal changes in state
    bool detectedStart = false;
    bool detectedDiagnosticsStart = false;
    bool detectedMainTuningStage = false;
    bool detectedMainDiagnosticsStage = false;
    bool detectedStopped = false;
    bool detectedDiagnosticsStopped = false;

    void Init();

    void OnClearButtonClicked(wxCommandEvent& evt);

    // Apply data to plot canvas
    void ApplyStartingValues();
    void ApplyPowerStartingValues();
    void ApplyMotorStartingValues();
    void ApplyTemperatureStartingValues();
    void ApplyCurrentValues();
    void ApplyFinalValues();
    void ApplyPowerFinalValues();
    void ApplyMotorFinalValues();
    void ApplyTemperatureFinalValues();

    // Apply diagnostic data to plot canvas
    void ApplyDiagnosticValues();
    void ApplyDiagnosticStartingValues();
    void SetValuesIfDiagnosticsStopped();

    // Helper functions
    void DisplayMessage(wxString newMessage, wxColor fontColor);
    void UpdatePanel();
    void SetParameterLabels();
    void SetValuesIfAutotuneStopped();
    void ClearCanvas();

};

