/**
* Motor Control Panel Linked - A panel containing simple controls for a
* "linked" motor. 
*
*   A linked motor is one of 4 virtual items represented in memory. 
*
*   There are 4 possible links: 0, 1, 2, or 3.
* 
*   Each link has 2 motors assigned to it: An X Motor ID and a Y Motor ID.
*  
*   This panel will display the labels of the 2 motors assigned to this
*   Linked Motor slot according to the example format "MX-MY" and provide
*   a drop-down choice allowing the user to change to any of the 36 possible
*   positions alongside the number of hours for each.
*
* @file MotorControlPanel.h
* @author James Butcher
* @version 1.0  6/8/2022
*/
#pragma once

#include <wx/wx.h>
#include <wx/collpane.h>
#include <wx/gbsizer.h>

#include "../CommonGUIComponents/NumericTextCtrl.h"
#include "MainLaserControllerInterface.h"


class MotorControlPanelLinked : public wxPanel {

public:

    MotorControlPanelLinked(
        std::shared_ptr<MainLaserControllerInterface> _lc,
        int linked_motor_id,
        wxWindow* parent,
        wxWindowID winid = wxID_ANY,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = wxTAB_TRAVERSAL | wxBORDER_THEME,
        const wxString& name = wxASCII_STR(wxPanelNameStr)
    );

    // Only make motor control panel visible to End User if there are
    // motor positions indices specified. User is allowed to change 
    // motor position only.
    bool ShouldBeVisibleToUser();

    void Init();
    void RefreshAll();
    void RefreshStrings();
    void RefreshVisibility();
    void RefreshControlEnabled();

    void RefreshCurrentPosition();
    void RefreshPositionsChoices();

    void OnLinkedMotorPositionSelected(wxCommandEvent& evt);


protected:
    std::shared_ptr<MainLaserControllerInterface> lc = nullptr;
    int linkedMotorID; // Used with MainLaserController to indicate link # 0 - 3
    int motorID_X; // Motor ID in the X slot for this linked motor
    int motorID_Y; // Motor ID in the Y slot for this linked motor

    wxStaticText* LinkedMotorLabel;
    wxArrayString positionsChoices;
    wxChoice* LinkedMotorPositionChoice;

};
