/**
* Motor Control Panel - A panel containing simple controls for a *single* motor.
*  These panels are stacked in a column in the motor settings page
*  One motor control panel appears for each motor that is enabled.
*  Each motor is identified by its motorId. This is used to map the motor
*  panel's controls to the correct motor control item in the MainLaserController.
* 
* @file MotorControlPanel.h
* @author James Butcher
* @created  6/8/2022
* @version 1.1  2/16/2024 - Added keyboard controls
*/
#pragma once

#include <wx/wx.h>
#include <wx/collpane.h>
#include <wx/gbsizer.h>

#include "../CommonGUIComponents/NumericTextCtrl.h"
#include "MainLaserControllerInterface.h"
#include "../CommonGUIComponents/FeatureTitle.h"


class MotorControlPanel : public wxPanel {

public:

    MotorControlPanel(
        std::shared_ptr<MainLaserControllerInterface> _lc,
        int motor_id,
        wxWindow* parent,
        wxWindowID winid = wxID_ANY,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = wxTAB_TRAVERSAL | wxBORDER_THEME | wxWANTS_CHARS,
        const wxString& name = wxASCII_STR(wxPanelNameStr)
    );

    // Only make motor control panel visible to End User if there are
    // motor positions indices specified. User is allowed to change 
    // motor position only.
    //bool ShouldBeVisibleToUser();

    void Init();
    void RefreshAll();
    void RefreshStrings();
    void RefreshVisibility();
    void RefreshControlEnabled();

    void RefreshCurrentIndexDependentWidgets();
    void RefreshGoToTargetButton();
    void RefreshMotorControlsEnabled();
    void ResetSliderPosition();
    void RefreshCurrentPosition();
    void RefreshPositionsChoices();

    void OnTargetIndexEntered(wxCommandEvent& evt);
    void OnGoToTargetIndexButtonClicked(wxCommandEvent& evt);
    void OnSliderMoved(wxScrollEvent& evt);

    void OnCCWButtonClicked(wxMouseEvent& evt);
    void OnCCWButtonClickStopped(wxMouseEvent& evt);

    void OnCWButtonClicked(wxMouseEvent& evt);
    void OnCWButtonClickStopped(wxMouseEvent& evt);

    void OnMotorStepSizeChanged(wxCommandEvent& evt);

    int GetKeyboardSlotSelection();
    void MoveCCW();
    void MoveCW();
    void StopMoving();

    void OnKeyDown(wxKeyEvent& evt);
    void OnKeyUp(wxKeyEvent& evt);

    void OnMotorLabelEntered(wxCommandEvent& evt);
    void OnRedefineCurrentIndexEntered(wxCommandEvent& evt);
    void OnSetMinIndexEntered(wxCommandEvent& evt);
    void OnSetMaxIndexEntered(wxCommandEvent& evt);
    void OnSetBacklashEntered(wxCommandEvent& evt);
    void OnMotorPositionSelected(wxCommandEvent& evt);
    void OnKeyboardChoiceSelected(wxCommandEvent& evt);

    void OnMotorSettingsCollapse(wxCollapsiblePaneEvent& evt);


    // Helper methods

    void GoToTargetIndex();
    int GetStepSize();
    bool CheckIfCantMoveMotorDueToLDDCurrentLimit(bool show_message_box = false);


protected:
    std::shared_ptr<MainLaserControllerInterface> lc = nullptr;
    int motorId; // Used with MainLaserController to indicate motor # - Motor ids: 0 - 7
    int minIndex;
    int maxIndex;
    bool movingMotor = false; // Need this to prevent trying to re-send move motor commands

    // For saving user config option to this PC for this specific motor on this specific laser
    std::string savedStepSizeKey = "";
    std::string savedKeyBindingKey = "";
    
    //bool moveCausedByButton; // Helper variable for controlling stop motor events
    int motorRefreshCountdown = 0;

    wxGridBagSizer* MotorControlsSizer;
    //wxStaticText* MotorLabel;
    FeatureTitle* MotorTitle;
    wxStaticText* MotorTargetIndexLabel;
    NumericTextCtrl* MotorTargetIndexTextCtrl;
    wxButton* MotorTargetIndexGoButton;

    wxArrayString keyboardSlots;
    wxChoice* AssignKeyboardChoice;

    wxPanel* MotorSliderPanel;
    wxSlider* MotorSlider;
    wxGauge* MotorGauge;
    wxPanel* MotorSimpleControls;
    wxBitmapButton* MotorCCWButton;
    wxStaticText* MotorCurrentIndexLabel;
    wxBitmapButton* MotorCWButton;
    wxStaticText* MotorStepSizeLabel;
    NumericTextCtrl* MotorStepSizeTextCtrl;
    wxStaticBitmap* MotorStepSizeInfoIcon;

    wxArrayString positionsChoices;
    wxChoice* MotorPositionChoice;

    wxCollapsiblePane* MotorCollapsibleSettingsPanel;
    wxPanel* MotorInnerSettingsPanel;
    wxStaticText* MotorLabelLabel;
    wxTextCtrl* MotorLabelTextCtrl;
    wxStaticText* MotorRedefineCurrentIndexLabel;
    wxTextCtrl* MotorRedefineCurrentIndexTextCtrl;
    wxStaticText* MotorBacklashLabel;
    wxTextCtrl* MotorBacklashTextCtrl;

    wxStaticText* MotorSetMinIndexLabel;
    wxTextCtrl* MotorSetMinIndexTextCtrl;
    wxStaticText* MotorSetMaxIndexLabel;
    wxTextCtrl* MotorSetMaxIndexTextCtrl;

};
