#include "AutotuneComponentPanel.h"
#include "../CommonFunctions_GUI.h"

using namespace std;

const wxString START_POWER_STR = _("Start Power");
const wxString FINAL_POWER_STR = _("Final Power");
const wxString START_PREFIX_STR = _("Start "); // Note the trailing space
const wxString FINAL_PREFIX_STR = _("Final "); // Note the trailing space
const wxString DONE_STR = _("DONE");
const wxString RETUNE_STR = _("Autotune");
const wxString RUN_DIAGNOSTICS_STR = _("Diagnostics");
const wxString CLEAR_STR = _("Clear");
const wxString INDEX_STR = _("Index");
const wxString TEMP_STR = _("Temp.");

const wxSize BUTTON_SIZE = wxSize(70, 25);

AutotuneComponentPanel::AutotuneComponentPanel(
	std::shared_ptr<MainLaserControllerInterface> laser_controller,
	std::shared_ptr<AutotunePowerManager> autotune_power,
	std::shared_ptr<PowerTuneData> _data,
	int panel_id,
	wxWindow* parent) :
	lc(laser_controller),
	autotunePower(autotune_power),
	data(_data),
	wxPanel(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL | wxBORDER_THEME) {


	this->SetBackgroundColour(FOREGROUND_PANEL_COLOR);


	wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);


	// Top section - Label, start and final values, progress bar, and plot

	wxBoxSizer* topSizer = new wxBoxSizer(wxHORIZONTAL);

	// Top-left section - Label, start and final values, progress bar

	wxBoxSizer* topLeftsizer = new wxBoxSizer(wxVERTICAL);

	wxString pmLabel = lc->GetPowerMonitorLabel(data->pmId);
	wxString componentLabel;
	if (data->type == MOTOR_STR)
		componentLabel = lc->GetMotorLabel(data->componentId);
	else
		componentLabel = lc->GetTemperatureControlLabel(data->componentId);
	wxString componentName = wxString(pmLabel + " - " + componentLabel);
	label = new wxStaticText(this, wxID_ANY, componentName, wxDefaultPosition, wxDefaultSize, 0);
	label->SetFont(FONT_MED_SMALL_BOLD);

	topLeftsizer->Add(label, 0, wxALIGN_CENTER_HORIZONTAL, 0);

	wxFlexGridSizer* TuningPlotStartFinishSizer = new wxFlexGridSizer(0, 2, 0, 8);

	startLabel = new wxStaticText(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	startLabel->SetFont(FONT_VERY_SMALL);
	TuningPlotStartFinishSizer->Add(startLabel, 0, wxALL | wxALIGN_CENTER_HORIZONTAL, 0);

	finalLabel = new wxStaticText(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	finalLabel->SetFont(FONT_VERY_SMALL);
	TuningPlotStartFinishSizer->Add(finalLabel, 0, wxALL | wxALIGN_CENTER_HORIZONTAL, 0);

	startValue = new wxStaticText(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	startValue->SetFont(FONT_VERY_SMALL);
	startValue->SetForegroundColour(TEXT_COLOR_GRAY);
	TuningPlotStartFinishSizer->Add(startValue, 0, wxALIGN_CENTER_HORIZONTAL | wxBOTTOM, 5);

	finalValue = new wxStaticText(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	finalValue->SetFont(FONT_VERY_SMALL);
	finalValue->SetForegroundColour(TEXT_COLOR_DARK_GREEN);
	TuningPlotStartFinishSizer->Add(finalValue, 0, wxALL | wxALIGN_CENTER_HORIZONTAL, 0);

	startPowerLabel = new wxStaticText(this, wxID_ANY, START_POWER_STR, wxDefaultPosition, wxDefaultSize, 0);
	startPowerLabel->SetFont(FONT_VERY_SMALL);
	TuningPlotStartFinishSizer->Add(startPowerLabel, 0, wxALL | wxALIGN_CENTER_HORIZONTAL, 0);

	finalPowerLabel = new wxStaticText(this, wxID_ANY, FINAL_POWER_STR, wxDefaultPosition, wxDefaultSize, 0);
	finalPowerLabel->SetFont(FONT_VERY_SMALL);
	TuningPlotStartFinishSizer->Add(finalPowerLabel, 0, wxALL | wxALIGN_CENTER_HORIZONTAL, 0);

	startPowerValue = new wxStaticText(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	startPowerValue->SetFont(FONT_VERY_SMALL);
	startPowerValue->SetForegroundColour(TEXT_COLOR_GRAY);
	TuningPlotStartFinishSizer->Add(startPowerValue, 0, wxALL | wxALIGN_CENTER_HORIZONTAL, 0);

	finalPowerValue = new wxStaticText(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	finalPowerValue->SetFont(FONT_VERY_SMALL);
	finalPowerValue->SetForegroundColour(TEXT_COLOR_DARK_GREEN);
	TuningPlotStartFinishSizer->Add(finalPowerValue, 0, wxALL | wxALIGN_CENTER_HORIZONTAL, 0);

	topLeftsizer->Add(TuningPlotStartFinishSizer, 0, wxALL, 3);

	gauge = new wxGauge(this, wxID_ANY, 100, wxDefaultPosition, wxSize(130, -1), wxGA_HORIZONTAL);
	topLeftsizer->Add(gauge, 0, wxLEFT, 2);

	topSizer->Add(topLeftsizer, 0, wxALL, 2);


	canvas = new AutotunePlotCanvas(data->type, this, wxID_ANY, wxDefaultPosition, wxSize(200, 110), wxBORDER_SIMPLE);
	topSizer->Add(canvas, 0, wxALL, 2);

	sizer->Add(topSizer, 0, wxALL, 0);



	// Bottom section - Retune, Diagnostic, and Clear buttons on left; message and diagnostics results on right

	wxBoxSizer* bottomSizer = new wxBoxSizer(wxHORIZONTAL);

	// Buttons section - Retune, Diagnostic, and Clear buttons

	wxBoxSizer* buttonsSizer = new wxBoxSizer(wxVERTICAL);

	retuneButton = new wxButton(this, panel_id, RETUNE_STR, wxDefaultPosition, BUTTON_SIZE, 0);
	retuneButton->SetFont(FONT_VERY_SMALL);
	retuneButton->SetBackgroundColour(BUTTON_COLOR_INACTIVE);
	buttonsSizer->Add(retuneButton, 0, wxLEFT | wxTOP, 2);

	runDiagnosticsButton = new wxButton(this, panel_id + 1, RUN_DIAGNOSTICS_STR, wxDefaultPosition, BUTTON_SIZE, 0);
	runDiagnosticsButton->SetFont(FONT_VERY_SMALL);
	runDiagnosticsButton->SetBackgroundColour(BUTTON_COLOR_INACTIVE);
	buttonsSizer->Add(runDiagnosticsButton, 0, wxLEFT | wxTOP, 2);

	clearButton = new wxButton(this, panel_id + 2, CLEAR_STR, wxDefaultPosition, BUTTON_SIZE, 0);
	clearButton->Bind(wxEVT_BUTTON, &AutotuneComponentPanel::OnClearButtonClicked, this);
	clearButton->SetFont(FONT_VERY_SMALL);
	clearButton->SetBackgroundColour(RESET_BUTTON_COLOR);
	buttonsSizer->Add(clearButton, 0, wxLEFT | wxTOP, 2);

	bottomSizer->Add(buttonsSizer, 0, wxALL, 0);


	// Messages section - Autotune results message and Diagnostics results

	wxBoxSizer* messagesSizer = new wxBoxSizer(wxVERTICAL);

	message = new wxStaticText(this, wxID_ANY, DONE_STR, wxDefaultPosition, wxDefaultSize, 0);
	message->Hide();
	message->SetFont(FONT_SMALL_BOLD);
	message->SetForegroundColour(TEXT_COLOR_GREEN);
	messagesSizer->Add(message, 0, wxALL, 2);

	DiagnosticsResultsPanel = new wxPanel(this);

	wxFlexGridSizer* diagnosticsResultsSizer = new wxFlexGridSizer(3, 4, 1, 8);

	const wxFont D_RESULTS_FONT = FONT_EXTRA_SMALL;
	const wxColour D_RESULTS_LABEL_COLOR = TEXT_COLOR_BLACK;
	const wxColour D_RESULTS_VALUE_COLOR = TEXT_COLOR_DARK_GRAY;

	// [Row, Col]:
	// [0, 0] - Empty top left pivot cell
	diagnosticsResultsSizer->Add(0, 0);

	// [0, 1]
	diagosticsForwardLabel = new wxStaticText(DiagnosticsResultsPanel, wxID_ANY, "Forward");
	diagosticsForwardLabel->SetFont(D_RESULTS_FONT);
	diagosticsForwardLabel->SetForegroundColour(D_RESULTS_LABEL_COLOR);
	diagnosticsResultsSizer->Add(diagosticsForwardLabel, 0, wxALIGN_CENTER_HORIZONTAL, 0);

	// [0, 2]
	diagosticsBackwardLabel = new wxStaticText(DiagnosticsResultsPanel, wxID_ANY, "Backward");
	diagosticsBackwardLabel->SetFont(D_RESULTS_FONT);
	diagosticsBackwardLabel->SetForegroundColour(D_RESULTS_LABEL_COLOR);
	diagnosticsResultsSizer->Add(diagosticsBackwardLabel, 0, wxALIGN_CENTER_HORIZONTAL, 0);

	// [0, 3]
	diagosticsDifferenceLabel = new wxStaticText(DiagnosticsResultsPanel, wxID_ANY, "Difference");
	diagosticsDifferenceLabel->SetFont(D_RESULTS_FONT);
	diagosticsDifferenceLabel->SetForegroundColour(D_RESULTS_LABEL_COLOR);
	diagnosticsResultsSizer->Add(diagosticsDifferenceLabel, 0, wxALIGN_CENTER_HORIZONTAL, 0);


	// [1, 0]
	diagosticsMaxPowerLabel = new wxStaticText(DiagnosticsResultsPanel, wxID_ANY, "Max Pow.");
	diagosticsMaxPowerLabel->SetFont(D_RESULTS_FONT);
	diagosticsMaxPowerLabel->SetForegroundColour(D_RESULTS_LABEL_COLOR);
	diagnosticsResultsSizer->Add(diagosticsMaxPowerLabel, 0, wxALIGN_CENTER_HORIZONTAL, 0);

	// [1, 1]
	diagosticsForwardMaxPower = new wxStaticText(DiagnosticsResultsPanel, wxID_ANY, wxEmptyString);
	diagosticsForwardMaxPower->SetFont(D_RESULTS_FONT);
	diagosticsForwardMaxPower->SetForegroundColour(D_RESULTS_VALUE_COLOR);
	diagnosticsResultsSizer->Add(diagosticsForwardMaxPower, 0, wxALIGN_CENTER_HORIZONTAL, 0);

	// [1, 2]
	diagosticsBackwardMaxPower = new wxStaticText(DiagnosticsResultsPanel, wxID_ANY, wxEmptyString);
	diagosticsBackwardMaxPower->SetFont(D_RESULTS_FONT);
	diagosticsBackwardMaxPower->SetForegroundColour(D_RESULTS_VALUE_COLOR);
	diagnosticsResultsSizer->Add(diagosticsBackwardMaxPower, 0, wxALIGN_CENTER_HORIZONTAL, 0);

	// [1, 3]
	diagosticsDifferenceMaxPower = new wxStaticText(DiagnosticsResultsPanel, wxID_ANY, wxEmptyString);
	diagosticsDifferenceMaxPower->SetFont(D_RESULTS_FONT);
	diagosticsDifferenceMaxPower->SetForegroundColour(D_RESULTS_VALUE_COLOR);
	diagnosticsResultsSizer->Add(diagosticsDifferenceMaxPower, 0, wxALIGN_CENTER_HORIZONTAL, 0);


	// [2, 0]
	diagosticsMaxLocLabel = new wxStaticText(DiagnosticsResultsPanel, wxID_ANY, "Max Loc.");
	diagosticsMaxLocLabel->SetFont(D_RESULTS_FONT);
	diagosticsMaxLocLabel->SetForegroundColour(D_RESULTS_LABEL_COLOR);
	diagnosticsResultsSizer->Add(diagosticsMaxLocLabel, 0, wxALIGN_CENTER_HORIZONTAL, 0);

	// [2, 1]
	diagosticsForwardMaxLoc = new wxStaticText(DiagnosticsResultsPanel, wxID_ANY, wxEmptyString);
	diagosticsForwardMaxLoc->SetFont(D_RESULTS_FONT);
	diagosticsForwardMaxLoc->SetForegroundColour(D_RESULTS_VALUE_COLOR);
	diagnosticsResultsSizer->Add(diagosticsForwardMaxLoc, 0, wxALIGN_CENTER_HORIZONTAL, 0);

	// [2, 2]
	diagosticsBackwardMaxLoc = new wxStaticText(DiagnosticsResultsPanel, wxID_ANY, wxEmptyString);
	diagosticsBackwardMaxLoc->SetFont(D_RESULTS_FONT);
	diagosticsBackwardMaxLoc->SetForegroundColour(D_RESULTS_VALUE_COLOR);
	diagnosticsResultsSizer->Add(diagosticsBackwardMaxLoc, 0, wxALIGN_CENTER_HORIZONTAL, 0);

	// [2, 3]
	diagosticsDifferenceMaxLoc = new wxStaticText(DiagnosticsResultsPanel, wxID_ANY, wxEmptyString);
	diagosticsDifferenceMaxLoc->SetFont(D_RESULTS_FONT);
	diagosticsDifferenceMaxLoc->SetForegroundColour(D_RESULTS_VALUE_COLOR);
	diagnosticsResultsSizer->Add(diagosticsDifferenceMaxLoc, 0, wxALIGN_CENTER_HORIZONTAL, 0);

	DiagnosticsResultsPanel->SetSizer(diagnosticsResultsSizer);
	DiagnosticsResultsPanel->Layout();
	diagnosticsResultsSizer->Fit(DiagnosticsResultsPanel);
	DiagnosticsResultsPanel->Hide();

	messagesSizer->Add(DiagnosticsResultsPanel, 0, wxALL, 2);

	bottomSizer->Add(messagesSizer, 0, wxALL, 2);
	
	sizer->Add(bottomSizer, 0, wxALL, 2);


	this->SetSizer(sizer);
	this->Layout();
	sizer->Fit(this);

	SetParameterLabels();
	Init();
}




void AutotuneComponentPanel::Init() {
}


void AutotuneComponentPanel::OnClearButtonClicked(wxCommandEvent& evt) {
	ClearCanvas();
}


void AutotuneComponentPanel::RefreshAll() {

	////////////////////////////////////////////////////////////////////////
	// Autotune-Diagnostics

	// If Autotune Diagnostics is enabled and running, perform updates here:
	if (diagnostics) {

		if (diagnostics->IsRunning()) {

			retuneButton->Disable();
			runDiagnosticsButton->Disable();

			gauge->SetValue(diagnosticData->progressPercentage);

			// Detect error
			if (diagnosticData->isError) {
				DisplayMessage(diagnosticData->status, TEXT_COLOR_RED);
				return;
			}

			// Detect when Diagnostics starts, and if so, apply the starting values
			if (!detectedDiagnosticsStart) {
				if (diagnosticData->stage == ATD_Stage::GO_TO_LOW) {
					detectedDiagnosticsStart = true;
					ApplyDiagnosticStartingValues();
				}
				return;
			}


			if (!detectedMainDiagnosticsStage) {

				// Detect when Autotune stops, either by finishing or by being cancelled
				if (!diagnosticData->isRunning) {
					SetValuesIfDiagnosticsStopped();
					return;
				}

				// Detect when main tuning stage starts
				if (diagnosticData->stage == ATD_Stage::FORWARD_RUN) {
					detectedMainDiagnosticsStage = true;
					return;
				}

				return;
			}

			if (!detectedDiagnosticsStopped) {

				// Detect when Autotune stops, either by finishing or by being cancelled
				if (!diagnosticData->isRunning) {
					SetValuesIfDiagnosticsStopped();
					return;
				}

				// If detected main tuning stage, but has not stopped yet, Autotune is in the main tuning stage
				// If in main tuning stage, increment the gauge (stopping at 80%)
				if (diagnosticData->stage == ATD_Stage::FORWARD_RUN or diagnosticData->stage == ATD_Stage::BACKWARD_RUN) {
					ApplyDiagnosticValues(); // Plot the (value, power) coordinates
				}
			}


			return;
		}
		else {
			retuneButton->Enable();
			runDiagnosticsButton->Enable();
		}
	}


	////////////////////////////////////////////////////////////////////////
	// Autotune-Power 


	if (autotunePower->IsRunning()) {

		retuneButton->Disable();
		runDiagnosticsButton->Disable();
		

		gauge->SetValue(data->progressPercentage);

		// Detect error
		if (data->isError) {
			DisplayMessage(data->status, TEXT_COLOR_RED);
			return;
		}

		if (data->attemptingRetune) {
			ClearAll();
			DisplayMessage(data->status, TEXT_COLOR_RED);
		}

		// Detect when Autotune starts, and if so, apply the starting values
		if (!detectedStart) {
			if (data->stage == AutotunePowerStage::FIND_LOWER_BOUND) {
				detectedStart = true;
				ApplyStartingValues();
			}
			return;
		}


		if (!detectedMainTuningStage) {

			// Detect when Autotune stops, either by finishing or by being cancelled
			if (!data->isRunning) {
				SetValuesIfAutotuneStopped();
				return;
			}

			// Detect when main tuning stage starts
			if (data->stage == AutotunePowerStage::MAIN_TUNING_STAGE) {
				detectedMainTuningStage = true;
				return;
			}

			return;
		}

		if (!detectedStopped) {

			// Detect when Autotune stops, either by finishing or by being cancelled
			if (!data->isRunning) {
				SetValuesIfAutotuneStopped();
				return;
			}

			// If detected main tuning stage, but has not stopped yet, Autotune is in the main tuning stage
			// If in main tuning stage, increment the gauge (stopping at 80%)
			if (data->stage == AutotunePowerStage::MAIN_TUNING_STAGE) {
				ApplyCurrentValues(); // Plot the (value, power) coordinates
			}
		}
	}
	else {
		retuneButton->Enable();
		runDiagnosticsButton->Enable();
	}
}


void AutotuneComponentPanel::ClearAll() {
	detectedStart = false;
	detectedDiagnosticsStart = false;
	detectedMainTuningStage = false;
	detectedMainDiagnosticsStage = false;
	detectedStopped = false;
	detectedDiagnosticsStopped = false;


	// Removed this to test new clear button - to clear canvas, you now have to press the Clear button
	//ClearCanvas();


	gauge->SetValue(0);
	message->Hide();
	finalValue->SetLabelText(wxEmptyString);
	finalPowerValue->SetLabelText(wxEmptyString);
	DiagnosticsResultsPanel->Hide();
}

void AutotuneComponentPanel::ClearCanvas() {
	canvas->Clear();
	canvas->Refresh();
}

void AutotuneComponentPanel::ApplyStartingValues() {
	ApplyPowerStartingValues();
	if (data->type == MOTOR_STR)
		ApplyMotorStartingValues();
	else
		ApplyTemperatureStartingValues();
}

void AutotuneComponentPanel::ApplyPowerStartingValues() {
	canvas->SetStartY(data->startPower);
	startPowerValue->SetLabelText(to_wx_string(data->startPower, 2) + " W");
	UpdatePanel();
}

void AutotuneComponentPanel::ApplyMotorStartingValues() {
	canvas->SetStartX(data->startIndex);
	startValue->SetLabelText(to_wx_string(data->startIndex));
	UpdatePanel();
}

void AutotuneComponentPanel::ApplyTemperatureStartingValues() {
	canvas->SetStartX(data->startTemp);
	startValue->SetLabelText(to_wx_string(data->startTemp, 2));
	UpdatePanel();
}


void AutotuneComponentPanel::ApplyCurrentValues() {
	if (data->type == MOTOR_STR)
		canvas->AddPoint(data->currentIndex, data->currentPower);
	else {

		// Check if this step's set temperature is the same as last step.
		// Only update if this step's set temp is different from last step's.
		// This avoids the power curve getting a jagged shape.
		static float lastTemp = 0;
		bool tempChangedSinceLastStep = (data->currentTemp != lastTemp);

		if (tempChangedSinceLastStep)
			canvas->AddPoint(data->currentTemp, data->currentPower);

		lastTemp = data->currentTemp;
	}
}


void AutotuneComponentPanel::ApplyDiagnosticValues() {
	if (diagnosticData->type == MOTOR_STR)
		canvas->AddDiagnosticPoint(diagnosticData->currentIndex, diagnosticData->currentPower);
	else {

		// Check if this step's set temperature is the same as last step.
		// Only update if this step's set temp is different from last step's.
		// This avoids the power curve getting a jagged shape.
		static float lastDTemp = 0;
		bool tempChangedSinceLastStep = (diagnosticData->currentTemp != lastDTemp);

		if (tempChangedSinceLastStep)
			canvas->AddDiagnosticPoint(diagnosticData->currentTemp, diagnosticData->currentPower);

		lastDTemp = diagnosticData->currentTemp;
	}
}


void AutotuneComponentPanel::ApplyDiagnosticStartingValues() {
	canvas->SetStartY(diagnosticData->startPower);
	if (data->type == MOTOR_STR)
		canvas->SetStartX(diagnosticData->startIndex);
	else
		canvas->SetStartX(diagnosticData->startTemp);
	UpdatePanel();
}


void AutotuneComponentPanel::ApplyFinalValues() {
	ApplyPowerFinalValues();
	if (data->type == MOTOR_STR)
		ApplyMotorFinalValues();
	else
		ApplyTemperatureFinalValues();
}

void AutotuneComponentPanel::ApplyPowerFinalValues() {
	canvas->SetFinishY(data->currentPower);
	finalPowerValue->SetLabelText(to_wx_string(data->currentPower, 2) + " W");
	UpdatePanel();
}

void AutotuneComponentPanel::ApplyTemperatureFinalValues() {
	canvas->SetFinishX(data->finalTemp);
	finalValue->SetLabelText(to_wx_string(data->finalTemp, 2));
	UpdatePanel();
}



void AutotuneComponentPanel::ApplyMotorFinalValues() {
	canvas->SetFinishX(data->finalIndex);
	finalValue->SetLabelText(to_wx_string(data->finalIndex));
	UpdatePanel();
}


void AutotuneComponentPanel::DisplayMessage(wxString newMessage, wxColor fontColor) {
	message->SetLabelText(_(newMessage));
	message->SetForegroundColour(fontColor);
	message->Show();
	UpdatePanel();
}


void AutotuneComponentPanel::UpdatePanel() {
	this->Layout();
	this->Update();
	canvas->Refresh();
}


void AutotuneComponentPanel::AddAutotuneDiagnostics(std::shared_ptr<AutotuneDiagnostics> _diagnostics) {
	diagnostics = _diagnostics;
}

void AutotuneComponentPanel::AddDiagnosticData(std::shared_ptr<ATD_Data> diagnostic_data) {
	diagnosticData = diagnostic_data;
}


void AutotuneComponentPanel::RefreshStrings() {
	SetParameterLabels();
	startPowerLabel->SetLabelText(_(START_POWER_STR));
	finalPowerLabel->SetLabelText(_(FINAL_POWER_STR));
	retuneButton->SetLabelText(_(RETUNE_STR));
	runDiagnosticsButton->SetLabelText(_(RUN_DIAGNOSTICS_STR));
	clearButton->SetLabelText(_(CLEAR_STR));
}


void AutotuneComponentPanel::SetParameterLabels() {
	// Put setting this two-part label string procedure into its own method so it can be called independently by RefreshStrings() when user changes GUI language
	wxString tuningVariableLabel;
	if (data->type == MOTOR_STR)
		tuningVariableLabel = _(INDEX_STR);
	else
		tuningVariableLabel = _(TEMP_STR);

	startLabel->SetLabelText(_(START_PREFIX_STR) + tuningVariableLabel);
	finalLabel->SetLabelText(_(FINAL_PREFIX_STR) + tuningVariableLabel);
}


void AutotuneComponentPanel::SetValuesIfAutotuneStopped() {
	detectedStopped = true;
	if (data->finished) {
		DisplayMessage(_(DONE_STR), TEXT_COLOR_GREEN);
		ApplyFinalValues();
	}
	else {
		DisplayMessage(data->status, TEXT_COLOR_RED);
	}
}

void AutotuneComponentPanel::SetValuesIfDiagnosticsStopped() {
	detectedDiagnosticsStopped = true;
	if (diagnosticData->finished) {
		DisplayMessage(_(DONE_STR), TEXT_COLOR_GREEN);

		int locDecimalPlaces = 2;
		if (diagnosticData->type == MOTOR_STR)
			locDecimalPlaces = 0;

		diagosticsForwardMaxPower->SetLabelText(to_wx_string(diagnosticData->fittedMaxPower_forward, 2));
		diagosticsBackwardMaxPower->SetLabelText(to_wx_string(diagnosticData->fittedMaxPower_backward, 2));
		diagosticsDifferenceMaxPower->SetLabelText(to_wx_string(diagnosticData->fittedMaxPower_difference, 3));
		diagosticsForwardMaxLoc->SetLabelText(to_wx_string(diagnosticData->fittedMaxLoc_forward, locDecimalPlaces));
		diagosticsBackwardMaxLoc->SetLabelText(to_wx_string(diagnosticData->fittedMaxLoc_backward, locDecimalPlaces));
		diagosticsDifferenceMaxLoc->SetLabelText(to_wx_string(diagnosticData->fittedMaxLoc_difference, locDecimalPlaces));

		DiagnosticsResultsPanel->Show();
		UpdatePanel();
	}
	else {
		DisplayMessage(diagnosticData->status, TEXT_COLOR_RED);
	}
}
