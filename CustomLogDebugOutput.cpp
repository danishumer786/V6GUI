#include "wx/wx.h"

#include "CustomLogDebugOutput.h"


void CustomLogDebugOutput::onDataPointLogged(std::map<std::string, std::string> data) {

	wxString msg = "";
	for (auto& [colName, value] : data) {
		std::string line = colName + "(" + value + ")\n";
		msg += line;
	}
	wxLogDebug(msg);

}
