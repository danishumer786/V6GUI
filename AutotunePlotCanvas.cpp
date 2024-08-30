#include "AutotunePlotCanvas.h"
#include "../CommonFunctions_GUI.h"

using namespace std;


BEGIN_EVENT_TABLE(AutotunePlotCanvas, wxPanel)

    EVT_PAINT(AutotunePlotCanvas::paintEvent)

END_EVENT_TABLE()


using std::min;
using std::max;


AutotunePlotCanvas::AutotunePlotCanvas(
    string _type, 
    wxWindow* parent, 
    wxWindowID winid, 
    const wxPoint& pos, 
    const wxSize& size, 
    long style) :
    wxPanel(parent, winid, pos, size, style) {

    padding = 10;
    innerPadding = 5;

    type = _type;
    width = size.x - padding;
    height = size.y - padding;

    start_x = -1;
    start_y = -1;
    finish_x = -1;
    finish_y = -1;
    min_x = -1;
    min_y = -1;
    max_x = -1;
    max_y = -1;

    

    this->SetBackgroundColour(EXTRA_LIGHT_BACKGROUND_COLOR);
    this->SetFont(FONT_EXTRA_SMALL);
    this->Layout();
    this->Refresh();

    Init();
}

void AutotunePlotCanvas::paintEvent(wxPaintEvent& evt) {
    wxPaintDC dc(this);
    render(dc);
    evt.Skip();
}

void AutotunePlotCanvas::paintNow() {
    wxClientDC dc(this);
    render(dc);
}

void AutotunePlotCanvas::render(wxDC& dc) {

    // Title
    if (type == MOTOR_STR) {
        dc.DrawText(_("Power vs. Motor Index"), padding + 5, 1);
    }
    else {
        dc.DrawText(_("Power vs. Temperature"), padding + 5, 1);
    }

    // Axes
    dc.SetPen(wxPen(TEXT_COLOR_GRAY, 2, wxPENSTYLE_SOLID));

    // X-axis
    Plot(dc, 0, 0 - padding, width, 0 - padding);

    // Y-axis
    Plot(dc, 0, 0 - padding, 0, height - padding);

    min_x = min(min(FindMin(x_points), start_x), FindMin(x_points_diagnostics));
    min_y = min(FindMin(y_points), FindMin(y_points_diagnostics));

    max_x = max(max(FindMax(x_points), start_x), FindMax(x_points_diagnostics));
    max_y = max(FindMax(y_points), FindMax(y_points_diagnostics));



    // Plot intermediate values
    if (x_points.size() > 1) {

        dc.SetPen(wxPen(TEXT_COLOR_BLUE, 2));
        for (unsigned int i = 0; i < x_points.size() - 1; i++) {
            Plot(dc, ScaleX(x_points[i]), ScaleY(y_points[i]), ScaleX(x_points[i + 1]), ScaleY(y_points[i + 1]));
        }

        if (x_points.size() > 3) {

            // Plot starting value lines
            dc.SetPen(wxPen(TEXT_COLOR_LIGHT_GRAY, 1, wxPENSTYLE_SOLID));
            if (start_x != -1)
                Plot(dc, ScaleX(start_x), 0 - 2 * padding, ScaleX(start_x), height);
            if (start_y != -1)
                Plot(dc, 0 - padding, ScaleY(start_y), width, ScaleY(start_y));

            // Plot final value lines
            dc.SetPen(wxPen(TEXT_COLOR_GREEN, 1, wxPENSTYLE_SOLID));
            if (finish_x != -1)
                Plot(dc, ScaleX(finish_x), 0 - 2 * padding, ScaleX(finish_x), height);
            if (finish_y != -1)
                Plot(dc, 0 - padding, ScaleY(finish_y), width, ScaleY(finish_y));
        }
    }


    // Plot diagnostics values
    if (x_points_diagnostics.size() > 1) {
        dc.SetPen(wxPen(TEXT_COLOR_LIGHT_BLUE, 1));
        for (unsigned int i = 0; i < x_points_diagnostics.size() - 1; i++) {
            Plot(dc, ScaleX(x_points_diagnostics[i]), ScaleY(y_points_diagnostics[i]), ScaleX(x_points_diagnostics[i + 1]), ScaleY(y_points_diagnostics[i + 1]));
        }
    }

}

void AutotunePlotCanvas::Init() {
}

void AutotunePlotCanvas::RefreshAll() {
}

void AutotunePlotCanvas::Clear() {
    x_points.clear();
    x_points_diagnostics.clear();
    y_points.clear();
    y_points_diagnostics.clear();
    start_x = -1;
    start_y = -1;
    finish_x = -1;
    finish_y = -1;
    min_x = -1;
    min_y = -1;
    max_x = -1;
    max_y = -1;
    this->Refresh();
}

void AutotunePlotCanvas::SetStartX(float x) {
    start_x = x;
}

void AutotunePlotCanvas::SetStartY(float y) {
    start_y = y;
}

void AutotunePlotCanvas::SetFinishX(float x) {
    finish_x = x;
}

void AutotunePlotCanvas::SetFinishY(float y) {
    finish_y = y;
}

void AutotunePlotCanvas::AddPoint(float x, float y) {
    x_points.push_back(x);
    y_points.push_back(y);
    this->Refresh();
}

void AutotunePlotCanvas::AddDiagnosticPoint(float x, float y) {
    x_points_diagnostics.push_back(x);
    y_points_diagnostics.push_back(y);
    this->Refresh();
}

float AutotunePlotCanvas::FindMin(vector<float> values) {
    float min = 99999999999.9f;
    for (float v : values) {
        if (v < min) {
            min = v;
        }
    }
    return min;
}

float AutotunePlotCanvas::FindMax(vector<float> values) {
    float max = 0;
    for (float v : values) {
        if (v > max) {
            max = v;
        }
    }
    return max;
}

int AutotunePlotCanvas::ScaleX(float value) {
    // Scale the value to be between the relative range of max and min, and scaled to the width and height of the canvas
    return ScaleValue(value, min_x, max_x, width - 2*padding);
}

int AutotunePlotCanvas::ScaleY(float value) {
    // Scale the value to be between the relative range of max and min, and scaled to the width and height of the canvas
    static const int gapUnderTopTitle = 7;
    return ScaleValue(value, min_y, max_y, height - 2*padding - gapUnderTopTitle);
}

int AutotunePlotCanvas::ScaleValue(float value, float valueMin, float valueMax, int pixelRange) {
    // Scale the value to be between the relative range of max and min, and scaled to the width and height of the canvas
    float valueRange = valueMax - valueMin;
    float valueRelativePosition = value - valueMin;
    float valueScaled = valueRelativePosition / valueRange;
    int pixelPosition = static_cast<int>(valueScaled * pixelRange);
    return pixelPosition;
}

void AutotunePlotCanvas::Plot(wxDC& dc, int x1, int y1, int x2, int y2) {
    // Adds padding and inverts y axis
    dc.DrawLine(x1 + padding, height - (y1 + padding), x2 + padding, height - (y2 + padding));
}
