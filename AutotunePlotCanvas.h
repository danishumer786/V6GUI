#pragma once

#include <string>
#include <vector>
#include <wx/wx.h>


class AutotunePlotCanvas : public wxPanel {

private:
    std::string type;
    int width;
    int height;
    int padding;
    int innerPadding;
    std::vector<float> x_points;
    std::vector<float> x_points_diagnostics;
    std::vector<float> y_points;
    std::vector<float> y_points_diagnostics;
    float start_x;
    float start_y;
    float finish_x;
    float finish_y;
    float min_x;
    float min_y;
    float max_x;
    float max_y;
    

public:

    AutotunePlotCanvas(
        std::string _type,
        wxWindow* parent, 
        wxWindowID winid = wxID_ANY,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = wxTAB_TRAVERSAL | wxBORDER_THEME
    );

    void paintEvent(wxPaintEvent& evt);
    void paintNow();
    void render(wxDC& dc);
    
    void Init();

    void RefreshAll();

    void Clear();

    void AddPoint(float x, float y);
    void AddDiagnosticPoint(float x, float y);
    void SetStartX(float x);
    void SetStartY(float y);
    void SetFinishX(float x);
    void SetFinishY(float y);
    int ScaleX(float value);
    int ScaleY(float value);
    int ScaleValue(float value, float valueMin, float valueMax, int pixelRange);
    void Plot(wxDC& dc, int x1, int y1, int x2, int y2);

    float FindMin(std::vector<float> values);
    float FindMax(std::vector<float> values);

    DECLARE_EVENT_TABLE();
};

