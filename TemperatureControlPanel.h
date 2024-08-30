
#pragma once

#include <wx/wx.h>
#include <wx/collpane.h>
#include <wx/gbsizer.h>
#include <wx/spinbutt.h>

#include "MainLaserControllerInterface.h"



class TemperatureControlPanel : public wxPanel {

public:

    TemperatureControlPanel(
        std::shared_ptr<MainLaserControllerInterface> _lc,
        int temperature_id,
        wxWindow* parent,
        wxWindowID winid = wxID_ANY,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = wxTAB_TRAVERSAL | wxBORDER_THEME,
        const wxString& name = wxASCII_STR(wxPanelNameStr)
    );

    void Init();
    void RefreshAll();
    void RefreshStrings();
    void RefreshVisibility();


protected:
    std::shared_ptr<MainLaserControllerInterface> lc = nullptr;
    int temperatureId;
    float currentTemp;

    float minTemp;
    float maxTemp;
    bool isDisplayOnly; // For example, thermistors that can't be set
    bool moveCausedByButton; // Helper variable for controlling stop motor events
    bool increaseTemperatureButtonPressed;
    bool decreaseTemperatureButtonPressed;

    // Main temperature control IDs
    int ID_COLLAPSIBLE_SETTINGS_PANEL;
    int ID_SLIDER;
    int ID_COLDER_BUTTON;
    int ID_HOTTER_BUTTON;
    int ID_SET_TEMP_TEXT_CTRL;
    int ID_SET_TEMP_BUTTON;

    // Temperature settings IDs
    int ID_TEC_LABEL_TEXT_CTRL;
    int ID_SET_HIGH_LIMIT_TEXT_CTRL;
    int ID_SET_LOW_LIMIT_TEXT_CTRL;


    wxGridBagSizer* TemperatureControlSizer;

    wxStaticText* TemperatureLabel;
    wxPanel* SetTemperaturePanel;
    wxStaticText* SetTemperatureLabel;
    wxTextCtrl* SetTemperatureTextCtrl;
    wxButton* SetTemperatureButton;

    wxPanel* TemperatureSliderPanel;
    wxStaticText* TemperatureSliderMax;
    wxSlider* TemperatureSlider;
    wxStaticText* TemperatureSliderMin;
    wxGauge* TemperatureGauge;
    wxPanel* TemperatureSimpleControls;
    wxBitmapButton* TemperatureHotterButton;
    wxStaticText* CurrentTemperatureLabel;
    wxStaticText* CurrentTemperatureUnits;
    wxBitmapButton* TemperatureColderButton;

    wxCollapsiblePane* CollapsibleSettingsPanel;
    wxPanel* TemperatureInnerSettingsPanel;

    wxStaticText* TemperatureLabelLabel;
    wxTextCtrl* TemperatureLabelTextCtrl;

    wxStaticText* TemperatureHighLimitLabel;
    wxTextCtrl* TemperatureHighLimitTextCtrl;

    wxStaticText* TemperatureLowLimitLabel;
    wxTextCtrl* TemperatureLowLimitTextCtrl;

    wxStaticText* AlarmHighLimitLabel;
    wxTextCtrl* AlarmHighLimitTextCtrl;

    wxStaticText* AlarmLowLimitLabel;
    wxTextCtrl* AlarmLowLimitTextCtrl;

    wxStaticText* AlarmEnabledLabel;
    wxCheckBox* AlarmEnabledCheckbox;


    void RefreshCurrentTempDependentWidgets();
    void RefreshSetTempButton();

    void OnEnterSetTemperature(wxCommandEvent& evt);

    void OnSetTempButtonClicked(wxCommandEvent& evt);
    void OnSliderMoved(wxScrollEvent& evt);

    void OnColderButtonClicked(wxMouseEvent& evt);
    void OnColderButtonClickStopped(wxMouseEvent& evt);

    void OnHotterButtonClicked(wxMouseEvent& evt);
    void OnHotterButtonClickStopped(wxMouseEvent& evt);

    void OnTempLabelEntered(wxCommandEvent& evt);
    void OnSetHighLimitEntered(wxCommandEvent& evt);
    void OnSetLowLimitEntered(wxCommandEvent& evt);

    void OnAlarmEnabledChecked(wxCommandEvent& evt);
    void OnAlarmHighLimitEntered(wxCommandEvent& evt);
    void OnAlarmLowLimitEntered(wxCommandEvent& evt);

    void OnSettingsCollapse(wxCollapsiblePaneEvent& evt);

    void CancelChangeTemperature();

    Alarm GetTemperatureAlarmFromComponentID();

};

