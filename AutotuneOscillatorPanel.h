#pragma once

#include <thread>

#include <wx/collpane.h>
#include <wx/gbsizer.h>
#include <wx/spinctrl.h>
#include <wx/wx.h>

#include "../CommonGUIComponents/DynamicStatusMessage.h"
#include "../CommonGUIComponents/FeatureTitle.h"
#include "LaserControlProcedures/AutotunePower/AutotunePowerManager.h"
#include "LaserControlProcedures/AutotuneOscillator/AutotuneOscillatorManager.h"


class AutotuneOscillatorPanel : public wxPanel {

private:
    std::shared_ptr<MainLaserControllerInterface> lc;
    std::shared_ptr<AutotunePowerManager> autotunePower;
    std::shared_ptr<AutotuneOscillatorManager> autotuneOscillator;

    std::shared_ptr<std::thread> autotuneOscillatorThread = nullptr;

    FeatureTitle* title;
    wxButton* startSeedOnlyButton;
    wxButton* startFullRunButton;
    DynamicStatusMessage* runningMessage;
    wxButton* saveLogButton;
    wxPanel* motorResultsPanel_X;
    wxStaticText* motorLabel_X;
    wxStaticText* startIndexLabel_X;
    wxStaticText* startIndexValue_X;
    wxStaticText* finalIndexLabel_X;
    wxStaticText* finalIndexValue_X;
    wxPanel* motorResultsPanel_Y;
    wxStaticText* motorLabel_Y;
    wxStaticText* startIndexLabel_Y;
    wxStaticText* startIndexValue_Y;
    wxStaticText* finalIndexLabel_Y;
    wxStaticText* finalIndexValue_Y;
    
    bool autotuneStarted = false;

    // Refresh methods
    void RefreshAutotuneProcedure();
    void RefreshControlsEnabled();

    // Main Autotune-power functionality
    void Start(bool fullRun);
    void StartAutotuneStepThread();
    void Cancel();

    // Callbacks
    void OnStartFullRunClicked(wxCommandEvent& evt);
    void OnStartSeedOnlyClicked(wxCommandEvent& evt);
    void OnCancelFullRunClicked(wxCommandEvent& evt);
    void OnCancelSeedOnlyClicked(wxCommandEvent& evt);
    void OnSaveLogButtonClicked(wxCommandEvent& evt);

    // Helper methods
    void SetButtonToCancelState();
    void SetButtonToStartState();
    void DisplayResults();
    void ResetWidgetsWhenAutotuneStops();


public:
    AutotuneOscillatorPanel(
        std::shared_ptr<MainLaserControllerInterface> laser_controller,
        std::shared_ptr<AutotunePowerManager> autotune_power,
        std::shared_ptr<AutotuneOscillatorManager> autotune_oscillator,
        wxWindow* parent);
    ~AutotuneOscillatorPanel();

    void RefreshAll();
    void RefreshVisibilityBasedOnAccessMode();
    void RefreshStrings();

};

